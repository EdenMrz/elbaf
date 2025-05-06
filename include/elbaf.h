#pragma once
#include <map>
#include <fstream>

namespace elbaf {

bool check_parameters(int argc, char** argv);

class ElbafFile {
public:
	ElbafFile(char* filename);
	void set_probabilities();
	void display_probabilities();
	bool compress();
private:
	void deltaCompress(std::ifstream& input, std::ofstream& output);
	void deltaDecompress(std::ifstream& input, std::ofstream& output);
private:
	std::string filename;
	std::map<char, double> probability;
};

}
