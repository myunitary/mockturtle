#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <optional>
#include <vector>

#include <bill/sat/interface/common.hpp>
#include <bill/sat/interface/glucose.hpp>
#include <kitty/constructors.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/stopwatch.hpp>
#include <mockturtle/views/cnf_view.hpp>

namespace mockturtle
{

struct md_optimal_exact_synthesis_params
{
	uint32_t conflict_limit{ 500000u };
	bool verbose{ false };
	bool verify_solution{ false };
};

struct md_optimal_exact_synthesis_stats
{
	stopwatch<>::duration time_total{};
	stopwatch<>::duration time_solving{};
	uint32_t num_vars{};
	uint32_t num_clauses{};

	void report() const
	{
		fmt::print( "[i] total time    = {:>5.2f} secs\n", to_seconds( time_total ) );
		fmt::print( "[i] solving time  = {:>5.2f} secs\n", to_seconds( time_solving ) );
    fmt::print( "[i] total vars    = {}\n", num_vars );
    fmt::print( "[i] total clauses = {}\n", num_clauses );
	}
};

namespace detail
{

template<class Ntk, bill::solvers Solver>
struct md_optimal_exact_synthesis_impl
{
	using problem_network_t = cnf_view<xag_network, false, Solver>;

public:
	md_optimal_exact_synthesis_impl( kitty::dynamic_truth_table const& func, std::vector<uint32_t> const& and_ladder,
																	 md_optimal_exact_synthesis_params const& ps, md_optimal_exact_synthesis_stats& st )
		: num_vars_( func.num_vars() ), 
			func_( kitty::get_bit( func, 0 ) ? ~func : func ), 
			invert_( kitty::get_bit( func, 0 ) ), 
			and_ladder_( and_ladder ), 
			ps_( ps ), 
			st_( st )
	{
	}

	std::optional<Ntk> run()
	{
		stopwatch<> time_total( st_.time_total );

		Ntk ntk;
		cnf_view_params cvps;
		problem_network_t pntk( cvps );
		reset( pntk );
		encode( pntk );

		if ( const auto sat = solve( pntk ); sat )
		{
			const auto sol = extract_network( pntk );
			return sol;
		}
		return std::nullopt;
	}

private:
	void reset( problem_network_t const& pntk )
	{
		ltfi_vars_.clear();
		truth_vars_.clear();
		truth_vars_.resize( func_.num_bits() );

		for ( auto i{ 0u }; i < num_vars_; ++i )
		{
			const auto var_tt = kitty::nth_var<kitty::dynamic_truth_table> ( num_vars_, i );
			for ( auto b{ 0u }; b < func_.num_bits(); ++b )
			{
				truth_vars_[b].emplace_back( pntk.get_constant( kitty::get_bit( var_tt, b ) ) );
			}
		}
	}

	void encode( problem_network_t& pntk )
	{
		/* add essential constraints */
		add_gate( pntk );
		add_output( pntk );
		add_func_constraint( pntk );
		add_struct_constraint( pntk );

		/* add extra constraints to prune searching space */

		st_.num_vars += pntk.num_vars();
		st_.num_clauses += pntk.num_clauses();
	}

	void add_gate( problem_network_t& pntk )
	{
		uint32_t i_ext{ 0u };
		for ( auto ands_each_depth: and_ladder_ )
		{
			for ( auto i_int{ 0u }; i_int < ands_each_depth; ++i_int )
			{
				for ( auto j{ 0u }; j < 2u; ++j )
				{
					ltfi_vars_.emplace_back( std::vector<signal<problem_network_t>>( num_vars_ + i_ext ) );
					/* create single selection variables */
					std::generate( ltfi_vars_.back().begin(), ltfi_vars_.back().end(), [&]() { return pntk.create_pi(); } );
				}
			}
			i_ext += ands_each_depth;
		}
	}

	void add_output( problem_network_t& pntk )
	{
		ltfi_vars_.emplace_back( std::vector<signal<problem_network_t>>( num_vars_ + ltfi_vars_.size() / 2 ) );
		std::generate( ltfi_vars_.back().begin(), ltfi_vars_.back().end(), [&]() { return pntk.create_pi(); } );
	}

	void add_func_constraint( problem_network_t& pntk )
	{
		const auto create_xor_clause = [&]( std::vector<signal<problem_network_t>> const& ltfi_vars, uint32_t bit ) -> signal<problem_network_t> {
      std::vector<signal<problem_network_t>> ltfi( ltfi_vars.size() );
      for ( auto j{ 0u }; j < ltfi.size(); ++j )
      {
        ltfi[j] = pntk.create_and( ltfi_vars[j], truth_vars_[bit][j] );
      }
      return pntk.create_nary_xor( ltfi );
    };

    for ( auto b{ 1u }; b < func_.num_bits(); ++b )
    {
    	for ( auto i = 0u; i < ltfi_vars_.size() / 2; ++i )
	    {
	      truth_vars_[b].emplace_back( pntk.create_and( create_xor_clause( ltfi_vars_[2 * i], b ), create_xor_clause( ltfi_vars_[2 * i + 1], b ) ) );
	    }

	    const auto po_signal = create_xor_clause( ltfi_vars_.back(), b );
      pntk.create_po( kitty::get_bit( func_, b ) ? po_signal : pntk.create_not( po_signal ) );
    }
	}

	void add_struct_constraint( problem_network_t& pntk )
	{
		/* (1) for any AND2 with a certain MD d, at least one of its fanins is an AND2 */
		/* with the MD of d-1                                                          */
		uint32_t range_begin = num_vars_;
		uint32_t range_end = num_vars_ + and_ladder_[0];
		uint32_t i_ext{ and_ladder_[0] };
		for ( auto d{ 1u }; d < and_ladder_.size() - 1u; ++d )
		{
			for ( auto i_int{ 0u }; i_int < and_ladder_[d]; ++i_int )
			{
				std::vector<signal<problem_network_t>> ltfi1;
				std::vector<signal<problem_network_t>> ltfi2;
				for ( auto j{ range_begin }; j < range_end; ++j )
				{
					ltfi1.emplace_back( ltfi_vars_[2 * i_ext][j] );
					ltfi2.emplace_back( ltfi_vars_[2 * i_ext + 1][j] );
				}
				pntk.create_or( pntk.create_nary_or( ltfi1 ), pntk.create_nary_or( ltfi2 ) );
				++i_ext;
			}
		}

		/* for any AND2 with a certain MD d, none of the AND2 with the same MD can be  */
		/* its fanin, which is naturally avoided by removing those selection variables */
	}

	bool solve( problem_network_t& pntk )
	{
		stopwatch<> time_solving( st_.time_solving );
    bill::result::clause_type assumptions;

    pntk.foreach_po( [&]( auto const& f ) {
      assumptions.emplace_back( pntk.lit( f ) );
    } );

    const auto res = pntk.solve( assumptions, ps_.conflict_limit );

    if ( res && *res )
    {
      return true;
    }

    return false;
	}

	Ntk extract_network( problem_network_t& pntk )
  {
    Ntk ntk;
    std::vector<signal<Ntk>> nodes( num_vars_ );
    std::generate( nodes.begin(), nodes.end(), [&]() { return ntk.create_pi(); } );

    const auto extract_ltfi = [&]( std::vector<signal<problem_network_t>> const& ltfi_vars ) ->signal<Ntk> {
      std::vector<signal<Ntk>> ltfi;
      for ( auto j{ 0u }; j < ltfi_vars.size(); ++j )
      {
        if ( pntk.model_value( ltfi_vars[j] ) )
        {
          ltfi.emplace_back( nodes[j] );
          //std::cout << ( nodes[j].complement ? "!" : "" ) << nodes[j].index << " ";
        }
      }
      //std::cout << "\n";
      return ntk.create_nary_xor( ltfi );
    };

    for ( auto i = 0u; i < ltfi_vars_.size() / 2; ++i )
    {
      nodes.emplace_back( ntk.create_and( extract_ltfi( ltfi_vars_[2 * i] ), extract_ltfi( ltfi_vars_[2 * i + 1] ) ) );
    }

    const auto po = extract_ltfi( ltfi_vars_.back() );
    ntk.create_po( invert_ ? ntk.create_not( po ) : po );

    if ( ps_.verify_solution )
    {
      auto result = simulate<kitty::static_truth_table<5u>>( ntk )[0];
      const std::string result_str = kitty::to_hex( result ).substr( 0, 1 << ( num_vars_ - 2 ) );
      const std::string func_str = kitty::to_hex( invert_ ? ~func_ : func_ );
      if ( result_str != func_str )
      {
        std::cerr << "[e] incorrect implementation for " << func_str << "; "
                  << "whose function is " << result_str << std::endl;
        abort();
      }
      else
      {
        std::cout << "[m] the solution is verified to be correct\n";
      }
    }

    return ntk;
  }

private:
	uint32_t num_vars_;
	kitty::dynamic_truth_table func_;
	std::vector<uint32_t> const& and_ladder_;
	bool invert_{ false };
	std::vector<std::vector<signal<problem_network_t>>> ltfi_vars_;
	std::vector<std::vector<signal<problem_network_t>>> truth_vars_;
	md_optimal_exact_synthesis_params const& ps_;
	md_optimal_exact_synthesis_stats& st_;
};

} /* detail */

template<class Ntk = xag_network, bill::solvers Solver = bill::solvers::glucose_41>
std::optional<Ntk> md_optimal_exact_synthesis( kitty::dynamic_truth_table const& func,
																							 std::vector<uint32_t> const& and_ladder,
																							 md_optimal_exact_synthesis_params const& ps = {}, 
																							 md_optimal_exact_synthesis_stats* pst = nullptr )
{
	md_optimal_exact_synthesis_stats st;
	const auto pres = detail::md_optimal_exact_synthesis_impl<Ntk, Solver>{ func, and_ladder, ps, st }.run();
	
	if ( ps.verbose )
	{
		st.report();
	}
	if ( pst )
	{
		*pst = st;
	}

	return pres;
}

} /* mockturtle */