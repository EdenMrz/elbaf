#include <fstream>
#include <iostream>
#include <string>

#include "elbaf.h"


int main(int argc, char** argv) {
	if (!check_parameters(argc, argv))
		return -1;
	
	ElbafFile file { argv[1] };
	bool compression_ok = file.compress();
	if (!compression_ok) {
		std::cerr << "Compression unsuccessful\n";
		return -1;
	}

	return 0;
}

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
