#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>

#include "circuit_validator.hpp"
#include "cut_enumeration.hpp"
#include "cleanup.hpp"
#include "../networks/xag.hpp"
#include "../utils/node_map.hpp"
#include "../utils/progress_bar.hpp"
#include "../utils/stopwatch.hpp"
#include "../views/cut_view.hpp"
#include "../views/topo_view.hpp"

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/spectral.hpp>
#include <kitty/operations.hpp>

namespace mockturtle
{

struct fhe_rule_miner_params
{
	cut_enumeration_params cut_enum_ps;
	uint32_t num_rules_limit{ std::numeric_limits<uint32_t>::max() };
	uint32_t num_node_rules_limit{ std::numeric_limits<uint32_t>::max() };
	bool progress{ true };
	bool verbose{ false };
	bool very_verbose{ false };
};

struct fhe_rule_miner_stats
{
	cut_enumeration_stats cut_enum_st;
	stopwatch<>::duration time_parse_db{ 0 };
	stopwatch<>::duration time_mine_rules{ 0 };
	uint32_t num_rules{ 0u };
	void report() const
	{
		// cut_enum_st.report();
		fmt::print( "[i] Parse database time          = {:>5.2f} secs\n", to_seconds( time_parse_db ) );
		fmt::print( "[i] Mine rewriting rules time    = {:>5.2f} secs\n", to_seconds( time_mine_rules ) );
		fmt::print( "[i] Number of mined rules        = {:>5}\n", num_rules );
	}
};

namespace detail
{

class fhe_rule_miner_impl
{
public:
	fhe_rule_miner_impl( xag_network const& ntk, std::string const& dbname, fhe_rule_miner_params const& ps, fhe_rule_miner_stats& st )
		: _ntk( ntk ), _ps( ps ), _st( st )
	{
		build_db( dbname );
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

		_st.num_rules = 0u;
		const auto cuts = cut_enumeration<xag_network, true>( _ntk, _ps.cut_enum_ps, &( _st.cut_enum_st ) );

		progress_bar pbar{ _ntk.size(), "Mining rules |{0}| node = {1:>4} / " + std::to_string( _ntk.size() ) , _ps.progress };
		topo_view<xag_network>{ _ntk }.foreach_node( [&]( auto const& n, uint32_t index ) {
			pbar( index, index );

			if ( _ntk.is_constant( n ) || _ntk.is_pi( n ) )
			{
				return true;
			}

			if ( _st.num_rules >= _ps.num_rules_limit )
			{
				return true;
			}

			uint32_t num_node_rules{ 0u };
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

				bool added_rules = mine_cut_rule( cuts.truth_table( *pcut ), leaves, _ntk.make_signal( n ), rules );
				if ( added_rules )
				{
					++_st.num_rules;
					++num_node_rules;
					if ( _st.num_rules >= _ps.num_rules_limit || num_node_rules >= _ps.num_node_rules_limit )
					{
						break;
					}
				}
			}

			return true;
		} );

		rules.close();
	}

private:
	void write_network_expr( xag_network const& ntk, std::ofstream& file )
	{
		if ( !file.is_open() )
		{
			fmt::print( "[e] File not open!" );
			abort();
		}

		node_map<std::string, xag_network> expr{ ntk };
		ntk.clear_visited();

		ntk.foreach_pi( [&]( auto const& pi, uint32_t index ) {
			expr[pi] = 'a' + index;
		} );

		topo_view<xag_network>( ntk ).foreach_node( [&]( auto const& n ) {
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
				assert( expr[ni] != "" );

				operands[index] = expr[ni];
				if ( !ntk.is_pi( ni ) )
				{
					operands[index] = "( " + operands[index] + " )";
				}
				if ( ntk.is_complemented( f ) )
				{
					operands[index] = "!" + operands[index];
				}
			} );

			if ( ntk.is_and( n ) )
			{
				expr[n] = operands[0] + " * " + operands[1];
			}
			else
			{
				expr[n] = operands[1] + " ^ " + operands[0];
			}

			return true;
		} );

		assert( ntk.num_pos() == 2u );
		ntk.foreach_po( [&]( auto const& po, uint32_t index ) {
			if ( ntk.is_complemented( po ) )
			{
				file << "!( " << expr[ntk.get_node( po )] << ")";
			}
			else
			{
				file << expr[ntk.get_node( po )];
			}

			if ( index == 0u )
			{
				file << " = ";
			}
			else
			{
				file << '\n';
			}
		} );
	}

	bool mine_cut_rule( kitty::dynamic_truth_table const& tt, std::vector<xag_network::node> const& leaves,
	                    xag_network::signal const& root, std::ofstream& file )
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

		const auto it_cache = _cache[num_vars - 2].find( *( tt.cbegin() ) );
		if ( it_cache != _cache[num_vars - 2].end() )
		{
			return false;
		}

		_cache[num_vars - 2].insert( *( tt.cbegin() ) );
		repr = *( kitty::hybrid_exact_spectral_canonization( tt, [&trans]( auto const& ops ) { trans = ops; } ).cbegin() );
		const auto it_db = _db[num_vars - 2].find( repr );
		if ( it_db == _db[num_vars - 2].end() )
		{
			return false;
		}

		/* Isolate the current cut and obtain an independent XAG */
		xag_network ntk_cut;
		std::vector<xag_network::signal> pis( num_vars, ntk_cut.get_constant( false ) );
		std::generate( pis.begin(), pis.end(), [&ntk_cut]() { return ntk_cut.create_pi(); } );
		cut_view<xag_network> ntk_partial{ _ntk, leaves, root };
		const auto po_orig = cleanup_dangling( ntk_partial, ntk_cut, pis.begin(), pis.end() ).front();
		ntk_cut.create_po( po_orig );
		// write_network_expr( ntk_cut, file );
		// file << " = ";

		/* Construct the optimal implementation of the current cut */
		const xag_network::signal po_db = it_db->second;
		bool po_opt_neg{ false };
		std::vector<xag_network::signal> xors;
		// xag_network ntk_opt;
		// pis.clear();
		// pis.resize( num_vars );
		// std::generate( pis.begin(), pis.end(), [&ntk_opt]() { return ntk_opt.create_pi(); } );

		for ( auto const& op : trans )
		{
			switch ( op._kind )
			{
				case kitty::detail::spectral_operation::kind::permutation:
				{
					const auto v1 = kitty::detail::log2[op._var1];
					const auto v2 = kitty::detail::log2[op._var2];
					std::swap( pis[v1], pis[v2] );
					break;
				}
				case kitty::detail::spectral_operation::kind::input_negation:
				{
					const auto v1 = kitty::detail::log2[op._var1];
					pis[v1] = ntk_cut.create_not( pis[v1] );
					break;
				}
				case kitty::detail::spectral_operation::kind::output_negation:
				{
					po_opt_neg = !po_opt_neg;
					break;
				}
				case kitty::detail::spectral_operation::kind::spectral_translation:
				{
					const auto v1 = kitty::detail::log2[op._var1];
					const auto v2 = kitty::detail::log2[op._var2];
					pis[v1] = ntk_cut.create_xor( pis[v1], pis[v2] );
					break;
				}
				case kitty::detail::spectral_operation::kind::disjoint_translation:
				{
					const auto v1 = kitty::detail::log2[op._var1];
					xors.emplace_back( pis[v1] );
					break;
				}
				default:
					abort();
			}
		}

		xag_network::signal po_opt{};
		if ( _ntk_db.is_constant( _ntk_db.get_node( po_db ) ) )
		{
			po_opt = ntk_cut.get_constant( _ntk_db.is_complemented( po_db ) );
		}
		else
		{
			cut_view<xag_network> ntk_db_partial{ _ntk_db, _db_pis, po_db };
			po_opt = cleanup_dangling( ntk_db_partial, ntk_cut, pis.begin(), pis.end() ).front();
		}
		for ( auto const& operand : xors )
		{
			po_opt = ntk_cut.create_xor( operand, po_opt );
		}
		if ( po_opt_neg )
		{
			po_opt = ntk_cut.create_not( po_opt );
		}

		/* skip trivial rules */
		if ( po_opt == po_orig )
		{
			return false;
		}

		ntk_cut.create_po( po_opt );

		circuit_validator<xag_network, bill::solvers::bsat2, false, false, false> validator( ntk_cut );
		std::optional<bool> eq = validator.validate( po_orig, po_opt );
		assert( eq && ( *eq ) );

		write_network_expr( ntk_cut, file );

		return true;
	}

	void build_db( std::string const& dbname )
	{
		if ( _ps.very_verbose )
		{
			fmt::print( "[i] Start loading database...\n" );
		}
		stopwatch t_parse_db( _st.time_parse_db );

		std::generate( _db_pis.begin(), _db_pis.end(), [this]() { return _ntk_db.create_pi(); } );

		std::ifstream db;
		db.open( dbname, std::ios::in );
		std::string line;
		uint32_t pos{ 0u };

		while ( std::getline( db, line ) )
		{
			pos = static_cast<uint32_t>( line.find( 'x' ) );
			const std::string repr_str = line.substr( ++pos, 8u );
			pos += 9u;
			line.erase( 0, pos );
			pos = line.find( ' ' );
			const uint32_t num_vars = std::stoul( line.substr( 0, pos++ ) );
			line.erase( 0, pos );
			pos = line.find( ' ' );
			const uint8_t mc = std::stoul( line.substr( 0, pos++ ) );
			line.erase( 0, pos );

			std::vector<uint8_t> delay;
			for ( auto i{ 0u }; i < num_vars; ++i )
			{
				pos = line.find( ' ' );
				delay.emplace_back( std::stoul( line.substr( 0, pos++ ) ) );
				line.erase( 0, pos );
			}

			std::vector<xag_network::signal> signals( _db_pis.begin(), _db_pis.begin() + num_vars );
			while ( line.size() > 3 )
			{
				std::array<uint32_t, 2> s_ind{};
				for ( uint32_t& ele : s_ind )
				{
					pos = line.find( ' ' );
					ele = std::stoul( line.substr( 0, pos++ ) );
					line.erase( 0, pos );
				}

				std::array<xag_network::signal, 2> s;
				for ( auto i{ 0u }; i < 2u; ++i )
				{
					if ( s_ind[i] == 0u )
					{
						s[i] = _ntk_db.get_constant( false );
					}
					else if ( s_ind[i] == 1u )
					{
						s[i] = _ntk_db.get_constant( true );
					}
					else
					{
						s[i] = signals[s_ind[i] / 2 - 1] ^ ( s_ind[i] % 2 );
					}
				}
				if ( s_ind[0] > s_ind[1] )
				{
					signals.emplace_back( _ntk_db.create_xor( s[0], s[1] ) );
				}
				else
				{
					signals.emplace_back( _ntk_db.create_and( s[0], s[1] ) );
				}
			}
			const uint32_t po_ind = std::stoul( line );
			const xag_network::signal po = signals[po_ind / 2 - 1] ^ ( po_ind % 2 );
			_ntk_db.create_po( po );

			for ( auto i{ num_vars }; i <= 5u; ++i )
			{
				const uint32_t repr = std::stoul( ( "0x" + repr_str.substr( 0, 1 << ( i - 2 ) ) ),  nullptr, 16 );
				_db[i - 2][repr] = po;
			}
		}
		for ( auto i{ 0u }; i < 4u; ++i )
		{
			_db[i][0] = _ntk_db.get_constant( false );
		}

		if ( _ps.very_verbose )
		{
			fmt::print( "[i] Finished loading database!\n" );
		}
	}

private:
	xag_network const& _ntk;
	xag_network _ntk_db;
	std::vector<xag_network::signal> _db_pis{ 5u };
	std::vector<std::unordered_map<uint32_t, xag_network::signal>> _db{ 4u };
	std::vector<std::unordered_set<uint32_t>> _cache{ 4u };
	fhe_rule_miner_params const& _ps;
	fhe_rule_miner_stats& _st;
};

} /* namespace detail */

void fhe_rule_miner( xag_network const& ntk, std::string const& dbname, std::string const& filename, fhe_rule_miner_params const& ps = {}, fhe_rule_miner_stats* pst = nullptr )
{
	fhe_rule_miner_stats st{};
	detail::fhe_rule_miner_impl( ntk, dbname, ps, st ).run( filename );
	if ( st.num_rules >= ps.num_rules_limit )
	{
		fmt::print( "[i] Early termination since sufficient rules have been mined\n" );
	}
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