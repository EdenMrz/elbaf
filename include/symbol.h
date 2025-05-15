#pragma once

#include <vector>

#include "elbaf.h"

// Symbol generator functions are allowed to modify the probabilities table
namespace elbaf::symbol {

class UnaryCodeGenerator {
public:
	constexpr UnaryCodeGenerator() {}
	virtual std::vector<bool> next();
private:
	size_t _index {0};
};

symbol_table unary_code(prob_table& probability);

}
