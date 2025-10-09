#include <cstring>
#include <iostream>
#include <cassert>
#include <filesystem>

#include "elbaf.h"

using namespace std;

namespace elbaf {

void print_usage() {
	std::cout
		<< "Usage:\n"
		<< "  elbaf [options] inputfile outputfile\n\n"
		<< "Options:\n"
		<< "  -x: decompression\n"
		<< '\n';
}

bool check_parameters(int argc, char** argv, options* opts) {
	for (int i = 1; i < argc && (argc == 3 || argc == 4); i++) {
		if (strcmp(argv[i], "-x") == 0)
			opts->compression = false;
		else if (opts->input_file == nullptr)
			opts->input_file = argv[i];
		else
			opts->output_file = argv[i];
	}

	if (opts->input_file == nullptr || opts->output_file == nullptr) {
		print_usage();
		return false;
	}

	if (!std::filesystem::exists(opts->input_file)) {
		std::cout << opts->input_file << " does not exist\n";
		return false;
	}

	return true;
}

}
