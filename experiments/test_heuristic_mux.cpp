#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/xag.hpp>
#include <kitty/constructors.hpp>
#include <kitty/print.hpp>
#include <fmt/printf.h>

uint32_t min_power_of_two( uint32_t const& n )
{
  assert( n > 1u );

  if ( ( n & ( n - 1 ) ) == 0u )
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

mockturtle::xag_network::signal construct_low_mc_mux( mockturtle::xag_network& ntk,
                                                      mockturtle::xag_network::signal const& d0,
                                                      mockturtle::xag_network::signal const& d1,
                                                      mockturtle::xag_network::signal const& s )
{
  mockturtle::xag_network::signal diff = ntk.create_xor( d0, d1 );
  diff = ntk.create_and( diff, s );
  return ntk.create_xor( diff, d0 );
}

mockturtle::xag_network::signal construct_mux_tree( mockturtle::xag_network& ntk,
                                                    std::vector<std::pair<uint8_t, mockturtle::xag_network::signal>> const& ds,
                                                    std::vector<mockturtle::xag_network::signal> const& ss,
                                                    uint8_t sel_polar )
{
  uint32_t num_signals_padded = min_power_of_two( ds.size() );
  // const mockturtle::xag_network::signal PAD_SIG = ntk.make_signal( ntk.index_to_node( std::numeric_limits<uint32_t>::max() ) );
  const mockturtle::xag_network::signal PAD_SIG = ntk.get_constant( false );
  std::vector<mockturtle::xag_network::signal> sig_to_sel( num_signals_padded, PAD_SIG );
  std::vector<mockturtle::xag_network::signal> out( num_signals_padded >> 1, PAD_SIG );
  uint32_t num_max{ 0u };
  for ( auto const& ele : ds )
  {
    assert( ele.first < num_signals_padded );
    assert( sig_to_sel[ele.first] == PAD_SIG );
    sig_to_sel[ele.first] = ele.second;
  }

  for ( auto i{ 0u }; i < ss.size(); ++i )
  {
    const mockturtle::xag_network::signal s = ( ( sel_polar >> i ) & 1 ) ? ntk.create_not( ss[i] ) : ss[i];
    for ( auto j{ 0u }; j < sig_to_sel.size() / 2; ++j )
    {
      const mockturtle::xag_network::signal d0 = sig_to_sel[2 * j];
      const mockturtle::xag_network::signal d1 = sig_to_sel[2 * j + 1];
      // if ( d0 == PAD_SIG && d1 == PAD_SIG )
      // {
      //   out[j] = PAD_SIG;
      // }
      // else if ( d0 == PAD_SIG )
      // {
      //   out[j] = d1;
      // }
      // else if ( d1 == PAD_SIG )
      // {
      //   out[j] = d0;
      // }
      // else
      // {
      //   out[j] = construct_low_mc_mux( ntk, d0, d1, s );
      //   ++num_max;
      // }
      if ( d0 == PAD_SIG && d1 == PAD_SIG )
      {
        out[j] = PAD_SIG;
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
    out = std::vector<mockturtle::xag_network::signal>( sig_to_sel.size() >> 1, PAD_SIG );
  }

  // assert( num_max == ds.size() - 1 );
  assert( sig_to_sel.size() == 1 );
  return sig_to_sel[0];
}

int main()
{
  mockturtle::xag_network ntk;
  // mockturtle::xag_network::signal d0 = ntk.get_constant( true );
  mockturtle::xag_network::signal d0 = ntk.create_pi();
  mockturtle::xag_network::signal d1 = ntk.create_pi();
  mockturtle::xag_network::signal d2 = ntk.create_pi();
  mockturtle::xag_network::signal d3 = ntk.create_pi();
  mockturtle::xag_network::signal d4 = ntk.create_pi();
  // mockturtle::xag_network::signal d5 = ntk.get_constant( false );
  mockturtle::xag_network::signal d5 = ntk.create_pi();
  mockturtle::xag_network::signal s0 = ntk.create_pi();
  mockturtle::xag_network::signal s1 = ntk.create_pi();
  mockturtle::xag_network::signal s2 = ntk.create_pi();
  std::vector<mockturtle::xag_network::signal> ss = { s0, s1, s2 };
  std::vector<std::pair<uint8_t, mockturtle::xag_network::signal>> ds = { { 7, d0 }, { 2, d1 }, { 1, d2 }, { 4, d3 }, { 0, d4 }, { 3, d5 } };
  std::sort( ds.begin(), ds.end(), []( auto const& a, auto const& b ) { return a.first < b.first; } );
  uint8_t sel_polar = 4u;

  // mockturtle::xag_network::signal d0 = ntk.create_pi();
  // mockturtle::xag_network::signal d1 = ntk.create_pi();
  // mockturtle::xag_network::signal d3 = ntk.create_pi();
  // mockturtle::xag_network::signal s0 = ntk.create_pi();
  // mockturtle::xag_network::signal s1 = ntk.create_pi();
  // std::vector<mockturtle::xag_network::signal> ss = { s0, s1 };
  // std::vector<std::pair<uint8_t, mockturtle::xag_network::signal>> ds = { { 0, d0 }, { 1, d1 }, { 3, d3 } };
  // uint8_t sel_polar = 0u;

  mockturtle::xag_network::signal out = construct_mux_tree( ntk, ds, ss, sel_polar );
  ntk.create_po( out );
  kitty::static_truth_table<9u> tt = mockturtle::simulate<kitty::static_truth_table<9u>>( ntk )[0];
  fmt::print( "[m] The truth table of the constructed MUX is " );
  kitty::print_hex( tt );
  fmt::print( "\n" );

  return 0;
}