#pragma once
#include <map>
#include <vector>
#include <fstream>
#include <optional>
#include <cstdint>

namespace elbaf {

using symbol_table = std::map<std::byte, std::vector<bool>>;
using symbol_list = std::vector<std::pair<std::byte, std::vector<bool>>>;
using reverse_symbol_table = std::map<std::vector<bool>, std::byte>;
using prob_table = std::map<std::byte, double>;

struct options {
	bool compression = true;
	const char* input_file = nullptr;
	const char* output_file = nullptr;
};

bool check_parameters(int argc, char** argv, options* opt);
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

// state machine
enum class HeaderState: char {
	nb_bytes, dict_key, dict_key_len, dict_value, content
};

void next_state(HeaderState* state);
symbol_list get_symbols_list(symbol_table& symbol);

class GenericReader {
public:
	virtual std::optional<std::byte> next_byte() = 0;
	const std::uint8_t BYTE_LEN = 8;
	using bit_number = std::uint8_t;
};

class CodewordReader: public GenericReader {
public:
	CodewordReader(symbol_table& symbol);
	CodewordReader(symbol_table& symbol, const char* filename, size_t nb_bytes);
	std::optional<std::byte> next_byte(std::ifstream& input);
	std::optional<std::byte> next_byte() override;
private:
	std::ifstream _input;
	symbol_table& _symbol;
	// bit number 0 is the left-most one
	bit_number output_bit_no = 0;
	bit_number input_bit_no = 0;
	std::byte current_byte {0x0};

	// file header state
	HeaderState _state = HeaderState::nb_bytes;
	symbol_list _symbol_list;
	size_t _symbol_index = 0;
	// NOTE: make nb_bytes fit in one byte for now
	size_t _nb_bytes_left;
};

class ReverseCodewordReader: public GenericReader {
public:
	ReverseCodewordReader();
	ReverseCodewordReader(const char* filename);
	std::optional<std::byte> next_byte(std::ifstream& input);
	std::optional<std::byte> next_byte() override;
private:
	std::ifstream _input;
	reverse_symbol_table _reverse_symbol;
	symbol_list _symbol_list;
	size_t _symbol_index = 0;
	// bit number 0 is the left-most one
	bit_number _output_bit_no = 0;
	bit_number _input_bit_no = 0;

	HeaderState _state = HeaderState::nb_bytes;
	size_t _nb_bytes_left;
	std::byte _prev_byte_key;
private:
	void increment_bit_no();
	void reset_read_states();
	void generate_symbols_map();
};

void write_to_file(std::ofstream& output, GenericReader& reader);

}
