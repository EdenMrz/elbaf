#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include "elbaf.h"
#include "symbol.h"

using namespace elbaf;

enum class Encoding {
	HUFFMAN
};

using freq_entry = std::pair<char, size_t>;
class frequency_compare {
public:
	bool operator()(const freq_entry& lhs, const freq_entry& rhs) const {
		return lhs.second < rhs.second;
	}
};


void encode(std::istream& input, std::ostream& output, Encoding encoding = Encoding::HUFFMAN) {
	std::map<char, size_t> freq{};
	char c;
	size_t total = 0;
	while (input.get(c)) {
		if (!freq.contains(c))
			freq[c] = 1;
		else
			++freq[c];

		++total;
	}

	// Do not use decltype(freq)::value_type as the map key will be set to const.
	// Making the key const will later create an error when pushing values to the priority_queue.
	// This is because when pushing, a priority_queue will first push_back(), then move/assign
	// some elements to keep the container sorted in some way.
	// That move/assign operation is not allowed if one of the std::pair parameters is const.
	std::priority_queue<freq_entry, std::vector<freq_entry>, frequency_compare> symbols_queue;
	for (const auto f: freq) {
		symbols_queue.push(f);
	}

	std::map<char, size_t> symbols;
	size_t i = 0;
	while (!symbols_queue.empty()) {
		const auto& tmp = symbols_queue.top();
		symbols.insert(std::make_pair(tmp.first, ++i));
		symbols_queue.pop();
	}

	std::cout << "symbols table has " << symbols.size() << " elements\n";
}

int main(int argc, char** argv)
{
	options opts;
	if (!check_parameters(argc, argv, &opts))
		return -1;

	auto input = std::ifstream(opts.input_file, std::ios_base::binary);
	auto output = std::ofstream{opts.output_file, std::ios_base::binary};

	if (opts.compression) {
		encode(input, output);
	} else {
		ReverseCodewordReader reader { opts.input_file };
		std::cout << "Writing the initial data to " << opts.output_file << '\n';
		write_to_file(output, reader);
	}

	return 0;
}
