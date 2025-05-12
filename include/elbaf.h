#pragma once
#include <map>
#include <vector>
#include <fstream>

namespace elbaf {

bool check_parameters(int argc, char** argv);

class ElbafFile {
public:
	ElbafFile(char* filename);
	void set_probabilities();
	void set_symbols();
	void display_probabilities();
	void display_symbols();
	bool compress();
private:
	void deltaCompress(std::ifstream& input, std::ofstream& output);
	void deltaDecompress(std::ifstream& input, std::ofstream& output);
private:
	std::string filename;
	std::map<std::byte, double> probability;
	std::map<std::byte, std::vector<bool>> symbol;
};

}
