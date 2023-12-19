#pragma once

#include <algorithm>
#include <cstdint>
#include <fmt/format.h>
#include <fstream>
#include <limits>
#include <optional>
#include <random>
#include <unordered_map>
#include <vector>

#include <bill/sat/interface/common.hpp>
#include <kitty/constructors.hpp>
#include <kitty/operations.hpp>
#include <kitty/print.hpp>
#include <kitty/spectral.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_enumeration.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/cost_functions.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/utils/stopwatch.hpp>
#include <mockturtle/utils/progress_bar.hpp>
#include <mockturtle/views/cnf_view.hpp>
#include <mockturtle/views/cut_view.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/topo_view.hpp>

namespace mockturtle
{

struct exact_tfhe_opt_params
{
	cut_enumeration_params cut_enum_ps;
	bool only_on_critical_path{ true };
	bool always_commit_impl{ false };
	bool randomly_commit_impl{ false };
	bool progress{ true };
	bool verbose{ false };
};

struct exact_tfhe_opt_stats
{
	cut_enumeration_stats cut_enum_st;
	void report() const
	{
		cut_enum_st.report();
	}
};

struct exact_tfhe_opt_agent_params
{
	uint32_t conflict_limit{ 0u };
	bool use_advanced_constraints{ false };
	bool use_exisiting_cache{ false };
	bool verbose{ false };
};

struct exact_tfhe_opt_agent_stats
{
	stopwatch<>::duration time_parse_cache{};
	stopwatch<>::duration time_mc_lookup{};
	stopwatch<>::duration time_solving{};
	// uint32_t num_instances{};

	void report() const
	{
		fmt::print( "[i] parsing time   = {:>5.2f} secs\n", to_seconds( time_parse_cache ) );
		fmt::print( "[i] mc-lookup time = {:>5.2f} secs\n", to_seconds( time_mc_lookup ) );
		fmt::print( "[i] solving time   = {:>5.2f} secs\n", to_seconds( time_solving ) );
		// fmt::print( "[i] num instances = {}\n", num_instances );
	}
};

namespace detail
{

struct num_and
{
	uint32_t operator()( xag_network const& ntk, xag_network::node const& n ) const
	{
		if ( ntk.is_and( n ) )
		{
			return 1u;
		}
		return 0u;
	}
};

struct signal_level_pair
{
	xag_network::signal f;
	uint32_t m_depth;
};

// A schedule indicates the depths, starting from which a variable is available
typedef std::vector<uint8_t> schedule;

struct AND_fence
{
	std::vector<uint8_t> and_fence;
	uint8_t mc;
	uint8_t md;
	std::vector<uint8_t> baseline_scheduling_req;
};

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
	{ { 1u, 8u }, 9u, 2u, { 2u, 0u } }, 
	{ { 2u, 7u }, 9u, 2u, { 3u, 0u } }, 
	{ { 3u, 6u }, 9u, 2u, { 3u, 0u } }, 
	{ { 4u, 5u }, 9u, 2u, { 3u, 0u } }, 
	{ { 5u, 4u }, 9u, 2u, { 3u, 0u } }, 
	{ { 6u, 3u }, 9u, 2u, { 3u, 0u } }, 
	{ { 7u, 2u }, 9u, 2u, { 3u, 0u } }, 
	{ { 8u, 1u }, 9u, 2u, { 3u, 0u } }, 
	{ { 1u, 9u }, 10u, 2u, { 2u, 1u } }, 
	{ { 2u, 8u }, 10u, 2u, { 3u, 0u } }, 
	{ { 3u, 7u }, 10u, 2u, { 3u, 0u } }, 
	{ { 4u, 6u }, 10u, 2u, { 3u, 0u } }, 
	{ { 5u, 5u }, 10u, 2u, { 3u, 0u } }, 
	{ { 6u, 4u }, 10u, 2u, { 3u, 0u } }, 
	{ { 7u, 3u }, 10u, 2u, { 3u, 0u } }, 
	{ { 8u, 2u }, 10u, 2u, { 3u, 0u } }, 
	{ { 9u, 1u }, 10u, 2u, { 3u, 0u } }, 
	{ { 1u, 10u }, 11u, 2u, { 2u, 1u } }, 
	{ { 2u, 9u }, 11u, 2u, { 3u, 0u } }, 
	{ { 3u, 8u }, 11u, 2u, { 3u, 0u } }, 
	{ { 4u, 7u }, 11u, 2u, { 3u, 0u } }, 
	{ { 5u, 6u }, 11u, 2u, { 3u, 0u } }, 
	{ { 6u, 5u }, 11u, 2u, { 3u, 0u } }, 
	{ { 7u, 4u }, 11u, 2u, { 3u, 0u } }, 
	{ { 8u, 3u }, 11u, 2u, { 3u, 0u } }, 
	{ { 9u, 2u }, 11u, 2u, { 3u, 0u } }, 
	{ { 10u, 1u }, 11u, 2u, { 4u, 0u } }, 
	{ { 1u, 11u }, 12u, 2u, { 2u, 1u } }, 
	{ { 2u, 10u }, 12u, 2u, { 3u, 0u } }, 
	{ { 3u, 9u }, 12u, 2u, { 3u, 0u } }, 
	{ { 4u, 8u }, 12u, 2u, { 3u, 0u } }, 
	{ { 5u, 7u }, 12u, 2u, { 3u, 0u } }, 
	{ { 6u, 6u }, 12u, 2u, { 3u, 0u } }, 
	{ { 7u, 5u }, 12u, 2u, { 3u, 0u } }, 
	{ { 8u, 4u }, 12u, 2u, { 3u, 0u } }, 
	{ { 9u, 3u }, 12u, 2u, { 3u, 0u } }, 
	{ { 10u, 2u }, 12u, 2u, { 4u, 0u } }, 
	{ { 11u, 1u }, 12u, 2u, { 4u, 0u } }, 
	{ { 1u, 12u }, 13u, 2u, { 2u, 1u } }, 
	{ { 2u, 11u }, 13u, 2u, { 3u, 0u } }, 
	{ { 3u, 10u }, 13u, 2u, { 3u, 0u } }, 
	{ { 4u, 9u }, 13u, 2u, { 3u, 0u } }, 
	{ { 5u, 8u }, 13u, 2u, { 3u, 0u } }, 
	{ { 6u, 7u }, 13u, 2u, { 3u, 0u } }, 
	{ { 7u, 6u }, 13u, 2u, { 3u, 0u } }, 
	{ { 8u, 5u }, 13u, 2u, { 3u, 0u } }, 
	{ { 9u, 4u }, 13u, 2u, { 3u, 0u } }, 
	{ { 10u, 3u }, 13u, 2u, { 4u, 0u } }, 
	{ { 11u, 2u }, 13u, 2u, { 4u, 0u } }, 
	{ { 12u, 1u }, 13u, 2u, { 4u, 0u } }, 
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
	{ { 5u, 1u, 1u }, 7u, 3u, { 3u, 0u, 0u } }, 
	{ { 1u, 1u, 6u }, 8u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 2u, 5u }, 8u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 3u, 4u }, 8u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 4u, 3u }, 8u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 5u, 2u }, 8u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 6u, 1u }, 8u, 3u, { 2u, 0u, 0u } }, 
	{ { 2u, 1u, 5u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 2u, 4u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 3u, 3u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 4u, 2u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 5u, 1u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 1u, 4u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 2u, 3u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 3u, 2u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 4u, 1u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 1u, 3u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 2u, 2u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 3u, 1u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 5u, 1u, 2u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 5u, 2u, 1u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 6u, 1u, 1u }, 8u, 3u, { 3u, 0u, 0u } }, 
	{ { 1u, 1u, 7u }, 9u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 2u, 6u }, 9u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 3u, 5u }, 9u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 4u, 4u }, 9u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 5u, 3u }, 9u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 6u, 2u }, 9u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 7u, 1u }, 9u, 3u, { 2u, 0u, 0u } }, 
	{ { 2u, 1u, 6u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 2u, 5u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 3u, 4u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 4u, 3u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 5u, 2u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 6u, 1u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 1u, 5u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 2u, 4u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 3u, 3u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 4u, 2u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 5u, 1u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 1u, 4u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 2u, 3u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 3u, 2u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 4u, 1u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 5u, 1u, 3u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 5u, 2u, 2u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 5u, 3u, 1u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 6u, 1u, 2u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 6u, 2u, 1u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 7u, 1u, 1u }, 9u, 3u, { 3u, 0u, 0u } }, 
	{ { 1u, 1u, 8u }, 10u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 2u, 7u }, 10u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 3u, 6u }, 10u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 4u, 5u }, 10u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 5u, 4u }, 10u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 6u, 3u }, 10u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 7u, 2u }, 10u, 3u, { 2u, 0u, 0u } }, 
	{ { 1u, 8u, 1u }, 10u, 3u, { 2u, 0u, 0u } }, 
	{ { 2u, 1u, 7u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 2u, 6u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 3u, 5u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 4u, 4u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 5u, 3u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 6u, 2u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 2u, 7u, 1u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 1u, 6u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 2u, 5u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 3u, 4u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 4u, 3u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 5u, 2u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 3u, 6u, 1u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 1u, 5u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 2u, 4u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 3u, 3u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 4u, 2u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 4u, 5u, 1u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 5u, 1u, 4u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 5u, 2u, 3u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 5u, 3u, 2u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 5u, 4u, 1u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 6u, 1u, 3u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 6u, 2u, 2u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 6u, 3u, 1u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 7u, 1u, 2u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 7u, 2u, 1u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 8u, 1u, 1u }, 10u, 3u, { 3u, 0u, 0u } }, 
	{ { 1u, 1u, 1u, 1u }, 4u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 1u, 1u, 2u }, 5u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 1u, 2u, 1u }, 5u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 2u, 1u, 1u }, 5u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 2u, 1u, 1u, 1u }, 5u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 1u, 1u, 1u, 3u }, 6u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 1u, 2u, 2u }, 6u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 1u, 3u, 1u }, 6u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 2u, 1u, 2u }, 6u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 2u, 2u, 1u }, 6u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 3u, 1u, 1u }, 6u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 2u, 1u, 1u, 2u }, 6u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 2u, 1u, 2u, 1u }, 6u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 2u, 2u, 1u, 1u }, 6u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 3u, 1u, 1u, 1u }, 6u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 1u, 1u, 1u, 4u }, 7u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 1u, 2u, 3u }, 7u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 1u, 3u, 2u }, 7u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 1u, 4u, 1u }, 7u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 2u, 1u, 3u }, 7u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 2u, 2u, 2u }, 7u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 2u, 3u, 1u }, 7u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 3u, 1u, 2u }, 7u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 3u, 2u, 1u }, 7u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 1u, 4u, 1u, 1u }, 7u, 4u, { 2u, 0u, 0u, 0u } }, 
	{ { 2u, 1u, 1u, 3u }, 7u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 2u, 1u, 2u, 2u }, 7u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 2u, 1u, 3u, 1u }, 7u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 2u, 2u, 1u, 2u }, 7u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 2u, 2u, 2u, 1u }, 7u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 2u, 3u, 1u, 1u }, 7u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 3u, 1u, 1u, 2u }, 7u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 3u, 1u, 2u, 1u }, 7u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 3u, 2u, 1u, 1u }, 7u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 4u, 1u, 1u, 1u }, 7u, 4u, { 3u, 0u, 0u, 0u } }, 
	{ { 1u, 1u, 1u, 1u, 1u }, 5u, 5u, { 2u, 0u, 0u, 0u, 0u } } 
};

static const uint8_t num_and_fences = 249u;

struct cache_entry
{
	uint64_t repr;
	std::vector<uint8_t> signature;

	bool operator==( cache_entry const& other ) const
	{
		return ( repr == other.repr ) && ( signature == other.signature );
	}
};

struct cache_entry_hash
{
	std::size_t operator()( cache_entry const& key ) const
	{
		std::size_t hash_value = std::hash<uint64_t>{}( key.repr );
		for ( auto const& byte : key.signature )
		{
			hash_value ^= std::hash<uint8_t>{}( byte ) + 0x9e3779b9 + ( hash_value << 6 ) + ( hash_value >> 2 );
		}

		return hash_value;
	}
};

std::vector<uint32_t> order_levels( std::vector<uint32_t> const& levels, std::vector<uint8_t>& idx )
{
	std::vector<uint32_t> levels_ordered( levels );
	std::sort( idx.begin(), idx.end(), [&levels_ordered]( uint8_t idx1, uint8_t idx2 ) { return levels_ordered[idx1] < levels_ordered[idx2]; } );
	for ( auto i{ 0u }; i < idx.size(); ++i )
	{
		levels_ordered[i] = levels[idx[i]];
	}

	return levels_ordered;
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
				// impossible to achieve the target depth using the current AND fence
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

void derive_signature( std::vector<uint32_t> const& levels, schedule const& scheduling, std::vector<uint8_t>& signature )
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
		for ( uint8_t i{ scheduling.size() - 1 }; i >= first_non_zero; --i )
		{
			if ( levels[i] - levels[i - 1] >= 4u )
			{


				/* for debugging */
				// fmt::print( "[m] has gap in input arrival time\n" );


				for ( uint8_t j{ 0u }; j < i; ++j )
				{
					signature[j] = 0;
				}
				for ( uint8_t j{ i }; j < scheduling.size(); ++j )
				{
					signature[j] = levels[j] - levels[i] + 4u;
				}

				return;
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
			signature[i] = levels[i] - levels[first_non_zero - 1];
		}

		return;
	}

	for ( auto i{ 0u }; i < levels.size(); ++i )
	{
		signature[i] = levels[i];
	}
	return;
}

template<bill::solvers Solver>
class exact_tfhe_opt_solver
{
	using problem_network_t = cnf_view<xag_network, false, Solver>;

public:
	exact_tfhe_opt_solver( kitty::dynamic_truth_table const& func, uint32_t old_impl_level, 
	                       uint32_t& new_impl_level, std::vector<signal_level_pair> const& inputs_levels, 
	                       uint8_t mc, xag_network* p_xag_res, std::shared_ptr<xag_network> p_xag_cache, 
	                       std::vector<xag_network::signal> const& pis_xag_cache, 
	                       std::shared_ptr<std::vector<std::vector<cache_entry>>> p_blacklist, 
	                       std::shared_ptr<std::vector<std::unordered_map<cache_entry, xag_network::signal, cache_entry_hash>>> p_cache, 
	                       exact_tfhe_opt_agent_params const& ps, exact_tfhe_opt_agent_stats* pst )
	  : func_( func ), md_( new_impl_level ), old_impl_level_( old_impl_level ), mc_( mc ), 
	    p_xag_res_( p_xag_res ), p_xag_cache_( p_xag_cache ), pis_xag_cache_( pis_xag_cache ), 
	    p_blacklist_( p_blacklist ), p_cache_( p_cache ), ps_( ps ), pst_( pst )
	{
		num_vars_ = func_.num_vars();
		std::tie( repr_, phase_, perm_ ) = kitty::exact_npn_canonization( func_ );
		invert_ = kitty::get_bit( repr_, 0 );
		repr_normal_ = invert_ ? ~repr_ : repr_;
		inputs_.resize( num_vars_ );
		levels_.resize( num_vars_ );
		for ( auto i{ 0u }; i < num_vars_; ++i )
		{
			inputs_[i] = inputs_levels[i].f;
			levels_[i] = inputs_levels[i].m_depth;
		}
	}

	~exact_tfhe_opt_solver()
	{}

	std::optional<xag_network::signal> run()
	{
		// aligning and ordering 'levels_'
		std::vector<uint8_t> idx( levels_.size() );  // the indices of the ordered variables in 'levels_'
		std::iota( idx.begin(), idx.end(), 0u );
		std::vector<uint32_t> levels_ordered = order_levels( levels_, idx );

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
			schedule s( levels_ordered.size(), schedule_req.size() );
			std::vector<uint32_t> depth( levels_ordered );
			uint8_t to_be_scheduled{ 0u };  // starting from scheduling variables of low level
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

		if ( md_ >= old_impl_level_ )
		{
			return std::nullopt;
		}

		while ( true )
		{
			bool matched{ false };


			/* for debugging */
			// fmt::print( "[m] trying to find an implementation whose m. depth is {} ( currently {} )\n", md_, old_impl_level_ );


			for ( auto i{ 0u }; i < num_and_fences; ++i )
			{
				if ( md_expected[i] == md_ )
				{
					matched = true;
					std::vector<uint8_t> signature_ordered( schedules[i].size(), 0u );
					derive_signature( levels_ordered, schedules[i], signature_ordered );

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
					// fmt::print( "[m] trying to solve such an eaxt synthesis problem : " );
					// fmt::print( "[m] ordered m. level of leaves is : " );
					// for ( auto t : levels_ordered )
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
					const auto it_cache = ( *p_cache_ )[num_vars_ - 1].find( { *( repr_.cbegin() ), signature } );
					if ( it_cache != ( *p_cache_ )[num_vars_ - 1].end() )
					{


						/* for debugging */
						// fmt::print( "[m] exact implementation found in the cache\n" );


						return construct_repr_based_xag( it_cache->second );
					}

					// solution unavailable in the cache, refer to the blacklist
					if ( std::find( ( ( *p_blacklist_ )[num_vars_ - 1] ).begin(), ( ( *p_blacklist_ )[num_vars_ - 1] ).end(), detail::cache_entry{ *( repr_.cbegin() ), signature } ) == ( ( *p_blacklist_ )[num_vars_ - 1] ).end() )
					{
						// not blacklisted, resort to exact synthesis
						// ++pst_->num_instances;
						auto p_sol = exact_syn_instance( and_fences_lib[i].and_fence, s, signature );
						if ( p_sol )
						{


							/* for debugging */
							// fmt::print( "[m] found an exact implementation\n" );


							return p_sol;
						}
						
						( ( *p_blacklist_ )[num_vars_ - 1] ).emplace_back( detail::cache_entry{ *( repr_.cbegin() ), signature } );
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
			if ( ++md_ >= old_impl_level_ )
			{


				/* for debugging */
				// fmt::print( "[m] failed to find a better implementation\n" );


				return std::nullopt;
			}
		}
	}

private:
	// initializing SAT instance
	void reset( problem_network_t const& pntk )
	{
		ltfi_vars_.clear();
		truth_vars_.clear();
		truth_vars_.resize( repr_normal_.num_bits() );

		for ( auto i{ 0u }; i < num_vars_; ++i )
		{
			const auto var_tt = kitty::nth_var<kitty::dynamic_truth_table> ( num_vars_, i );
			for ( auto b{ 0u }; b < repr_normal_.num_bits(); ++b )
			{
				truth_vars_[b].emplace_back( pntk.get_constant( kitty::get_bit( var_tt, b ) ) );
			}
		}
	}

	// adding variables
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

	// defining principle constraints
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

		for ( auto b{ 1u }; b < repr_normal_.num_bits(); ++b )
		{
			for ( auto i{ 0u }; i < ltfi_vars_.size() / 2; ++i )
			{
				truth_vars_[b].emplace_back( pntk.create_and( create_xor_clause( ltfi_vars_[2 * i], b ), create_xor_clause( ltfi_vars_[2 * i + 1], b ) ) );
			}
			const auto po = create_xor_clause( ltfi_vars_.back(), b );
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
			nodes[i] = pis_xag_cache_[i];
		}

		const auto decompose_ltfi = [&]( std::vector<signal<problem_network_t>> const& ltfi_vars ) -> xag_network::signal {
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

		( *p_cache_ )[num_vars_ - 1].insert( { { *( repr_.cbegin() ), signature }, po } );
		return po;
	}

	xag_network::signal construct_repr_based_xag( xag_network::signal const& po_xag_cache )
	{
		// input negation
		for ( auto i{ 0u }; i < num_vars_; ++i )
		{
			if ( ( phase_ >> i ) & 1 )
			{
				inputs_[i] = p_xag_res_->create_not( inputs_[i] );
			}
		}

		// input permutation
		std::vector<xag_network::signal> inputs_perm( num_vars_ );
		for ( auto i{ 0u }; i < inputs_perm.size(); ++i )
		{
			inputs_perm[i] = inputs_[perm_[i]];
		}

		xag_network::signal po;
		if ( p_xag_cache_->is_constant( p_xag_cache_->get_node( po_xag_cache ) ) )
		{
			po = p_xag_res_->get_constant( p_xag_cache_->is_complemented( po_xag_cache ) );
		}
		else
		{
			cut_view<xag_network> cache_partial{ *p_xag_cache_, pis_xag_cache_, po_xag_cache };
			po = cleanup_dangling( cache_partial, *p_xag_res_, inputs_perm.begin(), inputs_perm.end() ).front();
		}

		// output negation
		if ( ( phase_ >> num_vars_ ) & 1 )
		{
			po = p_xag_res_->create_not( po );
		}

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
			return construct_repr_based_xag( decompose_to_xag( pntk, signature ) );
		}

		return std::nullopt;
	}

private:
	uint8_t num_vars_;
	kitty::dynamic_truth_table func_;
	const uint8_t mc_;
	uint32_t& md_;
	uint32_t old_impl_level_;
	std::vector<uint32_t> levels_;
	std::vector<std::vector<signal<problem_network_t>>> ltfi_vars_;
	std::vector<std::vector<signal<problem_network_t>>> truth_vars_;
	const exact_tfhe_opt_agent_params ps_;
	exact_tfhe_opt_agent_stats* pst_;

	kitty::dynamic_truth_table repr_;
	bool invert_;
	kitty::dynamic_truth_table repr_normal_;
	uint32_t phase_;
	std::vector<uint8_t> perm_;
	xag_network* p_xag_res_;
	std::vector<xag_network::signal> inputs_;
	std::shared_ptr<xag_network> p_xag_cache_;
	std::vector<xag_network::signal> pis_xag_cache_{ 6u };
	std::shared_ptr<std::vector<std::vector<cache_entry>>> p_blacklist_;
	std::shared_ptr<std::vector<std::unordered_map<cache_entry, xag_network::signal, cache_entry_hash>>> p_cache_;
};

} /* detail */

template<bill::solvers Solver>
class exact_tfhe_opt_agent
{
public:
	exact_tfhe_opt_agent( exact_tfhe_opt_agent_params const& ps = {}, exact_tfhe_opt_agent_stats* pst = nullptr )
	  : ps_( ps ), pst_( pst ), p_xag_cache_( std::make_shared<xag_network>() ), 
	    p_blacklist_( std::make_shared<std::vector<std::vector<detail::cache_entry>>>() ), 
	    p_cache_( std::make_shared<std::vector<std::unordered_map<detail::cache_entry, xag_network::signal, detail::cache_entry_hash>>>() ), 
	    p_mc_lookup_( std::make_shared<std::unordered_map<std::string, uint8_t>>() )
	{
		std::generate( pis_xag_cache_.begin(), pis_xag_cache_.end(), [&]() { return p_xag_cache_->create_pi(); } );
		p_blacklist_->resize( 6u );
		p_cache_->resize( 6u );
		load_mc_lookup();

		if ( ps_.use_exisiting_cache )
		{
			stopwatch t_parse_cache( pst->time_parse_cache );
			load_cache();
			load_blacklist();
		}
		else if ( ps_.verbose )
		{
			fmt::print( "[i] skip loading exisiting cache...\n" );
		}
	}

	~exact_tfhe_opt_agent()
	{
		write_xag_cache();
		write_blacklist();

		if ( ps_.verbose )
		{
			pst_->report();
		}
	}

	template<typename Fn>
	detail::signal_level_pair run( xag_network& res, kitty::dynamic_truth_table const& func, std::vector<detail::signal_level_pair> const& inputs_levels, uint32_t old_impl_level, Fn&& on_signal ) const
	{
		const auto func_ext = kitty::extend_to<6u>( func );
		const auto spectral = call_with_stopwatch( pst_->time_mc_lookup, [&]() { return kitty::exact_spectral_canonization_limit( func_ext, 100000 ); } );
		const auto it_class = p_mc_lookup_->find( kitty::to_hex( spectral.first ) );
		if ( spectral.second && it_class != p_mc_lookup_->end() )
		{
			uint8_t mc = it_class->second;
			uint32_t new_impl_level{ old_impl_level };


			/* for debugging */
			// fmt::print( "[m] MC of current cut is {}\n", static_cast<uint32_t>( mc ) );


			auto p_sol = detail::exact_tfhe_opt_solver<Solver>( func, old_impl_level, new_impl_level, 
			                                                    inputs_levels, mc, &res, p_xag_cache_, 
			                                                    pis_xag_cache_, p_blacklist_, p_cache_, ps_, pst_ ).run();
			if ( !p_sol )
			{
				return { res.get_constant( false ), old_impl_level };
			}
			/* replace implementation */
			detail::signal_level_pair cand_impl{ *p_sol, new_impl_level };
			on_signal( cand_impl );
			return cand_impl;
		}
		else
		{


			/* for debugging */
			// fmt::print( "[m] MC of current cut is unknown\n" );


			/* TODO : MC is unknown does not necessarily mean finding the exact implementation is infeasible */
			return { res.get_constant( false ), old_impl_level };
		}

		
	}

	void load_mc_lookup( std::string mc_to_lookup = "dense_6_mc_lookup" )
	{
		stopwatch t_parse_cache( pst_->time_parse_cache );
		std::ifstream mc_lookup;
		mc_lookup.open( mc_to_lookup, std::ios::in );
		std::string line;
		uint32_t pos{ 0u };
		while ( std::getline( mc_lookup, line ) )
		{
			pos = static_cast<unsigned>( line.find( ' ' ) );
			const std::string repr = line.substr( 0, pos++ );
			line.erase( 0, pos );
			const uint8_t mc = static_cast<uint8_t>( std::stoul( line ) );
			( *p_mc_lookup_ )[repr] = mc;

			/* for debugging */
			// fmt::print( "[m] repr : {}, mc : {}\n", repr, mc );
		}
		mc_lookup.close();

		if ( ps_.verbose )
		{
			fmt::print( "[i] finished MC lookup loading...\n" );
		}
	}

	void load_cache( std::string cache_to_read = "exact_tfhe_opt_cache", std::string xag_cache_to_read = "exact_tfhe_opt_xag_cache")
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

		std::vector<xag_network::signal> xag_cache_signals( pis_xag_cache_ );
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
			( *p_cache_ )[var_cnt][{ std::stoul( info[0] ), signature }] = signal_repr;
			++num_cache_entry_old;
		}
		cache.close();

		if ( ps_.verbose )
		{
			fmt::print( "[i] finished cache loading\n" );
		}
	}

	void load_blacklist( std::string blacklist_to_read = "exact_tfhe_opt_blacklist" )
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
			( *p_blacklist_ )[var_cnt].emplace_back( detail::cache_entry{ std::stoul( info[0] ), signature } );
			++num_blacklist_entry_old;

		}
		blacklist.close();

		if ( ps_.verbose )
		{
			fmt::print( "[i] finished blacklist loading\n" );
		}
	}

	void write_xag_cache( std::string cache_to_write = "exact_tfhe_opt_cache", std::string xag_cache_to_write = "exact_tfhe_opt_xag_cache" )
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
						cache << it->first.repr << " ";
						std::vector<uint8_t> signature( it->first.signature );
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

	void write_blacklist( std::string blacklist_to_write = "exact_tfhe_opt_blacklist" )
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
					blacklist << item.repr << " ";
					std::vector<uint8_t> signature( item.signature );
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
	const exact_tfhe_opt_agent_params ps_;
	exact_tfhe_opt_agent_stats* pst_;

	std::shared_ptr<xag_network> p_xag_cache_;
	std::vector<xag_network::signal> pis_xag_cache_{ 6u };
	std::shared_ptr<std::vector<std::vector<detail::cache_entry>>> p_blacklist_;
	std::shared_ptr<std::vector<std::unordered_map<detail::cache_entry, xag_network::signal, detail::cache_entry_hash>>> p_cache_;
	std::shared_ptr<std::unordered_map<std::string, uint8_t>> p_mc_lookup_;
	uint32_t num_cache_entry_old{};
	uint32_t num_cache_entry_new{};
	uint32_t num_blacklist_entry_old{};
	uint32_t num_blacklist_entry_new{};
};

namespace detail
{

template<typename ExactAgent>
class exact_tfhe_opt_impl
{
public:
	exact_tfhe_opt_impl( xag_network const& ntk, ExactAgent const& exact_tfhe_opt_agent, 
	                     exact_tfhe_opt_params const& ps, exact_tfhe_opt_stats* pst )
			: ntk_( ntk ), exact_tfhe_opt_agent_( exact_tfhe_opt_agent ), ps_( ps ), pst_( pst )
	{}

	xag_network run()
	{
		xag_network res;
		depth_view<xag_network, num_and, false> ntk_md_{ ntk_ };
		uint32_t current_m_depth{ 0u };

		/* cut enumeration */
		const auto cuts = cut_enumeration<xag_network, true>( ntk_, ps_.cut_enum_ps, &( pst_->cut_enum_st ) );
		node_map<signal_level_pair, xag_network> old2new{ ntk_ };

		/* constants */
		old2new[ntk_.get_constant( false )] = { res.get_constant( false ), 0u };
		
		/* pis */
		ntk_.foreach_pi( [&]( auto const& n ) {
			old2new[n] = { res.create_pi(), 0u };
		} );

		/* gates */
		progress_bar pbar{ ntk_.size(), "TFHE exact optimization |{0}| node = {1:>4} / " + std::to_string( ntk_.size() ) + "   current mult. depth = {2}", ps_.progress };
		topo_view<xag_network>{ ntk_ }.foreach_node( [&]( auto const& n, auto index ) {
			pbar( index, index, current_m_depth );

			if ( ntk_.is_constant( n ) || ntk_.is_pi( n ) )
			{
				return;
			}

			if ( ps_.only_on_critical_path && !ntk_md_.is_on_critical_path( n ) )
			{
				/* duplicate this node */
				std::vector<xag_network::signal> children;
				ntk_.foreach_fanin( n, [&]( auto const& fi ) {
					auto const fi_new = old2new[fi].f;
					children.emplace_back( ntk_.is_complemented( fi ) ? res.create_not( fi_new ) : fi_new );
				} );
				old2new[n] = { res.clone_node( ntk_, n, children ), ntk_md_.level( n ) };
				return;
			}

			uint32_t old_impl_level{ ntk_md_.level( n ) };
			signal_level_pair best_impl{ {}, old_impl_level };
			for ( auto const& pcut: cuts.cuts( ntk_.node_to_index( n ) ) )
			{
				/* trivial cut */
				if ( pcut->size() == 1u || kitty::is_const0( cuts.truth_table( *pcut ) ) )
				{
					continue;
				}

				std::vector<signal_level_pair> inputs_levels( pcut->size() );
				std::transform( pcut->begin(), pcut->end(), inputs_levels.begin(), [&]( auto const& leaf ) { 
					return old2new[ntk_.index_to_node( leaf )];
				} );

				/* there is room exploring the metric evaluating the quality of a local implementation */
				/* currently, we simply adopt the mult. depth as the only criteria                     */
				const auto on_signal = [&]( signal_level_pair const& cand ) {
					if ( cand.m_depth < best_impl.m_depth )
					{
						best_impl = cand;
					}
					else if ( ps_.randomly_commit_impl && ( ( cand.m_depth == best_impl.m_depth ) && ( best_impl.m_depth < old_impl_level ) ) )
					{
						std::random_device rd;    // seed source
						std::mt19937 gen( rd() ); // Mersenne Twister algorithm-based random number engine, seeded with rd()
						std::uniform_int_distribution<unsigned int> random( 0, 1 );
						if ( random( gen ) )
						{
							best_impl = cand;
						}
					}
				};

				signal_level_pair cand_impl{ {}, std::numeric_limits<uint32_t>::max() };
				cand_impl = exact_tfhe_opt_agent_.run( res, cuts.truth_table( *pcut ), inputs_levels, old_impl_level, on_signal );

				if ( ps_.verbose )
				{
					if ( cand_impl.m_depth != old_impl_level )
					{
						fmt::print( "[m] managed to exactly synthesize a better partial network ({}->{})\n", old_impl_level, cand_impl.m_depth );
					}
					else
					{
						fmt::print( "[m] failed to exactly synthesize a better partial network\n" );
					}
				}

				if ( ps_.always_commit_impl && cand_impl.m_depth != old_impl_level )
				{
					old2new[n] = cand_impl;
					current_m_depth = std::max( current_m_depth, cand_impl.m_depth );


					/* for debugging */
					// fmt::print( "[m] Node {}{} is forced to be the new implementation of Node {}\n", ( res.is_complemented( c_impl ) ? "!" : "" ), res.get_node( c_impl ), n );


					return;
				}
			}

			if ( best_impl.m_depth != old_impl_level )
			{
				old2new[n] = best_impl;
				current_m_depth = std::max( current_m_depth, best_impl.m_depth );


				/* for debugging */
			// fmt::print( "[m] Node {}{} is selected as the new implementation of Node {}\n", ( res.is_complemented( best_impl.f ) ? "!" : "" ), res.get_node( best_impl.f ), n );
			}
			else
			{
				/* duplicate this node */
				std::vector<xag_network::signal> children;
				ntk_.foreach_fanin( n, [&]( auto const& fi ) {
					auto const fi_new = old2new[fi].f;
					children.emplace_back( ntk_.is_complemented( fi ) ? res.create_not( fi_new ) : fi_new );
				} );
				old2new[n] = { res.clone_node( ntk_, n, children ), old_impl_level };
				current_m_depth = std::max( current_m_depth, old_impl_level );
			}
		} );

		/* pos */
		ntk_.foreach_po( [&]( auto const& po_old ) {
			auto const po_new = old2new[po_old].f;
			res.create_po( ntk_.is_complemented( po_old ) ? res.create_not( po_new ) : po_new );
		} );

		return cleanup_dangling( res );
	}

private:
	xag_network const& ntk_;
	ExactAgent const& exact_tfhe_opt_agent_;
	const exact_tfhe_opt_params ps_;
	exact_tfhe_opt_stats* pst_;
};

} /* detail */

template<typename ExactAgent>
xag_network exact_tfhe_opt( xag_network const& ntk, ExactAgent const& exact_tfhe_opt_agent, exact_tfhe_opt_params const& ps = {}, exact_tfhe_opt_stats* pst = nullptr )
{
	const auto res = detail::exact_tfhe_opt_impl( ntk, exact_tfhe_opt_agent, ps, pst ).run();

	if ( ps.verbose )
	{
		pst->report();
	}

	return res;
}

} /* mockturtle */