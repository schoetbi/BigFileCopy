#include "checksumfile.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

using namespace std;
using namespace boost::filesystem;

CheckSumFile::CheckSumFile(string fn)
    : chunkSize(1024 * 1024 * 1 /* 1Mb */),
    filename(fn),
    hashFilename(fn + ".hc")
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

vector<Chunk> CheckSumFile::WriteHashFile()
{
    vector<Chunk> chunks = CalcHashes();

    ofstream hashFile(hashFilename.c_str());
    uintmax_t size = file_size(filename);
    hashFile << "size:" << size << endl;

    for(vector<Chunk>::iterator it = chunks.begin();
            it != chunks.end();
            it++)
    {
        hashFile << it->Offset() << ":" << it->Hash() << endl;
    }

    return chunks;
}

vector<Chunk> CheckSumFile::CalcHashes()
{
    vector<Chunk> chunks;

    ifstream file (filename.c_str(), ios::in|ios::binary);
    if (!file.is_open())
    {
        cerr << "could not open input file" << endl;
        return chunks;
    }

    // divide in chunks each with size chunkSize
    char chunk[chunkSize];
    unsigned long pos = 0;
    uintmax_t size = file_size(filename);
    while (pos < size)
    {
        long bytesToRead = min(size - pos, chunkSize);
        file.read(chunk, bytesToRead);
        string hash = GetHash(chunk, bytesToRead);
        Chunk chk(pos, hash);
        chunks.push_back(chk);

        pos += bytesToRead;
    }

    return chunks;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }

    return elems;
}


std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

vector<Chunk> CheckSumFile::Read()
{
    return Read(hashFilename.string());
}

vector<Chunk> CheckSumFile::Read(string chunkFileName)
{
    vector<Chunk> chunks;

    ifstream chunkFile(chunkFileName.c_str());
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
            Chunk c(offset, splitted[1], false);
            chunks.push_back(c);
        }
    }

    return chunks;
}

void CheckSumFile::ChunkSize(long cs)
{
    chunkSize = cs;
}

vector<Chunk> CheckSumFile::Verify()
{
    vector<Chunk> expected = Read();
    int expectedSize = expected.size();
    if (expectedSize  == 0)
    {
        return expected;
    }

    vector<Chunk> calculated = CalcHashes();
    int calculatedSize = calculated.size();

    // if chunk count is not equal
    if (expectedSize != calculatedSize)
    {
        return expected;
    }

    for (int i = 0; i < expectedSize; ++i)
    {
        Chunk &ex = expected[i];
        const Chunk &calc = calculated[i];

        bool isEqual =
            ex.Hash() == calc.Hash()
            && ex.Offset() == calc.Offset();
        ex.IsEqual(isEqual);
    }

    return expected;
}

void CheckSumFile::UpdateSnapshot(string snapshotFileName)
{
    path targetFile(snapshotFileName);
    path targetHashFile(snapshotFileName + ".hc");

    // calculate local hashes
    vector<Chunk> localChunks = WriteHashFile();
    if (!exists(targetFile) || !exists(targetHashFile))
    {
        copy_file (filename, targetFile, copy_option::overwrite_if_exists);
        copy_file (hashFilename, targetHashFile, copy_option::overwrite_if_exists);
        return;
    }

    if (exists(targetHashFile) && exists(targetFile))
    {
        vector<Chunk> targetChunks = Read(targetHashFile.string());
        // if count is not equal copy both files
        unsigned long chunkCount = targetChunks.size() ;
        if (chunkCount != localChunks.size())
        {
            copy_file (filename, targetFile, copy_option::overwrite_if_exists);
            copy_file (hashFilename, targetHashFile, copy_option::overwrite_if_exists);
            return;
        }

        char chunk[chunkSize];
        for (unsigned long c = 0; c < chunkCount; ++c)
        {
            if (targetChunks[c].Hash() == localChunks[c].Hash())
            {
                continue;
            }

            // write contents of local file to target file at the same position
            unsigned long offset = localChunks[c].Offset();
            ifstream src(filename.c_str(), ios::in | ios::binary);
            src.seekg(offset, ios::beg);
            src.read(chunk, chunkSize);
            src.close();

            fstream file(snapshotFileName.c_str(), ios::binary | ios::out | ios::in);
            file.seekg(offset, ios::beg);
            file.write(chunk, chunkSize);
            file.close();

            // todo: update target patchfile with new hash for current offset
        }

        copy_file (hashFilename, targetHashFile, copy_option::overwrite_if_exists);
    }
}

