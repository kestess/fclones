#ifndef __CLONE__H__
#define __CLONE__H__

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>
#include <vector>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class Clone
{

public:

    unsigned long long diskSpaceSaved;
	unsigned int numClones;
	unsigned long long fileSize;
	std::shared_ptr<std::vector<fs::path>> files;
	std::string nameList;

	Clone(//unsigned int numClones,
          //unsigned long long fileSize,
          std::shared_ptr<std::vector<fs::path>> files);

    // This is quadratic, but it's your own fault if you are using hard links.
    // Removes the disk space saved and reduces the number of clones to reflect only one inode per file.
    void adjustHardLinks();

    static void printHeading()
    {
    	std::cout << "\"Saved Space\",\"Number Of Duplicates\",\"File Size\",\"List of Names\"" << std::endl;
    }

    // Reports: Disk space saved, number of duplicates (the first isn't a duplicate), file size, name list
    // ESCAPISM - file names were double-quoted by default due to the use of boost::filesystem
	void print() const
	{
		std::cout << diskSpaceSaved << "," << numClones - 1 << "," << fileSize << "," << nameList << std::endl;
	}

	void prettyPrint() const
	{
		std::cout << formatFileSize(diskSpaceSaved) << "," << numClones - 1 << "," 
		          << formatFileSize(fileSize) << "," << nameList << std::endl;
	}

	static std::tuple<std::string, unsigned long long> getFileSizeFormatting(unsigned long long size);

	static std::string formatFileSize(unsigned long long size);

	friend bool operator< (const Clone& a, const Clone& b)
	{
		// Will be sorted in descending order
		return std::tie(b.diskSpaceSaved, b.numClones, b.fileSize, b.nameList) < std::tie(a.diskSpaceSaved, a.numClones, a.fileSize, a.nameList);
	}

};

typedef std::vector<Clone> CloneList;

#endif