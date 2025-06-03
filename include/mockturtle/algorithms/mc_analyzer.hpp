#pragma once

#include "../networks/xag.hpp"
#include "../views/cut_view.hpp"
#include "../views/ownership_view.hpp"
#include "acdecomp_max_local.hpp"
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

  xag_network::signal min_mc_resyn( xag_network& res, std::vector<xag_network::signal> const& leaves, std::vector<decomp_result> const& decomp_res )
  {
    /* for debugging */
    fmt::print( "[m] (Second check) Leaves of the current cut are [ " );
    for ( auto const& each_leaf : leaves )
    {
      fmt::print( "{}Node{} ", ( res.is_complemented( each_leaf ) ? "~" : "" ), ( res.node_to_index( res.get_node( each_leaf ) ) ) );
    }
    fmt::print( "]\n" );

    auto signals = leaves;
    for ( uint32_t i = 0u; i < decomp_res.size(); ++i )
    {
      /* for debugging */
      if ( i == ( decomp_res.size() - 1 ) )
      {
        fmt::print( "[m] Concerning top function " );
      }
      else
      {
        fmt::print( "[m] Concerning BS function " );
      }
      
      const kitty::dynamic_truth_table tt = decomp_res[i].tt;
      const std::vector<uint32_t> support = decomp_res[i].support;
      const auto tt_ext = kitty::extend_to<6u>( tt );

      kitty::print_hex( tt_ext );
      fmt::print( "\nLeaves: [ " );

      std::vector<xag_network::signal> pis ( 6u, res.get_constant( false ) );
      for ( uint32_t j = 0; j < support.size(); ++j )
      {
        pis[j] = signals[support[j]];

        fmt::print( "{}Node{} ", ( res.is_complemented( pis[j] ) ? "~" : "" ), ( res.node_to_index( res.get_node( pis[j] ) ) ) );
      }

      fmt::print( "]\n" );
      const auto pis_backup = pis; /* for evaluting the functionality of the newly generated implementation */

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
        fmt::print( "[m] Classify original function " );
        kitty::print_hex( tt_ext );
        fmt::print( " into " );
        kitty::print_hex( tt_ext_repr );
        fmt::print( "\n" );
        fmt::print( "[m] Testing the validity of `trans`: [" );
        for ( auto const& ele : trans )
        {
          fmt::print( "{} ", ele._kind );
        }
        fmt::print( "]\n" );
      }

      fmt::print( "Representative function: " );
      kitty::print_hex( tt_ext_repr );
      fmt::print( "\n" );

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
      fmt::print( "[m] The issue is not related to the interaction with `_func_mc`\n" );

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
          fmt::print( "[m] swap pis[{}] and pis[{}]\n", v1, v2 );

          break;
        }
        case kitty::detail::spectral_operation::kind::input_negation:
        {
          const auto v1 = log2( each_trans._var1 );
          pis[v1] = !pis[v1];

          /* for debugging */
          fmt::print( "[m] negate pis[{}]\n", v1 );

          break;
        }
        case kitty::detail::spectral_operation::kind::output_negation:
        {
          out_neg = !out_neg;

          /* for debugging */
          fmt::print( "[m] negate output\n" );

          break;
        }
        case kitty::detail::spectral_operation::kind::spectral_translation:
        {
          const auto v1 = log2( each_trans._var1 );
          const auto v2 = log2( each_trans._var2 );
          pis[v1] = res.create_xor( pis[v1], pis[v2] );

          /* for debugging */
          fmt::print( "[m] pis[{}] becomes XOR( pis[{}], pis[{}] )\n", v1, v1, v2 );

          break;
        }
        case kitty::detail::spectral_operation::kind::disjoint_translation:
        {
          const auto v1 = log2( each_trans._var1 );
          final_xors.push_back( pis[v1] );

          /* for debugging */
          fmt::print( "[m] output is XORed with pis[{}]\n", v1 );

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

      fmt::print( "Permuted leaves: [ " );
      for ( auto j = 0; j < pis.size(); ++j )
      {
        fmt::print( "{}Node{} ", ( res.is_complemented( pis[j] ) ? "~" : "" ), ( res.node_to_index( res.get_node( pis[j] ) ) ) );
      }
      fmt::print( "]\n" );
      /* for debugging */
      /* verify the functionality of the resynthesized circuit */
      cut_view<xag_network> new_impl{ res, pis_backup, output };
      kitty::static_truth_table<6u> tt_new_impl = simulate<kitty::static_truth_table<6u>>( new_impl )[0];
      for ( auto i = 0u; i < pis_backup.size(); ++i )
      {
        if ( res.is_complemented( pis_backup[i] ) )
        {
          kitty::flip_inplace( tt_new_impl, i );
        }
      }
      if ( tt_new_impl != tt_ext )
      {
        fmt::print( "[e] The circuit synthesized for the {}th BS function is incorrect!\n", ( i + 1 ) );
        std::cout << "[e] `tt_ext` = ";
        kitty::print_hex( tt_ext );
        fmt::print( "(support size = {})", tt.num_vars() );
        std::cout << ", `tt_new_impl` = ";
        kitty::print_hex( tt_new_impl );
        uint32_t support_size = 0u;
        for ( auto const& each_pi : pis )
        {
          if ( !res.is_constant( res.get_node( each_pi ) ) )
          {
            ++support_size;
          }
        }
        fmt::print( "(support size = {})\n", support_size );
        abort();
      }

      if ( i != decomp_res.size() - 1u )
      {
        if ( !res.is_constant( res.get_node( output ) ) )
        {
          signals.push_back( output );
        }

        /* for debugging */
        // fmt::print( "[m] The {}th BS function: ", ( i + 1 ) );
        // kitty::print_hex( tt_ext );
        // fmt::print( "\n" );
      }
      else
      {
        // fmt::print( "[m] The top function: " );
        // kitty::print_hex( tt_ext );
        // fmt::print( "\n" );

        return output;
      } 
    }
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
      cut_view<xag_network> new_impl{ _db, _pis, f };
      kitty::static_truth_table<6u> tt;
      kitty::create_from_hex_string( tt, original );
      auto result = simulate<kitty::static_truth_table<6u>>( new_impl )[0];
      if ( tt != result )
      {
        fmt::print( "[e] Invalid circuit for {}, got ", original );
        kitty::print_hex( result );
        fmt::print( "\n" );
        abort();
      }

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