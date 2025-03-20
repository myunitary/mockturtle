#include <experiments.hpp>
#include <lorina/verilog.hpp>
#include <mockturtle/algorithms/fhe_rule_miner.hpp>
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
		fmt::print( "[i] processing {}\n", benchmark );

		fhe_rule_miner_params ps{};
		ps.cut_enum_ps.cut_size = 5u;
		ps.cut_enum_ps.cut_limit = 25u; // limitations on amount of cuts enumerated at each root
		ps.cut_enum_ps.verbose = false;
		ps.cut_enum_ps.very_verbose = false;
		ps.num_rules_limit = 500u; // limitations on total amount of mined rules
		ps.num_node_rules_limit = 2u; // limitation on amount of rules mined at each root
		ps.progress = true;
		ps.verbose = true;
		ps.very_verbose = false;

		fhe_rule_miner_stats st{};

		std::string dbname = "db_fhe_5";
		std::string filename = fmt::format( "fhe_rules/{}", benchmark );

		fhe_rule_miner( ntk, dbname, filename, ps, &st );
	}

	return 0;
}