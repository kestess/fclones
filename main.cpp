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

#include "fclones.h"
#include "main.h"


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
  	Sorting is optional (perhaps for debugging only?)
  	sort(parent.begin(), parent.end()); 
    */

	for (auto dir : parent)
	{
		// using the non-throwing directory_iterator and catching permission error (and probably others) while processing entries
		std::for_each(boost::filesystem::directory_iterator(dir, ec), boost::filesystem::directory_iterator(), [&] (fs::path entry)
		{
      if (ec != no_error) {
        std::cerr << "ERROR: Entry " << entry << " resulted in error code: " << ec << "." << std::endl;
      }

      try {
        if (exists(entry))
        {
          if (is_regular_file(entry))
          {
            unsigned long long size = fs::file_size(entry);
            if (size >= clo::minbytes) {
              if (clo::isthisthingon) std::cout << "AL " << entry << std::endl;
              lengthMap->insert(std::pair<uintmax_t, fs::path>(fs::file_size(entry), entry));
            }  
          }
          else if (is_directory(entry))
          {
            dirs.push_back(entry);
          } 
        }
      } catch (...) {
        std::cerr << "Directory entry " << ec << " had error code " << ec << " and an exception was caught. Continuing processing." << std::endl;
      }
    });	
  }
  if (dirs.size() > 0) descend(dirs, lengthMap);
}

void addToBlockMap(uintmax_t fileSize, fs::path& file, std::shared_ptr<BlockMap> blockMap)
{

  try {
    std::ifstream f (fs::canonical(file).string(), std::ios::in | std::ios::binary);
    std::string checksum;
    char *block = nullptr;

    if (f.is_open())
    {
      int len = std::min(static_cast<unsigned long long>(fileSize), globals::BLOCKS_CHECK_SIZE);
      std::streambuf *raw_blocks = f.rdbuf();

      block = new char[len];
      raw_blocks->sgetn(block, len);

      // WARNING!!! binary files will have NULLS and special characters - must pass length without
      // using strlen or anything else that terminates with a NULL

      checksum = md5sum(block, len);
      delete[] block;

      std::string lengthAndMd5 = std::to_string(fileSize) + "_" + checksum;
      if (clo::isthisthingon) std::cout << "AB " << file << std::endl;
      blockMap->insert(std::pair<std::string, fs::path>(lengthAndMd5, file));

      f.close();
    }

  } catch (...) {
    std::cerr << "Exception caught in addToBlockMap." << std::endl;
  }
}

void findDupesByLengthAndBlocks(std::shared_ptr<LengthMap> lengthMap, std::shared_ptr<BlockMap> blockMap)
{
  unsigned int bucket = 0;
  unsigned int bucket_last = 0;
  unsigned int bucket_len = lengthMap->bucket_count();

  for (bucket = 0; bucket < bucket_len; ++bucket) 
  {

    if (clo::arewethereyet && bucket != bucket_last && (bucket % clo::BUCKET_INCREMENT == 0) )
    {
      std::cout << "Stage 2 of 3 is " <<  std::setprecision(0) << std::fixed
                << static_cast<float>(bucket)/bucket_len * 100 << "% done." << std::endl;
      bucket_last = bucket;
    }

    // Same size files go into the same buckets and the bucket is shared with other lengths
    if ( lengthMap->bucket_size(bucket) < 2 ) continue;
    for ( auto it = lengthMap->begin(bucket); it != lengthMap->end(bucket); ++it )
    {
      if ( lengthMap->count(it->first) > 1 )
      {
        addToBlockMap(it->first, it->second, blockMap);   
      }
    } 

  }
}

// Now for the md5 of the whole file
void addToMd5Map(fs::path& file, std::shared_ptr<Md5Map> md5Map)
{
  try {
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

     if (clo::isthisthingon) std::cout << "AH " << file << std::endl;
     md5Map->insert(std::pair<std::string, fs::path>(checksum, file));
     f.close();
    }
  } catch (...) {
    std::cerr << "Exception caught in addToMd5Map." << std::endl;
  }
}

void getAllMd5(std::shared_ptr<BlockMap> blockMap, std::shared_ptr<Md5Map> md5Map)
{
  unsigned int bucket = 0;
  unsigned int bucket_last = 0;
  unsigned int bucket_len = blockMap->bucket_count();

  for (bucket = 0; bucket < bucket_len; ++bucket)
  {

    if (clo::arewethereyet && bucket != bucket_last && (bucket % clo::BUCKET_INCREMENT == 0) )
    {
        std::cout << "Stage 3 of 3 is " <<  std::setprecision(0) << std::fixed
                  << static_cast<float>(bucket)/bucket_len * 100 << "% done." << std::endl;
        bucket_last = bucket;                   
    }

    if ( blockMap->bucket_size(bucket) < 2 ) continue;
    for ( auto it = blockMap->begin(bucket); it != blockMap->end(bucket); ++it )
    {
      if ( blockMap->count(it->first) > 1 )
      {
        addToMd5Map(it->second, md5Map);
      }
    }

  }
}

std::shared_ptr<CloneList> createCloneList(std::shared_ptr<Md5Map> md5Map)
{

  auto clones = std::make_shared<CloneList>();
  HashResults savedResults;

  for (auto bucket = md5Map->begin(); bucket != md5Map->end(); ++bucket)
  {

    std::string hash = bucket->first;
    // if the hash bucket has one item that item can be ignored.
    if ( md5Map->count(hash) < 2 ) continue;
    // gather all files with same hash
    auto copies = md5Map->equal_range(hash); 
    std::string names;

    try {

      for ( auto it = copies.first; it != copies.second; ++it )
      {
        names += fs::canonical(it->second).string() + ";";
      }
      if (names.length() > 0) names.pop_back(); // remove trailing semicolon

      auto alreadyDone = savedResults.find(hash);

      // hash has not been found already
      if (alreadyDone == savedResults.end())
      {
        //std::cout << "First time for  " << hash << std::endl;   
        unsigned long long filesize = fs::file_size(bucket->second);
        Clone clone(md5Map->count(bucket->first), filesize, names);
        savedResults.insert(hash);
        clones->push_back(clone);
      } else {
        //std::cout << "Found hash " << hash << " already."  << std::endl;
      }
    } catch (...) {
      std::cerr << "Exception caught in createCloneList." << std::endl;
    }

  }
  return clones;
}


int main(int argc, char *argv[])
{

  namespace po = boost::program_options;
  po::options_description desc("Options");
  desc.add_options()
    ("arewethereyet,a", "Provide progress indicators.")
    ("fastandloose,f", "Minimal check of length and a few file blocks.")
    ("help", "Show this message")
    ("isthisthingon,i", "Print file currently being processed.")
    ("minbytes,m", po::value<unsigned long long>(), "Minimum size of file to scan in bytes.")
    ("numbernice,n", "Output in fixed format with units.")
    ("directory,d", po::value<std::string>(), "Starting directory for scan. Could also be last unnamed parameter.");

  po::positional_options_description pod;
  pod.add("directory", 1);  
  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pod).run(), vm);

    if ( vm.count("help") )
    {
      std::cerr << "fclones - utility to find duplicate files based on contents." << std::endl
                << desc << std::endl;
      return 1;          
    } 

    if ( vm.count("arewethereyet") )
    {
       clo::arewethereyet = true;  
    } 

    if ( vm.count("isthisthingon") )
    {
       clo::isthisthingon = true;  
    }     

    if ( vm.count("minbytes") )
    {
      clo::minbytes = vm["minbytes"].as<unsigned long long>();     
    } 

    if ( vm.count("numbernice") )
    {
      clo::numbernice = true;              
    }     

    if ( vm.count("fastandloose") )
    {
      clo::fastandloose = true;
      std::cerr << "WARNING! Fast and loose option on. Only file size and first two block are"
                << " guaranteed to be checked."
                << " Use at your own risk. Do not delete files based on this output." << std::endl;       
    } 

    if ( vm.count("directory") )
    {
      clo::starting_directory = vm["directory"].as<std::string>();         
    } 
    else  // XXX remove after review
    {
      std::cerr << "Starting directory required." << std::endl;
      return 1;
    }  
    po::notify(vm); 
  }
  catch(po::error &e)
  {
    std::cerr << "MY ERROR: " << e.what() << std::endl << std::endl;
    std::cerr << desc << std::endl;
    return 1;
  }

  Directories dirs;

  auto lengthMap = std::make_shared<LengthMap>();
  auto blockMap  = std::make_shared<BlockMap>();
  auto md5Map    = std::make_shared<Md5Map>();

  // XXX As soon as this is reviewed this will be changed back.
  // fs::path dir = (clo::starting_directory == "") ? fs::current_path() : clo::starting_directory;
  fs::path dir(clo::starting_directory);

  if (!is_directory(dir))
  {
    std::cerr << dir << " is not a directory. Aborting..." << std::endl;
    return 1;    
  }

  dirs.push_back(dir);  
  descend(dirs, lengthMap); // creates the length Hash of all the files.

  if (clo::isthisthingon) std::cout << "HASH 1 of 3: Length hash has " << lengthMap->size() << " entries." << std::endl;

  findDupesByLengthAndBlocks(lengthMap, blockMap);

  if (clo::isthisthingon) std::cout << "HASH 2 of 3: Block hash has " << blockMap->size() << " entries." << std::endl;

/*
  for (auto bucket = blockMap->begin(); bucket != blockMap->end(); ++bucket)
  {
    std::cout << bucket->first << ":" << bucket->second << std::endl;
  }
*/
  //std::cout << "Blockmap size is " << blockMap->size() << std::endl;

  if ( !clo::fastandloose ) {
    getAllMd5(blockMap, md5Map);
    if (clo::isthisthingon) std::cout << "HASH 3 of 3: MD5 hash has " << md5Map->size() << " entries." << std::endl;
  }

/*
  for (auto bucket = md5Map->begin(); bucket != md5Map->end(); ++bucket)
  {
    std::cout << "MD5 " << bucket->first << ":" << bucket->second << std::endl;
  }
*/

  auto clones = createCloneList(( clo::fastandloose ) ? blockMap : md5Map);
  std::sort(clones->begin(), clones->end());
  unsigned long long savedSpace = 0;

  Clone::printHeading();
  for (auto clone = clones->begin(); clone != clones->end(); ++clone)
  {
    (clo::numbernice) ? clone->prettyPrint() : clone->print();
    savedSpace += clone->diskSpaceSaved;
  }
  std::cout << "Total space saved after deleting duplicate files over " << clo::minbytes 
            << " long: " << Clone::formatFileSize(savedSpace) << std::endl;

  return 0;
}
