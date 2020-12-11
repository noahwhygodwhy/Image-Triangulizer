#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <string>
#include <iostream>

#include "GLSLReader.hpp"

using namespace std;


string fileToString(string filename)
{
	//printf("opening file %s\n", filename.c_str());

	string content;
	int length;
	ifstream stream(filename);
	stream.seekg(0, ios::end);
	length = stream.tellg();
	stream.seekg(0, ios::beg);
	content.resize(length);
	//TODO: this might be broken, remove the (char*) and see what happens
	stream.read((char*)content.data(), length);
	//printf("returning\n");
	//cout << content << endl;
	return content;
	//stream.close();
	//return content;
}