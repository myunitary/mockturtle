#include <mockturtle/algorithms/xag_decomp_max_local.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_bench.hpp>

static const std::string mpc_two_party_benchmark[] = {
  "voting_N_1_M_3_opt", "voting_N_1_M_4_opt"
  // "voting_N_1_M_3", "voting_N_1_M_4"
};

std::vector<std::string> mpc_two_party_benchmarks()
{
  std::vector<std::string> benchmarks{};
  for ( auto i{ 0u }; i < 2u; ++i )
  {
    benchmarks.push_back( mpc_two_party_benchmark[i] );
  }
  return benchmarks;
}

std::string benchmark_path( std::string const& benchmark_name )
{
  return fmt::format( "../experiments/mpc_benchmarks/{}.v", benchmark_name );
}

bool abc_cec( mockturtle::xag_network const& ntk, std::string const& benchmark )
{
  mockturtle::write_bench( ntk, "/tmp/test.bench" );
  std::string command = fmt::format( "/mnt/d/GitHub/abc/build/abc -q \"cec -n {} /tmp/test.bench\"", benchmark_path( benchmark ) );

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
  auto const benchmarks = mpc_two_party_benchmarks();
  std::string db_name = "xag_mc_min_6_db";
  for ( auto const& benchmark : benchmarks )
  {
    if ( benchmark == "voting_N_1_M_3_opt" || benchmark == "voting_N_1_M_3" )
    {
      continue;
    }

    fmt::print( "[i] Processing {}...\n", benchmark );

    mockturtle::xag_network ntk;
    const auto read_stats = lorina::read_verilog( benchmark_path( benchmark ), mockturtle::verilog_reader( ntk, benchmark ) );
    if ( read_stats != lorina::return_code::success )
    {
      fmt::print( "[e] Failed to read benchmark...\n" );
      abort();
    }
    fmt::print( "[i] Read benchmark successfully.\n" );

    const uint32_t num_parties = 2u;
    mockturtle::xag_decomp_max_local_params ps;
    mockturtle::xag_decomp_max_local_stats st;
    mockturtle::xag_decomp_max_local decomp( num_parties, ntk, db_name, ps, st );
    mockturtle::xag_network ntk_opt = decomp.run();

    const bool eq = abc_cec( ntk_opt, benchmark );
    if ( !eq )
    {
      fmt::print( "[e] Fatal error: non-equivalence!\n" );
    }
    else
    {
      fmt::print( "[i] #cuts = {}, #decomp trials = {}, #suc. decomp = {}, #better local impl. = {}\n", st.num_cuts, st.num_decomp, st.num_decomp_success, st.num_better_impl );
      fmt::print( "[i] original cost = {}, optimized cost = {}\n", st.cost_original, st.cost_optimal );
    }

    fmt::print( "\n" );
  }

  return 0;
}