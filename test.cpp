#define BOOST_TEST_MODULE bfcptest

#include <boost/test/included/unit_test.hpp>
#include "checksumfile.h"

#include <fstream>

BOOST_AUTO_TEST_SUITE (bfcptest) 

BOOST_AUTO_TEST_CASE( writeCheckSumFile )
{
		const int SIZE = 1024;
		char testData[SIZE];
		for (int i = 0; i < SIZE; i++)
		{
			testData[i] = i % 255;
		}

		const string testFileName("data.dat");

		{
			ofstream testFile(testFileName.c_str(), ios::out | ios::binary);
			testFile.write(testData, SIZE);
		}
		
		CheckSumFile csf(testFileName);
		csf.ChunkSize(128);
		csf.Write();
}

BOOST_AUTO_TEST_SUITE_END( )
