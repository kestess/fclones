#include "fclones.h"

int main(int argc, char *argv[])
{

  namespace po = boost::program_options;
  po::options_description desc("Options");
  desc.add_options()
    ("arewethereyet,a", "Provide progress indicators.") // AREWETHEREYET 
    ("fastandloose,f", "Minimal check of length and a few file blocks.") // FASTANDLOOSE
    ("help", "Show this message")
    ("isthisthingon,i", "Print file currently being processed.") // ISTHISTHINGON
    ("minbytes,m", po::value<unsigned long long>(), "Minimum size of file to scan in bytes.")
    ("numbernice,n", "Output in fixed format with units.") // NUMBERNICE
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
                << " guaranteed to be compared. Use at your own risk. Do not delete files based on"
                << " this output." << std::endl;       
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
    std::cerr << "Error while processing command line options: " << e.what() << std::endl << std::endl;
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

  findDupesByLength(lengthMap, blockMap);

  if (clo::isthisthingon) std::cout << "HASH 2 of 3: Block hash has " << blockMap->size() << " entries." << std::endl;

/*
  for (auto bucket = blockMap->begin(); bucket != blockMap->end(); ++bucket)
  {
    std::cout << bucket->first << ":" << bucket->second << std::endl;
  }
*/
  //std::cout << "Blockmap size is " << blockMap->size() << std::endl;

  if ( !clo::fastandloose ) {
    findDupesByLengthAndBlocks(blockMap, md5Map);
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
