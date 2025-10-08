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

template <typename Compare = frequency_compare>
using symbol_queue = std::priority_queue<freq_entry, std::vector<freq_entry>, Compare>;

symbol_queue<> generate_symbols(std::istream& input, Encoding encoding = Encoding::HUFFMAN) {
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
	// Do not use decltype(freq)::value_type as the queue data type or else the map key will use
	// a const type.
	// Making the key const will later create an error when pushing values to the priority_queue.
	// This is because when pushing, a priority_queue will first push_back(), then move/assign
	// some elements to keep the container sorted in some way.
	// That move/assign operation is not allowed if one of the std::pair parameters is const.
	symbol_queue symbols_queue;
	for (const auto f: freq)
		symbols_queue.push(f);

	return symbols_queue;
}

symbol_queue<> generate_priority_queue(code_symbol& symbols) {
	symbol_queue q{};
	for (const auto& s: symbols)
		q.push(s);
	return q;
}

enum class filezone: std::int8_t {
	HEADER_SIZE,
	HEADER_CODES,
	DATA
};

class Compressor {
public:
	Compressor(symbol_queue<>& symbols);
	Compressor(symbol_queue<>&& symbols);
	std::optional<std::byte> next(std::istream& input);
private:
	void generate_symbols();
private:
	code_symbol _symbols;
	symbol_queue<> _queue;
	filezone _zone = filezone::HEADER_SIZE; 
	std::int8_t _code_number = 0;
	std::byte _current_byte;
	std::int8_t _bit_number = 8;

	static const std::int8_t BIT_LEN = 8;
};

Compressor::Compressor(symbol_queue<>& queue): _queue{queue} {
	generate_symbols();
}
Compressor::Compressor(symbol_queue<>&& queue): _queue{queue} {
	generate_symbols();
}
void Compressor::generate_symbols() {
	size_t i = 0;
	auto queue = _queue;
	while (!queue.empty()) {
		const auto& tmp = queue.top();
		_symbols.insert(std::make_pair(tmp.first, ++i));
		queue.pop();
	}
}

std::optional<std::byte> Compressor::next(std::istream& input) {
	std::optional<std::byte> opt;
	char c;

	switch (_zone) {
	case filezone::HEADER_SIZE:
		_zone = filezone::HEADER_CODES;
		return std::byte{static_cast<unsigned char>(_symbols.size())};
	case filezone::HEADER_CODES:
		while (_code_number < _symbols.size()) {
			++_code_number;
			std::byte tmp { static_cast<unsigned char>(_queue.top().first) };
			_queue.pop();
			return tmp;
		}
		_zone = filezone::DATA;
	default:
		// Do the rest of this function
		break;
	}

	if (!input.get(c))
		return opt;
	size_t len = _symbols[c];

	while (true) {
		_current_byte <<= 1;
		// BUG: the length of the longest (ie rarest) symbol should be len-1 but with the last bit set to 1
		std::byte mask = std::byte{0x1} << (_bit_number-1);
		if (len == 1)
			_current_byte &= ~mask;
		else
			_current_byte |= mask;

		--_bit_number;
		if (_bit_number == 0) {
			_bit_number = Compressor::BIT_LEN;
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
