#include <iostream>
#include <fstream>
#include <string>
#include <openssl/sha.h>
#include <iomanip> 
#include <sstream>
#include <boost/filesystem.hpp>
 
using namespace std;
using namespace boost::filesystem;

const long CHUNK_SIZE = 1024 * 1024 * 2; // 2 Mb

string getHash(char *data, long size)
{
	unsigned char sha[SHA_DIGEST_LENGTH]; // == 20
	ostringstream ret;

	SHA1((unsigned char*)data, size, sha);
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
	{
		int h = sha[i];
		ret<< hex << setfill('0') << setw(2) << h;
	}

	return ret.str();
}

void createHashFile(path &fileName)
{
	ifstream file (fileName.c_str(), ios::in|ios::binary);
  if (!file.is_open())
 	{
		cout << "could not open input file" << endl;
		return;
	}

	path hashFileName(fileName.string() + ".hc");
	cout << hashFileName;
	ofstream hashFile(hashFileName.c_str());

	hashFile << "file:" << fileName << endl;

	file.seekg(0, std::ifstream::end);
	long size = file.tellg();
	file.seekg(0, std::ifstream::beg);

	hashFile << "size:" << size << endl;	

	// divide in chunks each with size CHUNK_SIZE
	char chunk[CHUNK_SIZE];
	long pos = 0;
	while (pos < size)
	{
		long bytesToRead = min(size - pos, CHUNK_SIZE);
		file.read(chunk, bytesToRead);
		string hash = getHash(chunk, bytesToRead);
		// cout << "chunk hash is " << hash << endl;
		hashFile << pos << ":" << hash << endl;
		pos += bytesToRead;
	}
	
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		cout << "wrong usage" << endl;
		return -1;
	}

	path in(argv[1]);
	
	cout << "opened " << in.string() << endl;
	createHashFile(in);
}
