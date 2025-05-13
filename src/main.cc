#include <iostream>
#include "elbaf.h"

int main(int argc, char** argv)
{
	if (!elbaf::check_parameters(argc, argv))
		return -1;
	
	elbaf::ElbafFile file { argv[1] };
	bool compression_ok = file.compress();
	if (!compression_ok) {
		std::cerr << "Compression unsuccessful\n";
		return -1;
	}

	file.set_probabilities();
	file.set_symbols();
	file.display_symbols();

	std::cout << "\n\n";
	file.display_uncompressed_bytes();
	std::cout << "\n\n";
	file.display_compressed_bytes();

	return 0;
}
