#include "checksumfile.h"

#include <fstream>
#include <iomanip> 
#include <sstream>

using namespace std;
using namespace boost::filesystem;

CheckSumFile::CheckSumFile(string fn)
	: filename(fn), hashFilename(fn + ".hc"), chunkSize(1024 * 1024 * 1 /* 1Mb */)
{
	
}

string CheckSumFile::GetHash(char *data, long size)
{
	ostringstream ret;

	unsigned char sha[SHA_DIGEST_LENGTH]; // == 20

	SHA1((unsigned char*)data, size, sha);
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
	{
		int h = sha[i];
		ret << hex << setfill('0') << setw(2) << h;
	}

	return ret.str();
}

void CheckSumFile::Write()
{
	ifstream file (filename.c_str(), ios::in|ios::binary);
  if (!file.is_open())
 	{
		cerr << "could not open input file" << endl;
		return;
	}

	ofstream hashFile(hashFilename.c_str());

	uintmax_t size = file_size(filename);
	hashFile << "size:" << size << endl;	

	// divide in chunks each with size chunkSize
	char chunk[chunkSize];
	long pos = 0;
	while (pos < size)
	{
		long bytesToRead = min(size - pos, chunkSize);
		file.read(chunk, bytesToRead);
		string hash = GetHash(chunk, bytesToRead);
		hashFile << pos << ":" << hash << endl;
		pos += bytesToRead;
	}
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

vector<Chunk> CheckSumFile::Read()
{
	vector<Chunk> chunks;

	ifstream chunkFile(hashFilename.c_str());
	if (!chunkFile.is_open())
	{
		return chunks;
	}

	string line;
	getline(chunkFile, line); // size

  while(getline(chunkFile, line))
  {
		vector<string> splitted;
		split(line, ':', splitted);
		if (splitted.size() == 2)
		{
			unsigned long offset;
			istringstream offsetStream(splitted[0]);
			offsetStream >> offset;
			Chunk c(offset, splitted[1]);
			chunks.push_back(c);
    }
  }
}

void CheckSumFile::ChunkSize(long cs)
{
	chunkSize = cs;
}

