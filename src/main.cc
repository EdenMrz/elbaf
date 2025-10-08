#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <cstddef>
#include <sys/types.h>
#include "elbaf.h"
#include "symbol.h"

using namespace elbaf;

enum class Encoding {
	HUFFMAN
};

using code_symbol = std::map<char, size_t>;

// Cannot use code_symbol::value_type because this will make the key const.
// Find NOTE(1)
using freq_entry = std::pair<char, size_t>;
class frequency_compare {
public:
	bool operator()(const freq_entry& lhs, const freq_entry& rhs) const {
		return lhs.second < rhs.second;
	}
};

code_symbol generate_symbols(std::istream& input, Encoding encoding = Encoding::HUFFMAN) {
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

	// NOTE: (1)
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

	return symbols;
}

class Compressor {
public:
	Compressor(code_symbol& symbols);
	Compressor(code_symbol&& symbols);
	std::optional<std::byte> next(std::istream& input);
private:
	code_symbol _symbols;
	std::byte _current_byte;
	std::int8_t _bit_number = 0;

	static const std::int8_t BIT_LEN = 8;
};

Compressor::Compressor(code_symbol& symbols): _symbols{symbols} {}
Compressor::Compressor(code_symbol&& symbols): _symbols{symbols} {}

std::optional<std::byte> Compressor::next(std::istream& input) {
	std::optional<std::byte> opt;
	char c;
	if (!input.get(c))
		return opt;
	size_t len = _symbols[c];

	while (true) {
		_current_byte <<= 1;
		// BUG: the length of the longest (ie rarest) symbol should be len-1 but with the last bit set to 1
		if (len == 1)
			_current_byte &= std::byte{0b11111110};
		else
			_current_byte |= std::byte{0b00000001};

		++_bit_number;
		if (_bit_number == Compressor::BIT_LEN) {
			_bit_number = 0;
			break;
		}

		--len;
		if (len == 0) {
			if (!input.get(c))
				break;
			len = _symbols[c];
		}
	}

	opt = _current_byte;
	_current_byte = std::byte{0};
	return opt;
}

void compress(options& opts, Encoding encoding = Encoding::HUFFMAN) {
	auto input = std::ifstream(opts.input_file, std::ios_base::binary);
	Compressor comp { generate_symbols(input, encoding) };

	input.clear();
	input.seekg(0, std::ios::beg);
	auto output = std::ofstream{opts.output_file, std::ios_base::binary};

	for (auto byte = comp.next(input); byte.has_value(); byte = comp.next(input))
		output.put(static_cast<char>(byte.value()));
	output.flush();
}

int main(int argc, char** argv)
{
	options opts;
	if (!check_parameters(argc, argv, &opts))
		return -1;

	auto output = std::ofstream{opts.output_file, std::ios_base::binary};

	if (opts.compression) {
		compress(opts);
	} else {
		ReverseCodewordReader reader { opts.input_file };
		std::cout << "Writing the initial data to " << opts.output_file << '\n';
		write_to_file(output, reader);
	}

	return 0;
}
