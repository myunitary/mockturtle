#include <lorina/verilog.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_dot.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/views/ownership_view.hpp>

static const std::string benchmark_to_visualize[] = {
  // "bitonic_sort_inc_1_4",
  // "bitonic_sort_inc_1_4_ideal"
  // "bitonic_sort_inc_1_8"
  "bitonic_sort_inc_1_4_1",
  "bitonic_sort_inc_1_4_2",
  "bitonic_sort_inc_1_4_3"
};

using mpc_network = mockturtle::owner_view<mockturtle::xag_network>;

class mpc_network_dot_drawer : public mockturtle::default_dot_drawer<mpc_network>
{
public:
  virtual std::string node_label( mpc_network const& ntk, mpc_network::node const& n ) const override
  {
    if ( ntk.is_and( n ) )
    {
      return "AND";
    }

    if ( ntk.is_xor( n ) )
    {
      return "XOR";
    }

    return mockturtle::default_dot_drawer<mpc_network>::node_label( ntk, n );
  }

  virtual std::string node_fillcolor( mpc_network const& ntk, mpc_network::node const& n ) const override
  {
    uint32_t party_id = ntk.is_local( n );
    if ( party_id > 4u && !ntk.is_constant( n ) )
    {
      fmt::print( "[e] Node n{}'s party ID is {}!\n", ntk.node_to_index( n ), party_id );
    }
    assert( party_id <= 4u || ntk.is_constant( n ) );
    switch ( party_id )
    {
      case 0u:
        return "orange";
      case 1u:
        return "red";
      case 2u:
        return "skyblue";
      case 3u:
        return "green";
      case 4u:
        return "purple";
      default:
        return "white";
    }
  }
};

void write_dot( mpc_network const& ntk, std::ostream& os, mpc_network_dot_drawer const& drawer )
{
  std::stringstream nodes, edges, levels;
  std::vector<std::vector<uint32_t>> level_to_node_indexes;

  ntk.foreach_node( [&]( auto const& n ) {
    nodes << fmt::format( "{} [label=\"{}\",shape={},style=filled,fillcolor={}]\n",
                          ntk.node_to_index( n ),
                          drawer.node_label( ntk, n ),
                          drawer.node_shape( ntk, n ),
                          drawer.node_fillcolor( ntk, n ) );
    if ( !ntk.is_constant( n ) && !ntk.is_ci( n ) )
    {
      ntk.foreach_fanin( n, [&]( auto const& f ) {
        if ( !drawer.draw_signal( ntk, n, f ) )
        {
          return true;
        }

        edges << fmt::format( "{} -> {} [style={}]\n",
                              ntk.node_to_index( ntk.get_node( f ) ),
                              ntk.node_to_index( n ),
                              drawer.signal_style( ntk, f ) );
        return true;
      } );
    }

    const auto lvl = drawer.node_level( ntk, n );
    if ( level_to_node_indexes.size() <= lvl )
    {
      level_to_node_indexes.resize( lvl + 1 );
    }
    level_to_node_indexes[lvl].push_back( ntk.node_to_index( n ) );
  } );

  for ( auto const& indexes : level_to_node_indexes )
  {
    levels << "{rank = same; ";
    std::copy( indexes.begin(), indexes.end(), std::ostream_iterator<uint32_t>( levels, "; " ) );
    levels << "}\n";
  }

  levels << "{rank = same; ";
  ntk.foreach_po( [&]( auto const& f, auto i ) {
    nodes << fmt::format( "po{} [shape={},style=filled,fillcolor={}]\n", i, drawer.po_shape( ntk, i ), drawer.po_fillcolor( ntk, i ) );
    edges << fmt::format( "{} -> po{} [style={}]\n",
                          ntk.node_to_index( ntk.get_node( f ) ),
                          i,
                          drawer.signal_style( ntk, f ) );
    levels << fmt::format( "po{}; ", i );
  } );
  levels << "}\n";

  os << "digraph {\n"
     << "rankdir=BT;\n"
     << nodes.str() << edges.str() << levels.str() << "}\n";
}

void write_dot( mpc_network const& ntk, std::string const& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_dot( ntk, os, mpc_network_dot_drawer{} );
  os.close();
}

int main()
{
  // for ( auto i = 0u; i < std::size( benchmark_to_visualize ); ++i )
  for ( auto i = 0u; i < 1u; ++i )
  {
    // std::string benchmark = benchmark_to_visualize[i];
    std::string benchmark = "encoder_32bit_1cc_firstframe";
    // uint8_t bitwidth = 1 << i;
    // uint8_t bitwidth = 1u;
    uint32_t num_parties = 2u;
    fmt::print( "[i] Processing {}...\n", benchmark );
    mockturtle::xag_network ntk;
    const std::string path = "../experiments/tiny_garble/mc_opt/" + benchmark + ".v";
    const auto read_stats = lorina::read_verilog( ( path ), mockturtle::verilog_reader( ntk, benchmark ) );
    if ( read_stats != lorina::return_code::success )
    {
      fmt::print( "[e] Cannot access {}... Failed to read benchmark...\n", path );
      abort();
    }
    fmt::print( "[i] Read benchmark successfully.\n" );

    // std::array<uint32_t, 8u> pis_ownership_per_word = { 1u, 1u, 1u, 1u, 2u, 2u, 2u, 2u };
    // std::array<uint32_t, 8u> pis_ownership_per_word = { 1u, 1u, 1u, 2u, 1u, 2u, 2u, 2u };
    // std::array<uint32_t, 8u> pis_ownership_per_word = { 1u, 1u, 2u, 2u, 1u, 1u, 2u, 2u };
    // std::array<uint32_t, 8u> pis_ownership_per_word = { 1u, 2u, 1u, 2u, 1u, 2u, 1u, 2u };
    // std::array<uint32_t, 4u> pis_ownership_per_word = { 1u, 1u, 2u, 2u };
    // std::array<uint32_t, 4u> pis_ownership_per_word = { 1u, 2u, 1u, 2u };
    // std::vector<uint32_t> pis_ownership_per_bit( pis_ownership_per_word.size() * bitwidth );
    // for ( auto j{ 0u }; j < pis_ownership_per_word.size(); ++j )
    // {
    //   for ( auto k{ 0u }; k < bitwidth; ++k )
    //   {
    //     pis_ownership_per_bit[j * bitwidth + k] = pis_ownership_per_word[j];
    //   }
    // }
    // mpc_network ntk_mpc( ntk, num_parties, pis_ownership_per_bit, true );
    mpc_network ntk_mpc( ntk, num_parties );
    write_dot( ntk_mpc, benchmark + ".dot" );
    std::system( fmt::format( "dot -Tpng -o {}.png {}.dot", benchmark, benchmark ).c_str() );
    std::system( fmt::format( "rm {}.dot", benchmark ).c_str() );
    // std::system( fmt::format( "open {}.png", benchmark ).c_str() );

    fmt::print( "[i] Visualize {} successfully.\n", benchmark );
  }

  return 0u;
}