#pragma once
#include <fstream>

using namespace std;

bool check_parameters(int argc, char** argv);

class ElbafFile {
public:
	ElbafFile(char* filename);
	bool compress();
private:
	string filename;
};
