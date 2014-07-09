#include <boost/filesystem.hpp>
#include <vector>
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
      : offset(off), isEqual(equal), hash(hash_value)
    {
    }

    bool IsEqual() const
    {
        return isEqual;
    }

    void IsEqual(bool value)
    {
        isEqual = value;
    }

    string Hash() const
    {
        return hash;
    }

    unsigned long Offset() const
    {
        return offset;
    }
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
    vector<Chunk> WriteHashFile();
    void WriteHashFile(const vector<Chunk> &chunks);
    vector<Chunk> CalcHashes();
    vector<Chunk> Read();
    vector<Chunk> Read(string chunkFileName);
    vector<Chunk> Verify();
    void UpdateSnapshot(string snapshotFile);
    const path HashFileName()
    {
        return hashFilename;
    }

    void ChunkSize(long cs);
};
