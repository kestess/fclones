#include "clone.h"

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