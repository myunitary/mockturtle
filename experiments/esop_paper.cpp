#include <lorina/aiger.hpp>
#include <mockturtle/algorithms/balancing.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/mapper.hpp>
// #include <mockturtle/algorithms/node_resynthesis/bidecomposition.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_minmc.hpp>
// #include <mockturtle/algorithms/refactoring.hpp>
#include <mockturtle/algorithms/xag_resub_withDC.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/views/fanout_view.hpp>
#include <mockturtle/views/depth_view.hpp>

#include <chrono>
#include <iomanip>

#include <experiments.hpp>

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
    if ( ntk.is_dead( n ) )
    {
      return 999u;
    }

    return ntk.is_xor( n ) ? 0u : 1u;
  }
};

static const std::string PLDI_benchmarks[] = {
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

std::string bench_path( std::string const& benchmark_name )
{
  return fmt::format( "../experiments/pldi_benchmarks/{}.v", benchmark_name );
}

void cec( mockturtle::xag_network const& ntk, std::string const& benchmark )
{
  mockturtle::write_bench( ntk, "/tmp/test.bench" );
  std::string abc_path = "/Users/myu/Documents/GitHub/abc/";
  std::string command = fmt::format( "{}abc -q \"cec -n {} /tmp/test.bench\"", abc_path, bench_path( benchmark ) );

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
  std::string abc_path = "/Users/myu/Documents/GitHub/abc/";
  std::string command = fmt::format( "{}abc -q \"read /tmp/tmp.bench; write_verilog {}.v\"", abc_path, ( dir + benchmark ) );
  std::unique_ptr<FILE, decltype( &pclose )> pipe( popen( command.c_str(), "r" ), pclose );

  if ( !pipe )
  {
    throw std::runtime_error( "popen() failed" );
  }
}

int main()
{
  using namespace experiments;
  using namespace mockturtle;
  bool rewrite_converge_first{ false };
  std::string postfix = ( rewrite_converge_first ? "_rewrite_converge_first" : "" );

  experiment<std::string, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, double> exp( ( "esop_paper" + postfix ), "benchmark", "#mc_init", "#md_init", "#mc_mc_opt", "#md_mc_opt", "#mc_esop", "#md_esop", "runtime" );

  uint32_t mc_init{};
  uint32_t md_init{};
  uint32_t mc_mc_opt{};
  uint32_t md_mc_opt{};
  uint32_t mc_esop{};
  uint32_t md_esop{};
  uint32_t mc_tmp{};
  uint32_t md_tmp{};

  std::vector<std::string> benchmarks = pldi_benchmarks();

  for ( auto const& benchmark : benchmarks )
  {
    // if ( benchmark != "bar" )
    // {
    //   continue;
    // }

    fmt::print( "[i] processing {}\n", benchmark );

    resubstitution_params ps_resub;
    ps_resub.max_divisors = 100u;
    ps_resub.max_inserts = 4u;
    ps_resub.max_pis = 8u;
    ps_resub.progress = false;
    ps_resub.verbose = false;
    ps_resub.use_dont_cares = true;

    cut_rewriting_params ps_rewrite;
    ps_rewrite.cut_enumeration_ps.cut_size = 6;
    ps_rewrite.cut_enumeration_ps.cut_limit = 12;
    ps_rewrite.verbose = false;
    ps_rewrite.progress = false;
    ps_rewrite.min_cand_cut_size = 2u;

    xag_minmc_resynthesis xag_mc_resyn( "../experiments/db" );
    exact_library<xag_network> xag_mc_lib( xag_mc_resyn );

    // refactoring_params ps_refactor;
    // ps_refactor.verbose = false;
    // ps_refactor.progress = false;
    // ps_refactor.allow_zero_gain = false;
    // ps_refactor.max_pis = 15u;
    // ps_refactor.use_dont_cares = true;
    // bidecomposition_resynthesis<xag_network> xag_bidec_resyn;

    xag_network ntk;
    auto read_stats = lorina::read_verilog( bench_path( benchmark ), mockturtle::verilog_reader( ntk ) );
    if ( read_stats != lorina::return_code::success )
    {
      fmt::print( "[e] failed to read benchmark {}\n", benchmark );
      continue;
    }

    mc_init = 0u;
    mc_mc_opt = 0u;
    mc_esop = 0u;
    md_init = 0u;
    md_mc_opt = 0u;
    md_esop = 0u;

    mc_init = costs<xag_network, mc_count>( ntk );
    {
      depth_view<xag_network, mc_count, false> ntk_md{ ntk };
      md_init = ntk_md.depth();
    }
    fmt::print( "[i] MC : {}; MD : {}\n", mc_init, md_init );

    mc_tmp = mc_init;
    const auto begin_time = std::chrono::high_resolution_clock::now();

    uint32_t num_ite{};
    if ( rewrite_converge_first )
    {
      while ( mc_tmp > mc_mc_opt )
      {
        if ( num_ite > 0u )
        {
          mc_tmp = mc_mc_opt;
        }
  
        ++num_ite;
        /* Rewriting in DATE20 */
        xag_network ntk_tmp = cut_rewriting<xag_network, xag_minmc_resynthesis, mc_count>( ntk, xag_mc_resyn, ps_rewrite, nullptr );
        ntk_tmp = cleanup_dangling( ntk_tmp );
  
        mc_mc_opt = costs<xag_network, mc_count>( ntk_tmp );
        depth_view<xag_network, mc_count, false> ntk_md{ ntk_tmp };
        md_mc_opt = ntk_md.depth();
        fmt::print( "[i] MC : {}; MD : {}\n", mc_mc_opt, md_mc_opt );
  
        if ( mc_mc_opt < mc_tmp )
        {
          ntk = ntk_tmp;
        }
      }
    }

    num_ite = 0u;
    mc_mc_opt = 0u;
    {
      while ( mc_tmp > mc_mc_opt )
      {
        if ( num_ite > 0u )
        {
          mc_tmp = mc_mc_opt;
        }
  
        ++num_ite;
        /* Rewriting in DATE20 */
        xag_network ntk_tmp = cut_rewriting<xag_network, xag_minmc_resynthesis, mc_count>( ntk, xag_mc_resyn, ps_rewrite, nullptr );
        ntk_tmp = cleanup_dangling( ntk_tmp );

        // /* Refactoring in DATE20 */
        // refactoring( ntk_tmp, xag_bidec_resyn, ps_refactor, nullptr, free_xor_cost() );
        // ntk_tmp = cleanup_dangling( ntk_tmp );
  
        /* Resubstitution in DATE20 */
        fanout_view<xag_network> ntk_fo{ ntk_tmp };
        depth_view<fanout_view<xag_network>> ntk_resub{ ntk_fo };
        resubstitution_minmc_withDC( ntk_resub, ps_resub );
        ntk_tmp = cleanup_dangling( ntk_tmp );
  
        mc_mc_opt = costs<xag_network, mc_count>( ntk_tmp );
        depth_view<xag_network, mc_count, false> ntk_md{ ntk_tmp };
        md_mc_opt = ntk_md.depth();
        fmt::print( "[i] MC : {}; MD : {}\n", mc_mc_opt, md_mc_opt );
  
        if ( mc_mc_opt < mc_tmp )
        {
          ntk = ntk_tmp;
        }
      }
    }

    mockturtle::balancing_params balance_ps;
    balance_ps.cut_enumeration_ps.cut_size = 6;
    balance_ps.cut_enumeration_ps.cut_limit = 12;
    balance_ps.only_on_critical_path = true;
    balance_ps.progress = true;

    num_ite = 0u;
    md_tmp = md_mc_opt;
    {
      while ( md_tmp > md_esop )
      {
        if ( num_ite > 0u )
        {
          md_tmp = md_esop;
        }
  
        ++num_ite;
        xag_network ntk_tmp = mockturtle::balancing<mockturtle::xag_network, mc_count>( ntk, mockturtle::esop_rebalancing<mockturtle::xag_network>{}, balance_ps );
        depth_view<xag_network, mc_count, false> ntk_md{ ntk_tmp };
        md_esop = ntk_md.depth();
        mc_esop = costs<xag_network, mc_count>( ntk_tmp );
        fmt::print( "[i] MC : {}; MD : {}\n", mc_esop, md_esop );
        if ( md_esop < md_tmp || ( md_esop == md_tmp && mc_esop < mc_tmp ) )
        {
          ntk = ntk_tmp;
        }
      }
    } 

    mc_esop = costs<xag_network, mc_count>( ntk );
    depth_view<xag_network, mc_count, false> ntk_md{ ntk };
    md_esop = ntk_md.depth();
    fmt::print( "[i] MC : {}; MD : {}\n", mc_esop, md_esop );

    std::chrono::duration<float> duration = std::chrono::high_resolution_clock::now() - begin_time;
    float runtime = duration.count();
    cec( ntk, benchmark );
    save_verilog( ntk, benchmark, "../experiments/benchmarks/epfl_esop_paper/" );

    exp( benchmark, mc_init, md_init, mc_mc_opt, md_mc_opt, mc_esop, md_esop, runtime );
  }

  exp.save();
  exp.table();

  return 0;
}