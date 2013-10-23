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
unsigned int command_line_options::threads = 1;

std::atomic<int> atomic_counter(0);

// Add directories to working directory list
void descend(Directories &parent, Files &files, LengthMap *lengthMap)
{

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
        ec = no_error;
      }

      try {
        if (exists(entry) && !fs::is_symlink(entry) ) // SYMLINKY ignore symlinked directories and files.
        {
          if ( fs::is_regular_file(entry) )  
          {
            unsigned long long size = fs::file_size(entry);
            if (size >= clo::minbytes)
            {
              if (clo::isthisthingon) std::cout << "S1 " << entry << std::endl;
              lengthMap->insert(std::pair<uintmax_t, fs::path>(size, entry));
              files.push_back(std::pair<fs::path, uintmax_t>(entry, size));
            }  
          }
          else if ( fs::is_directory(entry) )
          {
            dirs.push_back(entry);
          } 
        }
      } catch (...) {
        std::cerr << "ERROR: Directory entry " << ec << " had error code " << ec << " and an exception was caught. Continuing processing." << std::endl;
      }
    });	
  }
  if (dirs.size() > 0) descend(dirs, files, lengthMap);
}

std::mutex md5_mutex; // md5 not thread safe
std::mutex block_map_mutex;

std::string md5sumThreadSafe(char *s, int len)
{
  std::lock_guard<std::mutex> guard(md5_mutex);
  std::string md5 = md5sum(s, len);
  return md5; 
}

std::mutex logging_map_mutex;

void blockMapInsertThreadSafe( std::shared_ptr<BlockMap> blockMap, std::string lengthAndMd5, fs::path file)
{
  std::lock_guard<std::mutex> guard(block_map_mutex);
  {
    std::lock_guard<std::mutex> log_guard(logging_map_mutex);
    if (clo::isthisthingon) std::cout << "S2 " << file << std::endl;
  }
  blockMap->insert(std::pair<std::string, fs::path>(lengthAndMd5, file)); 
}

std::string addToBlockMap(const uintmax_t fileSize, const fs::path file, std::shared_ptr<BlockMap> blockMap)
{

  std::string lengthAndMd5; 
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

      lengthAndMd5 = std::to_string(fileSize) + "_" + checksum;
      blockMapInsertThreadSafe(blockMap, lengthAndMd5, file);
      f.close();
    }

  } catch (...) {
    std::cerr << "ERROR: Exception caught in addToBlockMap." << std::endl;
    lengthAndMd5 = nullptr;
  }
  return lengthAndMd5;
}

void loggingLengthThreadSafe(const unsigned int size)
{
  std::lock_guard<std::mutex> guard(logging_map_mutex);
      std::cout << "Stage 2 of 3 is " << std::setprecision(1) << std::fixed
                << static_cast<float>(atomic_counter)/size * 100 << "% done." << std::endl;

}

void findDupesByLength(const unsigned int first, const unsigned int last,
                       const LengthMap * const lengthMap, const Files &files,
                       Hashes hashes, std::shared_ptr<BlockMap> blockMap)
{

  for ( unsigned int i = first; i < last; ++i )
  {
    atomic_counter++;
    

    if (clo::arewethereyet && (atomic_counter % globals::BUCKET_INCREMENT == 0) )
    {
      loggingLengthThreadSafe(files.size()); 
    }

    if ( lengthMap->count(files[i].second) > 1 )
    {
      hashes->at(i) = addToBlockMap(files[i].second, files[i].first, blockMap); 
    }
  }
 
}

std::mutex md5_map_mutex;

void md5MapInsertThreadSafe( std::shared_ptr<Md5Map> md5Map, std::string md5, fs::path file)
{
  std::lock_guard<std::mutex> guard(md5_map_mutex);
  {
    std::lock_guard<std::mutex> log_guard(logging_map_mutex);
    if (clo::isthisthingon) std::cout << "S3 " << file << std::endl;
  }
  md5Map->insert(std::pair<std::string, fs::path>(md5, file)); 
}

// Now for the md5 of the whole file
void addToMd5Map(std::string lenMd5, fs::path file, std::shared_ptr<Md5Map> md5Map)
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
      std::cerr << "ERROR: Exception caught in addToMd5Map." << std::endl;
  }
}

void loggingBlocksThreadSafe(const unsigned int size)
{
  std::lock_guard<std::mutex> guard(logging_map_mutex);
      std::cout << "Stage 3 of 3 is " << std::setprecision(1) << std::fixed
                << static_cast<float>(atomic_counter)/size * 100 << "% done." << std::endl;
}

void findDupesByLengthAndBlocks(const unsigned int first, const unsigned int last,
                                std::shared_ptr<BlockMap> blockMap, const Files &files,
                                Hashes hashes, std::shared_ptr<Md5Map> md5Map)
{

  atomic_counter = 0;

  for ( unsigned int i = first; i < last; ++i )
  {
    atomic_counter++;

    if (clo::arewethereyet && (atomic_counter % globals::BUCKET_INCREMENT == 0) )
    {
      loggingBlocksThreadSafe(files.size()); 
    }

    if ( blockMap->count(hashes->at(i)) > 1 )
    {
      addToMd5Map(hashes->at(i), files[i].first, md5Map); 
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
        std::shared_ptr<std::vector<fs::path>> files = std::make_shared<std::vector<fs::path>>();
        for ( auto it = copies.first; it != copies.second; ++it )
        {
          files->push_back(it->second);
        }
        Clone clone(files);
        savedResults.insert(hash);
        clones->push_back(clone);
      }

    } catch (...) {
      std::cerr << "ERROR: Exception caught in createCloneList." << std::endl;
    }

  }
  return clones;
}

