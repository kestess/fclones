#include <iostream>
#include <unordered_map>
#include <unordered_set>
//#include <boost/unordered_map.hpp>
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
//#include <boost/filesystem/operations.hpp>
#include <memory>
#include <exception>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <tuple>
#include "fclones.h"

std::string md5sum(char *s, int len);

//using namespace std;
//using namespace boost::filesystem;

// globals with default values


Clone::Clone(unsigned int numberOfClones,
 unsigned long long fileSize,
 std::string nameList)
{
  this->diskSpaceSaved = (numberOfClones - 1) * fileSize;
  this->numberOfClones = numberOfClones;
  this->fileSize = fileSize;
  this->nameList = nameList;
}


std::tuple<std::string, unsigned long long> Clone::getFileSizeFormatting(unsigned long long size)
{
  std::string units = "B";
  unsigned long long divisor = 1;

  if (size/1024ull > 0)
  {
    units = "KB";
    divisor = 1024ull;
  } 
  else 
  {
    return std::make_tuple(units, divisor); 
  }

  if (size/(1024ull * 1024ull) > 0)
  {
    units = "MB";
    divisor = 1024ull * 1024ull;
  }
  else 
  {
    return std::make_tuple(units, divisor); 
  }

  if (size/(1024ull * 1024ull * 1024ull) > 0)
  {
    units = "GB";
    divisor = 1024ull * 1024ull * 1024ull;
  }
  else 
  {
    return std::make_tuple(units, divisor); 
  }

  if (size/(1024ull * 1024ull * 1024ull * 1024ull) > 0) {
    units = "TB";
    divisor = 1024ull * 1024ull * 1024ull * 1024ull;
  }
  else 
  {
    return std::make_tuple(units, divisor); 
  }

  if (size/(1024ull * 1024ull * 1024ull * 1024ull * 1024ull) > 0) {
    units = "PB";
    divisor = 1024ull * 1024ull * 1024ull * 1024ull * 1024ull;
  }

  return std::make_tuple(units, divisor);

}

std::string Clone::formatFileSize(unsigned long long size)
{
  auto unitInfo = getFileSizeFormatting(size);
  std::stringstream ss; 
  ss << std::setprecision(1) << std::fixed << static_cast<long double>(size)/std::get<1>(unitInfo) ;
  return ss.str() + " " + std::get<0>(unitInfo);
}

namespace globals {
  unsigned long long BLOCKS_CHECK_SIZE = 8192;
  unsigned long long BLOCKS_SECOND_CHECK_MIN_SIZE = 50 * 1024;
}

namespace fs = boost::filesystem;

// g++ -o fclones fclones.cpp -I/usr/local/boost/include -L/usr/local/boost/lib -lboost_system -lboost_filesystem  --std=c++11

typedef std::vector<fs::path> Directories;
typedef std::unordered_multimap<uintmax_t, fs::path> LengthMap;
typedef std::unordered_multimap<std::string, fs::path> BlockMap;
typedef std::unordered_multimap<std::string, fs::path> Md5Map;
typedef std::unordered_set<std::string> HashResults;

/*
void process(const boost::filesystem::path &entry)
{
	std::cout << "File is " << entry << std::endl;
	if (exists(entry))
	{
		if (is_regular_file(entry))
		{
			std::cout << " regular file " << entry << '\n';
	  				//std::cout << " regular file " << file_size(*it) << '\n';

	  				//lengthMap.insert( std::pair<uintmax_t, boost::filesystem::path>(file_size(*it), *it) );
	  				// std::cout << " regular file " << *it.file_size() << '\n';

		}
		else if (is_directory(entry))
		{
			std::cout << " directory " << entry << '\n';
			dirs.push_back(entry);
			std::cout << " vector size is " << dirs.size() << std::endl;
		}	
	}
}
*/

/*

struct Process {
	Directories &dirs;
	std::shared_ptr<LengthMap> lengthMap;
	Process(Directories &d, std::shared_ptr<LengthMap> l) : dirs (d), lengthMap(l) {}
	void operator () (boost::filesystem::directory_entry &entry)
	{
		try {
			if (exists(entry))
			//file_status s = status(entry);
			//printf("%o\n",s.permissions());	
			{
				if (is_regular_file(entry))
				{
					std::cout << " regular file " << entry << '\n';
					std::cout << " regular file " << file_size(entry) << '\n';
					lengthMap->insert(std::pair<uintmax_t, boost::filesystem::path>(file_size(entry), entry));

  				//lengthMap.insert( std::pair<uintmax_t, boost::filesystem::path>(file_size(*it), *it) );
  				// std::cout << " regular file " << *it.file_size() << '\n';

				}
				else if (is_directory(entry))
				{
					std::cout << " directory " << entry << '\n';
					dirs.push_back(entry);
					std::cout << " vector size is " << dirs.size() << std::endl;
				}	
			}
		} catch (...) {
			std::cout << "Bad" << std::endl;
		}
	}
};
*/
// Add directories to working directory list

//void descend(const boost::shared_ptr<Directories>& parent, LengthMap &lengthMap)
// void descend(const boost::shared_ptr<Directories>& parent)
void descend(Directories &parent, std::shared_ptr<LengthMap> lengthMap)
{
  static int counter = 0;
  counter++;
  //std::cout << "Counter at " << counter << std::endl;
  boost::system::error_code ec, perfect;

  //std::vector<boost::filesystem::path> dirs; // dirs entries of a directory

  // boost::shared_ptr<Directories> dirs(new Directories);
  Directories dirs;

  //try
  {
  	// redundant check on directory, but a directory on the list might have 
  	// been replaced by a file of the same name
  	//if (exists(dir) && is_directory(dir))
  	//{


  	
  		// sorting should be optional when looking for increased speed
    // should probably be sorted when debugging
  	// sort(parent.begin(), parent.end()); 

  	// for (std::vector<boost::filesystem::path>::const_iterator it(parent.begin()), it_end(parent.end()); it != it_end; ++it)
  	//for ( auto it = parent.begin(); it != parent.end(); ++it)
  	for (auto dir : parent)
  	{

  		std::cout << "Parent is " << dir << std::endl;



  		//std::copy(boost::filesystem::directory_iterator(*it), boost::filesystem::directory_iterator(), std::back_inserter(dirs));
  		// using the non-throwing directory_iterator and catching permission error (and probably others) in the Process try block
  		//std::for_each(boost::filesystem::directory_iterator(*it, ec), boost::filesystem::directory_iterator(), Process(dirs, lengthMap));

  		std::for_each(boost::filesystem::directory_iterator(dir, ec), boost::filesystem::directory_iterator(), [&] (fs::path entry)
  		{


        if (ec != perfect) {
          std::cout << "ERROR: Entry " << entry << " had a problem." << std::endl;
        }

        try {

          if (exists(entry))
      //file_status s = status(entry);
      //printf("%o\n",s.permissions()); 
          {
            if (is_regular_file(entry))
            {
              std::cout << " regular file " << entry << '\n';
              std::cout << " regular file " << fs::file_size(entry) << '\n';
              lengthMap->insert(std::pair<uintmax_t, fs::path>(fs::file_size(entry), entry));

          //lengthMap.insert( std::pair<uintmax_t, boost::filesystem::path>(file_size(*it), *it) );
          // std::cout << " regular file " << *it.file_size() << '\n';

            }
            else if (is_directory(entry))
            {
              std::cout << " directory " << entry << '\n';
              dirs.push_back(entry);
              std::cout << " vector size is " << dirs.size() << std::endl;
            } 
          }
        } catch (...) {
          std::cout << "Bad" << std::endl;
        }


      });	







}

  	//for (std::vector<boost::filesystem::path>::const_iterator it(dirs.begin()), it_end(dirs.end()); it != it_end; ++it)
  	//{

/*
  		if (exists(*it))
  		{
  			if (is_regular_file(*it))
  			{
  				std::cout << " regular file " << *it << '\n';
  				//std::cout << " regular file " << file_size(*it) << '\n';

  				//lengthMap.insert( std::pair<uintmax_t, boost::filesystem::path>(file_size(*it), *it) );
  				// std::cout << " regular file " << *it.file_size() << '\n';

  			}
  			else if (is_directory(*it))
  			{
  				std::cout << " directory " << *it << '\n';
  				dirs.push_back(*it);
  				std::cout << " vector size is " << dirs.size() << std::endl;
  			}	

  		}
*/
  	//}
  	//descend(dirs, lengthMap); // breadth-first
/*
  	for (std::vector<boost::filesystem::path>::const_iterator it(dirs.begin()), it_end(dirs.end()); it != it_end; ++it)
  	{
  		std::cout << " before calling descend  " << *it << '\n';
  	}
*/

    //if (dirs.size() > 0) descend(dirs, lengthMap);
  }	
  /*  
  catch(const boost::filesystem::filesystem_error& ex)
  {
  	std::cout << ex.what() << std::endl;
  	exit(1); // if this becomes an automated process exit codes will need to be formalized
  }
*/
}

void addToBlockMap(uintmax_t fileSize, fs::path& file, std::shared_ptr<BlockMap> blockMap)
{
  std::cout << fileSize << "   "  << file << " IN THE Add" << std::endl;
  std::ifstream f (fs::canonical(file).string(), std::ios::in | std::ios::binary);
  std::string checksum;

  if (f.is_open())
  {
    int len = std::min(static_cast<unsigned long long>(fileSize), globals::BLOCKS_CHECK_SIZE);
    int total_len = len;

    if ( static_cast<unsigned long long>(fileSize) > globals::BLOCKS_SECOND_CHECK_MIN_SIZE )
    {
        // first 8K (two blocks - in one read on Linux) and arbitrary 
      int total_len = 2 * globals::BLOCKS_CHECK_SIZE;
    }


    std::streambuf *raw_blocks = f.rdbuf();
    char *block = new char[total_len];
    raw_blocks->sgetn(block, len);

    // binary files will have nulls - must pass length without using strlen or anything
    // else that terminates at a NULL



    if ( static_cast<unsigned long long>(fileSize) > globals::BLOCKS_SECOND_CHECK_MIN_SIZE )
    {
      raw_blocks->pubseekpos(globals::BLOCKS_SECOND_CHECK_MIN_SIZE - globals::BLOCKS_CHECK_SIZE);
      // char *second_block = new char[globals::BLOCKS_CHECK_SIZE];
      raw_blocks->sgetn(block + len, globals::BLOCKS_CHECK_SIZE);

      //checksum += md5sum(second_block, globals::BLOCKS_CHECK_SIZE);
      //std::cout << "BIG CHECKSUM IS::::: " << checksum << std::endl;
      // delete[] second_block;
    }

    checksum = md5sum(block, total_len);
    delete[] block;

    std::cout << "CHECKSUM " << checksum << std::endl;

    std::string lengthAndMd5 = std::to_string(fileSize) + "_" + checksum;

    blockMap->insert(std::pair<std::string, fs::path>(lengthAndMd5, file));
    f.close();
  }

}

void findDupesByLengthAndBlocks(std::shared_ptr<LengthMap> lengthMap, std::shared_ptr<BlockMap> blockMap)
{
    //for (auto lmap = lengthMap->begin(); lmap != lengthMap->end(); ++lmap)
    //{
    //    std::cout << lmap->first << "\t" << lmap->second << std::endl;
    //} 
  unsigned i = 0;

/*
  for (auto it = lengthMap->begin(); it != lengthMap->end(); ++it)
  {
    std::cout << it->first << ":" << it->second << std::endl;
  }
*/

  for (i = 0; i < lengthMap->bucket_count(); ++i) 
  {  
    // Same size files go into the same buckets and the bucket is shared with other lengths
    if ( lengthMap->bucket_size(i) < 2 ) continue;

    std::cout << "bucket size is " << lengthMap->bucket_size(i) << std::endl;
    //std::cout << "bucket #" << i << " contains:";
    for ( auto local_it = lengthMap->begin(i); local_it != lengthMap->end(i); ++local_it )
    {
      std::cout << "NUM of  " << lengthMap->count(local_it->first) << std::endl;
      if ( lengthMap->count(local_it->first) > 1 )
      {


        std::cout << "This is what we're looking for " << local_it->first << "    " << local_it->second << std::endl;
        addToBlockMap(local_it->first, local_it->second, blockMap);

          // blockMap->insert(std::pair<std::string, fs::path>(std::to_string(local_it->first), local_it->second));    
      }


      std::cout << " " << local_it->first << ":" << local_it->second;
    }  
    std::cout << std::endl;

    std::cout << "Done with lmap" << std::endl;
  }
}

// Now we get the md5 of the whole file
void addToMd5Map(fs::path& file, std::shared_ptr<Md5Map> md5Map)
{

  std::cout << file << " IN THE MD5" << std::endl;
  std::ifstream f (fs::canonical(file).string(), std::ios::in | std::ios::binary);
  std::string checksum;
  unsigned int size = fs::file_size(file);

  if (f.is_open())
  {
   std::streambuf* raw_buffer = f.rdbuf();
   char* block = new char[size];
   raw_buffer->sgetn(block, size);
   checksum = md5sum(block, size);
   delete[] block;
   std::cout << "FULL CHECKSUM " << checksum << std::endl;

   md5Map->insert(std::pair<std::string, fs::path>(checksum, file));
   f.close();
  }

}

void getAllMd5(std::shared_ptr<BlockMap> blockMap, std::shared_ptr<Md5Map> md5Map)
{
  unsigned i = 0;
  for (i = 0; i < blockMap->bucket_count(); ++i)
  {
    if ( blockMap->bucket_size(i) < 2 ) continue;
    for ( auto local_it = blockMap->begin(i); local_it != blockMap->end(i); ++local_it )
    {
      if ( blockMap->count(local_it->first) > 1 )
      {
        addToMd5Map(local_it->second, md5Map);
      }
    }
  }
}

std::shared_ptr<CloneList> createCloneList(std::shared_ptr<Md5Map> md5Map)
{

  auto clones = std::make_shared<CloneList>();
  HashResults savedResults;

  for (auto it = md5Map->begin(); it != md5Map->end(); ++it)
  {
    std::cout << "MD5 " << it->first << ":" << it->second << " has " << md5Map->count(it->first) << " entries." << std::endl;

    std::string hash = it->first;
    auto copies = md5Map->equal_range(hash);

    std::string names;
    //std::for_each(copies.first, copies.second, [&] (fs::path &filename){
      //names += fs::canonical(filename).string() + ";";

      //std::cout << fs::canonical(filename).string() << std::endl;
    //});

/*
    for (auto it = copies.first; it != copies.second; ++it)
    {
      std::cout << fs::canonical(it->second).string() << std::endl;
    }
*/
    for ( auto local_it = copies.first; local_it != copies.second; ++local_it )
    {
      //std::cout << fs::canonical(local_it->second).string() << "This is the real deal" << std::endl;
      //std::cout << fs::canonical(local_it->second).string() << std::endl;
      names += fs::canonical(local_it->second).string() + ";";
    }
    if (names.length() > 0) names.pop_back(); // remove trailing semicolon

    auto alreadyDone = savedResults.find(hash);

    if (alreadyDone == savedResults.end())
    {
      std::cout << "First time for  " << hash << std::endl;   
      unsigned long long filesize = fs::file_size(it->second);
      Clone clone(md5Map->count(it->first),
        filesize,
        names);

      savedResults.insert(hash);
      clones->push_back(clone);
    } else {
      std::cout << "Found hash " << hash << " already."  << std::endl;
    }

  }
  return clones;
}


int main(int argc, char *argv[])
{

    //Directories dirs; // vector of directories
    //boost::shared_ptr<Directories> dirs(new Directories);
  Directories dirs;
    //LengthMap lengthMap;
    /*
    std::shared_ptr<LengthMap> lengthMap(new LengthMap);
	  std::shared_ptr<BlockMap> blockMap(new BlockMap);
    std::shared_ptr<Md5Map> md5Map(new Md5Map);
*/
    auto lengthMap = std::make_shared<LengthMap>();
    auto blockMap  = std::make_shared<BlockMap>();
    auto md5Map    = std::make_shared<Md5Map>();

    fs::path dir( fs::current_path() );

    if (is_directory(dir))
    {
    	std::cout << "Starting in the " << dir << "directory." << std::endl;
    	dirs.push_back(dir);	
    }

    descend(dirs, lengthMap);
    std::cout << lengthMap->size() << std::endl;

    findDupesByLengthAndBlocks(lengthMap, blockMap);

    for (auto it = blockMap->begin(); it != blockMap->end(); ++it)
    {
      std::cout << it->first << ":" << it->second << std::endl;
    }

    std::cout << "Blockmap size is " << blockMap->size() << std::endl;

    getAllMd5(blockMap, md5Map);

    for (auto it = md5Map->begin(); it != md5Map->end(); ++it)
    {
      std::cout << "MD5 " << it->first << ":" << it->second << std::endl;
    }

    auto clones = createCloneList(md5Map);
    std::sort(clones->begin(), clones->end());

    Clone::printHeading();
    for (auto clone = clones->begin(); clone != clones->end(); ++clone)
    {
      clone->prettyPrint();
    }

    return 0;

  }



