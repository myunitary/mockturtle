#include <cstdint>
#include <optional>
#include <vector>

#include <bill/sat/interface/common.hpp>
#include <bill/sat/interface/abc_bsat2.hpp>
#include <fmt/format.h>
#include "../networks/xag.hpp"
#include "../utils/stopwatch.hpp"
#include "../views/cnf_view.hpp"

namespace mockturtle
{
struct and_forest_exact_syn_params
{
  uint32_t num_vars{ 3u };
  /* reserve for future extension */
  // std::vector<uint32_t> excluded_monos{};
  uint32_t conflict_limit{ 0u };
  std::optional<std::string> write_dimacs{};
  bool verbose{ false };
};

struct and_forest_exact_syn_stats
{
  stopwatch<>::duration time{};
  uint32_t num_vars{};
  uint32_t num_clauses{};
  
  void report()
  {
    fmt::print( "[i] Total time    = {:>5.2f} secs\n", to_seconds( time ) );
    fmt::print( "[i] Total vars    = {}\n", num_vars );
    fmt::print( "[i] Total clauses = {}\n", num_clauses );
  }
};

namespace detail
{

template<class Ntk, bill::solvers Solver>
struct and_forest_exact_synthesis_impl
{
  using cnf_view_network = cnf_view<Ntk, false, Solver>;

  and_forest_exact_synthesis_impl( and_forest_exact_syn_params const& ps = {}, and_forest_exact_syn_stats& st = {} )
    : _num_vars( ps.num_vars ),
      _num_monos( 1 << _num_vars - 1u ),
      // _excluded_monos( ps.excluded_monos ),
      _ps( ps ),
      _st( st )
  {}

private:
  void init( cnf_view_network& cvntk )
  {
    /* declare function variables */
    _func_vars.clear();
    _func_vars.resize( _num_monos );
    for ( auto& _func_vars_each_node : _func_vars )
    {
      _func_vars_each_node.resize( _num_vars );
    }

    /* assign value for function variables related to PIs */
    for ( uint8_t i{ 0u }; i <_num_vars; ++i )
    {
      auto& vars = _func_vars[i];
      for ( uint8_t j{ 0u }; j < vars.size(); ++j )
      {
        vars[j] = ( j == i ) ? cvntk.get_constant( true ) : cvntk.get_constant( false );
      }
    }

    /* declare connectivity variables */
    _conn_vars.clear();
    _conn_vars.resize( _num_monos - _num_vars );
    for ( auto& _conn_vars_each_node : _conn_vars )
    {
      _conn_vars_each_node.resize( _num_monos );
      std::generate( _conn_vars_each_node.begin(), _conn_vars_each_node.end(), [&]() { return cvntk.create_pi(); } );
    }
  }

  void add_two_fan_in_constraint( cnf_view_network& cvntk, std::vector<signal<cnf_view_network>> const& c_vars, uint32_t ind )
  {
    std::vector<signal<cnf_view_network>> products;
    /* ensure each fan-in is from a lower logical level */
    for ( uint32_t v1{ 0u }; v1 < ind - 1; ++v1 )
    {
      for ( uint32_t v2{ v1 + 1u }; v2 < ind; ++v2 )
      {
        std::vector<signal<cnf_view_network>> each_product{};
        for ( uint32_t i{ 0u }; i < c_vars.size(); ++i )
        {
          if ( i == v1 || i == v2 )
          {
            each_product.emplace_back( c_vars[i] );
          }
          else
          {
            each_product.emplace_back( cvntk.create_not( c_vars[i] ) );
          }
        }
        products.emplace_back( cvntk.create_nary_and( each_product ) );
      }
    }
    cvntk.create_po( cvntk.create_nary_or( products ) );
  }

  void add_ensure_reached_constraint( cnf_view_network& cvntk, uint32_t mono )
  {
    std::vector<bool> mono_bi( _num_vars );
    for ( uint8_t i{ 0u }; i < _num_vars; ++i, mono >>= 1 )
    {
      mono_bi[i] = mono & 1;
    }

    std::vector<signal<cnf_view_network>> products( _num_monos );
    for ( uint32_t j { 0u }; j < _num_monos; ++j )
    {
      std::vector<signal<cnf_view_network>> each_product( _num_vars );
      for ( uint8_t i{ 0u }; i <_num_vars; ++i )
      {
        each_product[i] = mono_bi[i] ? _func_vars[j][i] : cvntk.create_not( _func_vars[j][i] );
      }
      products[j] = cvntk.create_nary_and( each_product );
    }
    cvntk.create_po( cvntk.create_nary_or( products ) );
  }

  void add_constraints( cnf_view_network& cvntk )
  {
    /* assign value for function and connectivity variables related to 2-dim monomials */
    uint32_t cnt{ 0u };
    for ( uint8_t v1{ 0u }; v1 < _num_vars - 1u; ++v1 )
    {
      for ( uint8_t v2{ v1 + 1u }; v2 < _num_vars; ++v2, ++cnt )
      {
        auto& f_vars = _func_vars[_num_vars + cnt];
        auto& c_vars = _conn_vars[cnt];
        for ( auto& c_vars_each_conn: c_vars )
        {
          c_vars_each_conn = cvntk.get_constant( false );
        }
        for ( uint8_t j{ 0u }; j < _num_vars; ++j )
        {
          if ( j == v1 || j == v2 )
          {
            f_vars[j] = cvntk.get_constant( true );
            c_vars[1 << j - 1] = cvntk.get_constant( true );
          }
        }
      }
    }

    for ( ; cnt < _num_monos; ++cnt )
    {
      auto& f_vars = _func_vars[_num_vars + cnt];
      auto& c_vars = _conn_vars[cnt];
      for ( uint8_t i{ 0u }; i < _num_vars; ++i )
      {
        std::vector<signal<cnf_view_network>> products;
        for ( uint32_t j{ 0u }; j < _num_monos; ++j )
        {
          if ( ( j + 1 ) >> i & 1 )
          {
            products.emplace_back( c_vars[j] );
          }
        }
        /* determine value for the function variables of remaining nodes */
        f_vars[i] = cvntk.create_nary_or( products );
      }

      /* add constraints to guarantee that exactly two connectivity variables of a node are true */
      add_two_fan_in_constraint( cvntk, c_vars, ( _num_vars + cnt ) );
    }

    for ( uint32_t i{ 1u }; i <= _num_monos; ++i )
    {
      if ( __builtin_popcount( i ) > 2u )
      {
        add_ensure_reached_constraint( cvntk, i );
      }
    }
  }

  Ntk extract_ntk( cnf_view_network const& cvntk )
  {
    Ntk ntk;
    std::vector<signal<Ntk>> nodes( _num_vars );
    std::generate( nodes.begin(), nodes.end(), [&]() { return ntk.create_pi(); } );
    for ( auto const& _conn_vars_each_node : _conn_vars )
    {
      std::vector<signal<Ntk>> fan_ins;
      for ( uint32_t i{ 0u }; i < _conn_vars_each_node.size(); ++i )
      {
        if ( cvntk.model_value( _conn_vars_each_node[i] ) )
        {
          fan_ins.emplace_back( nodes[i] );
        }
      }
      assert( fan_ins.size() == 2u );
      signal<Ntk> new_node = ntk.create_nary_and( fan_ins );
      nodes.emplace_back( new_node );
      ntk.create_po( new_node );

      fmt::print( "[m] The fan-ins of Node {0} is Node {1} and Node {2}.\n", static_cast<uint32_t>( ntk.get_node( new_node ) ), static_cast<uint32_t>( ntk.get_node( fan_ins[0] ) ), static_cast<uint32_t>( ntk.get_node( fan_ins[1] ) ) );
    }
    return ntk;
  }

  std::optional<Ntk> solve( cnf_view_network& cvntk )
  {
    _st.num_vars += cvntk.num_vars();
    _st.num_clauses += cvntk.num_clauses();

    bill::result::clause_type assumptions;
    cvntk.foreach_po( [&]( auto const& f ) {
      assumptions.emplace_back( cvntk.lit( f ) );
    } );
    const auto res = cvntk.solve( assumptions, _ps.conflict_limit );
    if ( res && *res )
    {
      return extract_ntk( cvntk );
    }
    else{
      return std::nullopt;
    }
  }
  
public:
  Ntk run()
  {
    stopwatch<> t( _st.time );
    Ntk ntk;
    cnf_view_params cvps;
    cvps.write_dimacs = _ps.write_dimacs;
    cnf_view_network cvntk( cvps );
    init( cvntk );
    add_constraints( cvntk );
    auto res = solve( cvntk );
    if ( res )
    {
      return *res;
    }
    else
    {
      fmt::print( "[e] No solution found.\n" );
      abort();
    }
  }
private:
  uint32_t _num_vars;
  uint32_t _num_monos;
  // std::vector<uint32_t> const& _excluded_monos;
  std::vector<std::vector<signal<cnf_view_network>>> _func_vars;
  std::vector<std::vector<signal<cnf_view_network>>> _conn_vars;
  and_forest_exact_syn_params const& _ps;
  and_forest_exact_syn_stats& _st;
};

} /* namespace detail */

template<class Ntk = xag_network, bill::solvers Solver = bill::solvers::bsat2>
Ntk and_forest_exact_synthesis( and_forest_exact_syn_params const& ps = {}, and_forest_exact_syn_stats& st = {} )
{
  const Ntk ntk = detail::and_forest_exact_synthesis_impl<Ntk, Solver>{ ps, st }.run();
  
  if ( ps.verbose )
  {
    st.report();
  }
  
  return ntk;
}

} /* namespace mockturtle */