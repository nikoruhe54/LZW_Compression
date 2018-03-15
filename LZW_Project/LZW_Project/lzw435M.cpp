/*
This code is derived for UA CS435 from LZW@RosettaCode
Cody Skala
Algorithms Assignment 2 Part 2: LZW Compression
Created: 03/23/2016 14:22
This program will compress and decompress files in the LZW format with varying bit sizes.
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

//This function was provided and converts an int to a binary string with
//a certain amount of bits for the size of the string
string int2BinaryString(int c, int cl) {
	//a binary code string with code length = cl
	string p = "";
	while (c > 0) {
		if (c % 2 == 0)
			p = "0" + p;
		else
			p = "1" + p;
		c = c >> 1;
	}
	int zeros = cl - p.size();
	if (zeros < 0) {
		cout << "Warning: Overflow. code is too big to be coded by " << cl << " bits!\n";
		p = p.substr(p.size() - cl);
	}
	else {
		//pad 0s to left of the binary code if needed
		for (int i = 0; i<zeros; i++)
			p = "0" + p;
	}
	return p;
}

//This will take a binary string and convert it to an int
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

// This function was provided to compress the contents of the file using LZW
void compress(const string &uncompressed, string fileName) {
	//create the binary string to save the compressed file to
	string bcode = "";
	int dictSize = 256;
	map<string, int> dictionary;
	for (int i = 0; i < 256; i++)
		dictionary[string(1, i)] = i;

	string w;
	for (string::const_iterator it = uncompressed.begin(); it != uncompressed.end(); ++it) {
		char c = *it;
		string wc = w + c;
		if (dictionary.count(wc))
			w = wc;
		else {
			//save the numbers to the correct bit length, between 9 and 16
			int bits = ceil(log2(dictSize + 1));
			if (bits > 16)
				bits = 16;
			bcode += int2BinaryString(dictionary[w], bits);
			// Add wc to the dictionary if there is still room in the table of 16 bits
			if (dictSize < 65536)
				dictionary[wc] = dictSize++;
			w = string(1, c);
		}
	}
	// Output the code for w.
	if (!w.empty()) {
		bcode += int2BinaryString(dictionary[w], min((int)ceil(log2(dictSize + 1)), 16));
	}
	//make the compressed file saved with extension .lzw2
	fileName += ".lzw2";
	ofstream myfile;
	myfile.open(fileName.c_str(), ios::binary);

	string zeros = "00000000";
	//make sure the length of the binary string is a multiple of 8
	if (bcode.size() % 8 != 0)
		bcode += zeros.substr(0, 8 - bcode.size() % 8);

	//convert the binary string to characters for simple compression
	int b;
	for (int i = 0; i < bcode.size(); i += 8) {
		b = 1;
		for (int j = 0; j < 8; j++) {
			b = b << 1;
			if (bcode.at(i + j) == '1')
				b += 1;
		}
		//save the string byte by byte
		char c = (char)(b & 255);
		myfile.write(&c, 1);
	}
	myfile.close();
}

// This function was provided to decompress a compressed file back to original using LZW
template <typename Iterator>
string decompress(Iterator begin, Iterator end) {
	int dictSize = 256;
	//Reverse the disctionary created by reversing the map
	map<int, string> dictionary;
	for (int i = 0; i < 256; i++)
		dictionary[i] = string(1, i);

	string w(1, *begin++);
	string result = w;
	string entry;
	for (; begin != end; begin++) {
		int k = *begin;
		if (dictionary.count(k))
			entry = dictionary[k];
		else if (k == dictSize)
			entry = w + w[0];
		else throw "Bad compressed k";

		result += entry;

		// Add w+entry[0] to the dictionary.
		dictionary[dictSize++] = w + entry[0];

		w = entry;
	}
	return result;
}

int main(int argc, char* argv[]) {
	//check the arguments are valid for the program
	if (argc != 3 || (string(argv[1]) != "e" && string(argv[1]) != "c")) {
		cout << "Error, invalid entry! Usage:\n\t./lzw435 c filename\n\t./lzw435 e filename.lzw2" << endl;
		return 0;
	}

	//do this for compression
	if (string(argv[1]) == "c") {
		ifstream inputFile;
		inputFile.open(argv[2]);
		string temp = "";
		char keyChar;
		//get the original file one character at a time and put it in a string
		while (inputFile.get(keyChar))
			temp += keyChar;
		inputFile.close();
		//compress the file which is saved in the string temp and save it to a file
		compress(temp, argv[2]);
	}


	else {
		ifstream inputFile;
		ofstream outputFile;
		string str = string(argv[2]);
		if (str.substr(str.size() - 5, str.size()) != ".lzw2") {
			cout << "Error, file not .lzw2 extension!" << endl;
			return 0;
		}
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