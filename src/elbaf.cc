#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <iomanip>
#include <utility>
#include <vector>
#include <cstddef>
#include <bitset>
#include <cassert>

#include "elbaf.h"
#include "symbol.h"

using namespace std;

namespace elbaf {

bool check_parameters(int argc, char** argv) {
	if (argc == 3) {
		return true;
	}

	std::cout << "Usage: elbaf inputfile outputfile\n";

	return false;
}

ElbafFile::ElbafFile(const char *filename):
	_filename { filename }
{}

prob_table& ElbafFile::get_probabilities() {
	return this->_probability;
}

bool ElbafFile::compress() {
	string outname { this->_filename + ".elbaf" };
	ifstream input { this->_filename, ios_base::binary };
	if (!input.good()) {
		std::cerr << "Error opening file '" << this->_filename << "'\n";
		return false;
	}
	ofstream output { outname, ios_base::binary | ios_base::trunc };
	if (!output.good()) {
		std::cerr << "Error opening '" << outname << "'\n";
		return false;
	}

	deltaCompress(input, output);
	output.flush();

	ifstream compressed { outname, ios_base::binary };
	if (!compressed.good()) {
		std::cerr << "Error opening file '" << outname << "'\n";
		return false;
	}
	string decompressed_name = outname + ".back";
	ofstream decompressed { decompressed_name, ios_base::binary | ios_base::trunc };
	if (!decompressed.good()) {
		std::cerr << "Error opening '" << decompressed_name << "'\n";
		return false;
	}

	deltaDecompress(compressed, decompressed);
	
	return true;
}

void ElbafFile::set_probabilities() {
	string outname { this->_filename + ".elbaf" };
	ifstream input { this->_filename, ios_base::binary };
	if (!input.good()) {
		std::cerr << "Error opening file '" << this->_filename << "'\n";
		return;
	}

	_size = 0;
	char current;
	std::cout << "Data: ";
	while (input.get(current)) {
		auto current_byte = static_cast<std::byte>(current);
		if (this->_probability.count(current_byte) == 0)
			this->_probability[current_byte] = 0;
		this->_probability[current_byte]++;
		std::cout << static_cast<int>(current_byte) << ' ';
		_size++;
	}
	std::cout << '\n';

	if (_size == 0)
		return;

	for (auto& [key, value]: this->_probability)
		value = value * 100 / _size;
}

void ElbafFile::display_probabilities() {
	for (const auto& [key, value]: this->_probability)
		std::cout
			<< "0x" << std::hex << std::setw(8) << std::setfill('0')
			<< static_cast<unsigned int>(static_cast<unsigned char>(key))
			<< std::dec
			<< ": " << value << '\n';
	std::cout << "The dictionary has " << this->_probability.size() << " symbols\n";
}

void ElbafFile::display_symbols(symbol_table& symbol) {
	for (const auto& [key, value]: symbol) {
		auto i = static_cast<uint8_t>(key);
		std::cout << std::bitset<8>(i) << '(' << static_cast<int>(i) << ')';
		std::cout << ": 0x";
		for (auto bit: value) {
			if (bit)
				std::cout << '1';
			else
				std::cout << '0';
		}
		std::cout << '\n';
	}
	std::cout << "The dictionary has " << symbol.size() << " symbols\n";
}

void next_state(HeaderState* state) {
	switch (*state) {
	case HeaderState::nb_bytes:
		*state = HeaderState::dict_key;
		break;
	case HeaderState::dict_key:
		*state = HeaderState::dict_value;
		break;
	case HeaderState::dict_value:
		*state = HeaderState::content;
		break;
	default:
		*state = HeaderState::content;
		break;
	}
}

symbol_list get_symbols_list(symbol_table& symbol) {
	return {symbol.begin(), symbol.end()};
}

CodewordReader::CodewordReader(symbol_table& symbol)
	: _symbol{symbol}
{}
CodewordReader::CodewordReader(symbol_table& symbol, const char* filename, uint8_t nb_bytes)
	: _input{filename, std::ios_base::out | std::ios_base::binary}, _symbol{symbol},
	_nb_bytes_left{nb_bytes}
{}

std::optional<std::byte> CodewordReader::next_byte(std::ifstream& input) {
	if (_nb_bytes_left == 0)
		return std::nullopt;

	std::byte ret;
	if (_state == HeaderState::nb_bytes) {
		ret = static_cast<std::byte>(_nb_bytes_left);
		next_state(&_state);
		return ret;
	} else if (_state == HeaderState::dict_key) {
		if (_symbol_list.size() == 0)
			_symbol_list = get_symbols_list(_symbol);

		if (_symbol_index == _symbol_list.size()) {
			// Put the previous byte again. This is how the decompressor
			// will detect the end of the dict_key part
			ret = _symbol_list[_symbol_index-1].first;
			next_state(&_state);
		} else {
			ret = _symbol_list[_symbol_index].first;
			_symbol_index++;
		}

		return make_optional(ret);
	}

	if (!input.good())
		return std::nullopt;

	char tmp;
	input.get(tmp);
	const std::vector<bool>* current = &this->_symbol[std::byte{static_cast<uint8_t>(tmp)}];
	while (this->output_bit_no < BYTE_LEN) {
		uint8_t mask = 0b10000000 >> this->output_bit_no;
		if ((*current)[this->input_bit_no])
			this->current_byte |= std::byte{mask};

		this->output_bit_no++;
		this->input_bit_no++;
		if (this->input_bit_no == current->size()) {
			this->input_bit_no = 0;
			if (!input.get(tmp))
				break;
			current = &this->_symbol[std::byte{static_cast<uint8_t>(tmp)}];
		}
	}

	ret = this->current_byte;
	if (this->output_bit_no == BYTE_LEN) {
		this->output_bit_no = 0;
		this->current_byte = std::byte{0x0};
	}

	if (this->input_bit_no < current->size())
		input.putback(tmp);

	_nb_bytes_left--;
	return std::make_optional(ret);
}

std::optional<std::byte> CodewordReader::next_byte() {
	return next_byte(_input);
}

ReverseCodewordReader::ReverseCodewordReader(reverse_symbol_table *const symbol):
	_reverse_symbol{symbol}
{}

ReverseCodewordReader::ReverseCodewordReader(
	reverse_symbol_table *const symbol, const char* filename):
	_reverse_symbol {symbol},
	_input{filename, std::ios_base::in | std::ios_base::binary}
{}

std::optional<std::byte> ReverseCodewordReader::next_byte(std::ifstream& input) {
	if (_nb_bytes_left == 0 && _state != HeaderState::nb_bytes)
		return std::nullopt;

	if (!input.good()) {
		std::cout << "input is not good\n";
		return std::nullopt;
	}

	char tmp;
	input.get(tmp);

	if (_state == HeaderState::nb_bytes) {
		_nb_bytes_left = tmp;
		assert(_nb_bytes_left > 0);

		next_state(&_state);
		return next_byte(input);
	} else if (_state == HeaderState::dict_key) {
		auto key = static_cast<std::byte>(tmp);
		std::cout << "key = " << static_cast<int>(key) << '\n';

		if (_prev_byte_key == key && _symbol_list.size() > 0)
			next_state(&_state);
		else
			_symbol_list.push_back(std::make_pair(key, std::vector<bool>{}));

		_prev_byte_key = key;

		return next_byte(input);
	}

	std::vector<bool> current_symbol;
	while(_nb_bytes_left) {
		auto bool_val = symbol::is_set_bit(tmp, this->_input_bit_no);
		current_symbol.push_back(bool_val);
		this->increment_bit_no();

		auto it = this->_reverse_symbol->find(current_symbol);
		if (it != end(*this->_reverse_symbol)) {
			if (this->_input_bit_no > 0)
				input.putback(tmp);
			_nb_bytes_left--;
			return make_optional(it->second);
		}

		if (this->_input_bit_no == 0) {
			if (!input.good())
				break;
			input.get(tmp);
		}
	}
	std::cout << "last symbol length: " << current_symbol.size() << '\n';

	return std::nullopt;
}

std::optional<std::byte> ReverseCodewordReader::next_byte() {
	return this->next_byte(this->_input);
}

void ReverseCodewordReader::increment_bit_no() {
	this->_input_bit_no = (this->_input_bit_no + 1) % BYTE_LEN;
}


void ElbafFile::display_uncompressed_bytes() {
	ifstream input { this->_filename, ios_base::binary };
	if (!input.good()) {
		std::cerr << "Error opening file '" << this->_filename << "'\n";
		return;
	}

	char tmp;
	int i = 0;
	while (input.get(tmp)) {
		std::cout << std::bitset<8>(tmp) << '\n';
		i++;
	}
	std::cout << i << " bytes\n";
}

void ElbafFile::display_compressed_bytes(symbol_table& symbol) {
	ifstream input { this->_filename, ios_base::binary };
	if (!input.good()) {
		std::cerr << "Error opening file '" << this->_filename << "'\n";
		return;
	}

	char tmp;
	int i = 0;
	CodewordReader reader {symbol};
	while (input.good()) {
		auto next = reader.next_byte(input);
		if (next)
			std::cout << std::bitset<8>(static_cast<uint8_t>(next.value())) << '\n';
		i++;
	}
	std::cout << i << " bytes\n";
}

void write_to_file(std::ofstream& output, GenericReader& reader) {
	if (!output.good()) {
		std::cout << "Cannot write to the file\n";
		return;
	}

	while (true) {
		auto byte = reader.next_byte();
		if (!byte)
			break;

		output << static_cast<char>(byte.value());
	}
}

// NOTE: does not change the file size as data is grouped by byte,
//       so (current - previous) will take as many bits as current
//       or previous
void ElbafFile::deltaCompress(ifstream& input, ofstream& output) {
	int i = 0;
	char current;
	char previous = 0;
	while (input.get(current)) {
		if (i == 0)
			output.put(current);
		else
			output.put(current - previous);
		previous = current;
		i++;
	}
}

void ElbafFile::deltaDecompress(ifstream& input, ofstream& output) {
	int i = 0;
	char current;
	char previous = 0;
	while (input.get(current)) {
		char next = current + previous;
		if (i == 0)
			output.put(current);
		else
			output.put(next);
		previous = next;
		i++;
	}
}

}
