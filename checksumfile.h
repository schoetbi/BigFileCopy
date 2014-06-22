#include <boost/filesystem.hpp>
#include <vector>
#include <openssl/sha.h>
#include <string>

using namespace std;
using namespace boost::filesystem;

class Chunk
{
	private:
		unsigned long offset;
		bool isEqual;
		string hash;
	
	public:
		Chunk(unsigned long off, string hash_value, bool equal = false)
		{
			offset = off;
      hash = hash;
			isEqual = equal;
		}
			
		bool IsEqual() { return isEqual; }
		unsigned long Offset() { return offset; }
};

class CheckSumFile
{
	private:
		unsigned long chunkSize;
		path filename;
		path hashFilename;
		string GetHash(char *data, long size);
	public:
		CheckSumFile(string fn);
		void Write();
		vector<Chunk> Read();
		vector<Chunk> Verify();
		const path HashFileName() { return hashFilename; }

		void ChunkSize(long cs);
};
