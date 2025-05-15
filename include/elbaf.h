#pragma once
#include <map>
#include <vector>
#include <fstream>
#include <optional>

namespace elbaf {

using symbol_table = std::map<std::byte, std::vector<bool>>;
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
private:
	void deltaCompress(std::ifstream& input, std::ofstream& output);
	void deltaDecompress(std::ifstream& input, std::ofstream& output);
private:
	std::string _filename;
	prob_table _probability;
};

class CodewordReader {
public:
	using bit_number = std::uint8_t;
	CodewordReader(symbol_table& symbol);
	CodewordReader(symbol_table& symbol, const char* filename);
	std::byte next_byte(std::ifstream& input);
	std::optional<std::byte> next_byte();
private:
	std::ifstream _input;
	symbol_table& _symbol;
	bit_number output_bit_no = 0;
	bit_number input_bit_no = 0;
	std::byte current_byte {0x0};
};

void write_to_file(std::ofstream& output, CodewordReader& reader);

}
