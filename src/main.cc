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
	file.display_probabilities();

	return 0;
}
