#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>

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
		if (this->probability.count(current) == 0)
			this->probability[current] = 0;
		this->probability[current]++;
		total++;
	}

	if (total == 0)
		return;

	for (auto& [key, value]: this->probability)
		value = value * 100 / total;
}

void ElbafFile::display_probabilities() {
	for (const auto& [key, value]: this->probability)
		std::cout
			<< "0x" << std::hex << std::setw(8) << std::setfill('0')
			<< static_cast<unsigned int>(static_cast<unsigned char>(key))
			<< ": " << value << '\n';
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
