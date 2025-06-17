#pragma once
#include <map>
#include <vector>
#include <fstream>
#include <optional>

namespace elbaf {

using symbol_table = std::map<std::byte, std::vector<bool>>;
using reverse_symbol_table = std::map<std::vector<bool>, std::byte>;
using prob_table = std::map<std::byte, double>;

bool check_parameters(int argc, char** argv);
class ElbafFile {
public:
	ElbafFile(const char* filename);
	void set_probabilities();
	void display_probabilities();
	void display_symbols(symbol_table& symbol);
	void display_uncompressed_bytes();
	void display_compressed_bytes(symbol_table& symbol);
	bool compress();
	prob_table& get_probabilities();
	size_t size() { return _size; }
private:
	void deltaCompress(std::ifstream& input, std::ofstream& output);
	void deltaDecompress(std::ifstream& input, std::ofstream& output);
private:
	std::string _filename;
	prob_table _probability;
	size_t _size = 0;
};

class GenericReader {
public:
	virtual std::optional<std::byte> next_byte() = 0;
	const uint8_t BYTE_LEN = 8;
	using bit_number = std::uint8_t;
};

class CodewordReader: public GenericReader {
public:
	CodewordReader(symbol_table& symbol);
	CodewordReader(symbol_table& symbol, const char* filename);
	std::byte next_byte(std::ifstream& input);
	std::optional<std::byte> next_byte() override;
private:
	std::ifstream _input;
	symbol_table& _symbol;
	// bit number 0 is the left-most one
	bit_number output_bit_no = 0;
	bit_number input_bit_no = 0;
	std::byte current_byte {0x0};
};

class ReverseCodewordReader: public GenericReader {
public:
	ReverseCodewordReader(reverse_symbol_table * const symbol);
	ReverseCodewordReader(reverse_symbol_table * const symbol, const char* filename);
	std::optional<std::byte> next_byte(std::ifstream& input);
	std::optional<std::byte> next_byte() override;
private:
	std::ifstream _input;
	reverse_symbol_table* const _reverse_symbol;
	// bit number 0 is the left-most one
	bit_number _input_bit_no = 0;
private:
	void increment_bit_no();
};

void write_to_file(std::ofstream& output, GenericReader& reader, size_t size);

}
