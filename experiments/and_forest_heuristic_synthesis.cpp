#include <mockturtle/algorithms/and_forest_exact_synthesis.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/networks/aig.hpp>
#include <iostream>
#include <string_view>

constexpr std::string_view get_mode_name( const mockturtle::Solving_mode mode )
{
  switch ( mode )
  {
    case mockturtle::Solving_mode::Qubit: return "qubit"; break;
    case mockturtle::Solving_mode::Step: return "step"; break;
  }
}

int main()
{
  mockturtle::and_forest_heuristic_syn_params ps;
  ps.num_vars = 7u;
  ps.conflict_limit = 0u;
  // ps.solving_mode = mockturtle::Solving_mode::Qubit;
  ps.verbose = true;
  // mockturtle::and_forest_exact_syn_stats st;
  // const mockturtle::xag_network ntk = mockturtle::and_forest_exact_synthesis( ps, &st );
  // std::string filename = fmt::format( "benchmarks/and_forest_{0}_{1}.v", ps.num_vars, get_mode_name( ps.solving_mode ) );
  // mockturtle::write_verilog( ntk, filename );

  for ( uint8_t solving_mode_ind{ 1u }; solving_mode_ind < 2u; ++solving_mode_ind )
  {
    mockturtle::Solving_mode current_mode;
    switch ( solving_mode_ind )
    {
      case 0u: current_mode = mockturtle::Solving_mode::Qubit; break;
      case 1u: current_mode = mockturtle::Solving_mode::Step; break;
    }
    ps.solving_mode = current_mode;
    fmt::print( "[m] Working in {} mode...\n", get_mode_name( ps.solving_mode ) );

    mockturtle::and_forest_heuristic_syn_stats st;
    // const mockturtle::xag_network ntk = mockturtle::and_forest_heuristic_synthesis( ps, &st );
    const mockturtle::aig_network ntk = mockturtle::and_forest_heuristic_synthesis<mockturtle::aig_network, bill::solvers::bsat2>( ps, &st );
    std::string filename = fmt::format( "benchmarks/heuristic_and_forest_{0}_{1}.v", ps.num_vars, get_mode_name( ps.solving_mode ) );
    mockturtle::write_verilog( ntk, filename );
  }
  return 0;
}
