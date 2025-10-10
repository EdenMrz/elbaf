#include <cstdio>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <queue>
#include <cstddef>
#include <sys/types.h>
#include <optional>
#include "params.h"

using namespace elbaf;

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

reverse_code_symbol read_symbols(std::istream& input, Encoding encoding = Encoding::HUFFMAN) {
	reverse_code_symbol symbols;
	char c;
	input.get(c);

	// should not happen, the input stream should at least contain the file header
	if (!input.good())
		return {};

	int len = c;
	for (int i = 1; i <= len; ++i) {
		char key;
		if (!input.get(key))
			return {};

		symbols[i] = key;
	}

	return symbols;
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

Compressor::Compressor(filesize_t size, symbol_queue<>& queue): _filesize{size}, _queue{queue} {
	generate_symbols();
}
Compressor::Compressor(filesize_t size, symbol_queue<>&& queue): _filesize{size}, _queue{queue} {
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
		_zone = filezone::DATA_SIZE;
	case filezone::DATA_SIZE:
		if (_filesize_byte_number == 4) {
			_zone = filezone::DATA;
		} else {
			filesize_t tmp = _filesize >> (8*_filesize_byte_number) & 0xff;
			std::byte ret { static_cast<unsigned char>(tmp) };
			++_filesize_byte_number;
			return ret;
		}
	default:
		// Do the rest of this function
		break;
	}

	// all bytes have been read
	if (_current_len == 0 && !input.good())
		return {};

	while (true) {
		if (_current_len == 0) {
			char c;
			if (!input.get(c))
				break;
			_current_len = _symbols[c];
		}

		// BUG: the length of the longest (ie rarest) symbol should be len-1 but with the last bit set to 1
		std::byte mask = std::byte{0x1} << (_bit_number-1);
		if (_current_len == 1)
			_current_byte &= ~mask;
		else
			_current_byte |= mask;

		--_current_len;
		--_bit_number;
		if (_bit_number == 0) {
			_bit_number = BYTE_LEN;
			break;
		}
	}

	opt = _current_byte;
	_current_byte = std::byte{0};
	return opt;
}

std::optional<std::byte> Decompressor::next(std::istream& input) {
	int read_bit_len = 0;
	while (true) {
		++_current_len;
		--_bit_number;
		if (_bit_number == 0) {
			char c;
			if (!input.get(c))
				return {};
			_current_byte = std::byte{static_cast<unsigned char>(c)};
			_bit_number = BYTE_LEN;
		}

		std::byte mask = std::byte{0x1} << (_bit_number-1);
		std::byte tmp = _current_byte & mask;
		bool is_set = tmp == mask;
		if (!is_set) {
			auto key = _current_len;
			_current_len = 0;
			return std::byte{static_cast<unsigned char>(_symbols[key])};
		}
	}
}

void compress(options& opts, Encoding encoding = Encoding::HUFFMAN) {
	std::uint32_t filesize = std::filesystem::file_size(opts.input_file);
	auto input = std::ifstream(opts.input_file, std::ios_base::binary);
	Compressor comp { filesize, generate_symbols(input, encoding) };

	input.clear();
	input.seekg(0, std::ios::beg);
	auto output = std::ofstream{opts.output_file, std::ios_base::binary};

	for (auto byte = comp.next(input); byte.has_value(); byte = comp.next(input))
		output.put(static_cast<char>(byte.value()));
	output.flush();
}

void decompress(options& opts, Encoding encoding = Encoding::HUFFMAN) {
	auto input = std::ifstream(opts.input_file, std::ios_base::binary);
	Decompressor decomp { read_symbols(input, encoding) };

	auto output = std::ofstream{opts.output_file, std::ios_base::binary};

	// BUG: take system endianness into account
	filesize_t filesize = 0;
	for (int i = 0; i < FILESIZE_NB_BYTES; ++i) {
		char c;
		input.get(c);
		auto uc = static_cast<unsigned char>(c);
		auto uic = static_cast<filesize_t>(uc);
		// Be carefull of sign extension when casting from c's signed type to filesize's unsigned type
		filesize = filesize + (static_cast<unsigned char>(c) << (8*i));
	}

	for (auto byte = decomp.next(input); byte.has_value() && filesize > 0;) {
		output.put(static_cast<char>(byte.value()));

		byte = decomp.next(input);
		--filesize;
	}
}

int main(int argc, char** argv)
{
	options opts;
	if (!check_parameters(argc, argv, &opts))
		return -1;

	if (opts.compression)
		compress(opts);
	else
		decompress(opts);

	return 0;
}
