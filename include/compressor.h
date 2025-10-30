#pragma once

#include <cstdio>
#include <iostream>
#include <map>
#include <queue>
#include <cstddef>
#include <sys/types.h>
#include <optional>
#include "params.h"

namespace elbaf {

enum class Encoding {
	HUFFMAN
};

using code_symbol = std::map<char, size_t>;
using reverse_code_symbol = std::map<size_t, char>;

// Cannot use code_symbol::value_type because this will make the key const.
// Find NOTE(1)
using freq_entry = std::pair<char, size_t>;
class frequency_compare {
public:
	inline bool operator()(const freq_entry& lhs, const freq_entry& rhs) const {
		return lhs.second < rhs.second;
	}
};

template <typename Compare = frequency_compare>
using symbol_queue = std::priority_queue<freq_entry, std::vector<freq_entry>, Compare>;

symbol_queue<> generate_symbols(std::istream& input, Encoding encoding = Encoding::HUFFMAN);
reverse_code_symbol read_symbols(std::istream& input, Encoding encoding = Encoding::HUFFMAN);
symbol_queue<> generate_priority_queue(code_symbol& symbols);

enum class filezone: std::int8_t {
	HEADER_SIZE,
	HEADER_CODES,
	DATA_SIZE,
	DATA
};

const std::int8_t FILESIZE_NB_BYTES = 4;
const std::int8_t BYTE_LEN = 8;
using filesize_t = std::uint32_t;

class Compressor {
public:
	Compressor(filesize_t size, symbol_queue<>& symbols);
	Compressor(filesize_t size, symbol_queue<>&& symbols);
	std::optional<std::byte> next(std::istream& input);
private:
	void generate_symbols();
private:
	// stored as a 4 byte number in the header
	// in network byte order
	filesize_t _filesize;
	std::uint8_t _filesize_byte_number = 0;
	code_symbol _symbols;
	symbol_queue<> _queue;
	filezone _zone = filezone::HEADER_SIZE; 
	std::int8_t _code_number = 0;
	std::byte _current_byte;
	size_t _current_len = 0;
	std::int8_t _bit_number = 8;
};

class Decompressor {
public:
	Decompressor(reverse_code_symbol& symbols): _symbols{symbols} {}
	Decompressor(reverse_code_symbol&& symbols): _symbols{symbols} {}

	std::optional<std::byte> next(std::istream& input);
private:
	reverse_code_symbol _symbols;
	std::int8_t _bit_number = 1;
	std::byte _current_byte;
	int _current_len = 0;
};
void compress(options& opts, Encoding encoding = Encoding::HUFFMAN);
void decompress(options& opts, Encoding encoding = Encoding::HUFFMAN);

}
