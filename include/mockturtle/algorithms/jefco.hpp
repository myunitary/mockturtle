#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>

#include "balancing.hpp"
#include "circuit_validator.hpp"
#include "cut_enumeration.hpp"
#include "cleanup.hpp"
#include "equivalence_classes.hpp"
#include "detail/minmc_xags.hpp"
#include "../networks/xag.hpp"
#include "../utils/node_map.hpp"
#include "../utils/progress_bar.hpp"
#include "../utils/stopwatch.hpp"
#include "../views/cut_view.hpp"
#include "../views/depth_view.hpp"
#include "../views/topo_view.hpp"

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/spectral.hpp>
#include <kitty/operations.hpp>

namespace mockturtle
{

struct jefco_params
{
	cut_enumeration_params cut_enum_ps;
	bool esop_only_on_critical_path{ true };
	bool progress{ true };
	bool verbose{ false };
	bool very_verbose{ false };
};

struct jefco_stats
{
	cut_enumeration_stats cut_enum_st;
	stopwatch<>::duration time_mine_rules{ 0 };
	uint32_t num_mc_rules{};
	uint32_t num_esop_rules{};
	void report() const
	{
		// cut_enum_st.report();
		fmt::print( "[i] Mine rewriting rules time   = {:>5.2f} secs\n", to_seconds( time_mine_rules ) );
		fmt::print( "[i] Number of MC minimum rules  = {:>5}\n", num_mc_rules );
		fmt::print( "[i] Number of ESOP rules        = {:>5}\n", num_esop_rules );
	}
};

struct num_and
{
	uint32_t operator()( xag_network const& ntk, xag_network::node const& n ) const
	{
		if ( ntk.is_and( n ) )
		{
			return 1u;
		}
		return 0u;
	}
};

namespace detail
{

class jefco_impl
{
public:
	jefco_impl( xag_network const& ntk, jefco_params const& ps, jefco_stats& st )
		: _ntk( ntk ), _ps( ps ), _st( st )
	{
		build_db();
	}

	void run( std::string const& filename )
	{
		std::ofstream rules;
		rules.open( filename, std::ios::out );
		if ( !rules.is_open() )
		{
			fmt::print( "[e] Unable to open file {}...\n", filename );
			return;
		}

		if ( _ps.very_verbose )
		{
			fmt::print( "[i] Start mining rules ...\n" );
		}
		stopwatch t_mine_rules( _st.time_mine_rules );

		depth_view<xag_network, num_and, false> ntk_md{ _ntk };

		_st.num_mc_rules = 0u;
		_st.num_esop_rules = 0u;
		const auto cuts = cut_enumeration<xag_network, true>( _ntk, _ps.cut_enum_ps, &( _st.cut_enum_st ) );

		progress_bar pbar_mc{ _ntk.size(), "MC round |{0}| node = {1:>4} / " + std::to_string( _ntk.size() ) , _ps.progress };
		topo_view<xag_network>{ _ntk }.foreach_node( [&]( auto const& n, uint32_t index ) {
			pbar_mc( index, index );

			if ( _ntk.is_constant( n ) || _ntk.is_pi( n ) )
			{
				return true;
			}

			for ( auto const& pcut : cuts.cuts( _ntk.node_to_index( n ) ) )
			{
				if ( pcut->size() == 1u || kitty::is_const0( cuts.truth_table( *pcut ) ) )
				{
					continue;
				}

				std::vector<xag_network::node> leaves( pcut->size() );
				std::transform( pcut->begin(), pcut->end(), leaves.begin(), [&]( auto const& leaf ) { 
					return _ntk.index_to_node( leaf );
				} );

				gen_mc_opt_impl( cuts.truth_table( *pcut ), leaves, n, rules );
			}

			return true;
		} );

		node_map<arrival_time_pair<xag_network>, xag_network> old_to_new( _ntk );
		xag_network dest;
		esop_rebalancing<xag_network> esop_rebalance;
		/* input arrival times and mapping */
    old_to_new[_ntk.get_constant( false )] = { dest.get_constant( false ), 0u };
    if ( _ntk.get_node( _ntk.get_constant( false ) ) != _ntk.get_node( _ntk.get_constant( true ) ) )
    {
      old_to_new[_ntk.get_constant( true )] = { dest.get_constant( true ), 0u };
    }
    _ntk.foreach_pi( [&]( auto const& n ) {
      old_to_new[n] = { dest.create_pi(), 0u };
    } );

		uint32_t current_level{};
		progress_bar pbar_esop{ _ntk.size(), "ESOP round |{0}| node = {1:>4} / " + std::to_string( _ntk.size() ) + "   current level = {2}", _ps.progress };
		topo_view<xag_network>{ _ntk }.foreach_node( [&]( auto const& n, uint32_t index ) {
			pbar_esop( index, index, current_level );

			if ( _ntk.is_constant( n ) || _ntk.is_pi( n ) )
      {
        return true;
      }

      if ( _ps.esop_only_on_critical_path && !ntk_md.is_on_critical_path( n ) )
      {
      	std::vector<xag_network::signal> children;
        _ntk.foreach_fanin( n, [&]( auto const& f ) {
          const auto f_best = old_to_new[f].f;
          children.push_back( _ntk.is_complemented( f ) ? dest.create_not( f_best ) : f_best );
        } );
        old_to_new[n] = { dest.clone_node( _ntk, n, children ), ntk_md.level( n ) };
        return true;
      }

      arrival_time_pair<xag_network> best{ {}, std::numeric_limits<uint32_t>::max() };
      uint32_t best_size{};
      for ( auto const& pcut : cuts.cuts( _ntk.node_to_index( n ) ) )
      {
        if ( pcut->size() == 1u || kitty::is_const0( cuts.truth_table( *pcut ) ) )
        {
          continue;
        }

        uint32_t num_vars = pcut->size();
        std::vector<arrival_time_pair<xag_network>> arrival_times( num_vars );
        std::transform( pcut->begin(), pcut->end(), arrival_times.begin(), [&]( auto leaf ) {
        	return old_to_new[_ntk.index_to_node( leaf )];
        } );
        std::vector<xag_network::node> leaves( num_vars );
        std::transform( pcut->begin(), pcut->end(), leaves.begin(), [this]( auto leaf ) {
        	return _ntk.index_to_node( leaf );
        } );
        std::vector<xag_network::signal> leaves_dest( arrival_times.size() );
        std::transform( arrival_times.begin(), arrival_times.end(), leaves_dest.begin(), []( auto leaf_info ) {
        	return leaf_info.f;
        } );

        esop_rebalance( dest, cuts.truth_table( *pcut ), arrival_times, best.level, best_size, [&]( arrival_time_pair<xag_network> const& cand, uint32_t cand_size ) {
        	xag_network dest_cut;
        	std::vector<xag_network::signal> pis( num_vars, dest_cut.get_constant( false ) );
        	std::generate( pis.begin(), pis.end(), [&dest_cut]() { return dest_cut.create_pi(); } );
        	cut_view<xag_network> dest_partial{ dest, leaves_dest, cand.f };
        	const auto po_opt = cleanup_dangling( dest_partial, dest_cut, pis.begin(), pis.end() ).front();
        	dest_cut.create_po( po_opt );
        	write_expr( dest_cut, leaves, n, rules, false );

          if ( cand.level < best.level || ( cand.level == best.level && cand_size < best_size ) )
          {
            best = cand;
            best_size = cand_size;
          }
        } );
      }
      old_to_new[n] = best;
      current_level = std::max( current_level, best.level );
		} );

		rules.close();
	}

private:
	void write_expr( xag_network const& ntk, std::vector<xag_network::node> const& leaves, xag_network::node const& root, std::ofstream& file, bool is_mc_rule )
	{
		if ( !file.is_open() )
		{
			fmt::print( "[e] File not open!" );
			abort();
		}

		if ( is_mc_rule )
		{
			file << "mc" << _st.num_mc_rules++ << ":";
		}
		else
		{
			file << "esop" << _st.num_esop_rules++ << ":";
		}

		node_map<std::string, xag_network> expr{ ntk };

		ntk.foreach_pi( [&]( auto const& pi, uint32_t index ) {
			expr[pi] = fmt::format( "?{}", std::to_string( _ntk.node_to_index( leaves[index] ) ) );
		} );

		topo_view<xag_network> ntk_topo( ntk );
		ntk_topo.clear_visited();
		ntk_topo.foreach_node( [&]( auto const& n ) {
			if ( ntk.is_constant( n ) || ntk.is_pi( n ) )
			{
				return true;
			}

			if ( ntk.visited( n ) )
			{
				return true;
			}

			ntk.set_visited( n, 1u );

			std::array<std::string, 2u> operands{};
			ntk.foreach_fanin( n, [&]( auto const& f, uint32_t index ) {
				const auto ni = ntk.get_node( f );
				operands[index] = expr[ni];
				if ( ntk.is_complemented( f ) )
				{
					operands[index] = "(! " + operands[index] + ")";
				}
			} );

			if ( ntk.is_and( n ) )
			{
				expr[n] = "(* " + operands[0] + " " + operands[1] + ")";
			}
			else
			{
				expr[n] = "(^ " + operands[1] + " " + operands[0] + ")";
			}

			return true;
		} );

		assert( ntk.num_pos() == 1u );
		ntk.foreach_po( [&]( auto const& po, uint32_t index ) {
			file << _ntk.node_to_index( root ) << "=>";
			if ( ntk.is_complemented( po ) )
			{
				file << "(! " << expr[ntk.get_node( po )] << ")";
			}
			else
			{
				file << expr[ntk.get_node( po )];
			}

			file << '\n';
		} );

		return;
	}

	void gen_mc_opt_impl( kitty::dynamic_truth_table const& tt, std::vector<xag_network::node> const& leaves,
	                       xag_network::node const& root, std::ofstream& file )
	{
		if ( !file.is_open() )
		{
			fmt::print( "[e] File not open!" );
			abort();
		}

		const uint32_t num_vars = tt.num_vars();
		assert( leaves.size() == num_vars );
		uint32_t repr{ 0u };
		std::vector<kitty::detail::spectral_operation> trans;

		if ( const auto it_cache = _cache[num_vars].find( *tt.cbegin() ); it_cache != _cache[num_vars].end() )
    {
      repr = it_cache->second.first;
      trans = it_cache->second.second;
    }
    else
    {
      repr = *kitty::hybrid_exact_spectral_canonization( tt, [&]( auto const& ops ) { trans = ops; } ).cbegin();
      _cache[num_vars][*tt.cbegin()] = { repr, trans };
    }

    const auto it = _db[num_vars].find( repr );
    if ( it == _db[num_vars].end() )
    {
      fmt::print( "[e] Cannot find repr {:x} in database.\n", repr );
      return;
    }

		/* Construct the optimal implementation of the current cut as an independent XAG */
		xag_network ntk_cut;
		std::vector<xag_network::signal> pis( num_vars, ntk_cut.get_constant( false ) );
		std::generate( pis.begin(), pis.end(), [&ntk_cut]() { return ntk_cut.create_pi(); } );
		const xag_network::signal po_opt = apply_spectral_transformations( ntk_cut, trans, pis, [&]( xag_network& ntk, std::vector<xag_network::signal> const& leaves ) {
      xag_index_list il{ it->second };
      std::vector<xag_network::signal> pos;
      insert( ntk, std::begin( leaves ), std::begin( leaves ) + il.num_pis(), il,
              [&]( xag_network::signal const& f ) {
                pos.push_back( f );
              } );
      assert( pos.size() == 1u );
      return pos[0u];
    } );
    ntk_cut.create_po( po_opt );

		write_expr( ntk_cut, leaves, root, file, true );

		return;
	}

	void build_db()
	{
		for ( auto i = 0u; i < detail::minmc_xags.size(); ++i )
    {
      for ( auto const& [_, word, repr, expr] : detail::minmc_xags[i] )
      {
        (void)_;
        (void)expr;
        _db[i][word] = repr;
      }
    }
	}

private:
	xag_network const& _ntk;
	std::vector<std::unordered_map<uint32_t, std::vector<uint32_t>>> _db{ 6u };
	std::vector<std::unordered_map<uint32_t, std::pair<uint32_t, std::vector<kitty::detail::spectral_operation>>>> _cache{ 6u };
	jefco_params const& _ps;
	jefco_stats& _st;
};

} /* namespace detail */

void jefco( xag_network const& ntk, std::string const& filename, jefco_params const& ps = {}, jefco_stats* pst = nullptr )
{
	jefco_stats st{};
	detail::jefco_impl( ntk, ps, st ).run( filename );

	if ( ps.very_verbose )
	{
		fmt::format( "[m] Mined rules are saved in {}...\n", filename );
	}

	if ( pst )
	{
		*pst = st;
	}
	if ( ps.verbose )
	{
		st.report();
	}
}

} /* namespace mockturtle */