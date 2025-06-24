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
	auto file_size = file.size();
	auto& probability = file.get_probabilities();
	auto symbol = symbol::huffman_code(probability);
	std::cout << "symbol table:\n";
	file.display_symbols(symbol);
	std::cout << '\n';

	CodewordReader reader { symbol, input_filename, file_size };
	auto output = std::ofstream{output_filename, std::ios_base::binary};

	std::cout << "Writing compressed data to " << output_filename << '\n';
	write_to_file(output, reader);
	// close the file before reading it during decompression
	output.close();

	//auto reverse_symbol = symbol::reverse_symbols(symbol);
	ReverseCodewordReader reverse_reader {output_filename };
	auto reverse_output = std::ofstream{"original_file.txt", std::ios_base::binary};

	std::cout << "Writing the initial data to original_file.txt\n";
	write_to_file(reverse_output, reverse_reader);

	return 0;
}
