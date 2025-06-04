#include <queue>
#include <vector>
#include <list>
#include <iostream>

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

	std::cout << "Probabilities: ";
	for (const auto& [byte_value, prob]: probability) {
		std::cout << prob << ", ";
		ret.push(new HuffmanNode{ .probability = prob, .left = nullptr, .right = nullptr });
	}
	std::cout << '\n';

	return ret;
}

symbol_table huffman_code(prob_table& probability) {
	auto prob_queue = probabilities_queue(probability);
	HuffmanNode* root = build_huffman_tree(prob_queue);

	// TODO: get the symbols from the tree
	// ...

	free_huffman_tree(root);

	return {};
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
	std::cout
		<< "the 2 probabilities: " << lowest_prob_1->probability << ", "
		<< lowest_prob_2->probability << '\n';
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

}
