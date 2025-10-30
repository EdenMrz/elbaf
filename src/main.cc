#include "params.h"
#include "compressor.h"

using namespace elbaf;

int main(int argc, char** argv)
{
	options opts;
	if (!check_parameters(argc, argv, &opts))
		return -1;

	if (opts.compression)
		compress(opts);
	else
		decompress(opts);

	return 0;
}
