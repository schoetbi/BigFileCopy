#define BOOST_TEST_MODULE bfcptest

#include <boost/test/included/unit_test.hpp>
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

BOOST_AUTO_TEST_SUITE (bfcptest) 

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
	csf.Write();

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
  csf.Write();
	vector<Chunk> chunks = csf.Read();
	BOOST_CHECK(chunks.size() == 6);
}

BOOST_AUTO_TEST_SUITE_END( )
