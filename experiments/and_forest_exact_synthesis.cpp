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
    case mockturtle::Solving_mode::Normal: return "normal"; break;
    case mockturtle::Solving_mode::Qubit: return "qubit"; break;
    case mockturtle::Solving_mode::Qubit_step: return "qstep"; break;
    case mockturtle::Solving_mode::Step: return "step"; break;
  }
}

int main()
{
  mockturtle::and_forest_exact_syn_params ps;
  ps.num_vars = 4u;
  ps.conflict_limit = 0u;
  // ps.solving_mode = mockturtle::Solving_mode::Qubit_step;
  ps.verbose = true;
  // mockturtle::and_forest_exact_syn_stats st;
  // const mockturtle::xag_network ntk = mockturtle::and_forest_exact_synthesis( ps, &st );
  // std::string filename = fmt::format( "benchmarks/and_forest_{0}_{1}.v", ps.num_vars, get_mode_name( ps.solving_mode ) );
  // mockturtle::write_verilog( ntk, filename );

  for ( uint8_t solving_mode_ind{ 0u }; solving_mode_ind < 4u; ++solving_mode_ind )
  {
    mockturtle::Solving_mode current_mode;
    switch ( solving_mode_ind )
    {
      case 0u: current_mode = mockturtle::Solving_mode::Normal; break;
      case 1u: current_mode = mockturtle::Solving_mode::Qubit; break;
      case 2u: current_mode = mockturtle::Solving_mode::Qubit_step; break;
      case 3u: current_mode = mockturtle::Solving_mode::Step; break;
    }
    ps.solving_mode = current_mode;

    mockturtle::and_forest_exact_syn_stats st;
    // const mockturtle::xag_network ntk = mockturtle::and_forest_exact_synthesis( ps, &st );
    const mockturtle::aig_network ntk = mockturtle::and_forest_exact_synthesis<mockturtle::aig_network, bill::solvers::bsat2>( ps, &st );
    std::string filename = fmt::format( "benchmarks/aig_and_forest_{0}_{1}.v", ps.num_vars, get_mode_name( ps.solving_mode ) );
    mockturtle::write_verilog( ntk, filename );
  }
  return 0;
}
