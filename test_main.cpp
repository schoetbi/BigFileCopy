#define BOOST_TEST_MODULE bfcptest
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "checksumfile.h"

#include <fstream>
const string testFileName("data.dat");

void WriteTestFile()
{
    const int SIZE = 1024;
    char testData[SIZE];
    for (int i = 0; i < SIZE; i++)
    {
        testData[i] = i % 255;
    }

    ofstream testFile(testFileName.c_str(), ios::out | ios::binary);
    testFile.write(testData, SIZE);
}

BOOST_AUTO_TEST_SUITE ( bfcptest )

BOOST_AUTO_TEST_CASE( writeCheckSumFile )
{
    WriteTestFile();
    CheckSumFile csf(testFileName);
    csf.ChunkSize(15);

    path hashFile("data.dat.hc");
    if (exists(hashFile))
    {
        remove(hashFile);
    }

    csf.WriteHashFile();

    BOOST_CHECK (exists(hashFile));
}

BOOST_AUTO_TEST_CASE( readCheckSumFile )
{
    WriteTestFile();
    CheckSumFile csf(testFileName);
    csf.ChunkSize(200);

    path hashFile("data.dat.hc");
    if (exists(hashFile))
    {
        remove(hashFile);
    }

    csf.WriteHashFile();
    vector<Chunk> chunks = csf.Read();
    BOOST_CHECK(chunks.size() == 6);

    // test first
    BOOST_CHECK(chunks[0].Offset() == 0);
    BOOST_CHECK_EQUAL(chunks[0].Hash(), "54d11e99127d159799dbce10f51a75e697780478");

    // test last
    BOOST_CHECK(chunks[5].Offset() == 1000);
    BOOST_CHECK_EQUAL(chunks[5].Hash(), "c6014fd6eccd7d2dfa50238bcec147dba3f9d98d");

}

BOOST_AUTO_TEST_CASE( verifyCheckSumFile )
{
    WriteTestFile();
    CheckSumFile csf(testFileName);
    csf.ChunkSize(200);

    path hashFile("data.dat.hc");
    if (exists(hashFile))
    {
        remove(hashFile);
    }

    csf.WriteHashFile();

    // modify file
    fstream file(testFileName.c_str(), ios::binary | ios::out | ios::in);
    file.seekp(0l, ios::beg);
    char val = 99;
    file.write(&val, 1);

    file.seekp(400l, ios::beg);
    file.write(&val, 1);
    file.close();

    vector<Chunk> result = csf.Verify();

    // the chunk at 0 and 2 should be unequal the others equal
    BOOST_CHECK(!result[0].IsEqual());
    BOOST_CHECK(result[1].IsEqual());
    BOOST_CHECK(!result[2].IsEqual());
    BOOST_CHECK(result[3].IsEqual());
    BOOST_CHECK(result[4].IsEqual());
    BOOST_CHECK(result[5].IsEqual());
}

BOOST_AUTO_TEST_CASE( updateSnapshot )
{
    WriteTestFile();
    CheckSumFile csf(testFileName);
    csf.ChunkSize(200);

    path hashFile("data.dat.hc");
    if (exists(hashFile))
    {
        remove(hashFile);
    }

    csf.WriteHashFile();

    // write the very first time
    csf.UpdateSnapshot("snapshot");

    fstream file(testFileName.c_str(), ios::binary | ios::out | ios::in);
    file.seekp(0l, ios::beg);
    char val = 99;
    file.write(&val, 1);

    file.seekp(400l, ios::beg);
    file.write(&val, 1);
    file.close();

    // update again
    csf.UpdateSnapshot("snapshot");
}

BOOST_AUTO_TEST_SUITE_END( )
