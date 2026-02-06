#include <lorina/aiger.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/mapper.hpp>
#include <mockturtle/algorithms/node_resynthesis/bidecomposition.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_minmc.hpp>
#include <mockturtle/algorithms/refactoring.hpp>
#include <mockturtle/algorithms/rewrite.hpp>
#include <mockturtle/algorithms/xag_optimization.hpp>
#include <mockturtle/algorithms/xag_resub.hpp>
#include <mockturtle/algorithms/xag_resub_withDC.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/bristol_reader.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/views/fanout_view.hpp>
#include <mockturtle/views/depth_view.hpp>

#include <chrono>
#include <fstream>
#include <iomanip>

#include <experiments.hpp>

const uint32_t FBS_cost[] = {
  29u, 32u, 39u, 42u, 43u, 44u, 45u
};

static const std::string  float_arith_benchmark[] = {
  "FP-add", "FP-div", "FP-eq", "FP-lt", "FP-mul"
};

static const std::string tiny_garble_benchmark[] = {
  // "aes_1cc_firstframe",
  // "hamming_32bit_1cc_firstframe",
  // "Lite_MIPS_firstframe",
  // "mult_32bit_1cc_firstframe",
  // "sum_8bit_1cc_firstframe",
  "comparator_32bit_unsigned_lt",
  "encoder_32bit_1cc_firstframe",
  "sha3_24cc_firstframe"
};

std::vector<std::string> tiny_garble_benchmarks()
{
  std::vector<std::string> result;
  for ( auto i = 0u; i < std::size( tiny_garble_benchmark ); ++i )
  {
    result.emplace_back( tiny_garble_benchmark[i] );
  }

  return result;
}

const std::string PLDI_benchmarks[] = {
  "cardio", "dsort", "msort", "isort", "bsort", "osort", "hd01", "hd02", "hd03", 
  "hd04", "hd05", "hd06", "hd07", "hd08", "hd09", "hd10", "hd11", "hd12",
  "bar", "cavlc", "ctrl", "dec", "i2c", "int2float", "router"
};

std::vector<std::string> pldi_benchmarks()
{
  std::vector<std::string> result;
  for ( auto i = 0u; i < 25u; ++i )
  {
    result.emplace_back( PLDI_benchmarks[i] );
  }

  return result;
}

const std::string CRYPTO_benchmarks[] = {
  "aes_sbox", "ascon_lut", "kreyvium_iter_v1", "kreyvium_stream_v1",
  "simon_iter", "trivium_iter_v1", "trivium_stream_v1"
};

std::vector<std::string> cryptographic_benchmarks()
{
  std::vector<std::string> result;
  for ( auto i = 0u; i < 7u; ++i )
  {
    result.emplace_back( CRYPTO_benchmarks[i] );
  }

  return result;
}

const std::string BRISTOL_benchmarks[] = {
  "AES-non-expanded", "aes_128", "aes_192",
  "aes_256", "sha256", "sha512", "Keccak_f"
};

std::vector<std::string> bristol_benchmarks()
{
  std::vector<std::string> result;
  for ( auto i = 0u; i < 7u; ++i )
  {
    result.emplace_back( BRISTOL_benchmarks[i] );
  }

  return result;
}

std::string benchmark_path( std::string const& benchmark_suite, std::string const& benchmark_name )
{
  return fmt::format( "../experiments/{}_benchmarks/{}.aig", benchmark_suite, benchmark_name );
}

std::string benchmark_path_v( std::string const& benchmark_suite, std::string const& benchmark_name )
{
  return fmt::format( "../experiments/{}_benchmarks/{}.v", benchmark_suite, benchmark_name );
}

std::string full_benchmark_path_v( std::string const& benchmark_suite, std::string const& benchmark_name )
{
  return fmt::format( "../experiments/{}/{}.v", benchmark_suite, benchmark_name );
}

std::string full_benchmark_path_txt( std::string const& benchmark_suite, std::string const& benchmark_name )
{
  return fmt::format( "../experiments/{}/{}.txt", benchmark_suite, benchmark_name );
}

struct mc_count
{
  uint32_t operator()( mockturtle::xag_network const& ntk, mockturtle::xag_network::node const& n ) const
  {
    return ntk.is_and( n ) ? 1u : 0u;
  }
};

struct free_xor_cost
{
  uint32_t operator()( mockturtle::xag_network const& ntk, mockturtle::xag_network::node const& n ) const
  {
    return ntk.is_xor( n ) ? 0u : 1u;
  }
};

void cec( mockturtle::xag_network const& ntk, std::string const& benchmark_suite, std::string const& benchmark )
{
  mockturtle::write_bench( ntk, "/tmp/test.bench" );
  std::string command = fmt::format( "/mnt/d/GitHub/abc/build/abc -q \"cec -n {} /tmp/test.bench\"", full_benchmark_path_v( benchmark_suite, benchmark ) );

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

  if ( result.size() < 23 || result.substr( 0u, 23u ) != "Networks are equivalent" )
  {
    fmt::print( "[e] Equivalence checking failed...\n" );
    abort();
  }
}

void save_verilog( mockturtle::xag_network const& ntk, std::string const& benchmark, std::string const& dir )
{
  mockturtle::write_bench( ntk, "/tmp/tmp.bench" );
  std::string command = fmt::format( "/mnt/d/GitHub/abc/build/abc -q \"read /tmp/tmp.bench; write_verilog {}_opt.v\"", ( dir + benchmark ) );
  std::unique_ptr<FILE, decltype( &pclose )> pipe( popen( command.c_str(), "r" ), pclose );

  if ( !pipe )
  {
    throw std::runtime_error( "popen() failed" );
  }
}

int main()
{
  using namespace mockturtle;
  experiments::experiment<std::string, uint32_t, uint32_t> exp( "xag_mc_opt_float_div", "benchmark", "#mc_init", "#mc_opt" );

  uint32_t mc_init{};
  uint32_t mc_opt{};
  uint32_t num_ite{};
  uint32_t mc_tmp{};

  // const std::vector<std::string> benchmarks = tiny_garble_benchmarks();
  // const std::string benchmark_suite = "tiny_garble";
  // const std::string benchmark_suite = "EPFL_benchmarks";
  const std::string benchmark_suite = "float_arith_benchmarks";
  // for ( auto const& benchmark : benchmarks )
  // for ( auto const& benchmark : experiments::epfl_benchmarks() )
  // for ( auto const& benchmark : experiments::iscas_benchmarks() )
  for ( auto const& benchmark : float_arith_benchmark )
  {
    if ( benchmark != "FP-div" )
    {
      continue;
    }
    // if ( benchmark != "aes_1cc_firstframe" && benchmark != "hamming_32bit_1cc_firstframe" )
    // {
    //   continue;
    // }
  	fmt::print( "[i] processing {}\n", benchmark );

    xag_npn_resynthesis<xag_network, xag_network, xag_npn_db_kind::xag_complete> xag_area_resyn;
    exact_library<xag_network> xag_area_lib( xag_area_resyn );

    map_params ps_map;
    ps_map.skip_delay_round = true;
    ps_map.required_time = std::numeric_limits<double>::max();
    map_stats st_map;

    resubstitution_params ps_resub;
    ps_resub.max_divisors = 100u;
    ps_resub.max_inserts = 4u;
    ps_resub.max_pis = 8u;
    ps_resub.progress = false;
    ps_resub.verbose = false;
    ps_resub.use_dont_cares = true;

    // cut_rewriting_params ps_old_rewrite;
    // ps_old_rewrite.cut_enumeration_ps.cut_size = 6;
    // ps_old_rewrite.cut_enumeration_ps.cut_limit = 12;
    // ps_old_rewrite.verbose = false;
    // ps_old_rewrite.progress = false;
    // ps_old_rewrite.min_cand_cut_size = 2u;

    xag_minmc_resynthesis xag_mc_resyn( "db.txt" );
    exact_library<xag_network> xag_mc_lib( xag_mc_resyn );

    refactoring_params ps_refactor;
    ps_refactor.verbose = false;
    ps_refactor.progress = false;
    ps_refactor.allow_zero_gain = false;
    ps_refactor.max_pis = 15u;
    ps_refactor.use_dont_cares = true;
    bidecomposition_resynthesis<xag_network> xag_bidec_resyn;

  	xag_network ntk;
  	// auto read_stats = lorina::read_verilog( full_benchmark_path_v( benchmark_suite, benchmark ), mockturtle::verilog_reader( ntk, benchmark ) );
    auto read_stats = lorina::read_bristol( full_benchmark_path_txt( benchmark_suite, benchmark ), mockturtle::bristol_reader( ntk ) );
    if ( read_stats != lorina::return_code::success )
    {
      fmt::print( "[e] failed to read benchmark {}\n", benchmark );
      continue;
    }
    // cec( ntk, benchmark_suite, benchmark );

    mc_init = 0u;
    // mc_old_tech = 0u;
    // mc_new_tech = 0u;
    mc_opt = 0u;
    num_ite = 0u;
    // num_switch = 0u;
    // num_comput = 0u;
    // for ( uint32_t& ele : and_trees_arity )
    // {
    //   ele = 0u;
    // }

    /* preprocessing benchmarks */
    // ntk = map( ntk, xag_area_lib, ps_map, &st_map );
    // rewrite( ntk, xag_area_lib );
    // ntk = cleanup_dangling( ntk );

    ntk.foreach_node( [&ntk, &mc_init]( auto const& n ) {
      if ( ntk.is_and( n ) )
      {
        ++mc_init;
      }
    } );
    // {
    //   depth_view<xag_network, mc_count, false> ntk_md{ ntk };
    //   md_init = ntk_md.depth();
    // }

    mc_tmp = mc_init;
    // const auto begin_time = std::chrono::high_resolution_clock::now();

    while ( mc_tmp > mc_opt )
    {
      if ( num_ite > 0u )
      {
        mc_tmp = mc_opt;
      }

      ++num_ite;
      mc_opt = 0u;

      /* Rewriting in DAC23 */
      rewrite( ntk, xag_mc_lib );
      ntk = cleanup_dangling( ntk );

      ntk.foreach_node( [&ntk, &mc_opt]( auto const& n ) {
        if ( ntk.is_and( n ) )
        {
          ++mc_opt;
        }
      } );
    }
    fmt::print( "[m] preprocessing phase finished.\n" );

    mc_tmp = mc_opt;
    mc_opt = 0u;
    num_ite = 0u;
    while ( mc_tmp > mc_opt )
    {
      fmt::print( "[m] in the {}-th round, 'mc_tmp' is {}, 'mc_opt' is {}.\n", num_ite, mc_tmp, mc_opt );
      if ( num_ite > 0u )
      {
        mc_tmp = mc_opt;
      }

      ++num_ite;
      mc_opt = 0u;

      /* Rewriting in DAC23 */
      rewrite( ntk, xag_mc_lib );
      ntk = cleanup_dangling( ntk );
      fmt::print( "[i] Finished the {} round of rewrite\n", num_ite );

      // /* Rewriting in DATE20 */
      // ntk = cut_rewriting<xag_network, xag_minmc_resynthesis, mc_count>( ntk, xag_mc_resyn, ps_old_rewrite, nullptr );
      // ntk = cleanup_dangling( ntk );

      if ( benchmark != "Lite_MIPS_firstframe" && benchmark != "hyp" && benchmark != "FP-div" )
      {
        refactoring( ntk, xag_bidec_resyn, ps_refactor, nullptr, free_xor_cost() );
        ntk = cleanup_dangling( ntk );
        fmt::print( "[i] Finished the {} round of refactor\n", num_ite );
      }
      
      /* Resubstitution in DATE20 */
      fanout_view<xag_network> ntk_fo{ ntk };
      depth_view<fanout_view<xag_network>> ntk_resub{ ntk_fo };
      resubstitution_minmc_withDC( ntk_resub, ps_resub );
      ntk = cleanup_dangling( ntk );
      fmt::print( "[i] Finished the {} round of resub\n", num_ite );

      if ( ntk.size() <= 10000u )
      {
        ntk = xag_dont_cares_optimization( ntk );
        fmt::print( "[i] Finished the {} round of dc opt\n", num_ite );
      }

      ntk.foreach_node( [&ntk, &mc_opt]( auto const& n ) {
        if ( ntk.is_and( n ) )
        {
          ++mc_opt;
        }
      } );

      if ( mc_opt >= mc_tmp )
      {
        fmt::print( "[m] mc reduction has reached saturation. terminating...\n" );
      }
    }

    // std::chrono::duration<float> duration = std::chrono::high_resolution_clock::now() - begin_time;
    // float runtime = duration.count();
    // cec( ntk, benchmark_suite, benchmark );
    save_verilog( ntk, benchmark, fmt::format( "../experiments/{}/", benchmark_suite ) );
    // std::string output_filename = fmt::format( "../experiments/tiny_garble/mc_opt/{}.v", benchmark );
    // std::ofstream ofs( output_filename );
    // if ( !ofs.is_open() )
    // {
    //   fmt::print( "[e] Failed to open file {}\n", output_filename );
    // }
    // mockturtle::write_verilog( ntk, ofs );

    // {
    //   depth_view<xag_network, mc_count, false> ntk_md{ ntk };
    //   md_opt = ntk_md.depth();
    // }
    
    exp( benchmark, mc_init, mc_opt );
  }

  exp.save();
  exp.table();

  return 0;
}