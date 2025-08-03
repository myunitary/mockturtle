#pragma once

#include "../networks/xag.hpp"
#include "../views/cut_view.hpp"
#include "../views/ownership_view.hpp"
#include "cleanup.hpp"
#include "simulation.hpp"

#include <fmt/format.h>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/print.hpp>
#include <kitty/spectral.hpp>
#include <kitty/static_truth_table.hpp>

#include <fstream>

namespace mockturtle
{

// struct mc_analyzer_params
// {
//   bool verbose{ false };
// };

struct decomp_result
{
	kitty::dynamic_truth_table tt;
	std::vector<uint32_t> support;
};

struct masked_encoding
{
  uint8_t value;
  uint8_t mask; /* 1 : care; 0 : don't care */
};

class mc_analyzer
{
public:
  explicit mc_analyzer( std::string const& filename )
    : _db(),
      _pis( 6u ),
      _func_mc(),
      _classify_cache()
  {
    build_db( filename );
  }

  uint8_t get_mc( kitty::dynamic_truth_table const& tt )
  {
    uint8_t num_vars = tt.num_vars();
    if ( num_vars > 6u )
    {
      fmt::print( "[e] Unable to figure out the MC of functions with more than six variables, returning 0...\n" );
      return 0u;
    }

    const auto tt_ext = kitty::extend_to<6u>( tt );
    std::vector<kitty::detail::spectral_operation> trans;
    kitty::static_truth_table<6u> tt_ext_repr;

    const auto it_cache = _classify_cache.find( tt_ext );
    if ( it_cache != _classify_cache.end() )
    {
      if ( !std::get<0>( it_cache->second ) )
      {
        fmt::print( "[e] Unable to figure out the MC of this function, returning 0...\n" );
        return 0u;
      }

      tt_ext_repr = std::get<1>( it_cache->second );
      trans = std::get<2>( it_cache->second );
    }
    else
    {
      const auto spectral = kitty::exact_spectral_canonization_limit( tt_ext, 100000, [&trans]( auto const& ops ) {
        std::copy( ops.begin(), ops.end(), std::back_inserter( trans ) );
      } );
      _classify_cache.insert( { tt_ext, { spectral.second, spectral.first, trans } } );

      if ( !spectral.second )
      {
        fmt::print( "[e] Unknown representative function, returning 0...\n" );
        return 0u;
      }

      tt_ext_repr = spectral.first;
    }
    
    const auto it = _func_mc.find( kitty::to_hex( tt_ext_repr ) );
    if ( it != _func_mc.end() )
    {
      return std::get<1>( it->second );
    }

    fmt::print( "[e] Unknown representative function, returning 0...\n" );
    return 0u;
  }

  int32_t estimate_decomp_res_cost( std::vector<decomp_result> const& decomp_bs, uint32_t num_vars, uint32_t num_fs, std::vector<std::vector<uint32_t>> const& ownerships, uint32_t ind_free_party, uint32_t mu, bool& min_mc_mux )
  {
    int32_t cost{ 0 };
    /* construct a toy XAG for a precise cost estimation */
    xag_network ntk;
    std::vector<xag_network::signal> pis( num_vars );
    std::generate( pis.begin(), pis.end(), [&ntk]() { return ntk.create_pi(); } );

    /* reorder `ownerships` into `pi_ownerships` */
    std::vector<uint32_t> pis_ownership_per_pi( num_vars );
    for ( auto i{ 1u }; i < ownerships.size(); ++i )
    {
      for ( auto element : ownerships[i] )
      {
        pis_ownership_per_pi[element] = ( 1 << ( i - 1 ) );
      }
    }
    for ( auto element : ownerships[0] )
    {
      /* TODO : any better solution? */
      pis_ownership_per_pi[element] = 3u;
    }

    /* the constructed XAG only implements the BS functions */
    uint32_t num_funcs = ( decomp_bs.back().support.size() > 6u ) ? ( decomp_bs.size() - 1 ) : decomp_bs.size();
    if ( num_funcs == decomp_bs.size() )
    {
      pis.resize( num_vars + num_fs );
      std::generate( pis.begin() + num_vars, pis.end(), [&ntk]() { return ntk.create_pi(); } );
      pis_ownership_per_pi.resize( num_vars + num_fs );
      for ( auto i{ 0u }; i < num_fs; ++i )
      {
        pis_ownership_per_pi[i + num_vars] = ( 1 << ( ind_free_party - 1 ) );
      }
      min_mc_mux = true;
    }
    else
    {
      min_mc_mux = false;
    }
    std::vector<xag_network::signal> signals = pis;
    for ( uint32_t i{ 0u }; i < num_funcs; ++i )
    {
      const kitty::dynamic_truth_table tt = decomp_bs[i].tt;
      const std::vector<uint32_t> support = decomp_bs[i].support;
      const auto tt_ext = kitty::extend_to<6u>( tt );

      std::vector<xag_network::signal> pis_part( 6u, ntk.get_constant( false ) );
      for ( uint32_t j{ 0u }; j < support.size(); ++j )
      {
        pis_part[j] = signals[support[j]];
      }

      std::vector<kitty::detail::spectral_operation> trans;
      kitty::static_truth_table<6u> tt_ext_repr;

      const auto it_cache = _classify_cache.find( tt_ext );
      if ( it_cache != _classify_cache.end() )
      {
        if ( !std::get<0>( it_cache->second ) )
        {
          // fmt::print( "[e] Unable to figure out the MC of this function...\n" );
          return -1;
        }

        tt_ext_repr = std::get<1>( it_cache->second );
        trans = std::get<2>( it_cache->second );
      }
      else
      {
        const auto spectral = kitty::exact_spectral_canonization_limit( tt_ext, 100000, [&trans]( auto const& ops ) {
          std::copy( ops.begin(), ops.end(), std::back_inserter( trans ) );
        } );
        _classify_cache.insert( { tt_ext, { spectral.second, spectral.first, trans } } );

        if ( !spectral.second )
        {
          // fmt::print( "[e] Unknown representative function...\n" );
          return -1;
        }

        tt_ext_repr = spectral.first;
      }

      xag_network::signal sig = _db.get_constant( false );
      const auto it_search = _func_mc.find( kitty::to_hex( tt_ext_repr ) );
      if ( it_search != _func_mc.end() )
      {
        uint8_t mc = 0u;
        std::string db_repr_str = "";
        std::tie( db_repr_str, mc, sig ) = it_search->second;
        kitty::static_truth_table<6u> db_repr;
        kitty::create_from_hex_string( db_repr, db_repr_str );
        kitty::exact_spectral_canonization( db_repr, [&trans]( auto const& ops ) {
          std::copy( ops.rbegin(), ops.rend(), std::back_inserter( trans ) );
        } );
      }
      else if ( !kitty::is_const0( tt_ext_repr ) )
      {
        // fmt::print( "[e] Unknown representative function...\n" );
        return -1;
      }

      bool out_neg = false;
      std::vector<xag_network::signal> final_xors;
      for ( auto const& each_trans : trans )
      {
        switch ( each_trans._kind )
        {
        case kitty::detail::spectral_operation::kind::permutation:
        {
          const auto v1 = log2( each_trans._var1 );
          const auto v2 = log2( each_trans._var2 );
          std::swap( pis_part[v1], pis_part[v2] );
          break;
        }
        case kitty::detail::spectral_operation::kind::input_negation:
        {
          const auto v1 = log2( each_trans._var1 );
          pis_part[v1] = !pis_part[v1];
          break;
        }
        case kitty::detail::spectral_operation::kind::output_negation:
        {
          out_neg = !out_neg;
          break;
        }
        case kitty::detail::spectral_operation::kind::spectral_translation:
        {
          const auto v1 = log2( each_trans._var1 );
          const auto v2 = log2( each_trans._var2 );
          pis_part[v1] = ntk.create_xor( pis_part[v1], pis_part[v2] );
          break;
        }
        case kitty::detail::spectral_operation::kind::disjoint_translation:
        {
          const auto v1 = log2( each_trans._var1 );
          final_xors.push_back( pis_part[v1] );
          break;
        }
        default:
          break;
        }
      }

      xag_network::signal output;
      if ( _db.is_constant( _db.get_node( sig ) ) )
      {
        output = ntk.get_constant( _db.is_complemented( sig ) );
      }
      else
      {
        cut_view<xag_network> topo{ _db, _pis, sig };
        output = cleanup_dangling( topo, ntk, pis_part.begin(), pis_part.end() ).front();
      }
      for ( auto const& each_xor : final_xors )
      {
        output = ntk.create_xor( output, each_xor );
      }
      output = out_neg ? ntk.create_not( output ) : output;
      ntk.create_po( output );
      signals.push_back( output );
    }

    /* It's imprecise to pass constant 2 as the number of parties */
    /* but the correct number of parties functionally does not matter in this case */
    assert( ntk.num_pis() == pis_ownership_per_pi.size() );
    owner_view<xag_network> ntk_mpc( ntk, 2u, pis_ownership_per_pi, true );

    ntk_mpc.foreach_node( [&ntk_mpc, &cost]( auto const& n ) {
      if ( ( ntk_mpc.is_local( n ) == 0u ) && ntk_mpc.is_and( n ) )
      {
        ++cost;
      }
    } );

    if ( num_funcs < decomp_bs.size() )
    {
      cost += static_cast<int32_t>( mu );
    }

    return cost;
    /* pass ownership to realize rather precise cost estimation */
  }

  int32_t estimate_decomp_res_cost_new( std::vector<decomp_result> const& decomp_bs, uint32_t num_vars, uint32_t num_fs, std::vector<uint32_t> pis_ownership_per_pi, uint32_t ind_free_party, uint32_t mu, bool& min_mc_mux )
  {
    fmt::print( "[m] In cost estimation...\n" );
    bool has_solution{ false };
    int32_t cost{ 0 };
    int32_t cost_backup{ 0 };
    /* construct a toy XAG for a precise cost estimation */
    xag_network ntk;
    std::vector<xag_network::signal> pis( num_vars + num_fs );
    std::generate( pis.begin(), pis.end(), [&ntk]() { return ntk.create_pi(); } );
    assert( pis_ownership_per_pi.size() == num_vars );
    pis_ownership_per_pi.resize( num_vars + num_fs );
    for ( auto i{ 0u }; i < num_fs; ++i )
    {
      pis_ownership_per_pi[i + num_vars] = ( 1 << ( ind_free_party - 1 ) );
    }
    // fmt::print( "[m] Derived per-pi ownership: [ " );
    // for ( auto const& element : pis_ownership_per_pi )
    // {
    //   fmt::print( "{}, ", element );
    // }
    // fmt::print( "]\n" );

    std::vector<xag_network::signal> signals = pis;

    /* the constructed XAG only implements the BS functions */
    min_mc_mux = ( decomp_bs.back().support.size() > 6u ) ? false : true;
    if ( !min_mc_mux )
    {
      fmt::print( "[m] Only consider heuristic top MUX construction\n" );
    }
    else
    {
      fmt::print( "[m] Consider both exact and heuristic top MUX construction\n" );
    }

    /* TODO : did not address the case where all BS functions are trivial */
    for ( uint32_t i{ 0u }; i < decomp_bs.size(); ++i )
    {
      if ( i == ( decomp_bs.size() - 1 ) )
      {
        assert( ntk.num_pis() == pis_ownership_per_pi.size() );
        if ( decomp_bs.size() == 1u )
        {
          /* if all BS functions are trivial */
          fmt::print( "[m] cost of heuristic construction: {}(0 + {})\n", ( mu - 1 ) , ( mu - 1 ) );
          cost = static_cast<int32_t>( mu - 1 );
        }
        else
        {
          owner_view<xag_network> ntk_mpc( ntk, 2u, pis_ownership_per_pi, true );
          ntk_mpc.foreach_node( [&ntk_mpc, &cost]( auto const& n ) {
            if ( ( ntk_mpc.is_local( n ) == 0u ) && ntk_mpc.is_and( n ) )
            {
              ++cost;
            }
          } );
          fmt::print( "[m] cost of heuristic construction: {}({} + {})\n", ( cost + static_cast<int32_t>( mu - 1 ) ), cost, static_cast<int32_t>( mu - 1 ) );
          cost += static_cast<int32_t>( mu - 1 );
        }
        
        if ( !min_mc_mux )
        {
          return cost;
        }
        else
        {
          /* serve a guarantee if exact contruction fails */
          has_solution = true;
        }
        
        cost_backup = cost;
        cost = 0u;
      }

      const kitty::dynamic_truth_table tt = decomp_bs[i].tt;
      const std::vector<uint32_t> support = decomp_bs[i].support;
      const auto tt_ext = kitty::extend_to<6u>( tt );

      if ( i < decomp_bs.size() - 1 )
      {
        fmt::print( "[m] The {}-th BS function's supports are: [ ", ( i + 1 ) );
        for ( auto const& element : support )
        {
          fmt::print( "{} ", element );
        }
        // fmt::print( "], function is " );
        fmt::print( "]\n" );
        // kitty::print_hex( tt );
        // fmt::print( "\n" );
      }
      else
      {
        fmt::print( "[m] The top function's supports are: [ ", ( i + 1 ) );
        for ( auto const& element : support )
        {
          fmt::print( "{} ", element );
        }
        // fmt::print( "], function is " );
        fmt::print( "]\n" );
        // kitty::print_hex( tt );
        // fmt::print( "\n" );
      }

      std::vector<xag_network::signal> pis_part( 6u, ntk.get_constant( false ) );
      for ( uint32_t j{ 0u }; j < support.size(); ++j )
      {
        pis_part[j] = signals[support[j]];
      }

      std::vector<kitty::detail::spectral_operation> trans;
      kitty::static_truth_table<6u> tt_ext_repr;

      const auto it_cache = _classify_cache.find( tt_ext );
      if ( it_cache != _classify_cache.end() )
      {
        if ( !std::get<0>( it_cache->second ) )
        {
          // fmt::print( "[e] Unable to figure out the MC of this function...\n" );
          if ( has_solution )
          {
            min_mc_mux = false;
            return cost_backup;
          }
          return -1;
        }

        tt_ext_repr = std::get<1>( it_cache->second );
        trans = std::get<2>( it_cache->second );
      }
      else
      {
        const auto spectral = kitty::exact_spectral_canonization_limit( tt_ext, 100000, [&trans]( auto const& ops ) {
          std::copy( ops.begin(), ops.end(), std::back_inserter( trans ) );
        } );
        _classify_cache.insert( { tt_ext, { spectral.second, spectral.first, trans } } );

        if ( !spectral.second )
        {
          // fmt::print( "[e] Unknown representative function...\n" );
          if ( has_solution )
          {
            min_mc_mux = false;
            return cost_backup;
          }
          return -1;
        }

        tt_ext_repr = spectral.first;
      }

      xag_network::signal sig = _db.get_constant( false );
      const auto it_search = _func_mc.find( kitty::to_hex( tt_ext_repr ) );
      if ( it_search != _func_mc.end() )
      {
        uint8_t mc = 0u;
        std::string db_repr_str = "";
        std::tie( db_repr_str, mc, sig ) = it_search->second;
        kitty::static_truth_table<6u> db_repr;
        kitty::create_from_hex_string( db_repr, db_repr_str );
        kitty::exact_spectral_canonization( db_repr, [&trans]( auto const& ops ) {
          std::copy( ops.rbegin(), ops.rend(), std::back_inserter( trans ) );
        } );
      }
      else if ( !kitty::is_const0( tt_ext_repr ) )
      {
        // fmt::print( "[e] Unknown representative function...\n" );
        if ( has_solution )
        {
          min_mc_mux = false;
          return cost_backup;
        }
        return -1;
      }

      bool out_neg = false;
      std::vector<xag_network::signal> final_xors;
      for ( auto const& each_trans : trans )
      {
        switch ( each_trans._kind )
        {
        case kitty::detail::spectral_operation::kind::permutation:
        {
          const auto v1 = log2( each_trans._var1 );
          const auto v2 = log2( each_trans._var2 );
          std::swap( pis_part[v1], pis_part[v2] );
          break;
        }
        case kitty::detail::spectral_operation::kind::input_negation:
        {
          const auto v1 = log2( each_trans._var1 );
          pis_part[v1] = !pis_part[v1];
          break;
        }
        case kitty::detail::spectral_operation::kind::output_negation:
        {
          out_neg = !out_neg;
          break;
        }
        case kitty::detail::spectral_operation::kind::spectral_translation:
        {
          const auto v1 = log2( each_trans._var1 );
          const auto v2 = log2( each_trans._var2 );
          pis_part[v1] = ntk.create_xor( pis_part[v1], pis_part[v2] );
          break;
        }
        case kitty::detail::spectral_operation::kind::disjoint_translation:
        {
          const auto v1 = log2( each_trans._var1 );
          final_xors.push_back( pis_part[v1] );
          break;
        }
        default:
          break;
        }
      }

      xag_network::signal output;
      if ( _db.is_constant( _db.get_node( sig ) ) )
      {
        output = ntk.get_constant( _db.is_complemented( sig ) );
      }
      else
      {
        cut_view<xag_network> topo{ _db, _pis, sig };
        output = cleanup_dangling( topo, ntk, pis_part.begin(), pis_part.end() ).front();
      }
      for ( auto const& each_xor : final_xors )
      {
        output = ntk.create_xor( output, each_xor );
      }
      output = out_neg ? ntk.create_not( output ) : output;
      ntk.create_po( output );
      signals.push_back( output );
    }

    owner_view<xag_network> ntk_mpc( ntk, 2u, pis_ownership_per_pi, true );
    ntk_mpc.foreach_node( [&ntk_mpc, &cost]( auto const& n ) {
      if ( ( ntk_mpc.is_local( n ) == 0u ) && ntk_mpc.is_and( n ) )
      {
        ++cost;
      }
    } );
    fmt::print( "[m] cost of exact construction: {}\n", cost );

    assert( min_mc_mux );
    if ( cost > cost_backup )
    {
      min_mc_mux = false;
      cost = cost_backup;
    }
    return cost;
  }

  inline std::vector<uint8_t> expand_encoding( masked_encoding const& enc, uint8_t sel_bitwidth )
  {
    std::vector<uint8_t> result;
    
    std::vector<uint8_t> dc_pos;
    for ( auto i{ 0u }; i < sel_bitwidth; ++i )
    {
      if ( ( ( enc.mask >> i ) & 1 ) == 0 )
      {
        dc_pos.push_back( i );
      }
    }
    
    const uint8_t num_dc = dc_pos.size();
    const uint8_t total = 1 << num_dc;

    for ( auto combo{ 0u }; combo < total; ++combo )
    {
      uint8_t concrete = enc.value;
      for ( auto j{ 0u }; j < num_dc; ++j )
      {
        int bit_pos = dc_pos[j];
        int bit_val = ( combo >> j ) & 1;
        if ( bit_val )
        {
          concrete |= ( 1 << bit_pos );
        }
        else
        {
          concrete &= ~( 1 << bit_pos );
        }
      }
      
      result.push_back( concrete );
    }

    return result;
  }

  xag_network::signal min_mc_resyn( xag_network& res, std::vector<xag_network::signal> const& leaves, std::array<std::vector<decomp_result>, 2u> const& decomp_fs_bs, std::vector<masked_encoding> const& encodings, uint8_t sel_polar, bool min_mc_mux )
  {
    /* for debugging */
    // fmt::print( "[m] (Second check) Leaves of the current cut are [ " );
    // for ( auto const& each_leaf : leaves )
    // {
    //   fmt::print( "{}Node{} ", ( res.is_complemented( each_leaf ) ? "~" : "" ), ( res.node_to_index( res.get_node( each_leaf ) ) ) );
    // }
    // fmt::print( "]\n" );
    // fmt::print( "[m] Current solution consists of {} FS functions, {} BS functions, and the top function.\n", decomp_fs_bs[0].size(), ( decomp_fs_bs[1].size() - 1 ) );

    auto signals = leaves;
    std::vector<decomp_result> decomp_res = decomp_fs_bs[0];
    decomp_res.insert( decomp_res.end(), decomp_fs_bs[1].begin(), decomp_fs_bs[1].end() );
    for ( uint32_t i = 0u; i < decomp_res.size(); ++i )
    {
      /* for debugging */
      // if ( i < decomp_fs_bs[0].size() )
      // {
      //   fmt::print( "[m] Concerning FS function " );
      // }
      // else if ( i != ( decomp_res.size() - 1 ) )
      // {
      //   fmt::print( "[m] Concerning BS function " );
      // } 
      // else
      // {
      //   fmt::print( "[m] Concerning top function " );
      // }
      if ( ( i == ( decomp_res.size() - 1 ) ) && !min_mc_mux )
      // if ( i == ( decomp_res.size() - 1 ) )
      {
        /* resort to heuristic MUX network construction */
        const std::vector<uint32_t> support = decomp_res[i].support;
        std::vector<std::pair<uint8_t, xag_network::signal>> ds( decomp_fs_bs[0].size() );
        if ( ds.size() != encodings.size() )
        {
          fmt::print( "[m] Size of `support` is {}; Size of `ds` is {}; Size of `encodings` is {}\n", support.size(), ds.size(), encodings.size() );
        }
        assert( ds.size() == encodings.size() );

        std::vector<xag_network::signal> ss( support.size() - ds.size() );
        for ( auto j{ 0u }; j < ss.size(); ++j )
        {
          assert( j + ds.size() < support.size() );
          ss[j] = signals[support[j + ds.size()]];
        }

        ds.clear();
        const uint8_t sel_bitwidth = ss.size();
        for ( auto j{ 0u }; j < encodings.size(); ++j )
        {
          if ( encodings[j].mask == ( ( 1 << sel_bitwidth ) - 1 ) )
          {
            ds.emplace_back( std::make_pair( encodings[j].value, signals[support[j]] ) );
          }
          else
          {
            std::vector<uint8_t> concrete_encodings = expand_encoding( encodings[j], sel_bitwidth );
            for ( auto const& enc : concrete_encodings )
            {
              ds.emplace_back( std::make_pair( enc, signals[support[j]] ) );
            }
          }
        }
        assert( ds.size() <= ( 1 << sel_bitwidth ) );
        std::sort( ds.begin(), ds.end(), []( auto const& a, auto const& b ) { return a.first < b.first; } );

        return construct_mux_tree( res, ds, ss, sel_polar );
      }

      if ( i == ( decomp_res.size() - 1u ) )
      {
        assert( decomp_res[i].support.size() <= 6u );
      }
      
      const kitty::dynamic_truth_table tt = decomp_res[i].tt;
      const std::vector<uint32_t> support = decomp_res[i].support;
      const auto tt_ext = kitty::extend_to<6u>( tt );

      // kitty::print_hex( tt_ext );
      // fmt::print( "\nLeaves: [ " );

      std::vector<xag_network::signal> pis( 6u, res.get_constant( false ) );
      for ( uint32_t j = 0; j < support.size(); ++j )
      {
        pis[j] = signals[support[j]];

        // fmt::print( "{}Node{} ", ( res.is_complemented( pis[j] ) ? "~" : "" ), ( res.node_to_index( res.get_node( pis[j] ) ) ) );
      }

      // fmt::print( "]\n" );
      // const auto pis_backup = pis; /* for evaluting the functionality of the newly generated implementation */

      std::vector<kitty::detail::spectral_operation> trans;
      kitty::static_truth_table<6u> tt_ext_repr;
      
      const auto it_cache = _classify_cache.find( tt_ext );
      if ( it_cache != _classify_cache.end() )
      {
        if ( !std::get<0>( it_cache->second ) )
        {
          fmt::print( "[e] Unable to figure out the MC of this function, returning 0...\n" );
          return res.get_constant( false );
        }
        
        tt_ext_repr = std::get<1>( it_cache->second );
        trans = std::get<2>( it_cache->second );
      }
      else
      {
        const auto spectral = kitty::exact_spectral_canonization_limit( tt_ext, 100000, [&trans]( auto const& ops ) {
          std::copy( ops.begin(), ops.end(), std::back_inserter( trans ) );
        } );
        _classify_cache.insert( { tt_ext, { spectral.second, spectral.first, trans } } );
        
        if ( !spectral.second )
        {
          fmt::print( "[e] Unknown representative function, returning 0...\n" );
          return res.get_constant( false );
        }
        
        tt_ext_repr = spectral.first;

        /* for debugging */
        // fmt::print( "[m] Classify original function " );
        // kitty::print_hex( tt_ext );
        // fmt::print( " into " );
        // kitty::print_hex( tt_ext_repr );
        // fmt::print( "\n" );
        // fmt::print( "[m] Testing the validity of `trans`: [" );
        // for ( auto const& ele : trans )
        // {
        //   fmt::print( "{} ", ele._kind );
        // }
        // fmt::print( "]\n" );
      }

      // fmt::print( "Representative function: " );
      // kitty::print_hex( tt_ext_repr );
      // fmt::print( "\n" );

      xag_network::signal sig = _db.get_constant( false );
      const auto it_search = _func_mc.find( kitty::to_hex( tt_ext_repr ) );
      if ( it_search != _func_mc.end() )
      {
        uint8_t mc = 0u;
        std::string db_repr_str = "";
        std::tie( db_repr_str, mc, sig ) = it_search->second;
        kitty::static_truth_table<6u> db_repr;
        kitty::create_from_hex_string( db_repr, db_repr_str );
        kitty::exact_spectral_canonization( db_repr, [&trans]( auto const& ops ) {
          std::copy( ops.rbegin(), ops.rend(), std::back_inserter( trans ) );
        } );
      }
      else if ( !kitty::is_const0( tt_ext_repr ) )
      {
        fmt::print( "[e] Unknown representative function, returning 0...\n" );
        return res.get_constant( false );
      }

      /* for debugging */
      // fmt::print( "[m] The issue is not related to the interaction with `_func_mc`\n" );

      bool out_neg = false;
      std::vector<xag_network::signal> final_xors;
      for ( auto const& each_trans : trans )
      {
        switch ( each_trans._kind )
        {
        case kitty::detail::spectral_operation::kind::permutation:
        {
          const auto v1 = log2( each_trans._var1 );
          const auto v2 = log2( each_trans._var2 );
          std::swap( pis[v1], pis[v2] );

          /* for debugging */
          // fmt::print( "[m] swap pis[{}] and pis[{}]\n", v1, v2 );

          break;
        }
        case kitty::detail::spectral_operation::kind::input_negation:
        {
          const auto v1 = log2( each_trans._var1 );
          pis[v1] = !pis[v1];

          /* for debugging */
          // fmt::print( "[m] negate pis[{}]\n", v1 );

          break;
        }
        case kitty::detail::spectral_operation::kind::output_negation:
        {
          out_neg = !out_neg;

          /* for debugging */
          // fmt::print( "[m] negate output\n" );

          break;
        }
        case kitty::detail::spectral_operation::kind::spectral_translation:
        {
          const auto v1 = log2( each_trans._var1 );
          const auto v2 = log2( each_trans._var2 );
          pis[v1] = res.create_xor( pis[v1], pis[v2] );

          /* for debugging */
          // fmt::print( "[m] pis[{}] becomes XOR( pis[{}], pis[{}] )\n", v1, v1, v2 );

          break;
        }
        case kitty::detail::spectral_operation::kind::disjoint_translation:
        {
          const auto v1 = log2( each_trans._var1 );
          final_xors.push_back( pis[v1] );

          /* for debugging */
          // fmt::print( "[m] output is XORed with pis[{}]\n", v1 );

          break;
        }
        default:
          break;
        }
      }

      xag_network::signal output;
      if ( _db.is_constant( _db.get_node( sig ) ) )
      {
        output = res.get_constant( _db.is_complemented( sig ) );
      }
      else
      {
        cut_view<xag_network> topo{ _db, _pis, sig };
        output = cleanup_dangling( topo, res, pis.begin(), pis.end() ).front();
      }
      for ( auto const& each_xor : final_xors )
      {
        output = res.create_xor( output, each_xor );
      }
      output = out_neg ? res.create_not( output ) : output;

      // fmt::print( "Permuted leaves: [ " );
      // for ( auto j = 0; j < pis.size(); ++j )
      // {
      //   fmt::print( "{}Node{} ", ( res.is_complemented( pis[j] ) ? "~" : "" ), ( res.node_to_index( res.get_node( pis[j] ) ) ) );
      // }
      // fmt::print( "]\n" );
      /* for debugging */
      /* verify the functionality of the resynthesized circuit */
      // cut_view<xag_network> new_impl{ res, pis_backup, output };
      // kitty::static_truth_table<6u> tt_new_impl = simulate<kitty::static_truth_table<6u>>( new_impl )[0];
      // for ( auto i = 0u; i < pis_backup.size(); ++i )
      // {
      //   if ( res.is_complemented( pis_backup[i] ) )
      //   {
      //     kitty::flip_inplace( tt_new_impl, i );
      //   }
      // }
      // if ( tt_new_impl != tt_ext )
      // {
      //   fmt::print( "[e] The circuit synthesized for the {}th BS function is incorrect!\n", ( i + 1 ) );
      //   std::cout << "[e] `tt_ext` = ";
      //   kitty::print_hex( tt_ext );
      //   fmt::print( "(support size = {})", tt.num_vars() );
      //   std::cout << ", `tt_new_impl` = ";
      //   kitty::print_hex( tt_new_impl );
      //   uint32_t support_size = 0u;
      //   for ( auto const& each_pi : pis )
      //   {
      //     if ( !res.is_constant( res.get_node( each_pi ) ) )
      //     {
      //       ++support_size;
      //     }
      //   }
      //   fmt::print( "(support size = {})\n", support_size );
      //   abort();
      // }

      if ( i != decomp_res.size() - 1u )
      {
        // if ( !res.is_constant( res.get_node( output ) ) )
        {
          signals.push_back( output );
          // fmt::print( "[m] update: signals[{}] is {}node {}\n", signals.size(), ( res.is_complemented( output ) ? "~" : "" ), ( res.node_to_index( res.get_node( output ) ) ) );
        }

        /* for debugging */
        // fmt::print( "[m] The {}th BS function: ", ( i + 1 ) );
        // kitty::print_hex( tt_ext );
        // fmt::print( "\n" );
        // fmt::print( "[m] output is {}n{}\n", ( res.is_complemented( output ) ? "~" : "" ), res.node_to_index( res.get_node( output ) ) );
      }
      else
      {
        // fmt::print( "[m] The top function: " );
        // kitty::print_hex( tt_ext );
        // fmt::print( "\n" );
        // fmt::print( "[m] output is {}n{}\n", ( res.is_complemented( output ) ? "~" : "" ), res.node_to_index( res.get_node( output ) ) );
        return output;
      }
    }
  }

  using mpc_network = owner_view<xag_network>;

  mpc_network::signal min_mc_resyn_mpc( mpc_network& res, std::vector<mpc_network::signal> const& leaves, std::array<std::vector<decomp_result>, 2u> const& decomp_fs_bs, std::vector<masked_encoding> const& encodings, uint8_t sel_polar, bool min_mc_mux )
  {
    auto signals = leaves;
    // fmt::print( "[m] {} FS functions, {} BS funcitons, and 1 top function\n", decomp_fs_bs[0].size(), ( decomp_fs_bs[1].size() - 1 ) );
    std::vector<decomp_result> decomp_res = decomp_fs_bs[0];
    decomp_res.insert( decomp_res.end(), decomp_fs_bs[1].begin(), decomp_fs_bs[1].end() );
    for ( uint32_t i = 0u; i < decomp_res.size(); ++i )
    {
      /* for debugging */
      // std::string func_name = "";
      // if ( i < decomp_fs_bs[0].size() )
      // {
      //   func_name = std::to_string( i + 1 ) + "-th FS";
      // }
      // else if ( i != ( decomp_res.size() - 1 ) )
      // {
      //   func_name = std::to_string( i + 1 - decomp_fs_bs[0].size() ) + "-th BS";
      // }
      // else
      // {
      //   func_name = "top";
      // }
      // fmt::print( "[m] Working on the {} function...\n", func_name );

      if ( i == ( decomp_res.size() - 1 ) && !min_mc_mux )
      // if ( i == ( decomp_res.size() - 1 ) )
      {
        /* resort to heuristic MUX network construction */
        // fmt::print( "[m] resort to heuristic MUX network construction\n" );
        const std::vector<uint32_t> support = decomp_res[i].support;
        std::vector<std::pair<uint8_t, mpc_network::signal>> ds( decomp_fs_bs[0].size() );
        if ( ds.size() != encodings.size() )
        {
          fmt::print( "[m] Size of `support` is {}; Size of `ds` is {}; Size of `encodings` is {}\n", support.size(), ds.size(), encodings.size() );
        }
        assert( ds.size() == encodings.size() );

        // fmt::print( "[m] Selecting signals : " );
        std::vector<mpc_network::signal> ss( support.size() - ds.size() );
        for ( auto j{ 0u }; j < ss.size(); ++j )
        {
          assert( j + ds.size() < support.size() );
          ss[j] = signals[support[j + ds.size()]];
          // fmt::print( "{}n{}; ", ( res.is_complemented( ss[j] ) ? "~" : "" ), res.node_to_index( res.get_node( ss[j] ) ) );
        }
        // fmt::print( "\n" );

        ds.clear();
        const uint8_t sel_bitwidth = ss.size();
        for ( auto j{ 0u }; j < encodings.size(); ++j )
        {
          if ( encodings[j].mask == ( ( 1 << sel_bitwidth ) - 1 ) )
          {
            // fmt::print( "[m] The {}-th FS function appears 1 time\n", ( j + 1 ) );
            ds.emplace_back( std::make_pair( encodings[j].value, signals[support[j]] ) );
          }
          else
          {
            std::vector<uint8_t> concrete_encodings = expand_encoding( encodings[j], sel_bitwidth );
            for ( auto const& enc : concrete_encodings )
            {
              ds.emplace_back( std::make_pair( enc, signals[support[j]] ) );
            }
            // fmt::print( "[m] The {}-th FS function appears {} times\n", ( j + 1 ), concrete_encodings.size() );
          }
        }
        if ( ds.size() > ( 1 << sel_bitwidth ) )
        {
          fmt::print( "[e] {} data to be selected using {} selectors. Something is wrong...\n", ds.size(), sel_bitwidth );
        }
        assert( ds.size() <= ( 1 << sel_bitwidth ) );
        
        // fmt::print( "[m] Encoding assignment (unsorted): " );
        // for ( auto const& d : ds )
        // {
        //   fmt::print( "{}n{} : {:0{}b}; ", ( res.is_complemented( d.second ) ? "~" : "" ), res.node_to_index( res.get_node( d.second ) ), d.first, static_cast<uint32_t>( ceil( log2( static_cast<double>( ds.size() ) ) ) ) );
        // }
        // fmt::print( "\n" );
        std::sort( ds.begin(), ds.end(), []( auto const& a, auto const& b ) { return a.first < b.first; } );
        // fmt::print( "[m] Encoding assignment (sorted): " );
        // for ( auto const& d : ds )
        // {
        //   fmt::print( "{}n{} : {:0{}b}; ", ( res.is_complemented( d.second ) ? "~" : "" ), res.node_to_index( res.get_node( d.second ) ), d.first, static_cast<uint32_t>( ceil( log2( static_cast<double>( ds.size() ) ) ) ) );
        // }
        // fmt::print( "\n" );

        
        // fmt::print( "[m] sel_polar is {:0{}b}\n", sel_polar, ss.size() );

        return construct_mux_tree( res, ds, ss, sel_polar );
        // mpc_network::signal output = construct_mux_tree( res, ds, ss, sel_polar );
        // if ( ds.size() + ss.size() <= 8u )
        // {
        //   kitty::dynamic_truth_table tt_flip = decomp_res[i].tt;
        //   std::vector<mpc_network::node> leaves( ds.size() + ss.size() );
        //   for ( auto j{ 0u }; j < ds.size(); ++j )
        //   {
        //     leaves[j] = res.get_node( ds[j].second );
        //     if ( res.is_complemented( ds[j].second ) )
        //     {
        //       kitty::flip_inplace( tt_flip, j );
        //     }
        //   }
        //   for ( auto j{ 0u }; j < ss.size(); ++j )
        //   {
        //     leaves[j + ds.size()] = res.get_node( ss[j] );
        //     if ( res.is_complemented( ss[j] ) )
        //     {
        //       kitty::flip_inplace( tt_flip, ( j + ds.size() ) );
        //     }
        //   }
        //   fmt::print( "[m] support size of top mux is {}\n", leaves.size() );
        //   cut_view<mpc_network> new_impl{ res, leaves, output };
        //   kitty::static_truth_table<9u> tt_new_impl = simulate<kitty::static_truth_table<9u>>( new_impl )[0];
        //   fmt::print( "[m] For the top MUX, the target truth table is " );
        //   kitty::print_hex( tt_flip );
        //   fmt::print( ", get " );
        //   kitty::print_hex( tt_new_impl );
        //   fmt::print( "\n" );
        // }
        // return output;
      }

      if ( i == ( decomp_res.size() - 1 ) )
      {
        assert( decomp_res[i].support.size() <= 6u );
      }

      const kitty::dynamic_truth_table tt = decomp_res[i].tt;
      // if ( func_name.find( "FS" ) != std::string::npos )
      // {
      //   fmt::print( "[m] FS functions is: " );
      //   kitty::print_hex( tt );
      //   fmt::print( "\n" );
      // }
      const std::vector<uint32_t> support = decomp_res[i].support;
      const auto tt_ext = kitty::extend_to<6u>( tt );

      /* for debugging */
      // fmt::print( "[m] supports are: " );

      std::vector<mpc_network::signal> pis( 6u, res.get_constant( false ) );
      for ( uint32_t j = 0; j < support.size(); ++j )
      {
        pis[j] = signals[support[j]];
        // fmt::print( "node {}; ", res.node_to_index( res.get_node( pis[j] ) ) );
      }
      // fmt::print( "\n" );

      /* for debugging */
      // const auto pis_backup = pis; /* for evaluting the functionality of the newly generated implementation */

      std::vector<kitty::detail::spectral_operation> trans;
      kitty::static_truth_table<6u> tt_ext_repr;
      
      const auto it_cache = _classify_cache.find( tt_ext );
      if ( it_cache != _classify_cache.end() )
      {
        if ( !std::get<0>( it_cache->second ) )
        {
          fmt::print( "[e] Unable to figure out the MC of this function, returning 0...\n" );
          return res.get_constant( false );
        }
        
        tt_ext_repr = std::get<1>( it_cache->second );
        trans = std::get<2>( it_cache->second );
      }
      else
      {
        const auto spectral = kitty::exact_spectral_canonization_limit( tt_ext, 100000, [&trans]( auto const& ops ) {
          std::copy( ops.begin(), ops.end(), std::back_inserter( trans ) );
        } );
        _classify_cache.insert( { tt_ext, { spectral.second, spectral.first, trans } } );
        
        if ( !spectral.second )
        {
          fmt::print( "[e] Unknown representative function, returning 0...\n" );
          return res.get_constant( false );
        }
        
        tt_ext_repr = spectral.first;
      }

      xag_network::signal sig = _db.get_constant( false );
      const auto it_search = _func_mc.find( kitty::to_hex( tt_ext_repr ) );
      if ( it_search != _func_mc.end() )
      {
        uint8_t mc = 0u;
        std::string db_repr_str = "";
        std::tie( db_repr_str, mc, sig ) = it_search->second;
        kitty::static_truth_table<6u> db_repr;
        kitty::create_from_hex_string( db_repr, db_repr_str );
        kitty::exact_spectral_canonization( db_repr, [&trans]( auto const& ops ) {
          std::copy( ops.rbegin(), ops.rend(), std::back_inserter( trans ) );
        } );
      }
      else if ( !kitty::is_const0( tt_ext_repr ) )
      {
        fmt::print( "[e] Unknown representative function, returning 0...\n" );
        return res.get_constant( false ); /* TODO: Handle this case */
      }

      bool out_neg = false;
      std::vector<mpc_network::signal> final_xors;
      for ( auto const& each_trans : trans )
      {
        switch ( each_trans._kind )
        {
        case kitty::detail::spectral_operation::kind::permutation:
        {
          const auto v1 = log2( each_trans._var1 );
          const auto v2 = log2( each_trans._var2 );
          std::swap( pis[v1], pis[v2] );
          break;
        }
        case kitty::detail::spectral_operation::kind::input_negation:
        {
          const auto v1 = log2( each_trans._var1 );
          pis[v1] = !pis[v1];
          break;
        }
        case kitty::detail::spectral_operation::kind::output_negation:
        {
          out_neg = !out_neg;
          break;
        }
        case kitty::detail::spectral_operation::kind::spectral_translation:
        {
          const auto v1 = log2( each_trans._var1 );
          const auto v2 = log2( each_trans._var2 );
          pis[v1] = res.create_xor( pis[v1], pis[v2] );
          break;
        }
        case kitty::detail::spectral_operation::kind::disjoint_translation:
        {
          const auto v1 = log2( each_trans._var1 );
          final_xors.push_back( pis[v1] );
          break;
        }
        default:
          break;
        }
      }

      mpc_network::signal output;
      if ( _db.is_constant( _db.get_node( sig ) ) )
      {
        output = res.get_constant( _db.is_complemented( sig ) );
      }
      else
      {
        cut_view<xag_network> topo{ _db, _pis, sig };
        output = cleanup_dangling( topo, res, pis.begin(), pis.end() ).front();
      }
      for ( auto const& each_xor : final_xors )
      {
        output = res.create_xor( output, each_xor );
      }
      output = out_neg ? res.create_not( output ) : output;

      /* for debugging */
      /* verify the functionality of the resynthesized circuit */
      // cut_view<mpc_network> new_impl{ res, pis_backup, output };
      // kitty::static_truth_table<6u> tt_new_impl = simulate<kitty::static_truth_table<6u>>( new_impl )[0];
      // for ( auto i = 0u; i < pis_backup.size(); ++i )
      // {
      //   if ( res.is_complemented( pis_backup[i] ) )
      //   {
      //     kitty::flip_inplace( tt_new_impl, i );
      //   }
      // }
      // if ( tt_new_impl != tt_ext )
      // {
      //   fmt::print( "[e] The circuit synthesized for the {}th BS function is incorrect!\n", ( i + 1 ) );
      //   std::cout << "[e] `tt_ext` = ";
      //   kitty::print_hex( tt_ext );
      //   fmt::print( "(support size = {})", tt.num_vars() );
      //   std::cout << ", `tt_new_impl` = ";
      //   kitty::print_hex( tt_new_impl );
      //   uint32_t support_size = 0u;
      //   for ( auto const& each_pi : pis )
      //   {
      //     if ( !res.is_constant( res.get_node( each_pi ) ) )
      //     {
      //       ++support_size;
      //     }
      //   }
      //   fmt::print( "(support size = {})\n", support_size );
      //   abort();
      // }
      // else
      // {
      //   fmt::print( "[m] Correct resynthesis\n" );
      // }
      // fmt::print( "[m] output is {}n{}\n", ( res.is_complemented( output ) ? "~" : "" ), res.node_to_index( res.get_node( output ) ) );

      if ( i != decomp_res.size() - 1u )
      {
        // if ( !res.is_constant( res.get_node( output ) ) )
        {
          signals.push_back( output );
        }
      }
      else
      {
        return output;
      } 
    }
  }

  /* TODO : Ensure the network has `create_and` and `create_xor` methods */
  template<class Ntk>
  signal<Ntk> construct_low_mc_mux( Ntk& ntk, signal<Ntk> const& d0, signal<Ntk> const& d1, signal<Ntk> const& s )
  {
    signal<Ntk> diff = ntk.create_xor( d0, d1 );
    diff = ntk.create_and( diff, s );
    return ntk.create_xor( diff, d0 );
  }

  uint32_t min_power_of_two( uint32_t const& n )
  {
    assert( n > 1u );
    if ( n & ( n - 1 ) == 0u )
    {
      return n;
    }

    uint32_t m{ 2u };
    while( m < n )
    {
      m <<= 1u;
    }
    return m;
  }

  template<class Ntk>
  signal<Ntk> construct_mux_tree( Ntk& ntk, std::vector<std::pair<uint8_t, signal<Ntk>>> const& ds, std::vector<signal<Ntk>> const& ss, uint8_t sel_polar )
  {
    uint32_t num_signals_padded = min_power_of_two( ds.size() );
    const signal<Ntk> PAD_SIG = ntk.make_signal( ntk.index_to_node( std::numeric_limits<uint32_t>::max() ) );
    std::vector<signal<Ntk>> sig_to_sel( num_signals_padded, PAD_SIG );
    std::vector<signal<Ntk>> out( num_signals_padded >> 1, PAD_SIG );
    uint32_t num_max{ 0u };
    for ( auto const& ele : ds )
    {
      assert( ele.first < num_signals_padded );
      assert( sig_to_sel[ele.first] == PAD_SIG );
      sig_to_sel[ele.first] = ele.second;
    }

    for ( auto i{ 0u }; i < ss.size(); ++i )
    {
      const signal<Ntk> s = ( ( sel_polar >> i ) & 1 ) ? ntk.create_not( ss[i] ) : ss[i];
      for ( auto j{ 0u }; j < sig_to_sel.size() / 2; ++j )
      {
        const signal<Ntk> d0 = sig_to_sel[2 * j];
        const signal<Ntk> d1 = sig_to_sel[2 * j + 1];
        if ( d0 == PAD_SIG && d1 == PAD_SIG )
        {
          out[j] = PAD_SIG;
        }
        else if ( d0 == PAD_SIG )
        {
          out[j] = d1;
        }
        else if ( d1 == PAD_SIG )
        {
          out[j] = d0;
        }
        else
        {
          out[j] = construct_low_mc_mux( ntk, d0, d1, s );
          ++num_max;
        }
      }
      sig_to_sel.clear();
      sig_to_sel = out;
      out.clear();
      out = std::vector<signal<Ntk>>( sig_to_sel.size() >> 1, PAD_SIG );
    }

    assert( num_max == ds.size() - 1 );
    assert( sig_to_sel.size() == 1 );
    return sig_to_sel[0];
  }

private:
  void build_db( std::string const& filename )
  {
    std::generate( _pis.begin(), _pis.end(), [this]() { return _db.create_pi(); } );
    std::ifstream db_file( filename.c_str(), std::ifstream::in );
    std::string line;
    unsigned pos = 0u;

    while ( std::getline( db_file, line ) )
    {
      line.erase( std::remove( line.begin(), line.end(), '\r' ), line.end() );
      pos = static_cast<unsigned>( line.find( '\t' ) );
      const std::string name = line.substr( 0, pos++ );
      const std::string original = line.substr( pos, 16u );
      pos += 17u;
      const std::string token_f = line.substr( pos, 16u );
      pos += 17u;
      const uint8_t mc = static_cast<uint8_t>( std::stoul( line.substr( pos, 1u ) ) );
      pos += 2u;
      line.erase( 0, pos );

      auto circuit = line;
      std::string token = circuit.substr( 0, circuit.find( ' ' ) );
      circuit.erase( 0, circuit.find( ' ' ) + 1 );
      const auto inputs = std::stoul( token );
      std::vector<xag_network::signal> hashing_circ( _pis.begin(), _pis.begin() + inputs );

      while ( circuit.size() > 4 )
      {
        std::array<unsigned, 2> signals;
        std::vector<xag_network::signal> ff( 2 );
        for ( auto j = 0u; j < 2u; ++j )
        {
          token = circuit.substr( 0, circuit.find( ' ' ) );
          circuit.erase( 0, circuit.find( ' ' ) + 1 );
          signals[j] = std::stoul( token );
          if ( signals[j] == 0 )
          {
            ff[j] = _db.get_constant( false );
          }
          else if ( signals[j] == 1 )
          {
            ff[j] = _db.get_constant( true );
          }
          else
          {
            ff[j] = hashing_circ[signals[j] / 2 - 1] ^ ( signals[j] % 2 != 0 );
          }
        }
        circuit.erase( 0, circuit.find( ' ' ) + 1 );

        if ( signals[0] > signals[1] )
        {
          hashing_circ.push_back( _db.create_xor( ff[0], ff[1] ) );
        }
        else
        {
          hashing_circ.push_back( _db.create_and( ff[0], ff[1] ) );
        }
      }

      const auto output = std::stoul( circuit );
      const auto f = hashing_circ[output / 2 - 1] ^ ( output % 2 != 0 );
      _db.create_po( f );

      /* for debugging */
      // cut_view<xag_network> new_impl{ _db, _pis, f };
      // kitty::static_truth_table<6u> tt;
      // kitty::create_from_hex_string( tt, original );
      // auto result = simulate<kitty::static_truth_table<6u>>( new_impl )[0];
      // if ( tt != result )
      // {
      //   fmt::print( "[e] Invalid circuit for {}, got ", original );
      //   kitty::print_hex( result );
      //   fmt::print( "\n" );
      //   abort();
      // }

      _func_mc.insert( { token_f, { original, mc, f } } );
    }
    fmt::print( "[i] Load database successfully.\n" );
  }

private:
  xag_network _db;
  std::vector<xag_network::signal> _pis;
  std::unordered_map<std::string, std::tuple<std::string, uint8_t, xag_network::signal>> _func_mc; /* { func_repr, { func_db, mc, po_in_db } } */
  /* { func_orig, { func_repr, is_exact, spectral_trans } } */
  std::unordered_map<kitty::static_truth_table<6u>, std::tuple<bool, kitty::static_truth_table<6u>, std::vector<kitty::detail::spectral_operation>>, kitty::hash<kitty::static_truth_table<6u>>> _classify_cache;
};

} /* namespace mockturtle */