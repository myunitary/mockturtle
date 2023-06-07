#include <mockturtle/algorithms/merge_split_xag.hpp>
#include <mockturtle/io/verilog_reader.hpp>

#include <experiments.hpp>

static const uint32_t tcad22_epfl[] = {
	128u, 832u, 5132u, 8773u, 872u, 7585u, 1959u, 5217u, 4596u, 1174u, 394u, 
	45u, 328u, 557u, 85u, 4695u, 323u, 93u, 4257u, 0u};

std::vector<uint32_t> epfl_tcad22()
{
	std::vector<uint32_t> best_score;
	for( auto i = 0u; i < 20u; ++i )
	{
		best_score.emplace_back( tcad22_epfl[i] );
	}
	return best_score;
}

static const std::string EPFL_benchmarks[] = {
	"adder", "bar", "div", "log2", "max", "multiplier", "sin", "sqrt", "square", "arbiter", 
	"cavlc", "ctrl" , "dec", "i2c", "int2float" , "mem_ctrl", "priority", "router", "voter", 
	"hyp"};

std::vector<std::string> epfl_benchmarks()
{
	std::vector<std::string> result;
	for ( auto i = 0u; i < 20u; ++i )
	{
		result.emplace_back( EPFL_benchmarks[i] );
	}

	return result;
}

std::string benchmark_path( std::string const& benchmark_name, uint8_t opt )
{
	return fmt::format( "../experiments/{}/{}.v", ( opt ? ( ( opt == 1u ) ? "epfl_opt" : "epfl_tcad22" ) : "epfl_benchmarks" ), benchmark_name );
}

int main()
{
	std::string json_name = "merge_split_xag";
	experiments::experiment<std::string, uint32_t, uint32_t, float, float> exp_res( json_name, "benchmark", "T-count_before", "T-count_after", "improvement %", "runtime [s]" );
	/* 0u - unoptimized benchmarks; 1u - optimized benchmarks from DATE20; 2u - optimized benchmarks from TCAD22 */
	uint8_t opt = 2u;
	std::cout << "[i] working on " << ( opt ? ( ( opt == 1u ) ? "DATE20 benchmarks\n" : "TCAD22 benchmarks\n" ) : "unoptimized benchmarks\n" );
	auto const benchmarks = epfl_benchmarks();
	std::vector<uint32_t> const best_scores  = epfl_tcad22();

	for ( auto i{ 0u }; i < benchmarks.size(); ++i )
	{
		auto const benchmark = benchmarks[i];
		if ( best_scores[i] == 0u )
		{
			std::cout << "[i] skip " << ( opt ? "optimized " : "" ) << benchmark << std::endl;
			continue;
		}

		mockturtle::xag_network xag;

		auto const read_result = lorina::read_verilog( benchmark_path( benchmark, opt ), mockturtle::verilog_reader( xag ) );
		assert( read_result == lorina::return_code::success );

		if ( read_result == lorina::return_code::success )
		{
			std::cout << "[i] successfully read " << benchmark << std::endl;
		}
		else
		{
			std::cout << "[i] failed to read " << benchmark << std::endl;
		}
		uint32_t t_count_before = best_scores[i] * 4;
		if ( t_count_before == 0u )
		{
			exp_res( benchmark, 0u, 0u, 0., 0. );
			continue;
		}

		clock_t begin_time = clock();
		uint32_t t_count_after = mockturtle::merge_split_xag( xag );
		float improve = ( ( static_cast<float>( t_count_before ) - static_cast<float>( t_count_after ) ) / static_cast<float>( t_count_before ) ) * 100;
		exp_res( benchmark, t_count_before,t_count_after, improve, ( float( clock() - begin_time ) / CLOCKS_PER_SEC ) );
	}

	exp_res.save();
	exp_res.table();

	return 0;
}