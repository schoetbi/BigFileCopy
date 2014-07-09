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

void CheckSumFile::WriteHashFile(const vector<Chunk> &chunks)
{
    ofstream hashFile(hashFilename.c_str());
    uintmax_t size = file_size(filename);
    hashFile << "size:" << size << endl;

    for(vector<Chunk>::const_iterator it = chunks.begin();
            it != chunks.end();
            it++)
    {
        hashFile << it->Offset() << ":" << it->Hash() << endl;
    }
}


vector<Chunk> CheckSumFile::WriteHashFile()
{
    vector<Chunk> chunks = CalcHashes();

    if (chunks.size() == 0)
    {
        return chunks;
    }

    WriteHashFile(chunks);

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
        cout << "hash: " << pos << "/" << pos * 100 / size  << "%"<< endl;
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

void CheckSumFile::UpdateSnapshot(string targetFileName)
{
    path targetFile(targetFileName);
    path targetHashFile(targetFileName + ".hc");

    // if no target hash file exists or file exists
    // do raw copy of both files
    if (!exists(targetFile) || !exists(targetHashFile))
    {
        cout << "doing raw copy since there is no update file" << endl;
        copy_file (filename, targetFile, copy_option::overwrite_if_exists);
        if (!exists(hashFilename))
        {
            WriteHashFile();
        }
        copy_file (hashFilename, targetHashFile, copy_option::overwrite_if_exists);
        return;
    }

    ifstream localFileStream (filename.c_str(), ios::in|ios::binary);
    if (!localFileStream.is_open())
    {
        cerr << "could not open input file" << endl;
        return;
    }

    vector<Chunk> targetChunks = Read(targetHashFile.string());

    // check size of target and copy if not equal
    unsigned long chunkCount = targetChunks.size();
    unsigned long targetSize = chunkCount * chunkSize;
    uintmax_t localSize = file_size(filename);

    if (targetSize != localSize)
    {
        cout << "filesize differ, doing raw copy...";
        copy_file (filename, targetFile, copy_option::overwrite_if_exists);
        cout << "hashfile...";
        copy_file (hashFilename, targetHashFile, copy_option::overwrite_if_exists);
        cout << "OK" << endl;
        return;
    }

    // divide in chunks each with size chunkSize
    fstream targetFileStream(targetFileName.c_str(), ios::binary | ios::out | ios::in);
    if (!targetFileStream.is_open())
    {
        cerr << "could not open output file" << endl;
        return;
    }

    vector<Chunk> localChunks;
    char chunkData[chunkSize];
    unsigned long offset = 0;

    vector<Chunk>::iterator currentTargetChunk = targetChunks.begin();
    cout << endl;
    unsigned long currentPercentage = 0;
    unsigned long lastPercentage = -1;
    while (offset < localSize)
    {
        currentPercentage = offset * 100 / localSize;
        if (currentPercentage != lastPercentage)
        {
            cout << currentPercentage << "%\r";
            cout.flush();
            lastPercentage = currentPercentage;
        }

        long bytesToRead = min(localSize - offset, chunkSize);
        localFileStream.read(chunkData, bytesToRead);
        string hash = GetHash(chunkData, bytesToRead);
        Chunk chk(offset, hash);
        localChunks.push_back(chk);

        if (currentTargetChunk->Hash() != chk.Hash())
        {
            cout << "update " << offset << "          " << endl;
            cout << lastPercentage << "%\r";
            cout.flush();

            targetFileStream.seekg(offset, ios::beg);
            targetFileStream.write(chunkData, chunkSize);
            targetFileStream.flush();
        }

        offset += bytesToRead;
        currentTargetChunk++;
    }

    targetFileStream.close();
    localFileStream.close();

    WriteHashFile(localChunks);
    copy_file (hashFilename, targetHashFile, copy_option::overwrite_if_exists);
}

