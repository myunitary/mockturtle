#pragma once

#include "../utils/cost_functions.hpp"
#include "../utils/node_map.hpp"
#include "../utils/progress_bar.hpp"
#include "../views/fanout_view.hpp"
// #include "../views/topo_view.hpp"
#include "cut_enumeration.hpp"
#include "cut_enumeration/rewrite_cut.hpp"
#include "detail/mffc_utils.hpp"
#include "acdecomp_max_local.hpp"
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
    return 0u;
  }
};
struct mc_mpc_cost2
{
  uint32_t operator()( fanout_view<owner_view<xag_network>> const& ntk, fanout_view<owner_view<xag_network>>::node const& n ) const
  {
    if ( ( ntk.is_local( n ) == 0u ) && ntk.is_and( n ) )
    {
      return 1u;
    }
    return 0u;
  }
};

struct xag_decomp_max_local_params
{
  xag_decomp_max_local_params()
  {
    cut_enum_ps.cut_size = 8u;
    cut_enum_ps.cut_limit = 8u;
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
    mpc_network ntk_os{ _ntk };
    detail::initialize_values_with_fanout( ntk_os );
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
    progress_bar pbar{ _ntk.num_gates(), "[i] Decomposition-based local computation maximization |{0}| node = {1:>4} / " + std::to_string( _ntk.num_gates() ), true };
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
        std::vector<xag_network::node> leaves( cut->size() );
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
          // if ( old_to_new[leaf_node] == xag_network::signal{} )
          // {
          //   fmt::print( "[e] old_to_new[{}] is invalid\n", leaf_node );
          //   abort();
          
          // }
          leaves[ctr] = leaf_node;
          children[ctr++] = old_to_new[leaf_node];

          /* for debugging */
          // if ( res.is_constant( res.get_node( old_to_new[leaf_node]) ) )
          // {
          //   fmt::print( "[e] Something is wrong: Leaf {}Node{} ( Node{} in `ntk` ) is a constant\n", ( res.is_complemented( old_to_new[leaf_node]) ? "~" : "" ), res.get_node( old_to_new[leaf_node] ), leaf_node );
          //   abort();
          // }
        }

        /* calculate the cost of current implementation */
        const uint32_t cost_cut_current = detail::mffc_size<mpc_network, decltype( leaves.begin() ), mc_mpc_cost>( ntk_os, n, leaves.begin(), leaves.end() );
        if ( cost_cut_current == 0u )
        {
          continue;
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
        // fmt::print( "[m] (First check) Leaves of the current cut are [ " );
        // for ( auto const& each_leaf : children )
        // {
        //   fmt::print( "{}Node{} ", ( res.is_complemented( each_leaf ) ? "~" : "" ), ( res.node_to_index( res.get_node( each_leaf ) ) ) );
        // }
        // fmt::print( "]\n[m] Ownerships = [ " );
        // for ( auto const& ele : ownerships )
        // {
        //   fmt::print( "{} ", ele.size() );
        // }
        // fmt::print( "]\n" );

        acdecomp_max_local acd_max_local( tt, acdecomp_max_local_params{} );
        ++_st.num_decomp;
        if ( !acd_max_local.investigate_decomp( ownerships ) )
        {
          // fmt::print( "[m] failed paritioning signals\n" );
          continue;
        }
        /* for debugging */
        // else
        // {
        //   continue;
        // }
        if ( !acd_max_local.compute_decomp() )
        {
          // fmt::print( "[m] failed finding BS functions\n" );
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
        const xag_network::node impl_new_node = res_os.get_node( impl_new );
        // detail::recursive_deref<mpc_network, mc_mpc_cost>( res_os, impl_new_node );
        assert( res_os.value( impl_new_node ) == 0u );
        // res_os.profile_ntk();
        const uint32_t cost_cut_new = detail::recursive_ref<mpc_network, mc_mpc_cost>( res_os, impl_new_node );
        // const int32_t value_new = detail::recursive_ref<mpc_network, mc_mpc_cost>( res_os, impl_new_node );
        detail::recursive_deref<mpc_network, mc_mpc_cost>( res_os, res_os.get_node( impl_new ) );
        // const int32_t gain = value - value_new;
        const int32_t gain = ( ( cost_cut_current <= cost_cut_new ) ? 0u : ( cost_cut_current - cost_cut_new ) );
        // fmt::print( "[m] Previous local cost: {}, new local cost {}.\n", cost_cut_current, cost_cut_new );

        // /* for debugging */
        // cut_view<xag_network> new_impl{ res, children, impl_new };
        // kitty::static_truth_table<6u> tt_new_impl = simulate<kitty::static_truth_table<6u>>( new_impl )[0];
        // const auto tt_ext = kitty::extend_to<6u>( tt );
        // if ( tt_new_impl != tt_ext )
        // {
        //   fmt::print( "[m] target truth table is " );
        //   kitty::print_hex( tt_ext );
        //   fmt::print( ", get " );
        //   kitty::print_hex( tt_new_impl );
        //   fmt::print( "\n" );
        //   abort();
        // }

        if ( gain > gain_best )
        {
          impl_best = impl_new;
          gain_best = gain;
          ++_st.num_better_impl;
          // fmt::print( "[m] Found a better {}-cut implementation that reduces local cost from {} to {}\n", children.size(), cost_cut_current, cost_cut_new );
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
    // fmt::print( "\n" );

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

class xag_decomp_max_local_in_place
{
public:
  using mpc_network = owner_view<xag_network>;
  using node = mpc_network::node;
  using signal = mpc_network::signal;
  static constexpr uint32_t num_vars = 8u;
  using network_cuts_t = dynamic_network_cuts<mpc_network, num_vars, true, cut_enumeration_rewrite_cut>;
  using cut_manager_t = detail::dynamic_cut_enumeration_impl<mpc_network, num_vars, true, cut_enumeration_rewrite_cut>;
  using cut_t = typename network_cuts_t::cut_t;
  using node_data = typename mpc_network::storage::element_type::node_type;

  explicit xag_decomp_max_local_in_place( const uint32_t num_parties, mpc_network& ntk, std::string const& filename, xag_decomp_max_local_params const& ps, xag_decomp_max_local_stats& st )
    : _num_parties( num_parties ),
      _ntk( ntk ),
      _mc_analyzer( filename ),
      _ps( ps ),
      _st( st )
  {
    register_events();
  }

  ~xag_decomp_max_local_in_place()
  {
    _ntk.events().release_add_event( add_event );
    _ntk.events().release_modified_event( modified_event );
    _ntk.events().release_delete_event( delete_event );
  }

  void run()
  {
    _st.cost_original = costs<mpc_network, mc_mpc_cost>( _ntk );
    // _ntk.incr_trav_id();
    detail::initialize_values_with_fanout( _ntk );
    cut_enumeration_stats cst;
    network_cuts_t cuts( _ntk.size() + ( _ntk.size() >> 1 ) );
    cut_manager_t cut_manager( _ntk, _ps.cut_enum_ps, cst, cuts );
    cut_manager.init_cuts();

    _ntk.foreach_gate( [&]( node const& n, auto index ) {
      // fmt::print( "[i] Working on the {}-th node...\n", ( index + 1 ) );
      // if ( _ntk.fanout_size( n ) == 0u )
      if ( _ntk.value( n ) == 0u )
      {
        return;
      }

      int32_t gain_best = 0;
      uint32_t cut_index = 0u;
      uint32_t cut_best_index = 0u;
      std::vector<decomp_result> decomp_res_best{};
      cut_manager.clear_cuts( n );
      cut_manager.compute_cuts( n );
      
      for ( auto& cut : cuts.cuts( _ntk.node_to_index( n ) ) )
      {
        ++_st.num_cuts;
        ++cut_index; /* the real index is ( `cut_index` - 1 ) */
        if ( cut->size() <= 2u || kitty::is_const0( cuts.truth_table( *cut ) ) )
        {
          continue;
        }
        
        kitty::dynamic_truth_table tt( cut->size() );
        /* `cut->size()` <= 8u */
        for ( auto i{ 0u }; i < tt.num_blocks(); ++i )
        {
          tt._bits[i] = cuts.truth_table( *cut )._bits[i];
        }
        // std::copy( cuts.truth_table( *cut ).begin(), cuts.truth_table( *cut ).begin() + tt.num_blocks(), tt.begin() );
        std::vector<std::vector<uint32_t>> ownerships( _num_parties + 1u );
        std::vector<node> leaves( cut->size() );
        auto ctr = 0u;
        for ( auto leaf : *cut )
        {
          const node leaf_node = _ntk.index_to_node( leaf );
          const uint32_t party_ind = _ntk.is_local( leaf_node );
          assert ( party_ind <= ( ownerships.size() - 1 ) );
          ownerships[party_ind].push_back( ctr );
          leaves[ctr] = leaf_node;
          ++ctr;
        }
        ownerships.erase( std::remove_if( ownerships.begin() + 1u, ownerships.end(), []( auto const& v ){ return v.empty(); } ), ownerships.end() );
        std::sort( ownerships.begin() + 1u, ownerships.end(), []( auto const& v1, auto const& v2 ) { return ( v1.size() > v2.size() ); } );
        /* rule out pointless cases */
        if ( ( ownerships.size() == 1u ) ||
             ( ownerships.size() == 2u && ownerships[0].empty() ) )
        {
          continue;
        }

        // int32_t mffc_size = measure_mffc_deref( n, cut );
        uint32_t mffc_size = detail::mffc_size<mpc_network, decltype( leaves.begin() ), mc_mpc_cost>( _ntk, n, leaves.begin(), leaves.end() );
        if ( mffc_size <= 1u )
        {
          continue;
        }

        acdecomp_max_local acd_max_local{ tt, acdecomp_max_local_params{} };
        ++_st.num_decomp;

        /* push `investigate_decomp`, `compute_decomp`, and `get_decomp` into one function */
        /* so that multiple solutions to the same decomposition query could be evaluated */
        int gain = acd_max_local.multi_run( _mc_analyzer, ownerships );
        if ( gain < 0 )
        {
          // fmt::print( "[m] failed paritioning signals\n" );
          continue;
        }
        ++_st.num_decomp_success;

        /* measure the cost of `acd_max_local._decomp_res` */
        /* calculate the gain of current solution by substracting `mffc_size` from the cost */
        /* update `gain_best` and other records if necessary */
        // fmt::print( "[m] Removes nodes: {}; Added nodes: {}\n", mffc_size, gain );
        gain = static_cast<int32_t>( mffc_size ) - gain;
        
        if ( gain > gain_best )
        {
          gain_best = gain;
          cut_best_index = cut_index;
          decomp_res_best = acd_max_local._decomp_res;
        }
        // measure_mffc_ref( n, cut );

        /* is this line necessary? */
        if ( cut->size() == 0 || ( cut->size() == 1u && *cut->begin() != _ntk.node_to_index( n ) ) )
        {
          break;
        }

        // xag_network::signal impl_new = _mc_analyzer.min_mc_resyn( _ntk, children, acd_max_local._decomp_res );

        /* TODO: the cost model currently adopted is a too-loose upper bound */
        /* when there are more than one potential FS set */
      }

      if ( cut_best_index != 0u )
      {
        fmt::print( "[m] Network rewriting is happening\n" );
        auto best_cut = cuts.cuts( _ntk.node_to_index( n ) )[cut_best_index - 1];
        kitty::dynamic_truth_table tt( best_cut.size() );
        // std::copy( cuts.truth_table( best_cut ).begin(), cuts.truth_table( best_cut ).begin() + tt.num_blocks(), tt.begin() );
        for ( auto i{ 0u }; i < tt.num_blocks(); ++i )
        {
          tt._bits[i] = cuts.truth_table( best_cut )._bits[i];
        }
        std::vector<mpc_network::signal> children( best_cut.size() ); 
        std::vector<mpc_network::node> leaves( best_cut.size() );
        auto ctr = 0u;
        for ( auto leaf : best_cut )
        {
          leaves[ctr] = _ntk.index_to_node( leaf );
          children[ctr] = _ntk.make_signal( leaves[ctr] );
          ++ctr;
        }
        /* TODO: tune the order of node enumeration */
        // auto pi0 = _ntk.make_signal( _ntk.pi_at( 0u ) );
        // _ntk.create_xor( pi0, _ntk.make_signal( n ) );
        // fmt::print( "[m] Size of `_ntk` is {}\n", _ntk.size() );
        mpc_network::signal n_new_impl = _mc_analyzer.min_mc_resyn_mpc( _ntk, children, decomp_res_best );
        detail::recursive_deref<mpc_network, decltype( leaves.begin() ), mc_mpc_cost>( _ntk, n, leaves.begin(), leaves.end() );
        detail::recursive_ref<mpc_network, decltype( leaves.begin() ), mc_mpc_cost>( _ntk, _ntk.get_node( n_new_impl ), leaves.begin(), leaves.end() );
        // detail::recursive_deref<mpc_network>( _ntk, n);
        // detail::recursive_ref<mpc_network>( _ntk, _ntk.get_node( n_new_impl ) );
        _ntk.substitute_node_no_restrash( n, n_new_impl );
        clear_cuts_fanout_rec( cuts, cut_manager, _ntk.get_node( n_new_impl ) );
        ++_st.num_better_impl;
      }
    } );

    _st.cost_optimal = costs<mpc_network, mc_mpc_cost>( _ntk );
  }

  void run_reverse_topo()
  {
    _st.cost_original = costs<mpc_network, mc_mpc_cost>( _ntk );
    bool rewriten{ false };
    do
    {
      detail::initialize_values_with_fanout( _ntk );
      cut_enumeration_stats cst;
      network_cuts_t cuts( _ntk.size() + ( _ntk.size() >> 1 ) );
      cut_manager_t cut_manager( _ntk, _ps.cut_enum_ps, cst, cuts );
      cut_manager.init_cuts();
      rewriten = run_reverse_topo_each( cut_manager, cuts );
    } while ( rewriten );
    _st.cost_optimal = costs<mpc_network, mc_mpc_cost>( _ntk );
  }

  bool run_reverse_topo_each( cut_manager_t& cut_manager, network_cuts_t& cuts )
  {
    bool rewriten{ false };
    topo_view<mpc_network>{ _ntk }.foreach_gate_reverse( [&]( auto const& n ) {
      if ( _ntk.value( n ) == 0u )
      {
        return true;
      }

      int32_t gain_best = 0;
      uint32_t cut_index = 0u;
      uint32_t cut_best_index = 0u;
      std::vector<decomp_result> decomp_res_best{};
      cut_manager.clear_cuts( n );
      cut_manager.compute_cuts( n );
      
      for ( auto& cut : cuts.cuts( _ntk.node_to_index( n ) ) )
      {
        ++_st.num_cuts;
        ++cut_index; /* the real index is ( `cut_index` - 1 ) */
        if ( cut->size() <= 2u || kitty::is_const0( cuts.truth_table( *cut ) ) )
        {
          continue;
        }
        
        kitty::dynamic_truth_table tt( cut->size() );
        /* `cut->size()` <= 8u */
        for ( auto i{ 0u }; i < tt.num_blocks(); ++i )
        {
          tt._bits[i] = cuts.truth_table( *cut )._bits[i];
        }
        std::vector<std::vector<uint32_t>> ownerships( _num_parties + 1u );
        std::vector<node> leaves( cut->size() );
        auto ctr = 0u;
        for ( auto leaf : *cut )
        {
          const node leaf_node = _ntk.index_to_node( leaf );
          const uint32_t party_ind = _ntk.is_local( leaf_node );
          assert ( party_ind <= ( ownerships.size() - 1 ) );
          ownerships[party_ind].push_back( ctr );
          leaves[ctr] = leaf_node;
          ++ctr;
        }
        ownerships.erase( std::remove_if( ownerships.begin() + 1u, ownerships.end(), []( auto const& v ){ return v.empty(); } ), ownerships.end() );
        std::sort( ownerships.begin() + 1u, ownerships.end(), []( auto const& v1, auto const& v2 ) { return ( v1.size() > v2.size() ); } );
        /* rule out pointless cases */
        if ( ( ownerships.size() == 1u ) ||
             ( ownerships.size() == 2u && ownerships[0].empty() ) )
        {
          continue;
        }

        uint32_t mffc_size = detail::mffc_size<mpc_network, decltype( leaves.begin() ), mc_mpc_cost>( _ntk, n, leaves.begin(), leaves.end() );
        if ( mffc_size <= 1u )
        {
          continue;
        }

        acdecomp_max_local acd_max_local{ tt, acdecomp_max_local_params{} };
        ++_st.num_decomp;

        /* push `investigate_decomp`, `compute_decomp`, and `get_decomp` into one function */
        /* so that multiple solutions to the same decomposition query could be evaluated */
        int gain = acd_max_local.multi_run( _mc_analyzer, ownerships );
        if ( gain < 0 )
        {
          // fmt::print( "[m] failed paritioning signals\n" );
          continue;
        }
        ++_st.num_decomp_success;

        /* measure the cost of `acd_max_local._decomp_res` */
        /* calculate the gain of current solution by substracting `mffc_size` from the cost */
        /* update `gain_best` and other records if necessary */
        // fmt::print( "[m] Removes nodes: {}; Added nodes: {}\n", mffc_size, gain );
        gain = static_cast<int32_t>( mffc_size ) - gain;
        
        if ( gain > gain_best )
        {
          gain_best = gain;
          cut_best_index = cut_index;
          decomp_res_best = acd_max_local._decomp_res;
        }
        // measure_mffc_ref( n, cut );

        /* is this line necessary? */
        if ( cut->size() == 0 || ( cut->size() == 1u && *cut->begin() != _ntk.node_to_index( n ) ) )
        {
          break;
        }

        /* TODO: the cost model currently adopted is a too-loose upper bound */
        /* when there are more than one potential FS set */
      }

      if ( cut_best_index != 0u )
      {
        fmt::print( "[m] Network rewriting is happening\n" );
        auto best_cut = cuts.cuts( _ntk.node_to_index( n ) )[cut_best_index - 1];
        kitty::dynamic_truth_table tt( best_cut.size() );
        for ( auto i{ 0u }; i < tt.num_blocks(); ++i )
        {
          tt._bits[i] = cuts.truth_table( best_cut )._bits[i];
        }
        std::vector<mpc_network::signal> children( best_cut.size() ); 
        std::vector<mpc_network::node> leaves( best_cut.size() );
        auto ctr = 0u;
        for ( auto leaf : best_cut )
        {
          leaves[ctr] = _ntk.index_to_node( leaf );
          children[ctr] = _ntk.make_signal( leaves[ctr] );
          ++ctr;
        }

        mpc_network::signal n_new_impl = _mc_analyzer.min_mc_resyn_mpc( _ntk, children, decomp_res_best );
        detail::recursive_deref<mpc_network, decltype( leaves.begin() ), mc_mpc_cost>( _ntk, n, leaves.begin(), leaves.end() );
        detail::recursive_ref<mpc_network, decltype( leaves.begin() ), mc_mpc_cost>( _ntk, _ntk.get_node( n_new_impl ), leaves.begin(), leaves.end() );
        _ntk.substitute_node_no_restrash( n, n_new_impl );
        clear_cuts_fanout_rec( cuts, cut_manager, _ntk.get_node( n_new_impl ) );
        ++_st.num_better_impl;
        rewriten = true;
        return false;
      }

      return true;
    } );

    return rewriten;
  }

private:
  void register_events()
  {
    const auto update_ownership_of_new_node = [&]( node const& n ) {
      _ntk.resize_ownerships();
      _ntk.propogate_ownership_local( n );
    };

    const auto update_ownership_of_existing_node = [&]( node const& n, auto const& old_children ) {
      ( void )old_children;
      _ntk.resize_ownerships();
      _ntk.propogate_ownership_local( n );
    };

    const auto update_ownership_of_deleted_node = [&]( node const& n ) {
      _ntk.assign_ownership_local( n, UINT32_MAX );
    };

    add_event = _ntk.events().register_add_event( update_ownership_of_new_node );
    modified_event = _ntk.events().register_modified_event( update_ownership_of_existing_node );
    delete_event = _ntk.events().register_delete_event( update_ownership_of_deleted_node );
  }

  int32_t measure_mffc_ref( node const& n, cut_t const* cut )
  {
    for ( auto leaf : *cut )
    {
      _ntk.incr_fanout_size( _ntk.index_to_node( leaf ) );
    }

    int32_t mffc_size = static_cast<int32_t>( recursive_ref( n ) );

    for ( auto leaf : *cut )
    {
      _ntk.decr_fanout_size( _ntk.index_to_node( n ) );
    }

    return mffc_size;
  }

  int32_t measure_mffc_deref( node const& n, cut_t const* cut )
  {
    /* reference cut leaves */
    for ( auto leaf : *cut )
    {
      _ntk.incr_fanout_size( _ntk.index_to_node( leaf ) );
    }

    int32_t mffc_size = static_cast<int32_t>( recursive_deref( n ) );

    /* dereference leaves */
    for ( auto leaf : *cut )
    {
      _ntk.decr_fanout_size( _ntk.index_to_node( leaf ) );
    }

    return mffc_size;
  }

  uint32_t recursive_deref( node const& n )
  {
    if ( _ntk.is_constant( n ) || _ntk.is_pi( n ) )
      return 0u;

    uint32_t value{ mc_mpc_cost{}( _ntk, n ) };
    _ntk.foreach_fanin( n, [&]( auto const& fi ) {
      if ( _ntk.decr_fanout_size( _ntk.get_node( fi ) ) == 0 )
      {
        value += recursive_deref( _ntk.get_node( fi ) );
      }
    } );
    return value;
  }

  uint32_t recursive_ref( node const& n )
  {
    if ( _ntk.is_constant( n ) || _ntk.is_pi( n ) )
      return 0u;

    uint32_t value{ mc_mpc_cost{}( _ntk, n ) };
    _ntk.foreach_fanin( n, [&]( auto const& fi ) {
      if ( _ntk.incr_fanout_size( _ntk.get_node( fi ) ) == 0 )
      {
        value += recursive_ref( _ntk.get_node( fi ) );
      }
    } );
    return value;
  }

  void clear_cuts_fanout_rec( network_cuts_t& cuts, cut_manager_t& cut_manager, mpc_network::node const& n )
  {
    fanout_view<mpc_network> ntk_mpc_fanout( _ntk );
    ntk_mpc_fanout.foreach_fanout( n, [&]( auto const& no ) {
      const auto index = _ntk.node_to_index( no );
      if ( cuts.cuts( index ).size() > 0 )
      {
        cut_manager.clear_cuts( no );
        clear_cuts_fanout_rec( cuts, cut_manager, no);
      }
    } );
  }

private:
  uint32_t _num_parties;
  mpc_network& _ntk;
  mc_analyzer _mc_analyzer;
  xag_decomp_max_local_params const& _ps;
  xag_decomp_max_local_stats& _st;

  /* events */
  std::shared_ptr<typename network_events<mpc_network>::add_event_type> add_event;
  std::shared_ptr<typename network_events<mpc_network>::modified_event_type> modified_event;
  std::shared_ptr<typename network_events<mpc_network>::delete_event_type> delete_event;

};

} /* namespace mockturtle */