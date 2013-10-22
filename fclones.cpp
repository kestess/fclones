#include "fclones.h"

unsigned long long globals::BLOCKS_CHECK_SIZE = 8192;
unsigned int globals::BUCKET_INCREMENT = 500;

bool command_line_options::arewethereyet  = false;
bool command_line_options::fastandloose = false;
bool command_line_options::isthisthingon = false;
unsigned long long command_line_options::minbytes = 4096; 
bool command_line_options::numbernice = false;
bool command_line_options::perftest = false;
std::string command_line_options::starting_directory;

std::atomic<int> atomic_counter(0);

// Add directories to working directory list
void descend(Directories &parent, LengthMap *lengthMap)
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
        if (exists(entry) && !fs::is_symlink(entry) ) // SYMLINKY ignore symlinked directories and files.
        {
          if ( fs::is_regular_file(entry) )  
          {
            unsigned long long size = fs::file_size(entry);
            if (size >= clo::minbytes) {
              if (clo::isthisthingon) std::cout << "AL " << entry << std::endl;
              lengthMap->insert(std::pair<uintmax_t, fs::path>(fs::file_size(entry), entry));
            }  
          }
          else if ( fs::is_directory(entry) )
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

std::mutex md5_mutex; // md5 not thread safe
std::mutex block_map_mutex;

std::string md5sumThreadSafe(char *s, int len)
{
  std::lock_guard<std::mutex> guard(md5_mutex);
  std::string md5 = md5sum(s, len);
  return md5; 
}

void blockMapInsertThreadSafe( std::shared_ptr<BlockMap> blockMap, std::string lengthAndMd5, fs::path file)
{
  std::lock_guard<std::mutex> guard(block_map_mutex);
  if (clo::isthisthingon) std::cout << "AB " << file << std::endl;
  blockMap->insert(std::pair<std::string, fs::path>(lengthAndMd5, file)); 
}

void addToBlockMap(const uintmax_t& fileSize, const fs::path& file, std::shared_ptr<BlockMap> blockMap)
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

      checksum = md5sumThreadSafe(block, len);
      delete[] block;

      std::string lengthAndMd5 = std::to_string(fileSize) + "_" + checksum;
      blockMapInsertThreadSafe(blockMap, lengthAndMd5, file);
      f.close();
    }

  } catch (...) {
    std::cerr << "Exception caught in addToBlockMap." << std::endl;
  }
}

std::mutex logging_map_mutex;

void loggingLengthThreadSafe(const LengthMap * const lengthMap)
{
  std::lock_guard<std::mutex> guard(logging_map_mutex);
      std::cout << "Stage 2 of 3 is " << std::setprecision(1) << std::fixed
                << static_cast<float>(atomic_counter)/lengthMap->bucket_count() * 100 << "% done." << std::endl;

}

void findDupesByLength(const unsigned int first, const unsigned int last,
                       const LengthMap * const lengthMap, std::shared_ptr<BlockMap> blockMap)
{
  unsigned int bucket = first;
  unsigned int bucket_last = bucket;

  for (bucket = first; bucket < last; ++bucket) 
  {
    atomic_counter++;
    if (clo::arewethereyet && bucket != bucket_last && (bucket % globals::BUCKET_INCREMENT == 0) )
    {
      loggingLengthThreadSafe(lengthMap);
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

std::mutex md5_map_mutex;

void md5MapInsertThreadSafe( std::shared_ptr<Md5Map> md5Map, std::string md5, fs::path file)
{
  std::lock_guard<std::mutex> guard(md5_map_mutex);
  if (clo::isthisthingon) std::cout << "AH " << file << std::endl;
  md5Map->insert(std::pair<std::string, fs::path>(md5, file)); 
}

// Now for the md5 of the whole file
void addToMd5Map(std::string lenMd5, fs::path& file, std::shared_ptr<Md5Map> md5Map)
{
  try {
    std::ifstream f (fs::canonical(file).string(), std::ios::in | std::ios::binary);
    std::string checksum;
    unsigned int size = fs::file_size(file);

    if ( size > globals::BLOCKS_CHECK_SIZE )
    { 
      // get full MD5 of file 
      if (f.is_open())
      {
       std::streambuf* raw_buffer = f.rdbuf();
       char* block = new char[size];
       raw_buffer->sgetn(block, size);
       checksum = md5sumThreadSafe(block, size);
       delete[] block;

       md5MapInsertThreadSafe(md5Map, checksum, file);
       f.close();
      }
    } 
    else
    {
      std::string md5 = lenMd5.substr( lenMd5.find_first_of("_") + 1 );
      md5MapInsertThreadSafe(md5Map, md5, file);
    }
  } catch (...) {
      std::cerr << "Exception caught in addToMd5Map." << std::endl;
  }
}

void loggingBlocksThreadSafe(const std::shared_ptr<BlockMap> blockMap)
{
  std::lock_guard<std::mutex> guard(logging_map_mutex);
      std::cout << "Stage 3 of 3 is " << std::setprecision(1) << std::fixed
                << static_cast<float>(atomic_counter)/blockMap->bucket_count() * 100 << "% done." << std::endl;

}

void findDupesByLengthAndBlocks(const unsigned int first, const unsigned int last,
                                std::shared_ptr<BlockMap> blockMap, std::shared_ptr<Md5Map> md5Map)
{
  unsigned int bucket = first;
  unsigned int bucket_last = bucket;

  atomic_counter = 0;

  for (bucket = first; bucket < last; ++bucket) 
  {
    atomic_counter++;
    if ( clo::arewethereyet && bucket != bucket_last && (bucket % globals::BUCKET_INCREMENT == 0) )
    {
        loggingBlocksThreadSafe(blockMap);
        bucket_last = bucket;                   
    }

    if ( blockMap->bucket_size(bucket) < 2 ) continue;
    for ( auto it = blockMap->begin(bucket); it != blockMap->end(bucket); ++it )
    {
      if ( blockMap->count(it->first) > 1 )
      {
        addToMd5Map(it->first, it->second, md5Map);
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
    
    try {

      auto alreadyDone = savedResults.find(hash);

      // hash has not been found already
      if (alreadyDone == savedResults.end())
      {
        //std::string names;
        std::shared_ptr<std::vector<fs::path>> files = std::make_shared<std::vector<fs::path>>();
        for ( auto it = copies.first; it != copies.second; ++it )
        {
          // names += fs::canonical(it->second).string() + ";";
          files->push_back(it->second);
        }
        //if (names.length() > 0) names.pop_back(); // remove trailing semicolon
        //std::cout << "First time for  " << hash << std::endl;   
        // unsigned long long filesize = fs::file_size(bucket->second);
        Clone clone(files);
        savedResults.insert(hash);
        clones->push_back(clone);
      }
      else
      {
        //std::cout << "Found hash " << hash << " already."  << std::endl;
      }

    } catch (...) {
      std::cerr << "Exception caught in createCloneList." << std::endl;
    }

  }
  return clones;
}

