#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE fclones_test

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fstream>

#include "fclones.h"

// Lengths
BOOST_AUTO_TEST_CASE(first_stage)
{

	// This is the actual test from start to finish

	std::string testing_dir = "testing";

	fs::path dir(testing_dir);
	boost::filesystem::create_directory(dir);

	std::ofstream f1;
	f1.open(testing_dir + "/f1");
	f1 << "This is only a test - dupe";
	f1.close();

	std::ofstream f2;
	f2.open(testing_dir + "/f2");
	f2 << "This is only a test - dupe";
	f2.close();

	std::ofstream f3;
	f3.open(testing_dir + "/f3");
	f3 << "dog";
	f3.close();

	std::ofstream f4;
	f4.open(testing_dir + "/f4");
	f4 << "cat";
	f4.close();

	std::ofstream f5;
	f5.open(testing_dir + "/f5");
	f5 << "mouse";
	f5.close();

	std::ofstream f6;
	f6.open(testing_dir + "/f6");
	f6 << "This is only the second test - dupe";
	f6.close();	

	std::ofstream f7;
	f7.open(testing_dir + "/f7");
	f7 << "This is only the second test - dupe";
	f7.close();

	std::ofstream f8;
	f8.open(testing_dir + "/f8");
	f8 << "This is only the second test - dupe";
	f8.close();

	std::ofstream f9;
	f9.open(testing_dir + "/f9");
	f9 << 5;
	f9.close();

	std::ofstream f10;
	f10.open(testing_dir + "/f10");
	f10 << 6;
	f10.close();

	auto lengthMap = std::make_shared<LengthMap>();

	for ( unsigned int i = 1; i <= 10; i++ )
	{
		std::string name = testing_dir + "/f" + std::to_string(i);
		fs::path file(name);
		lengthMap->insert(std::pair<uintmax_t, fs::path>(fs::file_size(file), name));
	}	

	BOOST_CHECK(lengthMap->size() == 10);

	auto blockMap = std::make_shared<BlockMap>();
	findDupesByLength(lengthMap, blockMap);

	BOOST_CHECK(blockMap->size() == 9);

	auto md5Map = std::make_shared<Md5Map>();
	findDupesByLengthAndBlocks(blockMap, md5Map);

	BOOST_CHECK(md5Map->size() == 5);

	auto clones = createCloneList(md5Map);
    std::sort(clones->begin(), clones->end());

  	unsigned long long savedSpace = 0;

  	for ( auto clone = clones->begin(); clone != clones->end(); ++clone )
  	{
    	savedSpace += clone->diskSpaceSaved;
  	}

  	BOOST_CHECK( "96.0 B" == Clone::formatFileSize(savedSpace));
}

