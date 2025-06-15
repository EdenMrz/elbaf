#pragma once

#include <vector>
#include <utility>
#include <queue>

#include "elbaf.h"

// Symbol generator functions are allowed to modify the probabilities table
namespace elbaf::symbol {

using prob_element = std::pair<double, std::byte>;

class UnaryCodeGenerator {
public:
	constexpr UnaryCodeGenerator() {}
	virtual std::vector<bool> next();
private:
	size_t _index {0};
};

struct HuffmanNode {
	double probability;
	// matters only when left and right are null
	std::byte symbol;
	HuffmanNode* left;
	HuffmanNode* right;

	bool operator<(const HuffmanNode& operand) {
		return this->probability < operand.probability;
	}
};

class HuffmanNodeComparator {
public:
	bool operator()(const HuffmanNode* const left, const HuffmanNode* const right) {
		return left->probability > right->probability;
	}
};

using huffman_queue = std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, HuffmanNodeComparator>;

huffman_queue probabilities_queue(prob_table& probability);
HuffmanNode* build_huffman_tree(huffman_queue& prob_node);
void free_huffman_tree(HuffmanNode* root);

symbol_table unary_code(prob_table& probability);
symbol_table huffman_code(prob_table& probability);
reverse_symbol_table reverse_symbols(const symbol_table& symbols);

// left-most bit no is 0, and right-most is 7
bool is_set_bit(uint8_t value, const uint8_t bit_no);

void display_reverse_symbols(reverse_symbol_table& symbols);

}
