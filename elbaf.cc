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

	char tmp;
	while (input.get(tmp)) {
		output.put(tmp);
	}
	
	return true;
}
