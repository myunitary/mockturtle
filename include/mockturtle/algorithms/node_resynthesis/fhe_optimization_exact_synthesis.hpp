#pragma once

#include <algorithm>
#include <cstdint>
#include <fmt/format.h>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <vector>

#include <bill/sat/interface/common.hpp>
#include <kitty/constructors.hpp>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <kitty/spectral.hpp>
#include <mockturtle/algorithms/fhe_optimization.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/utils/stopwatch.hpp>
#include <mockturtle/views/cnf_view.hpp>
#include <mockturtle/views/cut_view.hpp>

namespace mockturtle
{

struct fhe_optimization_exact_syn_params
{
	uint32_t conflict_limit{ 0u };
	bool use_advanced_constraints{ false };
	bool use_exisiting_cache{ false };
	bool verbose{ false };
	bool verify_solution{ false };
};

struct fhe_optimization_exact_syn_stats
{
	stopwatch<>::duration time_parse_cache{};
	stopwatch<>::duration time_solving{};
	// uint32_t num_vars{};
	// uint32_t num_clauses{};
	// uint32_t num_instances{};

	void report() const
	{
		fmt::print( "[i] solving time  = {:>5.2f} secs\n", to_seconds( time_solving ) );
		// fmt::print( "[i] total vars    = {}\n", num_vars );
		// fmt::print( "[i] total clauses = {}\n", num_clauses );
		// fmt::print( "[i] num instances = {}\n", num_instances );
	}
};

namespace detail
{

// A schedule indicates the depths, starting from which a variable is available
typedef std::vector<uint8_t> schedule;

typedef struct AND_fence
{
	std::vector<uint8_t> and_fence;
	uint8_t mc;
	uint8_t md;
	std::vector<uint8_t> baseline_scheduling_req;
} AND_fence;

static const std::vector<AND_fence> and_fences_lib = {
	{ { 1u }, 1u, 1u, { 2u } },
	{ { 2u }, 2u, 1u, { 3u } },
	{ { 3u }, 3u, 1u, { 3u } },
	{ { 4u }, 4u, 1u, { 3u } },
	{ { 5u }, 5u, 1u, { 3u } },
	{ { 6u }, 6u, 1u, { 3u } },
	{ { 7u }, 7u, 1u, { 3u } },
	{ { 8u }, 8u, 1u, { 3u } },
	{ { 9u }, 9u, 1u, { 3u } },
	{ { 10u }, 10u, 1u, { 4u } },
	{ { 11u }, 11u, 1u, { 4u } },
	{ { 12u }, 12u, 1u, { 4u } },
	{ { 13u }, 13u, 1u, { 4u } },
	{ { 14u }, 14u, 1u, { 4u } },
	{ { 15u }, 15u, 1u, { 4u } },
	{ { 1u, 1u }, 2u, 2u, { 2u, 0u } },
	{ { 1u, 2u }, 3u, 2u, { 2u, 0u } },
	{ { 2u, 1u }, 3u, 2u, { 3u, 0u } },
	{ { 1u, 3u }, 4u, 2u, { 2u, 0u } },
	{ { 2u, 2u }, 4u, 2u, { 3u, 0u } },
	{ { 3u, 1u }, 4u, 2u, { 3u, 0u } },
	{ { 1u, 4u }, 5u, 2u, { 2u, 0u } },
	{ { 2u, 3u }, 5u, 2u, { 3u, 0u } },
	{ { 3u, 2u }, 5u, 2u, { 3u, 0u } },
	{ { 4u, 1u }, 5u, 2u, { 3u, 0u } },
	{ { 1u, 5u }, 6u, 2u, { 2u, 0u } },
	{ { 2u, 4u }, 6u, 2u, { 3u, 0u } },
	{ { 3u, 3u }, 6u, 2u, { 3u, 0u } },
	{ { 4u, 2u }, 6u, 2u, { 3u, 0u } },
	{ { 5u, 1u }, 6u, 2u, { 3u, 0u } },
	{ { 1u, 6u }, 7u, 2u, { 2u, 0u } },
	{ { 2u, 5u }, 7u, 2u, { 3u, 0u } },
	{ { 3u, 4u }, 7u, 2u, { 3u, 0u } },
	{ { 4u, 3u }, 7u, 2u, { 3u, 0u } },
	{ { 5u, 2u }, 7u, 2u, { 3u, 0u } },
	{ { 6u, 1u }, 7u, 2u, { 3u, 0u } },
	{ { 1u, 7u }, 8u, 2u, { 2u, 0u } },
	{ { 2u, 6u }, 8u, 2u, { 3u, 0u } },
	{ { 3u, 5u }, 8u, 2u, { 3u, 0u } },
	{ { 4u, 4u }, 8u, 2u, { 3u, 0u } },
	{ { 5u, 3u }, 8u, 2u, { 3u, 0u } },
	{ { 6u, 2u }, 8u, 2u, { 3u, 0u } },
	{ { 7u, 1u }, 8u, 2u, { 3u, 0u } },
	{ { 1u, 1u, 1u }, 3u, 3u, { 2u, 0u, 0u } },
	{ { 1u, 1u, 2u }, 4u, 3u, { 2u, 0u, 0u } },
	{ { 1u, 2u, 1u }, 4u, 3u, { 2u, 0u, 0u } },
	{ { 2u, 1u, 1u }, 4u, 3u, { 3u, 0u, 0u } },
	{ { 1u, 1u, 3u }, 5u, 3u, { 2u, 0u, 0u } },
	{ { 1u, 2u, 2u }, 5u, 3u, { 2u, 0u, 0u } },
	{ { 1u, 3u, 1u }, 5u, 3u, { 2u, 0u, 0u } },
	{ { 2u, 1u, 2u }, 5u, 3u, { 3u, 0u, 0u } },
	{ { 2u, 2u, 1u }, 5u, 3u, { 3u, 0u, 0u } },
	{ { 3u, 1u, 1u }, 5u, 3u, { 3u, 0u, 0u } },
	{ { 1u, 1u, 4u }, 6u, 3u, { 2u, 0u, 0u } },
	{ { 1u, 2u, 3u }, 6u, 3u, { 2u, 0u, 0u } },
	{ { 1u, 3u, 2u }, 6u, 3u, { 2u, 0u, 0u } },
	{ { 1u, 4u, 1u }, 6u, 3u, { 2u, 0u, 0u } },
	{ { 2u, 1u, 3u }, 6u, 3u, { 3u, 0u, 0u } },
	{ { 2u, 2u, 2u }, 6u, 3u, { 3u, 0u, 0u } },
	{ { 2u, 3u, 1u }, 6u, 3u, { 3u, 0u, 0u } },
	{ { 3u, 1u, 2u }, 6u, 3u, { 3u, 0u, 0u } },
	{ { 3u, 2u, 1u }, 6u, 3u, { 3u, 0u, 0u } },
	{ { 4u, 1u, 1u }, 6u, 3u, { 3u, 0u, 0u } },
	{ { 1u, 1u, 5u }, 7u, 3u, { 2u, 0u, 0u } },
	{ { 1u, 2u, 4u }, 7u, 3u, { 2u, 0u, 0u } },
	{ { 1u, 3u, 3u }, 7u, 3u, { 2u, 0u, 0u } },
	{ { 1u, 4u, 2u }, 7u, 3u, { 2u, 0u, 0u } },
	{ { 1u, 5u, 1u }, 7u, 3u, { 2u, 0u, 0u } },
	{ { 2u, 1u, 4u }, 7u, 3u, { 3u, 0u, 0u } },
	{ { 2u, 2u, 3u }, 7u, 3u, { 3u, 0u, 0u } },
	{ { 2u, 3u, 2u }, 7u, 3u, { 3u, 0u, 0u } },
	{ { 2u, 4u, 1u }, 7u, 3u, { 3u, 0u, 0u } },
	{ { 3u, 1u, 3u }, 7u, 3u, { 3u, 0u, 0u } },
	{ { 3u, 2u, 2u }, 7u, 3u, { 3u, 0u, 0u } },
	{ { 3u, 3u, 1u }, 7u, 3u, { 3u, 0u, 0u } },
	{ { 4u, 1u, 2u }, 7u, 3u, { 3u, 0u, 0u } },
	{ { 4u, 2u, 1u }, 7u, 3u, { 3u, 0u, 0u } },
	{ { 5u, 1u, 1u }, 7u, 3u, { 3u, 0u, 0u } }
};

static const uint8_t num_and_fences = 78u;

std::vector<uint32_t> process_input_arrival( std::vector<uint32_t> const& input_arrival_times, std::vector<uint8_t>& idx, uint32_t& diff )
{
	diff = *std::min_element( input_arrival_times.begin(), input_arrival_times.end() );
	std::vector<uint32_t> input_arrival_ordered( input_arrival_times );
	std::for_each( input_arrival_ordered.begin(), input_arrival_ordered.end(), [diff]( uint32_t& n ) { n -= diff; } );
	std::sort( idx.begin(), idx.end(), [&input_arrival_ordered]( uint8_t idx1, uint8_t idx2 ) { return input_arrival_ordered[idx1] < input_arrival_ordered[idx2]; } );
	for ( auto i{ 0u }; i < idx.size(); ++i )
	{
		input_arrival_ordered[i] = input_arrival_times[idx[i]];
	}

	return input_arrival_ordered;
}

bool reschedule( uint32_t depth_target, std::vector<uint32_t>& depth, schedule& scheduling )
{
	// while keeping the target depth achievable, scheduling each variable to an as-low-as-possible level
	bool updated{ false };

	for ( auto i{ 0u }; i < depth.size(); ++i )
	{
		if ( depth[i] < depth_target )
		{
			if ( scheduling[i] == 0u )
			{
				// // impossible to achieve the target depth using the current AND fence
				// return false;
				continue;
			}

			uint32_t depth_diff = depth_target - depth[i];
			if ( scheduling[i] < depth_diff )
			{
				depth[i] += scheduling[i];
				scheduling[i] = 0u;
			}
			else
			{
				depth[i] += depth_diff;
				scheduling[i] -= depth_diff;
			}
			updated = true;
		}
	}

	return updated;
}

bool derive_signature( std::vector<uint32_t> const& input_arrival_time, schedule const& scheduling, std::vector<uint8_t>& signature )
{
	uint8_t first_non_zero{ 0u };
	for ( uint8_t i{ 0u }; i < scheduling.size(); ++i )
	{
		if ( scheduling[i] != 0 )
		{
			first_non_zero = i;
			break;
		}
	}

	if ( first_non_zero != 0u )
	{
		bool derived{ false };
		for ( uint8_t i{ scheduling.size() - 1 }; i >= first_non_zero; --i )
		{
			if ( input_arrival_time[i] - input_arrival_time[i - 1] >= 3 )
			{
				/* for debugging */
				// fmt::print( "[m] has gap in input arrival time\n" );


				for ( uint8_t j{ 0u }; j < i; ++j )
				{
					signature[j] = 0;
				}
				for ( uint8_t j{ i }; j < scheduling.size(); ++j )
				{
					signature[j] = input_arrival_time[j] - input_arrival_time[i] + 3;
				}

				return true;
			}
		}

		/* for debugging */
		// fmt::print( "[m] doesn't have gap in input arrival time\n" );


		for ( uint8_t i{ 0u }; i < first_non_zero; ++i )
		{
			signature[i] = 0u;
		}
		for ( uint8_t i{ first_non_zero }; i < scheduling.size(); ++i )
		{
			signature[i] = input_arrival_time[i] - input_arrival_time[first_non_zero - 1];
		}

		return true;
	}

	// if a signature turns out to be all zero, it implies the corresponding fence cannot help

	return false;
}

std::vector<uint32_t> order_input_arrival( std::vector<uint32_t> const& input_arrival_times, std::vector<uint8_t>& idx )
{
	std::vector<uint32_t> input_arrival_ordered( input_arrival_times );
	std::sort( idx.begin(), idx.end(), [&input_arrival_ordered]( uint8_t idx1, uint8_t idx2 ) { return input_arrival_ordered[idx1] < input_arrival_ordered[idx2]; } );
	for ( auto i{ 0u }; i < idx.size(); ++i )
	{
		input_arrival_ordered[i] = input_arrival_times[idx[i]];
	}

	return input_arrival_ordered;
}

struct cache_entry_hash
{
	std::size_t operator()( std::pair<uint64_t, std::vector<uint8_t>> const& key ) const
	{
		std::size_t hash_value = std::hash<uint64_t>{}( key.first );
		for ( auto const& each : key.second )
		{
			hash_value ^= std::hash<uint8_t>{}( each ) + 0x9e3779b9 + ( hash_value << 6 ) + ( hash_value >> 2 );
		}

		return hash_value;
	}
};

template<bill::solvers Solver>
struct md_optimization_exact_syn
{
	using problem_network_t = cnf_view<xag_network, false, Solver>;


public:
	md_optimization_exact_syn( kitty::dynamic_truth_table const& func, uint32_t const& g_m_depth,
	                           uint32_t& c_m_depth, std::vector<signal_level_pair> const& inputs, 
	                           fhe_optimization_exact_syn_params const& ps,
	                           fhe_optimization_exact_syn_stats* pst,
	                           xag_network& xag_res,
	                           std::shared_ptr<xag_network> p_xag_cache,
	                           std::vector<xag_network::signal> const& pis_cache,
	                           std::shared_ptr<std::vector<std::vector<std::pair<uint64_t, std::vector<uint8_t>>>>> p_blacklist,
	                           std::shared_ptr<std::vector<std::unordered_map<std::pair<uint64_t, std::vector<uint8_t>>, xag_network::signal, cache_entry_hash>>> p_cache )
		: num_vars_( func.num_vars() ), func_( func ), md_( c_m_depth ), g_m_depth_( g_m_depth ),
	    ps_( ps ), pst_( pst ), p_xag_res_( &xag_res ), p_xag_cache_( p_xag_cache ), pis_cache_( pis_cache ),
	    p_blacklist_( p_blacklist ), p_cache_( p_cache )
	{
		const auto tt = ( num_vars_ < 5u ) ? kitty::extend_to( func_, 5u ) : func_;
		mc_ = kitty::get_spectral_mc( tt );
		std::tie( repr_, phase_, perm_ ) = kitty::exact_npn_canonization( func_ );
		invert_ = kitty::get_bit( repr_, 0 );
		repr_normal_ = invert_ ? ~repr_ : repr_;

		pis_res_.resize( num_vars_ );
		input_arrival_times_.resize( num_vars_ );
		for ( auto i{ 0u }; i < num_vars_; ++i )
		{
			pis_res_[i] = inputs[i].f;
			input_arrival_times_[i] = inputs[i].m_depth;
		}
	}

	~md_optimization_exact_syn()
	{
		/* for debugging */
		// fmt::print( "[m] {} entries in the blacklist\n", ( *p_blacklist_ )[num_vars_].size() );
		// fmt::print( "[m] {} entries in the cache\n", ( *p_cache_ )[num_vars_].size() );
	}

	std::optional<xag_network::signal> exact_syn_flow()
	{
		/* TO CONFIRM: is the case of 0-mc functions ruled out already? */
		if ( mc_ == 0u )
		{
			return std::nullopt;
		}

		// aligning and ordering input arrival times
		std::vector<uint8_t> idx( input_arrival_times_.size() );  // the indices of the ordered variables in 'input_arrival_times_'
		std::iota( idx.begin(), idx.end(), 0u );
		// std::vector<uint32_t> input_arrival_ordered = process_input_arrival( input_arrival_times_, idx, diff );
		std::vector<uint32_t> input_arrival_ordered = order_input_arrival( input_arrival_times_, idx );


		/* for debugging */
		// fmt::print( "[m] start exact synthesis\n" );
		// fmt::print( "[m] input arrival times are : " );
		// for ( auto pin_delay : input_arrival_times_ )
		// {
		// 	fmt::print( "{}, ", pin_delay );
		// }
		// fmt::print( "\n" );
		// fmt::print( "[m] aligned input arrival times are : " );
		// for ( auto pin_delay : input_arrival_ordered )
		// {
		// 	fmt::print( "{}, ", pin_delay );
		// }
		// fmt::print( "\n" );
		// fmt::print( "[m] indices are : " );
		// for ( auto index : idx )
		// {
		// 	fmt::print( "{}, ", index );
		// }
		// fmt::print( "\n" );


		// initializing 'md_expected', 'schedules' and 'depths'
		std::array<uint32_t, num_and_fences> md_expected;
		std::array<schedule, num_and_fences> schedules;
		std::array<std::vector<uint32_t>, num_and_fences> depths;
		for ( auto i{ 0u }; i < num_and_fences; ++i )
		{
			if ( and_fences_lib[i].mc < mc_ )
			{
				md_expected[i] = 0u;
				schedules[i].resize( 0 );
				depths[i].resize( 0 );
				continue;
			}

			auto const& schedule_req = and_fences_lib[i].baseline_scheduling_req;
			schedule s( input_arrival_ordered.size(), schedule_req.size() );
			std::vector<uint32_t> depth( input_arrival_ordered );
			uint8_t to_be_scheduled{ 0u };  // starting from scheduling variables with low input arrival time
			for ( auto d{ 0u }; d < schedule_req.size(); ++d )
			{
				if ( schedule_req[d] == 0u )
				{
					break;
				}
				for ( auto need_schedule{ 0u }; need_schedule < schedule_req[d]; ++need_schedule )
				{
					depth[to_be_scheduled] += ( schedule_req.size() - d );
					s[to_be_scheduled++] = d;
				}
			}

			md_expected[i] = *std::max_element( depth.begin(), depth.end() );
			if ( md_expected[i] < md_ )
			{
				md_ = md_expected[i];
			}
			bool reschedule_able = reschedule( md_expected[i], depth, s );
			// assert( reschedule_able );
			schedules[i] = s;
			depths[i] = depth;
		}

		if ( md_ >= g_m_depth_ )
		{


			/* for debugging */
			// fmt::print( "[m] impossible to realize a smaller depth, skipping...\n" );


			return std::nullopt;
		}

		while ( true )
		{
			bool matched{ false };
			for ( auto i{ 0u }; i < num_and_fences; ++i )
			{
				if ( md_expected[i] == md_ )
				{
					matched = true;

					std::vector<uint8_t> signature_ordered( schedules[i].size(), 0u );
					if ( !derive_signature( input_arrival_ordered, schedules[i], signature_ordered ) )
					{
						// known to be non-reschedulable, skipping...
						md_expected[i] = 0u;
						depths[i].resize( 0 );
						schedules[i].resize( 0 );
						continue;
					}
					std::vector<uint8_t> signature( signature_ordered.size() );
					std::vector<uint8_t> s( signature_ordered.size() );
					for ( auto j{ 0u }; j < signature.size(); ++j )
					{
						signature[idx[j]] = signature_ordered[j];
						s[idx[j]] = schedules[i][j];
					}
					std::vector<uint8_t> signature_tmp( signature );
					std::vector<uint8_t> s_tmp( s );
					for ( auto j{ 0u }; j < perm_.size(); ++j )
					{
						signature[j] = signature_tmp[perm_[j]];
						s[j] = s_tmp[perm_[j]];
					}


					/* for debugging */
					//fmt::print( "[m] trying to solve such an eaxt synthesis problem : " );
					// fmt::print( "[m] ordered input arrival time is : " );
					// for ( auto t : input_arrival_ordered )
					// {
					// 	fmt::print( "{}, ", t );
					// }
					// fmt::print( "\n" );
					// fmt::print( "[m] ordered scheduling is : " );
					// for ( auto schedule : schedules[i] )
					// {
					// 	fmt::print( "{}, ", schedule );
					// }
					// fmt::print( "\n" );
					// fmt::print( "[m] ordered depth is : " );
					// for ( auto depth : depths[i] )
					// {
					// 	fmt::print( "{}, ", depth );
					// }
					// fmt::print( "\n" );
					// fmt::print( "[m] ordered signature is : " );
					// for ( auto sig : signature_ordered )
					// {
					// 	fmt::print( "{}, ", sig );
					// }
					// fmt::print( "\n" );


					// refer to the cache
					const auto it_cache = ( *p_cache_ )[num_vars_].find( std::make_pair( *( repr_.cbegin() ), signature ) );
					if ( it_cache != ( *p_cache_ )[num_vars_].end() )
					{
						// fmt::print( "[m] exact implementation found in the cache\n" );
						return construct_repr_based_xag( it_cache->second );
					}

					// solution unavailable in the cache, refer to the blacklist
					if ( std::find( ( ( *p_blacklist_ )[num_vars_] ).begin(), ( ( *p_blacklist_ )[num_vars_] ).end(), std::make_pair( *( repr_.cbegin() ), signature ) ) == ( ( *p_blacklist_ )[num_vars_] ).end() )
					{
						// not blacklisted, resort to exact synthesis
						// ++pst_->num_instances;
						auto p_sol = exact_syn_instance( and_fences_lib[i].and_fence, s, signature );
						if ( p_sol )
						{
							return p_sol;
						}
						
						( ( *p_blacklist_ )[num_vars_] ).emplace_back( std::make_pair( *( repr_.cbegin() ), signature ) );
						// fmt::print( "[m] blacklist updated\n" );
					}

					bool reschedule_able = reschedule( ++md_expected[i], depths[i], schedules[i] );
					if ( !reschedule_able )
					{
						// abandoning this AND fence
						md_expected[i] = 0u;
						depths[i].resize( 0 );
						schedules[i].resize( 0 );
					}
				}
			}

			if ( !matched )
			{
				/* TODO: should not happen, add an assertion */
				return std::nullopt;
			}

			// relaxing the target depth
			if ( ++md_ >= g_m_depth_ )
			{
				return std::nullopt;
			}
		}
	}

private:
	/* initializing the instance */
	void reset( problem_network_t const& pntk )
	{
		ltfi_vars_.clear();
		truth_vars_.clear();
		// truth_vars_.resize( func_.num_bits() );
		truth_vars_.resize( repr_normal_.num_bits() );

		for ( auto i{ 0u }; i < num_vars_; ++i )
		{
			const auto var_tt = kitty::nth_var<kitty::dynamic_truth_table> ( num_vars_, i );
			// for ( auto b{ 0u }; b < func_.num_bits(); ++b )
			for ( auto b{ 0u }; b < repr_normal_.num_bits(); ++b )
			{
				truth_vars_[b].emplace_back( pntk.get_constant( kitty::get_bit( var_tt, b ) ) );
			}
		}
	}

	/* adding the variables */
	void add_gate( problem_network_t& pntk, std::vector<uint8_t> const& and_fence )
	{
		uint8_t i_ext{ 0u };
		for ( uint8_t ands_each_depth : and_fence )
		{
			for ( uint8_t i_int{ 0u }; i_int < ands_each_depth; ++i_int )
			{
				for ( auto j{ 0u }; j < 2u; ++j )
				{
					ltfi_vars_.emplace_back( std::vector<signal<problem_network_t>>( num_vars_ + i_ext ) );
					std::generate( ltfi_vars_.back().begin(), ltfi_vars_.back().end(), [&]() { return pntk.create_pi(); } );
				}
			}
			i_ext += ands_each_depth;
		}
	}

	void add_po( problem_network_t& pntk )
	{
		ltfi_vars_.emplace_back( std::vector<signal<problem_network_t>>( num_vars_ + ltfi_vars_.size() / 2 ) );
		std::generate( ltfi_vars_.back().begin(), ltfi_vars_.back().end(), [&]() { return pntk.create_pi(); } );
	}

	/* defining principle constraints */
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

		// for ( auto b{ 1u }; b < func_.num_bits(); ++b )
		for ( auto b{ 1u }; b < repr_normal_.num_bits(); ++b )
		{
			for ( auto i{ 0u }; i < ltfi_vars_.size() / 2; ++i )
			{
				truth_vars_[b].emplace_back( pntk.create_and( create_xor_clause( ltfi_vars_[2 * i], b ), create_xor_clause( ltfi_vars_[2 * i + 1], b ) ) );
			}
			const auto po = create_xor_clause( ltfi_vars_.back(), b );
			// pntk.create_po( kitty::get_bit( func_, b ) ? po : pntk.create_not( po ) );
			pntk.create_po( kitty::get_bit( repr_normal_, b ) ? po : pntk.create_not( po ) );
		}
	}

	void add_struct_constraint( problem_network_t& pntk, std::vector<uint8_t> const& and_fence )
	{
		// (1) for any AND with a certain MD d, at least one of its fanins is an AND with the MD of d - 1
		uint32_t range_begin{ num_vars_ };
		uint32_t range_end{ num_vars_ + and_fence[0] };
		uint32_t i_ext{ and_fence[0] };
		for ( auto d{ 1u }; d < and_fence.size(); ++d )
		{
			for ( auto i_int{ 0u }; i_int < and_fence[d]; ++i_int )
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
			range_begin = range_end;
			range_end += and_fence[d];
		}
		// (2) for any AND with a certain MD d, none of the AND with the same MD can be its fanin;
		// This is already guaranteed by removing the corresponding selection variables
	}

	void add_schedule_constraint( problem_network_t& pntk, std::vector<uint8_t> const& and_fence, schedule const& s )
	{
		// ensuring a variable is unavailable until the depth it is scheduled to
		for ( auto j{ 0u }; j < num_vars_; ++j )
		{
			uint32_t i_ext{ 0u };
			for ( auto d{ 0u }; d < s[j]; ++d )
			{
				for ( auto i_int{ 0u }; i_int < and_fence[d]; ++i_int )
				{
					pntk.add_clause( !ltfi_vars_[2 * i_ext][j] );
					pntk.add_clause( !ltfi_vars_[2 * i_ext + 1][j] );
					++i_ext;
				}
			}
		}
	}

	// defining advanced constraints
	void add_no_useless_signal_constraint( problem_network_t& pntk )
	{
		for ( auto j{ 0u }; j < num_vars_ + ltfi_vars_.size() / 2; ++j )
		{
			// if ( ( j < num_vars_ ) && !kitty::has_var( func_, j ) )
			if ( ( j < num_vars_ ) && !kitty::has_var( repr_normal_, j ) )
			{
				continue;
			}

			std::vector<signal<problem_network_t>> clause;
			for ( auto const& ltfi : ltfi_vars_ )
			{
				if ( j < ltfi.size() )
				{
					clause.emplace_back( ltfi[j] );
				}
			}

			pntk.create_nary_or( clause );
		}
	}

	void add_no_empty_ltfi_constraint( problem_network_t& pntk )
	{
		for ( auto const& ltfi : ltfi_vars_ )
		{
			pntk.create_nary_or( ltfi );
		}
	}

	void add_no_subset_ltfi_constraint( problem_network_t& pntk )
	{
		for ( auto i{ 0u }; i < ltfi_vars_.size() / 2; ++i )
		{
			auto const& ltfi1 = ltfi_vars_[2 * i];
			auto const& ltfi2 = ltfi_vars_[2 * i + 1];
			std::vector<signal<problem_network_t>> interm1( ltfi1.size() );
			std::vector<signal<problem_network_t>> interm2( ltfi1.size() );

			for ( auto j{ 0u }; j < ltfi1.size(); ++j )
			{
				interm1[j] = pntk.create_and( ltfi1[j], pntk.create_not( ltfi2[j] ) );
				interm2[j] = pntk.create_and( pntk.create_not( ltfi1[j] ), ltfi2[j] );
			}
			pntk.create_nary_or( interm1 );
			pntk.create_nary_or( interm2 );
		}
	}

	void add_symmetry_breaking_constraint( problem_network_t& pntk )
	{
		// forcing the left LTFI always lexicographically smaller than the right one
		for ( auto i{ 0u }; i < ltfi_vars_.size() / 2; ++i )
		{
			auto const& ltfi1 = ltfi_vars_[2 * i];
			auto const& ltfi2 = ltfi_vars_[2 * i + 1];
			std::vector<signal<problem_network_t>> helper( ltfi1.size() - 1u );
			std::generate( helper.begin(), helper.end(), [&]() { return pntk.create_pi(); } );

			// letting the LSB of an LTFI be the MSB in lexicographical ordering
			pntk.create_or( !ltfi1[0], ltfi2[0] );
			pntk.add_clause(  ltfi1[0],  ltfi2[0],  helper[0] );
			pntk.add_clause( !ltfi1[0], !ltfi2[0],  helper[0] );
			pntk.add_clause(  ltfi1[0], !ltfi2[0], !helper[0] );

			// introducing induction to less significant variables in the LTFIs
			for ( auto j{ 1u }; j < helper.size() - 1; ++j )
			{
				pntk.create_or( helper[j - 1], !helper[j] );
				pntk.add_clause( !ltfi1[j],  ltfi2[j], !helper[j - 1] );
				pntk.add_clause( !ltfi1[j], !helper[j - 1], helper[j] );
				pntk.add_clause(  ltfi2[j], !helper[j - 1], helper[j] );
				pntk.add_clause(  ltfi1[j], !ltfi2[j], !helper[j] );
			}

			pntk.create_or( !ltfi1.back(), !helper.back() );
			pntk.create_or(  ltfi2.back(), !helper.back() );
		}
	}

	// putting all stuffs together
	void encode( problem_network_t& pntk, std::vector<uint8_t> const& and_fence, schedule const& s )
	{
		// adding principle constraints
		add_gate( pntk, and_fence );
		add_po( pntk );
		add_func_constraint( pntk );
		add_struct_constraint( pntk, and_fence );
		add_schedule_constraint( pntk, and_fence, s );

		if ( ps_.use_advanced_constraints )
		{
			// adding advanced constraints to prune searching space
			add_no_useless_signal_constraint( pntk );
			add_no_empty_ltfi_constraint( pntk );
			add_no_subset_ltfi_constraint( pntk );
			add_symmetry_breaking_constraint( pntk );
		}
	}

	bool solve( problem_network_t& pntk )
	{
		stopwatch t_solving( pst_->time_solving );
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

	xag_network::signal decompose_to_xag( problem_network_t& pntk, std::vector<uint8_t> const& signature )
	{
		std::vector<signal<xag_network>> nodes( num_vars_ );
		for ( auto i{ 0u }; i < nodes.size(); ++i )
		{
			nodes[i] = pis_cache_[i];
		}

		const auto decompose_ltfi = [&]( std::vector<signal<problem_network_t>> const& ltfi_vars ) ->signal<xag_network> {
			std::vector<signal<xag_network>> ltfi;
			for ( auto j{ 0u }; j < ltfi_vars.size(); ++j )
			{
				if ( pntk.model_value( ltfi_vars[j] ) )
				{
					ltfi.emplace_back( nodes[j] );
				}
			}
			return p_xag_cache_->create_nary_xor( ltfi );
		};

		for ( auto i{ 0u }; i < ltfi_vars_.size() / 2; ++i )
		{
			nodes.emplace_back( p_xag_cache_->create_and( decompose_ltfi( ltfi_vars_[2 * i] ), decompose_ltfi( ltfi_vars_[2 * i + 1] ) ) );
		}

		auto po = decompose_ltfi( ltfi_vars_.back() );
		po = invert_ ? p_xag_cache_->create_not( po ) : po;
		p_xag_cache_->create_po( po );

		( *p_cache_ )[num_vars_].insert( { std::make_pair( *( repr_.cbegin() ), signature ), po } );

		// verifying the Boolean function implemented by the constructed XAG
		// if ( ps_.verify_solution )
		// {
		// 	auto result = simulate<kitty::static_truth_table<5u>>( *p_xag_cache_ )[0];
		// 	const std::string result_str = kitty::to_hex( result ).substr( 0, 1 << ( num_vars_ - 2 ) );

		// 	const std::string func_str = kitty::to_hex( repr_ );
		// 	if ( result_str != func_str )
		// 	{
		// 		fmt::print( "[e] incorrect XAG implementation for {}, whose implemented function is {}...\n", func_str, result_str );
		// 		abort();
		// 	}
		// 	else
		// 	{
		// 		fmt::print( "[m] the XAG implementation is verified to be correct.\n" );
		// 	}
		// }

		return po;
	}

	std::optional<xag_network::signal> exact_syn_instance( std::vector<uint8_t> const& and_fence, schedule const& s, std::vector<uint8_t> const& signature )
	{
		cnf_view_params cvps;
		problem_network_t pntk( cvps );

		reset( pntk );
		encode( pntk, and_fence, s );
		if( const auto sat = solve( pntk ); sat )
		{


			/* for debugging */
			// fmt::print( "[m] found an exact implementation\n" );


			return construct_repr_based_xag( decompose_to_xag( pntk, signature ) );
		}

		return std::nullopt;
	}

	xag_network::signal construct_repr_based_xag( xag_network::signal const& po_cache )
	{
		// xag_network xag;
		// std::vector<xag_network::signal> pis( num_vars_ );
		// std::generate( pis.begin(), pis.end(), [&]() { return xag.create_pi(); } );

		// input negation
		for ( auto i{ 0u }; i < num_vars_; ++i )
		{
			if ( ( phase_ >> i ) & 1 )
			{
				// pis[i] = xag.create_not( pis[i] );
				pis_res_[i] = p_xag_res_->create_not( pis_res_[i] );
			}
		}

		// input permutation
		std::vector<xag_network::signal> pis_res_perm( num_vars_ );
		for ( auto i{ 0u }; i < pis_res_perm.size(); ++i )
		{
			// pis_perm[i] = pis[perm_[i]];
			pis_res_perm[i] = pis_res_[perm_[i]];
		}

		xag_network::signal po;
		if ( p_xag_cache_->is_constant( p_xag_cache_->get_node( po_cache ) ) )
		{
			// po = xag.get_constant( p_xag_cache_->is_complemented( po_cache ) );
			po = p_xag_res_->get_constant( p_xag_cache_->is_complemented( po_cache ) );
		}
		else
		{
			cut_view<xag_network> cache_partial{ *p_xag_cache_, pis_cache_, po_cache };

			// po = cleanup_dangling( cache_partial, xag, pis_perm.begin(), pis_perm.end() ).front();
			po = cleanup_dangling( cache_partial, *p_xag_res_, pis_res_perm.begin(), pis_res_perm.end() ).front();
		}

		// output negation
		if ( ( phase_ >> num_vars_ ) & 1 )
		{
			// po = xag.create_not( po );
			po = p_xag_res_->create_not( po );
		}

		// xag.create_po( po );

		// verifying the Boolean function implemented by the constructed XAG
		// if ( ps_.verify_solution )
		// {
		// 	auto result = simulate<kitty::static_truth_table<5u>>( xag )[0];
		// 	const std::string result_str = kitty::to_hex( result ).substr( 0, 1 << ( num_vars_ - 2 ) );
		// 	const std::string func_str = kitty::to_hex( func_ );
		// 	if ( result_str != func_str )
		// 	{
		// 		fmt::print( "[e] incorrect XAG implementation for {}, whose implemented function is {}...\n", func_str, result_str );
		// 		abort();
		// 	}
		// 	else
		// 	{
		// 		fmt::print( "[m] the XAG implementation is verified to be correct.\n" );
		// 	}
		// }

		// return xag;

		return po;
	}


private:
	uint8_t num_vars_;
	kitty::dynamic_truth_table func_;
	uint8_t mc_;
	uint32_t& md_;
	uint32_t g_m_depth_;
	std::vector<uint32_t> input_arrival_times_;
	std::vector<std::vector<signal<problem_network_t>>> ltfi_vars_;
	std::vector<std::vector<signal<problem_network_t>>> truth_vars_;
	const fhe_optimization_exact_syn_params ps_;
	fhe_optimization_exact_syn_stats* pst_;

	kitty::dynamic_truth_table repr_;
	bool invert_;
	kitty::dynamic_truth_table repr_normal_;
	uint32_t phase_;
	std::vector<uint8_t> perm_;
	xag_network* p_xag_res_;
	std::vector<xag_network::signal> pis_res_;
	std::shared_ptr<xag_network> p_xag_cache_;
	std::vector<xag_network::signal> pis_cache_{ 5u };
	std::shared_ptr<std::vector<std::vector<std::pair<uint64_t, std::vector<uint8_t>>>>> p_blacklist_;
	std::shared_ptr<std::vector<std::unordered_map<std::pair<uint64_t, std::vector<uint8_t>>, xag_network::signal, cache_entry_hash>>> p_cache_;
};

/* TODO : XOR minimization */

} /* detail */

template<bill::solvers Solver>
class fhe_optimization_exact_syn
{
public:
	fhe_optimization_exact_syn( fhe_optimization_exact_syn_params const& ps = {}, fhe_optimization_exact_syn_stats* pst = nullptr )
		: ps_( ps ), pst_( pst ),
	    p_xag_cache_( std::make_shared<xag_network>() ),
	    p_blacklist_( std::make_shared<std::vector<std::vector<std::pair<uint64_t, std::vector<uint8_t>>>>>() ),
	    p_cache_( std::make_shared<std::vector<std::unordered_map<std::pair<uint64_t, std::vector<uint8_t>>, xag_network::signal, detail::cache_entry_hash>>>() )
	{
		std::generate( pis_cache_.begin(), pis_cache_.end(), [&]() { return p_xag_cache_->create_pi(); } );
		p_blacklist_->resize( 6u );
		p_cache_->resize( 6u );
		load_cache();
		load_blacklist();
	}

	~fhe_optimization_exact_syn()
	{
		write_xag_cache();
		write_blacklist();

		if ( ps_.verbose )
		{
			pst_->report();
		}
	}

	template<typename Fn>
	std::pair<uint32_t, xag_network::signal> run( xag_network& res, kitty::dynamic_truth_table const& func, std::vector<signal_level_pair> const& inputs, uint32_t g_m_depth, Fn&& on_signal ) const
	{
		uint32_t c_m_depth{ g_m_depth };
		auto p_po = detail::md_optimization_exact_syn<Solver>( func, g_m_depth, c_m_depth, inputs, ps_, pst_, res,
		                                                       p_xag_cache_, pis_cache_, p_blacklist_, p_cache_ ).exact_syn_flow();

		if ( !p_po )
		{
			return { g_m_depth, res.get_constant( false ) };
		}

		on_signal( { *p_po, c_m_depth } );


		/* for debugging */
		// fmt::print( "[m] exact synthesis-based new implementation is : Node {}{}, with the delay of {}\n", ( res.is_complemented( *p_po ) ? "!" : "" ), res.get_node( *p_po ), c_m_depth );


		return { c_m_depth, *p_po };
	}

	// template<typename Fn>
	// uint32_t run1( xag_network& res, kitty::dynamic_truth_table const& func, std::vector<signal_level_pair> const& inputs, uint32_t g_m_depth, Fn&& on_signal ) const
	// {
	// 	const auto num_vars = func.num_vars();
	// 	assert( num_vars == inputs.size() );
	// 	uint32_t c_m_depth{ g_m_depth };
	// 	std::vector<xag_network::signal> pis( num_vars, res.get_constant( false ) );
	// 	std::vector<uint32_t> delays( num_vars, 0u );
	// 	for ( auto i{ 0u }; i < num_vars; ++i )
	// 	{
	// 		pis[i] = inputs[i].f;
	// 		delays[i] = inputs[i].m_depth;
	// 	}

	// 	auto p_sol = detail::md_optimization_exact_syn<Solver>( func, g_m_depth, c_m_depth, delays, ps_, pst_,
	// 	                                                        std::make_shared<xag_network>( xag_cache_ ), pis_cache_,
	// 	                                                        std::make_shared<std::vector<std::vector<std::pair<uint64_t, std::vector<uint8_t>>>>>( blacklist_ ),
	// 	                                                        std::make_shared<std::vector<std::unordered_map<std::pair<uint64_t, std::vector<uint8_t>>, xag_network::signal, detail::cache_entry_hash>>>( cache_ ) ).exact_syn_flow();
	// 	if ( !p_sol )
	// 	{
	// 		return g_m_depth;
	// 	}


	// 	/* for debugging */
	// 	if ( ps_.verify_solution )
	// 	{
	// 		auto result = simulate<kitty::static_truth_table<5u>>( *p_sol )[0];
	// 		const std::string result_str = kitty::to_hex( result ).substr( 0, 1 << ( func.num_vars() - 2 ) );
	// 		const std::string func_str = kitty::to_hex( func );
	// 		if ( result_str != func_str )
	// 		{
	// 			fmt::print( "[e] incorrect XAG implementation for {}, whose implemented function is {}...\n", func_str, result_str );
	// 			abort();
	// 		}
	// 		else
	// 		{
	// 			// fmt::print( "[m] the XAG implementation is verified to be correct ( {} v.s. {} ).\n", func_str, result_str );
	// 			fmt::print( "[m] the XAG implementation is verified to be correct.\n" );
	// 		}
	// 	}

	// 	std::vector<xag_network::signal> sol_pis;
	// 	p_sol->foreach_pi( [&sol_pis]( auto const& pi ) { sol_pis.emplace_back( pi ); } );
	// 	if ( sol_pis.size() != pis.size() )
	// 	{
	// 		fmt::print( "[e] the numbers of inputs do not match!\n" );
	// 		abort();
	// 	}
	// 	std::vector<xag_network::signal> sol_pos;
	// 	p_sol->foreach_po( [&sol_pos]( auto const& po ) { sol_pos.emplace_back( po ); } );
	// 	// if ( sol_pos.size() != 1 )
	// 	// {
	// 	// 	fmt::print( "[e] exactly synthesized partial network has {} pos\n", sol_pos.size() );
	// 	// 	abort();
	// 	// }
		
	// 	xag_network::signal po;
	// 	if ( p_sol->is_constant( p_sol->get_node( sol_pos[0] ) ) )
	// 	{
	// 		fmt::print( "[m] new implementation is a constant!\n" );
	// 		po = res.get_constant( p_sol->is_complemented( sol_pos[0] ) );
	// 	}
	// 	else
	// 	{
	// 		cut_view<xag_network> sol{ *p_sol, sol_pis, sol_pos[0] };
	// 		po = cleanup_dangling( sol, res, pis.begin(), pis.end() ).front();
	// 	}
	// 	on_signal( { po, c_m_depth } );


	// 	/* for debugging */
	// 	fmt::print( "[m] exact synthesis-based new implementation is : Node {}{}, with the delay of {}\n", ( res.is_complemented( po ) ? "!" : "" ), res.get_node( po ), c_m_depth );


	// 	return c_m_depth;
	// }

void load_cache( std::string cache_to_read = "fhe_opt_cache", std::string xag_cache_to_read = "fhe_opt_xag_cache")
{
	if ( !ps_.use_exisiting_cache )
	{
		if ( ps_.verbose )
		{
			fmt::print( "[i] skip loading exisiting cache...\n" );
		}
		return;
	}

	stopwatch t_parse_cache( pst_->time_parse_cache );

	std::ifstream xag_cache;
	xag_cache.open( xag_cache_to_read, std::ios::in );
	std::string item;
	std::getline( xag_cache, item );
	xag_cache.close();

	std::vector<xag_network::signal> xag_cache_signals( pis_cache_ );
	std::vector<std::string> signals;
	std::stringstream ss( item );
	while ( std::getline( ss, item, ' ' ) )
	{
		signals.emplace_back( item );
	}

	assert( signals.size() % 2 == 0 );
	for ( auto i{ 0u }; i < signals.size(); i += 2u )
	{
		auto signal1_ul = std::stoul( signals[i] );
		auto signal2_ul = std::stoul( signals[i + 1] );

		xag_network::signal signal1{}, signal2{};

		if ( signal1_ul == 0u )
		{
			signal1 = p_xag_cache_->get_constant( false );
		}
		else if ( signal1_ul == 1u )
		{
			signal1 = p_xag_cache_->get_constant( true );
		}
		else
		{
			signal1 = xag_cache_signals[signal1_ul / 2 - 1] ^ ( signal1_ul % 2 );
		}
		if ( signal2_ul == 0u )
		{
			signal2 = p_xag_cache_->get_constant( false );
		}
		else if ( signal2_ul == 1u )
		{
			signal2 = p_xag_cache_->get_constant( true );
		}
		else
		{
			signal2 = xag_cache_signals[signal2_ul / 2 - 1] ^ ( signal2_ul % 2 );
		}

		if ( signal1_ul > signal2_ul )
		{
			xag_cache_signals.emplace_back( p_xag_cache_->create_xor( signal1, signal2 ) );
		}
		else
		{
			xag_cache_signals.emplace_back( p_xag_cache_->create_and( signal1, signal2 ) );
		}
	}

	std::ifstream cache;
	cache.open( cache_to_read, std::ios::in );
	uint8_t var_cnt{ 0u };

	while ( std::getline( cache, item ) )
	{
		if ( item.empty() )
		{
			if ( ++var_cnt > 5u )
			{
				break;
			}
			continue;
		}

		std::vector<std::string> info;
		std::stringstream ss( item );
		while ( std::getline( ss, item, ' ' ) )
		{
			info.emplace_back( item );
		}
		std::vector<uint8_t> signature( var_cnt );
		for ( auto i{ 0u }; i < var_cnt; ++i )
		{
			signature[i] = static_cast<uint8_t>( std::stoul( info[i + 1] ) );
		}
		xag_network::signal signal_repr = xag_cache_signals[std::stoul( info.back() ) / 2 - 1] ^ ( std::stoul( info.back() ) % 2 );
		p_xag_cache_->create_po( signal_repr );
		( *p_cache_ )[var_cnt][std::make_pair( std::stoul( info[0] ), signature )] = signal_repr;
		++num_cache_entry_old;
	}
	cache.close();

	if ( ps_.verbose )
	{
		fmt::print( "[i] finished cache loading\n" );
	}
}

void load_blacklist( std::string blacklist_to_read = "fhe_opt_blacklist" )
{
	if ( !ps_.use_exisiting_cache )
	{
		if ( ps_.verbose )
		{
			fmt::print( "[i] skip loading exisiting blacklist...\n" );
		}
		return;
	}

	stopwatch t_parse_cache( pst_->time_parse_cache );

	std::ifstream blacklist;
	blacklist.open( blacklist_to_read, std::ios::in );
	uint8_t var_cnt{ 0u };
	std::string item;

	while ( std::getline( blacklist, item ) )
	{
		if ( item.empty() )
		{
			if ( ++var_cnt > 5u )
			{
				break;
			}
			continue;
		}

		std::stringstream ss( item );
		std::vector<std::string> info;
		while ( std::getline( ss, item, ' ' ) )
		{
			info.emplace_back( item );
		}
		std::vector<uint8_t> signature( var_cnt );
		for ( auto i{ 0u }; i < var_cnt; ++i )
		{
			signature[i] = static_cast<uint8_t>( std::stoul( info[i + 1] ) );
		}
		( *p_blacklist_ )[var_cnt].emplace_back( std::make_pair( std::stoul( info[0] ), signature ) );
		++num_blacklist_entry_old;

	}
	blacklist.close();

	if ( ps_.verbose )
	{
		fmt::print( "[i] finished blacklist loading\n" );
	}
}

void write_xag_cache( std::string cache_to_write = "fhe_opt_cache", std::string xag_cache_to_write = "fhe_opt_xag_cache" )
{
	num_cache_entry_new = 0u;
	for ( auto i{ 0u }; i < ( *p_cache_ ).size(); ++i )
	{
		num_cache_entry_new += ( *p_cache_ )[i].size();
	}
	if ( num_cache_entry_new == num_cache_entry_old )
	{
		fmt::print( "[i] no new implementation synthesized, skipping recording cache...\n" );
		return;
	}
	else
	{
		fmt::print( "[i] more implementation synthesized : {} -> {}\n", num_cache_entry_old, num_cache_entry_new );
	}

	std::ofstream xag_cache;
	xag_cache.open( xag_cache_to_write, std::ios::out );

	if ( xag_cache.is_open() )
	{
		node_map<xag_network::signal, xag_network> old2new{ *p_xag_cache_ };
		xag_network xag;

		// constants
		old2new[p_xag_cache_->get_constant( false )] = xag.get_constant( false );

		// pis
		p_xag_cache_->foreach_pi( [&]( auto const& n ) {
			old2new[n] = xag.create_pi();
		} );

		// gates
		progress_bar pbar{ ( *p_xag_cache_ ).size(), "Recording cache |{0}| node = {1:>4} / " + std::to_string( ( *p_xag_cache_ ).size() ), true };
		topo_view<xag_network>{ *p_xag_cache_ }.foreach_node( [&]( auto const& n, auto index ) {
			pbar( index, index );

			if ( p_xag_cache_->is_constant( n ) || p_xag_cache_->is_pi( n ) )
			{
				return;
			}

			std::vector<xag_network::signal> children;
			p_xag_cache_->foreach_fanin( n, [&]( auto const& fi ) {
				auto const fi_new = old2new[fi];
				children.emplace_back( p_xag_cache_->is_complemented( fi ) ? xag.create_not( fi_new ) : fi_new );
				xag_cache << static_cast<uint64_t>( ( children.back().index << 1 ) + children.back().complement ) << " ";
			} );
			old2new[n] = xag.clone_node( *p_xag_cache_, n, children );
			return;
		} );

		xag_cache.close();
		fmt::print( "\n[i] cached xag recorded into file {}\n", xag_cache_to_write );

		std::ofstream cache;
		cache.open( cache_to_write, std::ios::out );  // trunc mode instead?

		if ( cache.is_open() )
		{
			for ( auto i{ 0u }; i < ( *p_cache_ ).size(); ++i )
			{
				for ( auto it{ ( *p_cache_ )[i].begin() }; it != ( *p_cache_ )[i].end(); ++it )
				{
					cache << it->first.first << " ";
					std::vector<uint8_t> signature( it->first.second );
					// assert( signature.size() == i );
					for ( uint8_t byte : signature )
					{
						cache << static_cast<uint32_t>( byte ) << " ";
					}
					auto const f_new = old2new[it->second];
					cache << static_cast<uint32_t>( ( f_new.index << 1 ) + ( f_new.complement ^ p_xag_cache_->is_complemented( it->second ) ) ) << "\n";
				}
				cache << "\n";
			}

			cache.close();
			fmt::print( "[i] cached correspondence recorded into file {}\n", cache_to_write );
		}
		else
		{
			fmt::print( "[i] unable to open file {}\n", cache_to_write );
		}
	}
	else
	{
		fmt::print( "[i] unable to open file {}\n", xag_cache_to_write );
	}
}

void write_blacklist( std::string blacklist_to_write = "fhe_opt_blacklist" )
{
	num_blacklist_entry_new = 0u;
	for ( auto i{ 0u }; i < ( *p_blacklist_ ).size(); ++i )
	{
		num_blacklist_entry_new += ( *p_blacklist_ )[i].size();
	}
	if ( num_blacklist_entry_new == num_blacklist_entry_old )
	{
		fmt::print( "[i] no new instances add to the blacklist, skipping recording blacklist...\n" );
		return;
	}
	else
	{
		fmt::print( "[i] more instances found to be blacklisted : {} -> {}\n", num_blacklist_entry_old, num_blacklist_entry_new );
	}

	std::ofstream blacklist;
	blacklist.open( blacklist_to_write, std::ios::out );

	if ( blacklist.is_open() )
	{
		for ( auto i{ 0u }; i < ( *p_blacklist_ ).size(); ++i )
		{
			/* for debugging */
			// fmt::print( "[m] in the {}th entry, there are {} items in the blacklist\n", i, ( *p_blacklist_ )[i].size() );
			

			for ( auto const& item : ( *p_blacklist_ )[i] )
			{
				blacklist << item.first << " ";
				std::vector<uint8_t> signature( item.second );
				// assert( signature.size() == i );
				for ( uint8_t byte : signature )
				{
					blacklist << static_cast<uint32_t>( byte ) << " ";
				}
				blacklist << "\n";
			}
			blacklist << "\n";
		}

		blacklist.close();
		fmt::print( "[i] cached blacklist recorded into file {}\n", blacklist_to_write );
	}
	else
	{
		fmt::print( "[i] unable to open file {}\n", blacklist_to_write );
	}
}

private:
	const fhe_optimization_exact_syn_params ps_;
	fhe_optimization_exact_syn_stats* pst_;

	std::shared_ptr<xag_network> p_xag_cache_;
	std::vector<xag_network::signal> pis_cache_{ 5u };
	std::shared_ptr<std::vector<std::vector<std::pair<uint64_t, std::vector<uint8_t>>>>> p_blacklist_;
	std::shared_ptr<std::vector<std::unordered_map<std::pair<uint64_t, std::vector<uint8_t>>, xag_network::signal, detail::cache_entry_hash>>> p_cache_;
	uint32_t num_cache_entry_old{};
	uint32_t num_cache_entry_new{};
	uint32_t num_blacklist_entry_old{};
	uint32_t num_blacklist_entry_new{};
};

} /* mockturtle */