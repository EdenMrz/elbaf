#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>

#include "elbaf.h"

using namespace std;

namespace elbaf {

bool check_parameters(int argc, char** argv) {
	if (argc == 2) {
		return true;
	}

	std::cout << "Usage: elbaf file\n";

	return false;
}

ElbafFile::ElbafFile(char *filename):
	filename { filename }
{
}

bool ElbafFile::compress() {
	string outname { this->filename + ".elbaf" };
	ifstream input { this-> filename, ios_base::binary };
	if (!input.good()) {
		std::cerr << "Error opening file '" << this->filename << "'\n";
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
	string outname { this->filename + ".elbaf" };
	ifstream input { this-> filename, ios_base::binary };
	if (!input.good()) {
		std::cerr << "Error opening file '" << this->filename << "'\n";
		return;
	}

	double total = 0;
	char current;
	while (input.get(current)) {
		auto current_byte = static_cast<std::byte>(current);
		if (this->probability.count(current_byte) == 0)
			this->probability[current_byte] = 0;
		this->probability[current_byte]++;
		total++;
	}

	if (total == 0)
		return;

	for (auto& [key, value]: this->probability)
		value = value * 100 / total;
}

// Uses the data in the probabilites table.
// After this call, that table is empty.
void ElbafFile::set_symbols() {
	if (this->probability.size() == 0)
		return;

	std::vector<bool> new_symbol { true };
	while (this->probability.size() > 0) {
		int max = 0;
		std::byte max_key = std::begin(this->probability)->first;
		for (const auto& [ key, value ] : this->probability) {
			if (value < max)
				continue;

			max = value;
			max_key = key;
		}

		this->symbol[max_key] = new_symbol;
		this->probability.erase(max_key);
	}
}

void ElbafFile::display_probabilities() {
	for (const auto& [key, value]: this->probability)
		std::cout
			<< "0x" << std::hex << std::setw(8) << std::setfill('0')
			<< static_cast<unsigned int>(static_cast<unsigned char>(key))
			<< std::dec
			<< ": " << value << '\n';
	std::cout << "The dictionary has " << this->probability.size() << " symbols\n";
}

void ElbafFile::display_symbols() {
	for (const auto& [key, value]: this->symbol) {
		std::cout
			<< "0x" << std::hex << std::setw(8) << std::setfill('0')
			<< static_cast<unsigned int>(static_cast<unsigned char>(key))
			<< std::dec
			<< ": 0x";
		for (auto bit: value) {
			if (bit)
				std::cout << '1';
			else
				std::cout << '0';
		}
		std::cout << '\n';
	}
	std::cout << "The dictionary has " << this->symbol.size() << " symbols\n";
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
