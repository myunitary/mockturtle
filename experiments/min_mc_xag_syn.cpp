#include <bill/sat/interface/abc_bsat2.hpp>
#include <mockturtle/algorithms/exact_mc_synthesis.hpp>
#include <mockturtle/algorithms/xag_optimization.hpp>
#include <mockturtle/properties/mccost.hpp>
#include <kitty/constructors.hpp>

int main()
{
  using namespace mockturtle;

  kitty::dynamic_truth_table tt( 3u );
  kitty::create_from_hex_string( tt, "d8" ); // PI 1 ? PI 2 : PI 3

  xag_network ntk = exact_mc_synthesis<xag_network, bill::solvers::bsat2>( tt );
  ntk = exact_linear_resynthesis_optimization( ntk, 500000u );
  const uint32_t num_ands = *multiplicative_complexity( ntk );
  const uint32_t num_xors = ntk.num_gates() - num_ands;

  fmt::print( "[i] The MC-minimum XAG implementation contains {} AND gates and {} XOR gates.\n", num_ands, num_xors );

  fmt::print( "[i] Printing out the network...\n" );
  ntk.foreach_gate( [&]( auto const& n ) {
    fmt::print( "Gate {}: ", ntk.node_to_index( n ) );
    if ( ntk.is_and( n ) )
    {
      fmt::print( "AND( " );
    }
    else
    {
      fmt::print( "XOR( " );
    }
    ntk.foreach_fanin( n, [&]( auto const& fi, auto const& index ) {
      auto const fi_index = ntk.node_to_index( ntk.get_node( fi ) );
      fmt::print( "{}{}{}{}", ( ntk.is_complemented( fi ) ? "!" : "" ), ( ( fi_index > ntk.num_pis() ) ? "Gate " : "PI " ), fi_index, ( ( index == 0u ) ? ", " : "" ) );
    } );
    fmt::print( " )\n" );
  } );
  fmt::print( "[i] Finished printing out the network\n" );

  return 0;
}