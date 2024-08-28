#include <mockturtle/algorithms/and_forest_exact_synthesis.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/xag.hpp>

int main()
{
  mockturtle::and_forest_exact_syn_params ps;
  ps.num_vars = 3u;
  ps.conflict_limit = 0u;
  ps.verbose = true;
  mockturtle::and_forest_exact_syn_stats st;
  const mockturtle::xag_network ntk = mockturtle::and_forest_exact_synthesis( ps, st );
  return 0;
}