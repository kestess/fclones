#ifndef __FCLONES__H__
#define __FCLONES__H__

#include "clone.h"

#include <algorithm>
#include <atomic>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <vector>

namespace fs = boost::filesystem;

typedef std::vector<fs::path> Directories;
typedef std::unordered_multimap<uintmax_t, fs::path> LengthMap;
typedef std::unordered_multimap<std::string, fs::path> BlockMap;
typedef std::unordered_multimap<std::string, fs::path> Md5Map;
typedef std::unordered_set<std::string> HashResults;
typedef std::unordered_set<fs::path> HardLinks;

// Descends recursively - breadth-first so a depth limiter might easily be
// added later. Adds all files and their lengths to the lengthMap.
void descend(Directories &parent, LengthMap *lengthMap);
void addToBlockMap(const uintmax_t& fileSize, const fs::path& file, std::shared_ptr<BlockMap> blockMap);

// Finds the files with the same lengths in lengthMap and gets the md5 of the
// first two blocks and saves to blockMap.
void findDupesByLength(const unsigned int first, const unsigned int last,
                       const LengthMap * const lengthMap, std::shared_ptr<BlockMap> blockMap);

std::string md5sumThreadSafe(char *s, int len);
void blockMapInsertThreadSafe(std::shared_ptr<BlockMap> blockMap, std::string lengthAndMd5, fs::path file);
void md5MapInsertThreadSafe(std::shared_ptr<Md5Map> md5Map, std::string md5, fs::path file);

void addToMd5Map(std::string lenMd5, fs::path& file, std::shared_ptr<Md5Map> md5Map);

// Finds the files with the same lengths and md5 of first two blocks and
// determines md5 of entire file and saves to md5Map.
void findDupesByLengthAndBlocks(const unsigned int first, const unsigned int last,
                                std::shared_ptr<BlockMap> blockMap, std::shared_ptr<Md5Map> md5Map);

// Determines the md5 hashes that are not unique and adds to them to the CloneList
std::shared_ptr<CloneList> createCloneList(std::shared_ptr<Md5Map> md5Map);

std::string md5sum(char *s, int len);

namespace globals {
  extern unsigned long long BLOCKS_CHECK_SIZE;
  extern unsigned long long BLOCKS_SECOND_CHECK_MIN_SIZE;
  extern unsigned int BUCKET_INCREMENT;
}

namespace command_line_options {
  extern bool arewethereyet;
  extern bool fastandloose;
  extern bool isthisthingon;
  extern unsigned long long minbytes; 
  extern bool numbernice;
  extern bool perftest;
  extern std::string starting_directory;
}

namespace clo = command_line_options;

#endif