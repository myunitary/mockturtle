#include <mockturtle/algorithms/fhe_rule_gen.hpp>

int main( void )
{
	std::string dbname = "db_fhe_5";
	std::string filename = "fhe_rules";
	mockturtle::fhe_rule_gen fhe_rule_gen{};
	fhe_rule_gen.run( dbname, filename );

	return 0;
}