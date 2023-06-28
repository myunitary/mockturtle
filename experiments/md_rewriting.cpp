#include <mockturtle/algorithms/md_rewriting.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_bench.hpp>

#include <experiments.hpp>

static const uint32_t tcad22_epfl[] = {
	9u, 4u, 523u, 104u, 25u, 56u, 61u, 769u, 20u, 
	10u, 7u, 4u, 3u, 7u, 6u, 14u, 10u, 8u, 17u};

std::vector<uint32_t> epfl_tcad22()
{
	std::vector<uint32_t> best_score;
	for( auto i = 0u; i < 19u; ++i )
	{
		best_score.emplace_back( tcad22_epfl[i] );
	}
	return best_score;
}

static const std::string EPFL_benchmarks[] = {
	"adder", "bar", "div", "log2", "max", "multiplier", "sin", "sqrt", "square", 
	"arbiter", "cavlc", "ctrl" , "dec", "i2c", "int2float" , "mem_ctrl", "priority", "router", "voter"};

std::vector<std::string> epfl_benchmarks()
{
	std::vector<std::string> result;
	for ( auto i = 0u; i < 19u; ++i )
	{
		result.emplace_back( EPFL_benchmarks[i] );
	}

	return result;
}

std::string benchmark_path( std::string const& benchmark_name )
{
	return fmt::format( "../experiments/epfl_tcad22/{}.v", benchmark_name );
}

template<class Ntk>
bool abc_cec( Ntk const& ntk, std::string const& benchmark )
{
	mockturtle::write_bench( ntk, "/tmp/test.bench" );
	std::string abc_path = "/Users/myu/Documents/GitHub/abc/";
	std::string command = fmt::format( "{}abc -q \"cec -n {} /tmp/test.bench\"", abc_path, benchmark_path( benchmark ) );

	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype( &pclose )> pipe( popen( command.c_str(), "r" ), pclose );

	if ( !pipe )
	{
		throw std::runtime_error( "popen() failed" );
	}

	while ( fgets( buffer.data(), buffer.size(), pipe.get() ) != nullptr )
	{
		result += buffer.data();
	}

	return result.size() >= 23 && result.substr( 0u, 23u ) == "Networks are equivalent";
}

int main()
{
	std::string json_name = "xag_low_md";
	experiments::experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, float, float, uint32_t, float, bool> exp_res( json_name, "benchmark", "MC before", "MC after", "MD before", "MD after", "MD best", "improvement % (local)", "improvement % (global)", "iterations", "runtime [s]", "equivalent" );
	auto const benchmarks = epfl_benchmarks();
	std::vector<uint32_t> const best_scores  = epfl_tcad22();

	mockturtle::md_rewriting_params ps;
	ps.cut_enumeration_ps.cut_size = 5u;
	ps.cut_enumeration_ps.cut_limit = 12u;
	ps.allow_zero_gain = false;
	ps.progress = true;
	ps.verbose = false;
	ps.min_cand_cut_size = 2u;
	ps.verify_database = false;
	

	for ( auto i{ 0u }; i < benchmarks.size(); ++i )
	//for ( auto i{ 3u }; i < 4u; ++i )
	{
		auto const benchmark = benchmarks[i];
		std::cout << "[i] processing " << benchmark << "\n";

		mockturtle::xag_network xag;
		auto const read_result = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( xag ) );
		if ( read_result != lorina::return_code::success )
		{
			std::cout << "[e] failed to read benchmark\n";
			abort();
		}
		std::cout << "[m] read benchmark successfully\n";

		uint32_t md_best = best_scores[i];
		mockturtle::depth_view<mockturtle::xag_network, mockturtle::mc_cost<mockturtle::xag_network>, false> xag_md{ xag };
		uint32_t md_before = xag_md.depth();
		//std::cout << "[m] before optimization, the md is " << md_before << "\n";
		uint32_t md_init = md_before;
		uint32_t md_after = md_before - 1;
		uint32_t ite_ctr{ 0u };
		mockturtle::md_rewriting_stats st;
		mockturtle::md_rewriting_stats* pst = &st;
		mockturtle::detail::md_rewriting_impl md_rewriting( "db_md_5", xag, ps, pst );
		uint32_t mc_init{ 0u };
		xag.foreach_gate( [&]( auto const& n ) {
			if ( xag.is_and( n ) )
			{
				++mc_init;
			}
		} );
		uint32_t mc_after{ 0u };
		//std::cout << "[m] before optimization, the mc is " << mc_init << "\n";
		clock_t begin_time = clock();

		if ( false ) {
		//auto ctr_upper{ 3u };
		//while ( ite_ctr <= ctr_upper )
		while ( md_before > md_after )
		{
			if ( ite_ctr > 0 )
			{
				md_before = md_after;
			}
			//std::cout << "[m] round " << ite_ctr << ": \n";
			++ite_ctr;

			xag = md_rewriting.run();
			xag_md.update_levels();
			md_after = xag_md.depth();
		}}

		if ( true ) {
			xag = md_rewriting.run2();
			xag_md.update_levels();
			md_after = xag_md.depth();
		}

		const auto cec = abc_cec( xag, benchmark );
		if ( !cec )
		{
			std::cout << "[e] not equivalent\n";
			abort();
		}
		xag.foreach_gate( [&]( auto const& n ) {
			if ( xag.is_and( n ) )
			{
				++mc_after;
			}
		} );
		float improve_local = ( ( static_cast<float>( md_init ) - static_cast<float>( md_after ) ) / static_cast<float>( md_init ) ) * 100;
		float improve_global = ( ( static_cast<float>( md_best ) - static_cast<float>( md_after ) ) / static_cast<float>( md_best ) ) * 100;
		exp_res( benchmark, mc_init, mc_after, md_init, md_after, md_best, improve_local, improve_global, ite_ctr, ( float( clock() - begin_time ) / CLOCKS_PER_SEC ), cec );
	}

	exp_res.save();
	exp_res.table();
}