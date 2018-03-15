#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <cmath>
#include <sys/stat.h>

/*
This code is derived for UA CS435 from LZW@RosettaCode
*/

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
			// Add wc to the dictionary.
			if (dictSize < 65536)
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
	std::string entry;
	for (; begin != end; begin++) {
		int k = *begin;
		if (dictionary.count(k))
			entry = dictionary[k];
		else if (k == dictSize)
			entry = w + w[0];
		else
			std::cout << k << "\n";

		result += entry;

		// Add w+entry[0] to the dictionary.
		if (dictSize < 65536)
			dictionary[dictSize++] = w + entry[0];


		w = entry;
	}
	return result;
}

std::string int2BinaryString(int c, int cl) {
	std::string p = ""; //a binary code string with code length = cl
	while (c>0) {
		if (c % 2 == 0)
			p = "0" + p;
		else
			p = "1" + p;
		c = c >> 1;
	}
	int zeros = cl - p.size();
	if (zeros<0) {
		std::cout << "\nWarning: Overflow. code is too big to be coded by " << cl << " bits!\n";
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

void binaryIODemo(std::vector<int> compressed, std::string daName) {
	int bits;
	std::string p;

	std::string bcode = "";
	for (std::vector<int>::iterator it = compressed.begin(); it != compressed.end(); ++it) {
		int where = it - compressed.begin();
		if (where<256)
			bits = 9;
		else if (256 <= where && where<768)
			bits = 10;
		else if (768 <= where && where<1792)
			bits = 11;
		else if (1792 <= where && where<3840)
			bits = 12;
		else if (3840 <= where && where<7936)
			bits = 13;
		else if (7936 <= where && where<16128)
			bits = 14;
		else if (16128 <= where && where<32512)
			bits = 15;
		else
			bits = 16;
		p = int2BinaryString(*it, bits);
		bcode += p;
	}

	//writing to file
	std::string fileName = daName + ".lzw";
	std::ofstream myfile;
	myfile.open(fileName.c_str(), std::ios::binary);

	std::string zeros = "00000000";
	if (bcode.size() % 8 != 0) //make sure the length of the binary string is a multiple of 8
		bcode += zeros.substr(0, 8 - bcode.size() % 8);

	int b;
	for (int i = 0; i < bcode.size(); i += 8) {
		b = 1;
		for (int j = 0; j < 8; j++) {
			b = b << 1;
			if (bcode.at(i + j) == '1')
				b += 1;
		}
		char c = (char)(b & 255); //save the string byte by byte
		myfile.write(&c, 1);
	}
	myfile.close();

}


std::string ReadFile(std::string fileName) {
	//reading from a file
	std::string zeros = "00000000";
	std::ifstream myfile2;
	myfile2.open(fileName.c_str(), std::ios::binary);

	struct stat filestatus;
	stat(fileName.c_str(), &filestatus);
	long fsize = filestatus.st_size; //get the size of the file in bytes

	char c2[fsize];
	myfile2.read(c2, fsize);

	std::string s = "";
	long count = 0;
	while (count<fsize) {
		unsigned char uc = (unsigned char)c2[count];
		std::string p = ""; //a binary string
		for (int j = 0; j<8 && uc>0; j++) {
			if (uc % 2 == 0)
				p = "0" + p;
			else
				p = "1" + p;
			uc = uc >> 1;
		}
		p = zeros.substr(0, 8 - p.size()) + p; //pad 0s to left if needed
		s += p;
		count++;
	}
	myfile2.close();

	return s;
}



int main(int argc, char* argv[]) {

	// if 0 arguments
	if (argc != 3)
	{
		std::cout << "Invalid arguments.You should run this program in terminal with 2 arguments." << std::endl;
		exit(1);
	}

	// if incorrect arguments
	else if (*argv[1] != 'c' && *argv[1] != 'e') {
		std::cout << "Run Program with c or e followed by filename as specified in directions on class site." << std::endl;
		exit(1);
	}

	else {
		//Compress
		if (*argv[1] == 'c') {
			// open file to compress
			std::ifstream inputFile;
			inputFile.open(argv[2]);

			//convert file bytes to string

			std::string fBytes((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());


			std::vector<int> compressed;
			compress(fBytes, std::back_inserter(compressed));

			binaryIODemo(compressed, argv[2]);

		}

		else {

			std::string fThings = ReadFile(argv[2]);
			std::string temp;

			std::vector<int> compressed;

			int here = 0;


			for (int i = 0; i<256; i++) {
				for (int j = 0; j<9; j++) {
					if (fThings.size() <= here)
						break;
					temp += fThings.at(here);
					here++;
				}
				if (fThings.size() <= here)
					break;
				int nums = binaryString2Int(temp);
				compressed.push_back(nums);
				temp = "";
			}

			for (int i = 0; i<512; i++) {
				for (int j = 0; j<10; j++) {
					if (fThings.size() <= here)
						break;
					temp += fThings.at(here);
					here++;
				}
				if (fThings.size() <= here)
					break;
				int nums = binaryString2Int(temp);
				compressed.push_back(nums);
				temp = "";
			}


			for (int i = 0; i<1024; i++) {
				for (int j = 0; j<11; j++) {
					if (fThings.size() <= here)
						break;
					temp += fThings.at(here);
					here++;
				}
				if (fThings.size() <= here)
					break;
				int nums = binaryString2Int(temp);
				compressed.push_back(nums);
				temp = "";
			}


			for (int i = 0; i<2048; i++) {
				for (int j = 0; j<12; j++) {
					if (fThings.size() <= here)
						break;
					temp += fThings.at(here);
					here++;
				}
				if (fThings.size() <= here)
					break;
				int nums = binaryString2Int(temp);
				compressed.push_back(nums);
				temp = "";
			}


			for (int i = 0; i<4096; i++) {
				for (int j = 0; j<13; j++) {
					if (fThings.size() <= here)
						break;
					temp += fThings.at(here);
					here++;
				}
				if (fThings.size() <= here)
					break;
				int nums = binaryString2Int(temp);
				compressed.push_back(nums);
				temp = "";
			}


			for (int i = 0; i<8192; i++) {
				for (int j = 0; j<14; j++) {
					if (fThings.size() <= here)
						break;
					temp += fThings.at(here);
					here++;
				}
				if (fThings.size() <= here)
					break;
				int nums = binaryString2Int(temp);
				compressed.push_back(nums);
				temp = "";
			}


			for (int i = 0; i<16384; i++) {
				for (int j = 0; j<15; j++) {
					if (fThings.size() <= here)
						break;
					temp += fThings.at(here);
					here++;
				}
				if (fThings.size() <= here)
					break;
				int nums = binaryString2Int(temp);
				compressed.push_back(nums);
				temp = "";
			}

			int last = here - compressed.size();

			for (int i = 0; i<last; i++) {
				for (int j = 0; j<16; j++) {
					if (fThings.size() <= here)
						break;
					temp += fThings.at(here);
					here++;
				}
				if (fThings.size() <= here)
					break;
				int nums = binaryString2Int(temp);
				compressed.push_back(nums);
				temp = "";
			}

			std::string decompressed = decompress(compressed.begin(), compressed.end());


			std::string outName = argv[2];
			for (int i = 0; i<4; i++)
				outName.erase(outName.end() - 1);

			std::ofstream outputFile;
			outputFile.open("Expanded " + outName);
			outputFile << decompressed;
			outputFile.close();

		}
	}

	return 0;
}