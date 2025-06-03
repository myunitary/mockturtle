#pragma once

#include "../utils/cost_functions.hpp"
#include "../utils/node_map.hpp"
#include "../utils/progress_bar.hpp"
// #include "../views/topo_view.hpp"
#include "cut_enumeration.hpp"
#include "detail/mffc_utils.hpp"
#include "mc_analyzer.hpp"

#include <algorithm>

namespace mockturtle
{

struct mc_mpc_cost
{
  uint32_t operator()( owner_view<xag_network> const& ntk, owner_view<xag_network>::node const& n ) const
  {
    if ( ( ntk.is_local( n ) == 0u ) && ntk.is_and( n ) )
    {
      return 1u;
    }
    return 0;
  }
};

struct xag_decomp_max_local_params
{
  xag_decomp_max_local_params()
  {
    cut_enum_ps.cut_size = 8u;
    cut_enum_ps.cut_limit = 12u;
    cut_enum_ps.minimize_truth_table = true;
  }
  cut_enumeration_params cut_enum_ps{};
};

struct xag_decomp_max_local_stats
{
  uint32_t num_cuts = 0u;
  uint32_t num_decomp = 0u;
  uint32_t num_decomp_success = 0u;
  uint32_t num_better_impl = 0u;
  uint32_t cost_original = 0u;
  uint32_t cost_optimal = 0u;
};

class xag_decomp_max_local
{
public:
  using mpc_network = owner_view<xag_network>;
  explicit xag_decomp_max_local( const uint32_t num_parties, xag_network const& ntk, std::string const& filename, xag_decomp_max_local_params const& ps, xag_decomp_max_local_stats& st )
    : _num_parties( num_parties ),
      _ntk( ntk ),
      _mc_analyzer( filename ),
      _ps( ps ),
      _st( st )
  {}

  xag_network run()
  {
    xag_network res{};
    node_map<xag_network::signal, xag_network> old_to_new{ _ntk };
    detail::initialize_values_with_fanout( _ntk );
    mpc_network ntk_os{ _ntk };
    _st.cost_original = costs<mpc_network, mc_mpc_cost>( ntk_os );

    /* enumerate cuts */
    const auto cuts = cut_enumeration<xag_network, true>( _ntk, _ps.cut_enum_ps );

    /* constants */
    old_to_new[_ntk.get_constant( false )] = res.get_constant( false );

    /* pis */
    std::vector<uint32_t> pi_ownerships( _num_parties );
    _ntk.foreach_pi( [&]( auto const& pi ) {
      auto new_pi = res.create_pi();
      old_to_new[pi] = new_pi;
      const uint32_t pi_ownership = ntk_os.is_local( pi );
      assert( pi_ownership != 0u );
      ++( pi_ownerships[pi_ownership - 1] );
    } );
    /* for debugging */
    // {
    //   fmt::print( "pi_ownerships = [" );
    //   for ( auto ele : pi_ownerships )
    //   {
    //     fmt::print( "{} ", ele );
    //   }
    //   fmt::print( "]\n" );
    // }

    fmt::print( "[m] Ready to optimize!\n" );

    /* gates */
    progress_bar pbar{ _ntk.num_gates(), "[i] Decomposition-based local computation maximization |{0}| node = {1:>4} / " + std::to_string( _ntk.num_gates() ) + "   original cost = " + std::to_string( _st.cost_original ), true };
    topo_view<xag_network>{ _ntk }.foreach_gate( [&]( auto const& n, auto index ) {
    // _ntk.foreach_gate( [&]( auto const& n, auto index ) {
      pbar( index, index );

      const int32_t value = detail::mffc_size<mpc_network, mc_mpc_cost>( ntk_os, n );
      if ( value <= 1 )
      /* for debugging */
      // if ( true )
      {
        /* impossible to achieve a lower-cost implementation */
        /* copy-paste the original implementation of node `n` */
        std::vector<xag_network::signal> children_new = { res.get_constant( false ), res.get_constant( false ) };
        std::vector<xag_network::signal> children_old = { _ntk._storage->nodes[n].children[0], _ntk._storage->nodes[n].children[1] };
        children_new[0] = _ntk.is_complemented( children_old[0] ) ? res.create_not( old_to_new[children_old[0]] ) : old_to_new[children_old[0]];
        children_new[1] = _ntk.is_complemented( children_old[1] ) ? res.create_not( old_to_new[children_old[1]] ) : old_to_new[children_old[1]];
        old_to_new[n] = res.clone_node( _ntk, n, children_new );
        assert( !res.is_constant( res.get_node( old_to_new[n] ) ) );

        return true;
      }

      /* enumerating cuts to look for the best implementation */
      xag_network::signal impl_best = res.get_constant( false );
      int32_t gain_best = 0;
      for ( auto& cut : cuts.cuts( _ntk.node_to_index( n ) ) )
      {
        ++_st.num_cuts;
        if ( cut->size() <= 2u || kitty::is_const0( cuts.truth_table( *cut ) ) )
        {
          continue;
        }

        const kitty::dynamic_truth_table tt = cuts.truth_table( *cut );
        std::vector<xag_network::signal> children( cut->size() );
        std::vector<std::vector<uint32_t>> ownerships( _num_parties + 1u );
        /* create `ownwership` and `children` */
        auto ctr = 0u;
        for ( auto leaf : *cut )
        {
          const auto leaf_node = _ntk.index_to_node( leaf );
          const uint32_t party_ind = ntk_os.is_local( leaf_node );
          if ( party_ind > ( ownerships.size() - 1 ) )
          {
            fmt::print( "The ownership id of the current signal is {}! Something is Wrong!\n", party_ind );
            abort();
          }
          ownerships[party_ind].push_back( ctr );

          /* for debugging */
          if ( old_to_new[leaf_node] == xag_network::signal{} )
          {
            fmt::print( "[e] old_to_new[{}] is invalid\n", leaf_node );
            abort();
          
          }
          children[ctr++] = old_to_new[leaf_node];

          /* for debugging */
          if ( res.is_constant( res.get_node( old_to_new[leaf_node]) ) )
          {
            fmt::print( "[e] Something is wrong: Leaf {}Node{} ( Node{} in `ntk` ) is a constant\n", ( res.is_complemented( old_to_new[leaf_node]) ? "~" : "" ), res.get_node( old_to_new[leaf_node] ), leaf_node );
            abort();
          }
        }
        ownerships.erase( std::remove_if( ownerships.begin() + 1u, ownerships.end(), []( auto const& v ){ return v.empty(); } ), ownerships.end() );
        std::sort( ownerships.begin() + 1u, ownerships.end(), []( auto const& v1, auto const& v2 ) { return ( v1.size() > v2.size() ); } );
        /* rule out pointless cases */
        if ( ( ownerships.size() == 1u ) ||
             ( ownerships.size() == 2u && ownerships[0].empty() ) )
        {
          continue;
        }

        /* for debugging */
        fmt::print( "[m] (First check) Leaves of the current cut are [ " );
        for ( auto const& each_leaf : children )
        {
          fmt::print( "{}Node{} ", ( res.is_complemented( each_leaf ) ? "~" : "" ), ( res.node_to_index( res.get_node( each_leaf ) ) ) );
        }
        fmt::print( "]\n[m] Ownerships = [ " );
        for ( auto const& ele : ownerships )
        {
          fmt::print( "{} ", ele.size() );
        }
        fmt::print( "]\n" );

        acdecomp_max_local acd_max_local( tt, acdecomp_max_local_params{} );
        ++_st.num_decomp;
        if ( !acd_max_local.investigate_decomp( ownerships ) )
        {
          fmt::print( "[m] failed paritioning signals\n" );
          continue;
        }
        /* for debugging */
        // else
        // {
        //   continue;
        // }
        if ( !acd_max_local.compute_decomp() )
        {
          fmt::print( "[m] failed finding BS functions\n" );
          continue;
        }
        /* for debugging */
        // else
        // {
        //   ++_st.num_decomp_success;
        //   continue;
        // }
        ++_st.num_decomp_success;
        acd_max_local.get_decomp();

        /* address the discrepancy within the obrained solution `acd_max_local._decomp_res`: */
        /* the available index vector `ownerships` and the required leaf node vector `children` */
        xag_network::signal impl_new = _mc_analyzer.min_mc_resyn( res, children, acd_max_local._decomp_res );
        if ( res.is_constant( res.get_node( impl_new ) ) )
        {
          continue;
        }
        mpc_network res_os( res, pi_ownerships );
        const int32_t value_new = detail::recursive_ref<mpc_network, mc_mpc_cost>( res_os, res_os.get_node( impl_new ) );
        detail::recursive_deref<mpc_network, mc_mpc_cost>( res_os, res_os.get_node( impl_new ) );
        const int32_t gain = value - value_new;

        /* for debugging */
        cut_view<xag_network> new_impl{ res, children, impl_new };
        kitty::static_truth_table<6u> tt_new_impl = simulate<kitty::static_truth_table<6u>>( new_impl )[0];
        const auto tt_ext = kitty::extend_to<6u>( tt );
        if ( tt_new_impl != tt_ext )
        {
          fmt::print( "[m] target truth table is " );
          kitty::print_hex( tt_ext );
          fmt::print( ", get " );
          kitty::print_hex( tt_new_impl );
          fmt::print( "\n" );
          abort();
        }

        if ( gain > gain_best )
        {
          impl_best = impl_new;
          gain_best = gain;
          ++_st.num_better_impl;
        }
      }

      if ( gain_best == 0 )
      {
        /* copy-paste the original implementation of node `n` */
        std::vector<xag_network::signal> children_new = { res.get_constant( false ), res.get_constant( false ) };
        std::vector<xag_network::signal> children_old = { _ntk._storage->nodes[n].children[0], _ntk._storage->nodes[n].children[1] };
        children_new[0] = _ntk.is_complemented( children_old[0] ) ? res.create_not( old_to_new[children_old[0]] ) : old_to_new[children_old[0]];
        children_new[1] = _ntk.is_complemented( children_old[1] ) ? res.create_not( old_to_new[children_old[1]] ) : old_to_new[children_old[1]];
        old_to_new[n] = res.clone_node( _ntk, n, children_new );
        assert( !res.is_constant( res.get_node( old_to_new[n] ) ) );
      }
      else
      {
        old_to_new[n] = impl_best;
      }

      detail::recursive_ref<xag_network>( res, res.get_node( old_to_new[n] ) );
      return true;
    } );
    fmt::print( "\n" );

    /* pos */
    _ntk.foreach_po( [&]( auto const& po ) {
      res.create_po( _ntk.is_complemented( po ) ? res.create_not( old_to_new[_ntk.get_node( po )]) : old_to_new[_ntk.get_node( po )] );
    } );

    xag_network res_cleanup = cleanup_dangling<xag_network>( res );
    // mpc_network res_cleanup_os( res_cleanup, pi_ownerships );
    // _st.cost_optimal = costs<mpc_network, mc_mpc_cost>( res_cleanup_os );
    _st.cost_optimal = costs<mpc_network, mc_mpc_cost>( mpc_network( res_cleanup, pi_ownerships ) );
    return ( _st.cost_optimal < _st.cost_original ) ? res : _ntk;
  }

private:
  uint32_t _num_parties;
  xag_network const& _ntk;
  mc_analyzer _mc_analyzer;
  xag_decomp_max_local_params const& _ps;
  xag_decomp_max_local_stats& _st;
};
} /* namespace mockturtle */