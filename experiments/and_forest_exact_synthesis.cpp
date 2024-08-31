#include <mockturtle/algorithms/and_forest_exact_synthesis.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/xag.hpp>
#include <iostream>
#include <string_view>

constexpr std::string_view get_mode_name( const mockturtle::Solving_mode mode )
{
  switch ( mode )
  {
    case mockturtle::Solving_mode::Normal: return "normal";
    case mockturtle::Solving_mode::Qubit: return "qubit";
    case mockturtle::Solving_mode::Qubit_step: return "qstep";
    case mockturtle::Solving_mode::Step: return "step";
  }
}

int main()
{
  mockturtle::and_forest_exact_syn_params ps;
  ps.num_vars = 3u;
  ps.conflict_limit = 0u;
  ps.solving_mode = mockturtle::Solving_mode::Step;
  ps.verbose = true;
  mockturtle::and_forest_exact_syn_stats st;
  const mockturtle::xag_network ntk = mockturtle::and_forest_exact_synthesis( ps, &st );
  // mockturtle::and_forest_exact_synthesis( ps, &st );
  return 0;
}
