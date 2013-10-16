#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "fclones.h"
#include "main.h"


// globals with default values
namespace globals {
  unsigned long long BLOCKS_CHECK_SIZE = 8192;
  unsigned long long BLOCKS_SECOND_CHECK_MIN_SIZE = 50 * 1024;
}

namespace fs = boost::filesystem;

typedef std::vector<fs::path> Directories;
typedef std::unordered_multimap<uintmax_t, fs::path> LengthMap;
typedef std::unordered_multimap<std::string, fs::path> BlockMap;
typedef std::unordered_multimap<std::string, fs::path> Md5Map;
typedef std::unordered_set<std::string> HashResults;

// Add directories to working directory list
void descend(Directories &parent, std::shared_ptr<LengthMap> lengthMap)
{
  // XXX not currently used
  static int counter = 0;
  counter++;

  boost::system::error_code ec, no_error;
  Directories dirs;

  	/*
  	Sorting should be optional when looking for increased speed (debugging only?)
  	sort(parent.begin(), parent.end()); 
    */

  	for (auto dir : parent)
  	{

  		std::cout << "Parent is " << dir << std::endl;

  		// using the non-throwing directory_iterator and catching permission error (and probably others) while processing entries
  		std::for_each(boost::filesystem::directory_iterator(dir, ec), boost::filesystem::directory_iterator(), [&] (fs::path entry)
  		{
        if (ec != no_error) {
          std::cout << "ERROR: Entry " << entry << " had a problem." << std::endl;
        }

        try {
          if (exists(entry))
          {
            if (is_regular_file(entry))
            {
              std::cout << " regular file " << entry << '\n';
              std::cout << " regular file " << fs::file_size(entry) << '\n';
              lengthMap->insert(std::pair<uintmax_t, fs::path>(fs::file_size(entry), entry));
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
    if (dirs.size() > 0) descend(dirs, lengthMap);
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
      // first 8K (two blocks - read at one time with Linux)
      int total_len = 2 * globals::BLOCKS_CHECK_SIZE;
    }


    std::streambuf *raw_blocks = f.rdbuf();
    char *block = new char[total_len];
    raw_blocks->sgetn(block, len);

    // WARNING!!! binary files will have nulls - must pass length without using strlen or anything
    // else that terminates with a NULL

    if ( static_cast<unsigned long long>(fileSize) > globals::BLOCKS_SECOND_CHECK_MIN_SIZE )
    {
      raw_blocks->pubseekpos(globals::BLOCKS_SECOND_CHECK_MIN_SIZE - globals::BLOCKS_CHECK_SIZE);
      raw_blocks->sgetn(block + len, globals::BLOCKS_CHECK_SIZE);
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
  unsigned int bucket = 0;
  for (bucket = 0; bucket < lengthMap->bucket_count(); ++bucket) 
  {  
    // Same size files go into the same buckets and the bucket is shared with other lengths
    if ( lengthMap->bucket_size(bucket) < 2 ) continue;

    std::cout << "bucket size is " << lengthMap->bucket_size(bucket) << std::endl;
    //std::cout << "bucket #" << bucket << " contains:";
    for ( auto local_it = lengthMap->begin(bucket); local_it != lengthMap->end(bucket); ++local_it )
    {
      std::cout << "NUM of  " << lengthMap->count(local_it->first) << std::endl;
      if ( lengthMap->count(local_it->first) > 1 )
      {
        std::cout << "This is what we're looking for " << local_it->first << "    " << local_it->second << std::endl;
        addToBlockMap(local_it->first, local_it->second, blockMap);   
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
  unsigned bucket = 0;
  for (bucket = 0; bucket < blockMap->bucket_count(); ++bucket)
  {
    if ( blockMap->bucket_size(bucket) < 2 ) continue;
    for ( auto local_it = blockMap->begin(bucket); local_it != blockMap->end(bucket); ++local_it )
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

    for ( auto local_it = copies.first; local_it != copies.second; ++local_it )
    {
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
  Directories dirs;

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
