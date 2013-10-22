#include "clone.h"

Clone::Clone(//unsigned int numClones,
             //unsigned long long fileSize,
             std::shared_ptr<std::vector<fs::path>> files)
{
  this->fileSize = fs::file_size(files->at(0)); // Will throw exception if no clones, but shouldn't be able to get here.
  this->numClones = files->size();
  this->diskSpaceSaved = (numClones - 1) * fileSize;
  this->files = files;
  adjustHardLinks();
}


// HARDLINKY
// Only counts the files with unique inodes. Adjust numClones and diskSpaceSaved accordingly.
void Clone::adjustHardLinks()
{
  // We've already visited this file.
  bool *already_counted = new bool[numClones](); // zero init

  for ( auto file = files->begin(); file != files->end(); ++file )
  {
    nameList += fs::canonical(*file).string() + ";";
    if ( fs::hard_link_count(*file) > 1 )
    {
      // Just because it is hard-linked doesn't mean both links exist in the directory tree that was searched.
      for ( auto fileredux = files->begin(); fileredux != files->end(); ++fileredux )
      {
        already_counted[file - files->begin()] = true;
        if ( (!already_counted[fileredux - files->begin()]) && fs::equivalent(*file, *fileredux) )
        {
          already_counted[fileredux - files->begin()] = true;
          diskSpaceSaved -= fileSize;
          numClones--;
        }
      }
    }
  } 

  delete[] already_counted;
  if (nameList.length() > 0) nameList.pop_back(); 
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