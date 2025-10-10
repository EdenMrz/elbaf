#pragma once


namespace elbaf {

struct options {
	bool compression = true;
	const char* input_file = nullptr;
	const char* output_file = nullptr;
};

bool check_parameters(int argc, char** argv, options* opt);

}
