#include <iostream>
#include <fstream>
#include "elbaf.h"
#include "symbol.h"

using namespace elbaf;

int main(int argc, char** argv)
{
	if (!check_parameters(argc, argv))
		return -1;
	
	const char* input_filename = argv[1];
	const char* output_filename = argv[2];

	ElbafFile file { input_filename };
	file.set_probabilities();
	auto& probability = file.get_probabilities();
	auto symbol = symbol::unary_code(probability);

	CodewordReader reader { symbol, input_filename };
	auto output = std::ofstream{output_filename, std::ios_base::binary};

	std::cout << "Writing compressed data to " << output_filename << '\n';
	write_to_file(output, reader);

	return 0;
}
