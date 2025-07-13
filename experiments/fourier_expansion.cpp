#include <kitty/constructors.hpp>
#include <kitty/print.hpp>
#include <kitty/spectral.hpp>
#include <fmt/format.h>

int main()
{
  uint32_t num_vars = 3u;
  kitty::dynamic_truth_table tt( num_vars );
  kitty::create_from_hex_string( tt, "60" );
  kitty::detail::fourier_spectrum fs{ kitty::detail::fourier_spectrum::from_truth_table( tt ) };
  const std::vector<uint32_t> order = kitty::detail::get_rw_coeffecient_order( num_vars );

  std::string info = "[i] The Fourier coefficients of function ";
  std::ostringstream oss;
  kitty::print_hex( tt, oss );
  info += ( oss.str() + ": " );
  fmt::print( "{}", std::string( info.size(), ' ' ) );
  for ( auto const& ele : order )
  {
    fmt::print( "{:>8b}", ele );
  }
  std::cout << std::endl;
  fmt::print( "{}", info );
  fs.print( std::cout, order );
  fmt::print( "\n" );

  return 0;
}