#pragma once
#include <map>
#include <fstream>

using namespace std;

bool check_parameters(int argc, char** argv);

class ElbafFile {
public:
	ElbafFile(char* filename);
	void set_probabilities();
	void display_probabilities();
	bool compress();
private:
	void deltaCompress(ifstream& input, ofstream& output);
	void deltaDecompress(ifstream& input, ofstream& output);
private:
	string filename;
	std::map<char, double> probability;
};
