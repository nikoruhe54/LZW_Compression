#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector> 
#include <sys/stat.h>

/*
This code is derived from LZW@RosettaCode for UA CS435
*/

std::string int2BinaryString(int c, int cl);
int binaryString2Int(std::string p);

std::string readFileBlockIO(std::string filename) {
	std::ifstream inputFile(filename.c_str(), std::ios::binary);
	std::streampos begin;
	std::streampos end;

	begin = inputFile.tellg();
	inputFile.seekg(0, std::ios::end);
	end = inputFile.tellg();
	inputFile.seekg(0, std::ios::beg);

	std::streampos size = end - begin;

	char* memoryBlock = new char[size];

	inputFile.read(memoryBlock, size);
	memoryBlock[size] = '\0';

	inputFile.close();

	return std::string(memoryBlock, size);
}

void writeFile(std::vector<int> compressed, std::string filename) {

	int c = 69;
	int bits = 9;
	std::string p = int2BinaryString(c, bits);
	std::cout << "c=" << c << " : binary string=" << p << "; back to code=" << binaryString2Int(p) << "\n";

	std::string bcode = "";
	for (std::vector<int>::iterator it = compressed.begin(); it != compressed.end(); ++it) {
		if (*it<256) {
			bits = 8;
		}
		else {
			bits = 9;
		}
		bits = 12;
		p = int2BinaryString(*it, bits);
		std::cout << "c=" << *it << " : binary string=" << p << "; back to code=" << binaryString2Int(p) << "\n";
		bcode += p;
	}

	std::ofstream outputFile(filename.c_str(), std::ios::binary);

	std::string zeros = "00000000";
	if (bcode.size() % 8 != 0) {  //make sure the length of the binary string is a multiple of 8
		bcode += zeros.substr(0, 8 - bcode.size() % 8);
	}

	int b;
	for (int i = 0; i < bcode.size(); i += 8) {
		b = 1;
		for (int j = 0; j < 8; j++) {
			b = b << 1;
			if (bcode.at(i + j) == '1') {
				b += 1;
			}
		}
		char c = (char)(b & 255); //save the string byte by byte
		outputFile.write(&c, 1);
	}
	outputFile.close();
}

std::vector<int> readBinaryFile(std::string filename) {
	std::ifstream myfile2(filename.c_str(), std::ios::binary);

	std::string zeros = "00000000";

	struct stat filestatus;
	stat(filename.c_str(), &filestatus);
	long fsize = filestatus.st_size; //get the size of the file in bytes

	char c2[fsize];
	myfile2.read(c2, fsize);

	std::string s = "";
	long count = 0;
	while (count < fsize) {
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
											   // result.push_back(binaryString2Int(p)); 
		s += p;
		count++;
	}
	myfile2.close();

	int bits = 12;

	if (s.size() % bits != 0) {
		s = std::string(s.data(), (s.size() / bits) * bits);
	}

	std::vector<int> result;

	for (int i = 0; i < s.length(); i += bits) {
		result.push_back(binaryString2Int(s.substr(i, bits)));
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
/*
void binaryIODemo(std::vector<int> compressed) {
	int c = 69;
	int bits = 9;
	std::string p = int2BinaryString(c, bits);
	std::cout << "c=" << c << " : binary string=" << p << "; back to code=" << binaryString2Int(p) << "\n";

	std::string bcode = "";
	for (std::vector<int>::iterator it = compressed.begin(); it != compressed.end(); ++it) {
		if (*it<256)
			bits = 8;
		else
			bits = 9;

		bits = 12;
		p = int2BinaryString(*it, bits);
		std::cout << "c=" << *it << " : binary string=" << p << "; back to code=" << binaryString2Int(p) << "\n";
		bcode += p;
	}

	//writing to file
	std::cout << "string 2 save : " << bcode << "\n";
	//std::string fileName = "example435.lzw";
	std::string fileName = "example.txt";
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

	//reading from a file
	std::ifstream myfile2;
	myfile2.open(fileName.c_str(), std::ios::binary);

	struct stat filestatus;
	stat(fileName.c_str(), &filestatus);
	const long fsize = filestatus.st_size; //get the size of the file in bytes

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
	std::cout << " saved string : " << s << "\n";
}
*/
/*
int main() {
	std::vector<int> compressed;
	compress("AAAAAAABBBBBB", std::back_inserter(compressed));
	copy(compressed.begin(), compressed.end(), std::ostream_iterator<int>(std::cout, ", "));
	std::cout << std::endl;
	std::string decompressed = decompress(compressed.begin(), compressed.end());
	std::cout << decompressed << std::endl;

	binaryIODemo(compressed);

	return 0;
}
*/

int main(int argc, char* argv[]) {
	std::ifstream inputfile;
	std::string filename(argv[2]);

	if (*argv[1] == 'c') {
		// Compress
		std::string document = readFileBlockIO(filename);
		std::vector<int> compressedDocument;
		compress(document, std::back_inserter(compressedDocument));

		filename += ".lzw";

		writeFile(compressedDocument, filename);
	}

	if (*argv[1] == 'e') {
		// Expand
		std::vector<int> compressedDocument = readBinaryFile(filename);
		std::cout << "print\n";
		std::string document = decompress(compressedDocument.begin(), compressedDocument.end());
		std::cout << "print2\n";


		filename = filename.substr(0, filename.find_last_of("."));
		auto extension = filename.find_first_of(".");
		if (extension != std::string::npos) {
			filename.insert(filename.find_first_of("."), "2");
		}
		else {
			filename += 2;
		}

		std::ofstream outputFile(filename, std::ios::binary);
		outputFile.write(document.data(), document.size());
	}
}