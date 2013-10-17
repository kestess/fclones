#ifndef __MAIN__H__
#define __MAIN__H__

#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
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

namespace fs = boost::filesystem;

typedef std::vector<fs::path> Directories;
typedef std::unordered_multimap<uintmax_t, fs::path> LengthMap;
typedef std::unordered_multimap<std::string, fs::path> BlockMap;
typedef std::unordered_multimap<std::string, fs::path> Md5Map;
typedef std::unordered_set<std::string> HashResults;

void descend(Directories &parent, std::shared_ptr<LengthMap> lengthMap);
void addToBlockMap(uintmax_t fileSize, fs::path& file, std::shared_ptr<BlockMap> blockMap);
void findDupesByLength(std::shared_ptr<LengthMap> lengthMap, std::shared_ptr<BlockMap> blockMap);
void addToMd5Map(fs::path& file, std::shared_ptr<Md5Map> md5Map);
void findDupesByLengthAndBlocks(std::shared_ptr<BlockMap> blockMap, std::shared_ptr<Md5Map> md5Map);
std::shared_ptr<CloneList> createCloneList(std::shared_ptr<Md5Map> md5Map);

std::string md5sum(char *s, int len);

// globals with default values
namespace globals {
  unsigned long long BLOCKS_CHECK_SIZE = 4096;
  unsigned long long BLOCKS_SECOND_CHECK_MIN_SIZE = 50 * 1024;
  unsigned int BUCKET_INCREMENT = 500;
}

namespace command_line_options {

  bool arewethereyet  = false;
  bool fastandloose = false;
  bool isthisthingon = false;
  unsigned long long minbytes = 4096; 
  bool numbernice = false;
  std::string starting_directory;

}

namespace clo = command_line_options;

#endif