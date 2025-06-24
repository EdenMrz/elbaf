#include <iostream>
#include <fstream>
#include "elbaf.h"
#include "symbol.h"

using namespace elbaf;

int main(int argc, char** argv)
{
	options opts;
	if (!check_parameters(argc, argv, &opts))
		return -1;

	auto output = std::ofstream{opts.output_file, std::ios_base::binary};

	if (opts.compression) {
		ElbafFile file { opts.input_file };
		file.set_probabilities();
		auto file_size = file.size();
		auto& probability = file.get_probabilities();
		auto symbol = symbol::huffman_code(probability);
		std::cout << "symbol table:\n";
		file.display_symbols(symbol);
		std::cout << '\n';

		CodewordReader reader { symbol, opts.input_file, file_size };

		std::cout << "Writing compressed data to " << opts.output_file << '\n';
		write_to_file(output, reader);
	} else {
		ReverseCodewordReader reader { opts.input_file };
		std::cout << "Writing the initial data to " << opts.output_file << '\n';
		write_to_file(output, reader);
	}

	return 0;
}
