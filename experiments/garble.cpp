/* mockturtle: C++ logic network library
 * Copyright (C) 2018-2022  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <string>
#include <vector>
#include <cassert>

#include <fmt/format.h>
#include <lorina/aiger.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis/bidecomposition.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_minmc.hpp>
#include <mockturtle/algorithms/refactoring.hpp>
#include <mockturtle/algorithms/xag_resub_withDC.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/networks/xmg.hpp>
#include <mockturtle/algorithms/node_resynthesis/xmg_npn.hpp>
#include <mockturtle/algorithms/mapper.hpp>
#include <mockturtle/algorithms/xmg_resub.hpp>
#include <mockturtle/algorithms/xmg_optimization.hpp>
#include <mockturtle/algorithms/xmg_algebraic_rewriting.hpp>
#include <mockturtle/views/topo_view.hpp>
#include <mockturtle/algorithms/experimental/cost_generic_resub.hpp>
#include <mockturtle/views/cost_view.hpp>
#include <mockturtle/utils/recursive_cost_functions.hpp>

#include <experiments.hpp>

using namespace experiments;
using namespace mockturtle;

static const char* crypto_epfl_benchmarks[] = {
  /*
    "adder_untilsat", "bar_untilsat", "div_untilsat", "log2_untilsat", "max_untilsat", "multiplier_untilsat", "sin_untilsat", "sqrt_untilsat", "square_untilsat",
    "arbiter_untilsat", "cavlc_untilsat", "ctrl_untilsat" , "dec_untilsat", "i2c_untilsat", "int2float_untilsat" , "mem_ctrl_untilsat", "priority_untilsat", "router_untilsat", "voter_untilsat", 
    "adder_32bit_untilsat", "adder_64bit_untilsat", "adder_1bit", 
    "AES-expanded_untilsat", "AES-non-expanded_unstilsat", 
    */
    "comparator_32bit_signed_lt_untilsat", "comparator_32bit_signed_lteq_untilsat", "comparator_32bit_unsigned_lt_untilsat", "comparator_32bit_unsigned_lteq_untilsat", 
    "DES-expanded_untilsat", "DES-non-expanded_untilsat", "md5_untilsat", "mult_32x32_untilsat", "sha-1_untilsat", 
    "sha-256_untilsat"};

std::vector<std::string> crypto_benchmarks( )
{
  std::vector<std::string> result;
  for ( uint32_t i = 0u; i < 9u; ++i ) // change this number to 33u to run also the sha-256 benchmark 
  {
      result.push_back( crypto_epfl_benchmarks[i] );
  }
  return result;
}

std::string crypto_benchmark_path( std::string const& benchmark_name )
{
#ifndef EXPERIMENTS_PATH
  return fmt::format( "{}.v", benchmark_name );
#else
  return fmt::format( "{}crypto_benchmarks/{}.v", EXPERIMENTS_PATH, benchmark_name );
#endif
}

template<class Ntk>
bool abc_cec_crypto( Ntk const& ntk, std::string const& benchmark )
{
  mockturtle::write_bench( ntk, "/tmp/test.bench" );
  std::string command = fmt::format( "abc -q \"cec -n {} /tmp/test.bench\"", crypto_benchmark_path( benchmark ) );

  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype( &pclose )> pipe( popen( command.c_str(), "r" ), pclose );
  if ( !pipe )
  {
    throw std::runtime_error( "popen() failed" );
  }
  while ( fgets( buffer.data(), buffer.size(), pipe.get() ) != nullptr )
  {
    result += buffer.data();
  }

  return result.size() >= 23 && result.substr( 0u, 23u ) == "Networks are equivalent";
}

template<class Ntk>
struct xmg_gc_cost : recursive_cost_functions<Ntk>
{
public:
  using context_t = uint32_t;
  context_t operator()( Ntk const& ntk, node<Ntk> const& n, std::vector<context_t> const& fanin_contexts = {} ) const
  {
    return 0; /* no context required */
  }
  void operator()( Ntk const& ntk, node<Ntk> const& n, uint32_t& total_cost, context_t const context ) const
  {
    (void)context;
    total_cost += ( !ntk.is_pi( n ) && ntk.visited( n ) != ntk.trav_id() ) ? ntk.is_maj( n ) : 0;
  }
};

namespace detail
{

template<class Ntk>
struct and_cost
{
  uint32_t operator()( Ntk const& ntk, node<Ntk> const& n ) const
  {
    return ntk.is_and( n ) ? 1 : 0;
  }
};

template<class Ntk>
struct free_xor_cost
{
  uint32_t operator()( Ntk const& ntk, node<Ntk> const& n ) const
  {
    return ntk.is_xor( n ) ? 0 : 1;
  }
};

template<class Ntk>
struct maj_cost
{
  uint32_t operator()( Ntk const& ntk, node<Ntk> const& n ) const
  {
    return ntk.is_maj( n ) ? 1 : 0;
  }
};
} // namespace detail*/

int main( int argc, char** argv )
{
  /*
  if ( argc <= 1 )
  {
    std::cout << "input should be ''db [database]''\n";
    return 0;
  }
  */

  /* best results obtained using Eleonora's XAG optimization algorithm */
  const std::vector<uint32_t> cost_xags = {184, 184, 184, 184, 18414, 18088, 18734, 3378, 23030};
  uint32_t cost_xags_ptr = 0;

  experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, float, float, float> exp( "gc", "benchmark", "cost_xag", "cost_xmg_shu", "cost_xmg_han", "cost_xmg_comb", "improvement_shu %", "improvement_han %", "improvement_comb %" );

  for ( auto const& benchmark : crypto_benchmarks() )
  {
    fmt::print( "[i] processing {}\n", benchmark );

    uint32_t cost_xag = cost_xags[cost_xags_ptr++];
    uint32_t cost_xmg_shu = 0u, cost_xmg_han = 0u, cost_xmg_comb = 0u;

    /* xag optimization flow from Eleonora */
    /*
    xag_network xag;
    lorina::read_verilog( crypto_benchmark_path( benchmark ), verilog_reader( xag ) );

    xag.foreach_gate( [&]( auto f ) {
      if ( xag.is_and( f ) )
        num_and_xag++;
    } );
    xag.foreach_gate( [&]( auto f ) {
      if ( xag.is_xor( f ) )
        num_xor_xag++;
    } );

    num_and_xag_opt = num_and_xag - 1;
    uint32_t num_and_xag_tmp = num_and_xag;

    cut_rewriting_params ps;
    ps.cut_enumeration_ps.cut_size = 6;
    ps.cut_enumeration_ps.cut_limit = 12;
    ps.verbose = true;
    ps.progress = false;
    ps.min_cand_cut_size = 2u;

    refactoring_params ps2;
    ps2.verbose = true;
    ps2.progress = false;
    ps2.allow_zero_gain = false;
    ps2.max_pis = 15;
    ps2.use_dont_cares = true;

    resubstitution_params ps3;
    ps3.max_divisors = 100;
    ps3.max_inserts = 4;
    ps3.max_pis = 8u;
    ps3.progress = false;
    ps3.verbose = true;
    ps3.use_dont_cares = true;

    xag_minmc_resynthesis resyn( argv[1] );
    bidecomposition_resynthesis<xag_network> resyn2;

    auto it_cnt = 0u;

    while ( num_and_xag > num_and_xag_opt )
    {
      if ( it_cnt > 0 )
        num_and_xag = num_and_xag_opt;
      it_cnt++;
      num_and_xag_opt = 0u;
      num_xor_xag_opt = 0u;

      cut_rewriting_with_compatibility_graph( xag, resyn, ps, nullptr, ::detail::and_cost<xag_network>() );
      xag = cleanup_dangling( xag );
      refactoring( xag, resyn2, ps2, nullptr, ::detail::free_xor_cost<xag_network>() );
      xag = cleanup_dangling( xag );
      using xag_view_t = depth_view<fanout_view<xag_network>>;
      fanout_view<xag_network> xag_fanout_view{xag};
      xag_view_t xag_resub_view{xag_fanout_view};
      resubstitution_minmc_withDC( xag_resub_view, ps3 );
      xag = cleanup_dangling( xag );

      xag.foreach_gate( [&]( auto f ) {
        if ( xag.is_and( f ) )
          num_and_xag_opt++;
      } );
      xag.foreach_gate( [&]( auto f ) {
        if ( xag.is_xor( f ) )
          num_xor_xag_opt++;
      } );
    }

    std::cout << "Verify the function correctness of the optimized XAG network. \n";
    assert(abc_cec_crypto( xag, benchmark ));
    std::cout << "Passed verification. Proceed to XMG. \n\n";
    */

    xmg_network xmg;

    /* synthesis using mapper */
    lorina::read_verilog( crypto_benchmark_path( benchmark ), verilog_reader( xmg ) );
    xmg_npn_resynthesis xmg_resyn;
    exact_library<xmg_network, xmg_npn_resynthesis> lib( xmg_resyn );
    xmg =  map( xmg, lib );
    xmg_network xmg_ = xmg.clone();

    /* synthesis based on the optimized xag */
    /*
    using sig_xmg = typename xmg_network::signal;
    // derive signals in the targeted xmg from nodes in the original xag
    node_map<sig_xmg, xag_network> xag_node2xmg_sig( xag );

    // create const
    xag_node2xmg_sig[xag.get_node( xag.get_constant( false ) )] = xmg.get_constant( false );

    // create pis
    xag.foreach_pi( [&]( auto n ) {
      xag_node2xmg_sig[n] = xmg.create_pi();
    } );
    */

    /* create MAJ-3s and XOR-3s */
    /*
    topo_view xag_topo{xag};
    xag_topo.foreach_node( [&]( auto n ) {
      if ( xag.is_constant( n ) || xag.is_pi( n ) )
        return;
      std::vector<sig_xmg> children;
      xag.foreach_fanin( n, [&]( auto const& f ) {
        children.push_back( xag.is_complemented( f ) ? xmg.create_not( xag_node2xmg_sig[f] ) : xag_node2xmg_sig[f] );
      } );

      assert( children.size() == 2u );
      if ( xag.is_and( n ) )
      {
        xag_node2xmg_sig[n] = xmg.create_maj( xmg.get_constant( false ), children[0], children[1] );
      }
      else if ( xag.is_xor( n ) )
      {
        xag_node2xmg_sig[n] = xmg.create_xor3( xmg.get_constant( false ), children[0], children[1] );
      }
      else
      {
        std::cerr << "Unknown gate type detected! \n";
        abort();
      }
    } );
    */

    /* create pos */
    /*
    xag.foreach_po( [&]( auto const& f ) {
      auto const o = xag.is_complemented( f ) ? xmg.create_not( xag_node2xmg_sig[f] ) : xag_node2xmg_sig[f];
      xmg.create_po( o );
    } );
    */

    uint32_t num_maj_xmg = 0, num_xor_xmg = 0;

    xmg.foreach_gate( [&]( auto f ) {
      if ( xmg.is_maj( f ) )
        num_maj_xmg++;
    } );
    xmg.foreach_gate( [&]( auto f ) {
      if ( xmg.is_xor3( f ) )
        num_xor_xmg++;
    } );

    /* a hybrid flow with existing xmg resub & rewrite algorithms applied */
    uint32_t it_cnt = 0u;
    uint32_t num_maj_xmg_opt = num_maj_xmg - 1;
    while ( num_maj_xmg > num_maj_xmg_opt )
    {
      if ( it_cnt > 0 )
        num_maj_xmg = num_maj_xmg_opt;
      it_cnt++;
      num_maj_xmg_opt = 0u;

      xmg_resubstitution( xmg );
      xmg = cleanup_dangling( xmg );
      depth_view<xmg_network> xmg_depth_view{xmg};
      xmg_algebraic_depth_rewriting( xmg_depth_view );
      xmg = cleanup_dangling( xmg );
      xmg = xmg_dont_cares_optimization( xmg );

      xmg.foreach_gate( [&](auto f) {
        if ( xmg.is_maj( f ) )
        {
          num_maj_xmg_opt += 1;
          if ( xmg.is_fakemaj( f ) )
            cost_xmg_shu += 2;
          else
            cost_xmg_shu += 3;
        }
      } );
    }

    assert(abc_cec_crypto( xmg, benchmark ));

    /* cost function-based optimization */
    auto costfn = xmg_gc_cost<xmg_network>();
    experimental::cost_generic_resub( xmg_, costfn );
    xmg_ = cleanup_dangling( xmg_ );

    //cost_xmg_han = cost_view( xmg_, costfn ).get_cost() * 3;

    assert(abc_cec_crypto( xmg_, benchmark ));

    num_maj_xmg = 0;
    xmg_.foreach_gate ( [&](auto f) {
      if ( xmg_.is_maj( f ) )
      {
        num_maj_xmg += 1;
        if ( xmg_.is_fakemaj( f ) )
          cost_xmg_han += 2;
        else
          cost_xmg_han += 3;
      }
    } );

    // apply the hybrid flow to xmg optimized by cost function-aware method
    it_cnt = 0;
    num_maj_xmg_opt = num_maj_xmg - 1;
    while ( num_maj_xmg > num_maj_xmg_opt )
    {
      if ( it_cnt > 0 )
        num_maj_xmg = num_maj_xmg_opt;
      it_cnt++;
      num_maj_xmg_opt = 0u;

      xmg_resubstitution( xmg_ );
      xmg_ = cleanup_dangling( xmg_ );
      depth_view<xmg_network> xmg_depth_view_{xmg_};
      xmg_algebraic_depth_rewriting( xmg_depth_view_ );
      xmg_ = cleanup_dangling( xmg_ );
      xmg_ = xmg_dont_cares_optimization( xmg_ );

      xmg_.foreach_gate( [&](auto f) {
        if ( xmg_.is_maj( f ) )
        {
          if ( xmg_.is_fakemaj( f ) )
            cost_xmg_comb += 2;
          else
            cost_xmg_comb += 3;
        }
      } );
    }

    assert(abc_cec_crypto( xmg_, benchmark ));

    float improve_shu = ( ( (float) cost_xag - (float) cost_xmg_shu ) / (float) cost_xag ) * 100;
    float improve_han = ( ( (float) cost_xag - (float) cost_xmg_han ) / (float) cost_xag ) * 100;
    float improve_comb = ( ( (float) cost_xag - (float) cost_xmg_comb ) / (float) cost_xag ) * 100;
  
    exp( benchmark, cost_xag, cost_xmg_shu, cost_xmg_han, cost_xmg_comb, improve_shu, improve_han, improve_comb );
  }

  exp.save();
  exp.table();

  return 0;
}
