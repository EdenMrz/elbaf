#include <vector>
#include <list>
#include <iostream>
#include <bitset>
#include <cassert>

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

huffman_queue probabilities_queue(prob_table& probability) {
	// TODO: use a better algorithm - currently using a brute force approach
	huffman_queue ret;

	for (const auto& [byte_value, prob]: probability) {
		ret.push(new HuffmanNode{
			.probability = prob,
			.symbol = byte_value,
			.left = nullptr, .right = nullptr });
	}

	return ret;
}

void read_huffman_tree(HuffmanNode* node, symbol_table& probability, std::vector<bool>& path) {
	assert(node != nullptr);

	if (node->left == nullptr && node->right == nullptr)
		probability[node->symbol] = path;

	if (node->left) {
		path.push_back(false);
		read_huffman_tree(node->left, probability, path);
	}

	if (node->right) {
		path.push_back(true);
		read_huffman_tree(node->right, probability, path);
	}

	// moving back up a level in the tree, so remove the last element
	path.erase(end(path));
}

void print_huffman_tree(HuffmanNode* root, int level = 0) {
	if (root == nullptr)
		return;

	for (int i = 0; i < level; i++)
		std::cout << "\t";
	std::cout << "-> " << static_cast<int>(root->symbol) << '\n';
	level++;
	print_huffman_tree(root->left, level);
	print_huffman_tree(root->right, level);
}

symbol_table huffman_code(prob_table& probability) {
	auto prob_queue = probabilities_queue(probability);
	HuffmanNode* root = build_huffman_tree(prob_queue);

	// TODO: get the symbols from the tree
	symbol_table symbols;
	std::vector<bool> path;
	read_huffman_tree(root, symbols, path);
	//print_huffman_tree(root);

	free_huffman_tree(root);

	return symbols;
}

reverse_symbol_table reverse_symbols(const symbol_table& symbols) {
	reverse_symbol_table ret;

	for (auto [ key, value ] : symbols)
		ret[value] = key;

	return ret;
}

HuffmanNode* build_huffman_tree(huffman_queue& prob_node) {
	if (prob_node.size() == 0)
		return nullptr;

	HuffmanNode* lowest_prob_1 = prob_node.top();
	prob_node.pop();

	// lowest_prob_1 was the only element in the priority queue
	if (prob_node.size() == 0)
		return lowest_prob_1;

	HuffmanNode* lowest_prob_2 = prob_node.top();
	prob_node.pop();

	HuffmanNode* parent = new HuffmanNode{};
	parent->probability = lowest_prob_1->probability + lowest_prob_2->probability;
	parent->left = lowest_prob_1;
	parent->right = lowest_prob_2;

	prob_node.push(parent);

	return build_huffman_tree(prob_node);
}

void free_huffman_tree(HuffmanNode* root) {
	if (root == nullptr)
		return;

	free_huffman_tree(root->left);
	free_huffman_tree(root->right);
	delete root;
}

bool is_set_bit(const uint8_t value, uint8_t bit_no) {
	const uint8_t BYTE_LEN = 8;
	uint8_t mask = 1 << (BYTE_LEN - 1 - bit_no);
	return value & mask;
}


void display_reverse_symbols(reverse_symbol_table& symbol) {
	for (const auto& [key, value]: symbol) {
		std::cout << "0x";
		for (auto bit: key) {
			if (bit)
				std::cout << '1';
			else
				std::cout << '0';
		}
		std::cout << ": " << std::bitset<8>(static_cast<uint8_t>(value));
		std::cout << '\n';
	}
	std::cout << "The dictionary has " << symbol.size() << " symbols\n";
}

}
