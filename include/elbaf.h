#pragma once
#include <map>
#include <vector>
#include <fstream>
#include <optional>

namespace elbaf {

bool check_parameters(int argc, char** argv);

class ElbafFile {
public:
	using symbol_table = std::map<std::byte, std::vector<bool>>;
	ElbafFile(const char* filename);
	void set_probabilities();
	void set_symbols();
	void display_probabilities();
	void display_symbols();
	void display_uncompressed_bytes();
	void display_compressed_bytes();
	bool compress();
	symbol_table& get_symbols();
private:
	void deltaCompress(std::ifstream& input, std::ofstream& output);
	void deltaDecompress(std::ifstream& input, std::ofstream& output);
private:
	std::string filename;
	std::map<std::byte, double> probability;
	symbol_table symbol;
};

class CodeGenerator {
public:
	virtual std::vector<bool> next() = 0;
};

class UnaryCodeGenerator : public CodeGenerator {
public:
	constexpr UnaryCodeGenerator() {}
	virtual std::vector<bool> next() override;
private:
	size_t _index {0};
};

class CodewordReader {
public:
	using symbol_table = ElbafFile::symbol_table;
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
