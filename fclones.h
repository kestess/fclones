#ifndef __FCLONES__H__
#define __FCLONES__H__

class Clone
{

public:

    unsigned long long diskSpaceSaved;
	unsigned int numberOfClones;
	unsigned long long fileSize;
	std::string nameList;

	Clone(unsigned int numberOfClones,
          unsigned long long fileSize,
          std::string nameList);

    static void printHeading()
    {
    	std::cout << "\"Saved Space\",\"Number Of Duplicates\",\"File Size\",\"List of Names\"" << std::endl;
    }

    // Reports: Disk space saved, number of duplicates (the first isn't a duplicate), file size, name list
	void print()
	{
		std::cout << diskSpaceSaved << "," << numberOfClones - 1 << "," << fileSize << "," << nameList << std::endl;
	}

	void prettyPrint()
	{
		std::cout << formatFileSize(diskSpaceSaved) << "," << numberOfClones - 1 << "," 
		          << formatFileSize(fileSize) << "," << nameList << std::endl;
	}

	static std::tuple<std::string, unsigned long long> getFileSizeFormatting(unsigned long long size);

	static std::string formatFileSize(unsigned long long size);

	friend bool operator< (const Clone& a, const Clone& b)
	{
		// will be sorted in descending order - default is ascending
		return std::tie(b.diskSpaceSaved, b.fileSize) < std::tie(a.diskSpaceSaved, a.fileSize);
	}

};

typedef std::vector<Clone> CloneList;

#endif