#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>

#include <kitty/operations.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_enumeration.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/cost_functions.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/utils/progress_bar.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/topo_view.hpp>

namespace mockturtle
{

struct fhe_optimization_params
{
	bool only_on_critical_path{ true };
	cut_enumeration_params cut_enum_ps;
	bool progress{ true };
	bool verbose{ false };
};

struct fhe_optimization_stats
{
	cut_enumeration_stats cut_enum_st;
	void report() const
	{
		cut_enum_st.report();
	}
};

struct arrival_time_pair
{
	xag_network::signal f;
	uint32_t m_depth;
};

namespace detail
{

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

template<class GLocalOptFn, class CMLocalOptFn>
struct fhe_optimization_impl
{
	// fhe_optimization_impl( xag_network const& ntk, GLocalOptFn const& g_local_opt_fn, fhe_optimization_params const& ps, fhe_optimization_stats& st )
	// 		: ntk_( ntk ), g_local_opt_fn_( g_local_opt_fn ), ps_( ps ), st_( st )
	// {
	// }
	fhe_optimization_impl( xag_network const& ntk, GLocalOptFn const& g_local_opt_fn, CMLocalOptFn const& cm_local_opt_fn, fhe_optimization_params const& ps, fhe_optimization_stats& st )
			: ntk_( ntk ), g_local_opt_fn_( g_local_opt_fn ), cm_local_opt_fn_( cm_local_opt_fn ), ps_( ps ), st_( st )
	{}

	xag_network run()
	{
		xag_network res;
		depth_view<xag_network, num_and, false> ntk_md_{ ntk_ };
		//uint32_t circuit_m_depth{ ntk_md_.depth() };
		//uint32_t circuit_m_compl{ costs<xag_network, num_and>( ntk_ ) };
		uint32_t current_m_depth{ 0u };

		/* cut enumeration */
		const auto cuts = cut_enumeration<xag_network, true>( ntk_, ps_.cut_enum_ps, &( st_.cut_enum_st ) );

		node_map<arrival_time_pair, xag_network> old2new{ ntk_ };

		/* constants */
		old2new[ntk_.get_constant( false )] = { res.get_constant( false ), 0u };
		
		/* pis */
		ntk_.foreach_pi( [&]( auto const& n ) {
			old2new[n] = { res.create_pi(), 0u };
		} );

		/* gates */
		progress_bar pbar{ ntk_.size(), "FHE optimization |{0}| node = {1:>4} / " + std::to_string( ntk_.size() ) + "   current mult. depth = {2}", ps_.progress };
		topo_view<xag_network>{ ntk_ }.foreach_node( [&]( auto const& n, auto index ) {
			pbar( index, index, current_m_depth );

			if ( ntk_.is_constant( n ) || ntk_.is_pi( n ) )
			{
				return;
			}

			if ( ps_.only_on_critical_path && !ntk_md_.is_on_critical_path( n ) )
			{
				/* duplicate this node */
				std::vector<xag_network::signal> children;
				ntk_.foreach_fanin( n, [&]( auto const& fi ) {
					auto const fi_new = old2new[fi].f;
					children.emplace_back( ntk_.is_complemented( fi ) ? res.create_not( fi_new ) : fi_new );
				} );
				old2new[n] = { res.clone_node( ntk_, n, children ), ntk_md_.level( n ) };
				return;
			}

			arrival_time_pair best_impl{ {}, std::numeric_limits<uint32_t>::max() };
			for ( auto const& pcut: cuts.cuts( ntk_.node_to_index( n ) ) )
			{
				/* trivial cut */
				if ( pcut->size() == 1u || kitty::is_const0( cuts.truth_table( *pcut ) ) )
				{
					continue;
				}

				std::vector<arrival_time_pair> arrival_times( pcut->size() );
				bool equal_arrival_times{ true };
				const uint32_t first_arrival_time{ old2new[ntk_.index_to_node( *( pcut->begin() ) )].m_depth };
				std::transform( pcut->begin(), pcut->end(), arrival_times.begin(), [&]( auto const& leaf ) { 
					const auto arrival_time_p = old2new[ntk_.index_to_node( leaf )];
					equal_arrival_times &= ( arrival_time_p.m_depth == first_arrival_time );
					return arrival_time_p;
				} );

				/* there is room exploring the metric evaluating the quality of a local implementation */
				/* currently, we simply adopt the mult. depth as the only criteria                     */
				const auto on_signal = [&]( arrival_time_pair const& cand ) {
					if ( cand.m_depth < best_impl.m_depth )
					{
						best_impl = cand;
					}
				};

				/* run the generalized local optimization function anyway, since there is no guarantee */
				/* that the customized local optimization function can find a solution                 */
				/* TODO: how to skip cuts whose local function's functional MC is 0?                   */
				const uint32_t g_m_depth = g_local_opt_fn_.run( res, cuts.truth_table( *pcut ), arrival_times, on_signal );
				if ( ps_.verbose )
				{
					fmt::print( "[m] managed to construct a partial network based on DB ({})\n", g_m_depth );
				}

				/* run the customized local optimization function only if the arrival times of leaves  */
				/* are not equal                                                                       */
				uint32_t c_m_depth{ g_m_depth };
				if ( !equal_arrival_times )
				{
					c_m_depth = cm_local_opt_fn_.run( res, cuts.truth_table( *pcut ), arrival_times, g_m_depth, on_signal );
					if ( ps_.verbose )
					{
						if ( c_m_depth != g_m_depth )
						{
							fmt::print( "[m] managed to exactly synthesize a better partial network ({}->{})\n", g_m_depth, c_m_depth );
						}
						else
						{
							fmt::print( "[m] failed to exactly synthesize a better partial network\n" );
						}
					}
				}
			}
			old2new[n] = best_impl;
			current_m_depth = std::max( current_m_depth, best_impl.m_depth );
		} );

		/* pos */
		ntk_.foreach_po( [&]( auto const& po_old ) {
			auto const po_new = old2new[po_old].f;
			res.create_po( ntk_.is_complemented( po_old ) ? res.create_not( po_new ) : po_new );
		} );

		return cleanup_dangling( res );
	}

private:
	xag_network const& ntk_;
	GLocalOptFn const& g_local_opt_fn_;
	CMLocalOptFn const& cm_local_opt_fn_;
	fhe_optimization_params const& ps_;
	fhe_optimization_stats& st_;
};

} /* detail */

/* TODO: debug the generic local optimization function */
template<class GLocalOptFn, class CMLocalOptFn>
xag_network fhe_optimization( xag_network const& ntk, GLocalOptFn const& g_local_opt_fn, CMLocalOptFn const& cm_local_opt_fn, fhe_optimization_params const& ps = {}, fhe_optimization_stats* pst = nullptr )
// template<class GLocalOptFn>
// xag_network fhe_optimization( xag_network const& ntk, GLocalOptFn const& g_local_opt_fn, fhe_optimization_params const& ps = {}, fhe_optimization_stats* pst = nullptr )
{
	fhe_optimization_stats st;
	// const auto res = detail::fhe_optimization_impl( ntk, g_local_opt_fn, ps, st ).run();
	const auto res = detail::fhe_optimization_impl( ntk, g_local_opt_fn, cm_local_opt_fn, ps, *pst ).run();

	if ( pst )
	{
		*pst = st;
	}
	if ( ps.verbose )
	{
		st.report();
	}

	return res;
}

} /* mockturtle */