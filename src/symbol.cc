#include <vector>

#include "symbol.h"

namespace elbaf::symbol {

std::vector<bool> UnaryCodeGenerator::next() {
	std::vector<bool> ret;
	for (int i = 0; i < _index; ++i)
		ret.push_back(true);
	ret.push_back(false);
	_index++;
	return ret;
}

symbol_table unary_code(prob_table& probability) {
	UnaryCodeGenerator generator;
	symbol_table symbol;
	while (probability.size() > 0) {
		int max = 0;
		std::byte max_key = std::begin(probability)->first;
		for (const auto& [ key, value ] :probability) {
			if (value < max)
				continue;

			max = value;
			max_key = key;
		}

		symbol[max_key] = generator.next();
		probability.erase(max_key);
	}

	return symbol;
}

}
