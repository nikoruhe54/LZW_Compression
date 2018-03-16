/*
Nikolai Ruhe
Algorithms Project 2: LZW Compression, Part I
March 16, 2018
Dr. Duan
Source code is derived and modified from LZW@RosettaCode for UA CS435
*/
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <math.h>
#include <sys/stat.h>
using namespace std;

std::string int2BinaryString(int c, int cl) {
	std::string p = ""; //a binary code string with code length = cl
	int code = c;
	while (c>0) {
		if (c % 2 == 0)
			p = "0" + p;
		else
			p = "1" + p;
		c = c >> 1;
	}
	int zeros = cl - p.size();
	if (zeros<0) {
		std::cout << "\nWarning: Overflow. code " << code << " is too big to be coded by " << cl << " bits!\n";
		p = p.substr(p.size() - cl);
	}
	else {
		for (int i = 0; i<zeros; i++)  //pad 0s to left of the binary code if needed
			p = "0" + p;
	}
	return p;
}

int binaryString2Int(string p) {
	int code = 0;
	if (p.size()>0) {
		if (p.at(0) == '1')
			code = 1;
		p = p.substr(1);
		while (p.size() > 0) {
			code = code << 1;
			if (p.at(0) == '1')
				code++;
			p = p.substr(1);
		}
	}
	return code;
}

void getNumberofBits(int &bits, int dictionarySize) {
	bits = ceil(log2(dictionarySize + 1));
	if (bits > 16) {
		bits = 16;
	}
}

void getBinaryCode(string &binaryCode, map<string, int>dictionary, int dictionarySize, string w) {
	if (!w.empty()) {
		binaryCode += int2BinaryString(dictionary[w], min((int)ceil(log2(dictionarySize + 1)), 16));
	}
}

void populateDictionary(map<string, int> &dictionary, int &dictionarySize, string &word, string wc, char c) {
	if (dictionarySize < 65536)
		dictionary[wc] = dictionarySize++;
	word = string(1, c);
}

// Compress a string to a list of output symbols.
// The result will be written to the output iterator
// starting at "result"; the final iterator is returned.
void compress(const string &uncompressed, string fileName) {
	// Build the dictionary
	string binaryCode = "";
	int tableSize = 256, bytes = 1, bits;
	map<string, int> dictionary;
	string w;
	for (int i = 0; i < 256; i++)
		dictionary[string(1, i)] = i;

	cout << "Writing File.... may take a few seconds for large files \n";
	for (string::const_iterator it = uncompressed.begin(); 
		it != uncompressed.end(); ++it) {
		char c = *it;
		string wc = w + c;
		if (dictionary.count(wc))
			w = wc;
		else {
			//ensure that there are between 9 and 16 bits
			getNumberofBits(bits, tableSize);
			binaryCode += int2BinaryString(dictionary[w], bits);

			//if space is available, add wc to dictionary
			populateDictionary(dictionary, tableSize, w, wc, c);
		}
	}

	getBinaryCode(binaryCode, dictionary, tableSize, w);

	//give compressed file lzwM extension
	fileName += ".lzwM";

	ofstream myfile;
	myfile.open(fileName.c_str(), ios::binary);
	string zeros = "00000000";
	//make the length of the binary divisible by 8
	if (binaryCode.size() % 8 != 0)
		binaryCode += zeros.substr(0, 8 - binaryCode.size() % 8);

	//make the binary stream form characters
	//this will help with compression
	for (int i = 0; i < binaryCode.size(); i += 8) {
		for (int j = 0; j < 8; j++) {
			bytes = bytes << 1;
			if (binaryCode.at(i + j) == '1')
				bytes += 1;
		}
		//the string should be saved in each byte
		char c = (char)(bytes & 255);
		myfile.write(&c, 1);
	}
	myfile.close();
}

// Decompress a list of output ks to a string.
// "begin" and "end" must form a valid range of ints
template <typename Iterator>
string decompress(Iterator begin, Iterator end) {
	//build the dictionary
	int tableSize = 256;
	map<int, string> dictionary;
	for (int i = 0; i < 256; i++)
		dictionary[i] = string(1, i);

	string w(1, *begin++);
	string result = w;
	cout << result << "???:::\n";
	string entry;
	for (; begin != end; begin++) {
		int k = *begin;
		if (dictionary.count(k))
			entry = dictionary[k];
		else if (k == tableSize)
			entry = w + w[0];
		else 
			throw "Bad compressed k";

		result += entry;

		// Add w+entry[0] to the dictionary.
		dictionary[tableSize++] = w + entry[0];

		w = entry;
	}
	return result;
}

string readFileIO(string filename) {
	std::ifstream infile(filename.c_str(), std::ios::binary);
	std::streampos begin;
	std::streampos end;
	begin = infile.tellg();
	infile.seekg(0, std::ios::end);
	end = infile.tellg();
	infile.seekg(0, std::ios::beg);
	std::streampos size = end - begin;

	char* mem_Block = new char[size];
	infile.read(mem_Block, size);
	mem_Block[size] = '\0';
	infile.close();

	return string(mem_Block, size);
}

int main(int argc, char* argv[]) {

	//check the arguments are valid for the program
	if (argc != 3 || (*argv[1] != 'e' && *argv[1] != 'c')) {
		cout << "Error, not valid argument parameters" << endl;
		return 0;
	}

	//compress the file
	if (*argv[1] == 'c') {
		ifstream infile;
		string filename(argv[2]);
		std::vector<int>compressDoc;
		string doc;
		doc = readFileIO(filename);
		compress(doc, argv[2]);
		infile.close();
	}

	//decompress the file
	if (*argv[1] == 'e') {
		ifstream inputFile;
		ofstream outputFile;
		string str = string(argv[2]);

		//return an error if the wrong type of file is tried to be expanded
		/*if (str.substr(str.size() - 5, str.size()) != ".lzwM") {
			cout << "Error, file not .lzwM extension!" << endl;
			return 0;
		}
		*/
		str = str.substr(0, str.size() - 5) + "2M";
		inputFile.open(argv[2], ios::binary);
		outputFile.open(str.c_str());
		vector<int> compressed;
		string binNumber = "";
		char keyChar;
		//get the original file one character at a time and put it in a string
		while (inputFile.get(keyChar)) {
			for (int i = 7; i >= 0; --i)
				binNumber += ((keyChar >> i) & 1) ? "1" : "0";
		}
		int bitMath = 1;
		int test = 0;
		//read certain number of bits at a time, find the interger equivalent,
		//and push it to the vector
		for (int i = 0; i < binNumber.size(); i = i + min((int)ceil(log2(bitMath + 255)), 16)) {
			int bits = ceil(log2(bitMath + 256));
			if (bits > 16) bits = 16;
			bitMath++;
			string current = "";
			for (int j = i; j < i + bits; ++j) {
				test++;
				if (test > binNumber.size())
					break;
				current += binNumber[j];
			}
			if (test > binNumber.size())
				break;
			compressed.push_back(binaryString2Int(current));
		}
		//decompress the vector and get the original contents back
		string decompressed = decompress(compressed.begin(), compressed.end());
		//save the original contents to filename2M
		outputFile << decompressed;
		inputFile.close();
		outputFile.close();
	}
	return 0;
}