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
#include <sys/stat.h>

std::string int2BinaryString(int c, int cl);
int binaryString2Int(std::string p);

int CreateBinaryCode(std::string binaryCode, std::string filename) {
	std::ofstream outFile(filename.c_str(), std::ios::binary);
	std::string zeros = "00000000";
	int bytes = 1;

	//make the length of the binary divisible by 8
	if (binaryCode.size() % 8 != 0) {
		binaryCode += zeros.substr(0, 8 - (binaryCode.size() % 8));
	}

	//build the binary code
	for (int i = 0; i < binaryCode.size(); i += 8) {
		for (int j = 0; j < 8; j++) {
			bytes = bytes << 1;
			if (binaryCode.at(i + j) == '1') {
				bytes += 1;
			}
		}
		//the string should be saved in each byte
		char c = (char)(bytes & 255);
		outFile.write(&c, 1);
	}
	outFile.close();
}

void write(std::vector<int> compressed, std::string filename) {
	int c = 65, bits = 12, bytes = 1;

	std::string tempStr = int2BinaryString(c, bits);
	std::cout << "Writing File.... may take a few seconds for large files \n";

	std::string binaryCode = "";
	for (std::vector<int>::iterator it = compressed.begin(); it != compressed.end(); ++it) {
		tempStr = int2BinaryString(*it, bits);
		binaryCode += tempStr;
	}
	CreateBinaryCode(binaryCode, filename);
}

void buildBinaryString(char c2[], long fileSize, std::string zeros, std::string &str) {
	long iterator = 0;

	while (iterator < fileSize) {
		unsigned char temp = (unsigned char)c2[iterator];
		//open a binary string
		std::string buildStr = "";
		for (int j = 0; j < 8 && temp > 0; j++) {
			if (temp % 2 == 0)
				buildStr = "0" + buildStr;
			else
				buildStr = "1" + buildStr;
			temp = temp >> 1;
		}
		//if the string is not an exact byte, add extra 0s
		buildStr = zeros.substr(0, 8 - buildStr.size()) + buildStr;
		str += buildStr;
		iterator++;
	}
}

std::vector<int> BinaryFileInput(std::string filename) {
	std::ifstream myfile2(filename.c_str(), std::ios::binary);
	struct stat stateOfFile;
	std::string zeros = "00000000";
	stat(filename.c_str(), &stateOfFile);
	std::string str = "";
	int bits = 12;
	std::vector<int> result;

	//determine how many bytes the file has
	long fileSize = stateOfFile.st_size;
	char c2[fileSize];
	myfile2.read(c2, fileSize);

	//develope the binary string that will be read from
	buildBinaryString(c2, fileSize, zeros, str);
	myfile2.close();
	if (str.size() % bits != 0) {
		str = std::string(str.data(), (str.size() / bits) * bits);
	}
	for (int i = 0; i < str.length(); i += bits) {
		result.push_back(binaryString2Int(str.substr(i, bits)));
	}
	return result;
}


// Compress a string to a list of output symbols.
// The result will be written to the output iterator
// starting at "result"; the final iterator is returned.
template <typename Iterator>
Iterator compress(const std::string &uncompressed, Iterator result) {
	// Build the dictionary.
	int dictSize = 256;
	std::map<std::string, int> dictionary;
	for (int i = 0; i < 256; i++)
		dictionary[std::string(1, i)] = i;

	std::string w;
	for (std::string::const_iterator it = uncompressed.begin();
		it != uncompressed.end(); ++it) {
		char c = *it;
		std::string wc = w + c;
		if (dictionary.count(wc))
			w = wc;
		else {
			*result++ = dictionary[w];
			// Add wc to the dictionary. Assuming the size is 4096!!!
			if (dictionary.size()<4096)
				dictionary[wc] = dictSize++;
			w = std::string(1, c);
		}
	}

	// Output the code for w.
	if (!w.empty())
		*result++ = dictionary[w];
	return result;
}

// Decompress a list of output ks to a string.
// "begin" and "end" must form a valid range of ints
template <typename Iterator>
std::string decompress(Iterator begin, Iterator end) {
	// Build the dictionary.
	int dictSize = 256;
	std::map<int, std::string> dictionary;
	for (int i = 0; i < 256; i++)
		dictionary[i] = std::string(1, i);

	std::string w(1, *begin++);
	std::string result = w;
	std::cout << result << "???:::\n";
	std::string entry;
	for (; begin != end; begin++) {
		int k = *begin;
		if (dictionary.count(k))
			entry = dictionary[k];
		else if (k == dictSize)
			entry = w + w[0];
		else
			throw "Bad compressed k";

		result += entry;

		// Add w+entry[0] to the dictionary.
		if (dictionary.size()<4096)
			dictionary[dictSize++] = w + entry[0];

		w = entry;
	}
	return result;
}

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

int binaryString2Int(std::string p) {
	int code = 0;
	if (p.size()>0) {
		if (p.at(0) == '1')
			code = 1;
		p = p.substr(1);
		while (p.size()>0) {
			code = code << 1;
			if (p.at(0) == '1')
				code++;
			p = p.substr(1);
		}
	}
	return code;
}

std::string readFileIO(std::string filename) {
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

	return std::string(mem_Block, size);
}

int main(int argc, char* argv[]) {
	
	std::ifstream infile;
	std::string filename(argv[2]);
	std::vector<int>compressDoc;
	std::string doc;

	//compress the file
	if (*argv[1] == 'c') {
		doc = readFileIO(filename);
		compress(doc, std::back_inserter(compressDoc));
		filename += ".lzw";
		write(compressDoc, filename);
	}

	//decompress the file
	if (*argv[1] == 'e') {
		compressDoc = BinaryFileInput(filename);
		doc = decompress(compressDoc.begin(), compressDoc.end());

		filename = filename.substr(0, filename.find_last_of("."));
		auto extension = filename.find_first_of(".");

		if (extension != std::string::npos) {
			filename.insert(filename.find_first_of("."), "2");
		}
		else {
			filename += 2;
		}

		std::ofstream outFile(filename, std::ios::binary);
		outFile.write(doc.data(), doc.size());
	}
}