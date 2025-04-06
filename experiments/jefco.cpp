#include <experiments.hpp>
#include <lorina/verilog.hpp>
#include <mockturtle/algorithms/jefco.hpp>
#include <mockturtle/io/verilog_reader.hpp>

std::string bench_path( std::string const& benchmark_name )
{
  return fmt::format( "{}BEST_RESULTS/EPFL/{}.v", EXPERIMENTS_PATH, benchmark_name );
}

int main()
{
	using namespace mockturtle;
	using namespace experiments;

	for ( auto const& benchmark : epfl_benchmarks() )
	{
		xag_network ntk;
		if ( lorina::read_verilog( bench_path( benchmark ) , verilog_reader( ntk ) ) != lorina::return_code::success )
		{
			continue;
		}

		// if ( benchmark != "adder" )
		// {
		// 	break;
		// }

		fmt::print( "[i] processing {}\n", benchmark );

		jefco_params ps{};
		ps.cut_enum_ps.cut_size = 5u;
		ps.cut_enum_ps.cut_limit = 25u; // limitations on amount of cuts enumerated at each root
		ps.cut_enum_ps.verbose = false;
		ps.cut_enum_ps.very_verbose = false;
		ps.esop_only_on_critical_path = true; // generate an ESOP implementation for a cut only if its root node is on the MD critial path
		ps.progress = true;
		ps.verbose = true;
		ps.very_verbose = false;

		jefco_stats st{};

		std::string filename = fmt::format( "fhe_rules/{}", benchmark );

		jefco( ntk, filename, ps, &st );
	}

	return 0;
}