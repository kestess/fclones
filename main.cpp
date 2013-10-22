#include "fclones.h"

#include <chrono>
#include <ctime>
#

int main(int argc, char *argv[])
{

  namespace po = boost::program_options;
  po::options_description desc("Options");
  desc.add_options()
    ("arewethereyet,a", "Provide progress indicators.") // AREWETHEREYET 
    ("directory,d", po::value<std::string>(), "Starting directory for scan. Could also be last unnamed parameter.")
    ("fastandloose,f", "Minimal check of length and a few file blocks.") // FASTANDLOOSE
    ("help,h", "Show this message")
    ("isthisthingon,i", "Print file currently being processed.") // ISTHISTHINGON
    ("minbytes,m", po::value<unsigned long long>(), "Minimum size of file to scan in bytes.")
    ("numbernice,n", "Output in fixed format with units.") // NUMBERNICE
    ("perftest,p", "Output elapsed time for each stage."); // PERFTEST

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

    if ( vm.count("directory") )
    {
      clo::starting_directory = vm["directory"].as<std::string>();         
    } 
    else  // XXX remove after review
    {
      std::cerr << "Starting directory required." << std::endl;
      return 1;
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

    if ( vm.count("perftest") )
    {
      clo::perftest = true;              
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

  // auto lengthMap = std::make_shared<LengthMap>();
  LengthMap *lengthMap = new LengthMap();
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

  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();
  descend(dirs, lengthMap); // creates the length Hash of all the files.
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds_1 = end - start;
  unsigned int lengthMapSize = lengthMap->size();

  if ( clo::isthisthingon || clo::perftest ) std::cout << "Stage 1 of 3 (L): Inserted "
    << lengthMapSize << " entries in " << elapsed_seconds_1.count() << " seconds." << std::endl;

  start = std::chrono::system_clock::now();

  // hardware_concurrency might return 0 if not defined
  unsigned int num_threads = std::max(std::thread::hardware_concurrency(), 1U);
  unsigned int num_buckets = lengthMap->bucket_count();

  // 10 is a magic constant that should never have to be changed. Famous last words.
  // Prevents from having a range of 0 and starting threads for a trivial task.
  if (num_threads > num_buckets/10) num_threads = 1;   

  unsigned int range = num_buckets/num_threads;

  std::vector<std::thread> threads(num_threads - 1);

  for ( unsigned int i = 0; i < (num_threads - 1); ++i )
  { 
    threads[i] = std::thread(findDupesByLength, i * range, i * range + range, lengthMap, blockMap); 
  }

  findDupesByLength( (num_threads - 1) * range, num_buckets, lengthMap, blockMap);

  if (num_threads > 1)
  {
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
  }

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds_2 = end - start;  

  delete lengthMap;

  if ( clo::isthisthingon || clo::perftest  ) std::cout << "Stage 2 of 3 (B): Inserted "
    << blockMap->size() << " entries in " << elapsed_seconds_2.count() << " seconds." << std::endl;

/*
  for (auto bucket = blockMap->begin(); bucket != blockMap->end(); ++bucket)
  {
    std::cout << bucket->first << ":" << bucket->second << std::endl;
  }
*/
  //std::cout << "Blockmap size is " << blockMap->size() << std::endl;

  std::chrono::duration<double> elapsed_seconds_3;
  if ( !clo::fastandloose ) {
    start = std::chrono::system_clock::now();

    num_buckets = blockMap->bucket_count();

  // 10 is a magic constant that should never have to be changed. Famous last words.
  // Prevents from having a range of 0 and starting threads for a trivial task.
    if (num_threads > num_buckets/10) num_threads = 1;    

    range = num_buckets/num_threads;

    for ( unsigned int i = 0; i < (num_threads - 1); ++i )
    { 
      threads[i] = std::thread(findDupesByLengthAndBlocks, i * range, i * range + range, blockMap, md5Map); 
    }

    findDupesByLengthAndBlocks( (num_threads - 1) * range, num_buckets, blockMap, md5Map);

    if (num_threads > 1)
    {
      std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    }

    end = std::chrono::system_clock::now();
    elapsed_seconds_3 = end - start;
    if ( clo::isthisthingon || clo::perftest ) std::cout << "Stage 3 of 3 (H): Inserted " 
      << md5Map->size() << " entries in " << elapsed_seconds_3.count() << " seconds." << std::endl;
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
  unsigned int numOfFilesToDelete = 0;

  Clone::printHeading();
  for ( auto clone = clones->begin(); clone != clones->end(); ++clone )
  {
    (clo::numbernice) ? clone->prettyPrint() : clone->print(); 
    savedSpace += clone->diskSpaceSaved;
    numOfFilesToDelete += (clone->numClones - 1);
  }
  std::cout << "Total space saved after deleting duplicate files over " << clo::minbytes 
            << " long: " << Clone::formatFileSize(savedSpace) << std::endl;
  if ( clo::perftest )
  {          
    std::cout << "Total number of files that are copies (excluding one considered an original): " 
              << numOfFilesToDelete << std::endl; 
  }       
  if ( clo::perftest )
  {
    std::cout << "Stage 1 completed in " << std::setprecision(2) << std::fixed
              << elapsed_seconds_1.count() << " seconds. Inserted " << lengthMapSize << " entries." << std::endl;
    std::cout << "Stage 2 completed in " << std::setprecision(2) << std::fixed 
              << elapsed_seconds_2.count() << " seconds. Inserted " << blockMap->size() << " entries." << std::endl;
    if ( !clo::fastandloose )
    {
      std::cout << "Stage 3 completed in " << std::setprecision(2) << std::fixed
                << elapsed_seconds_3.count() << " seconds. Inserted " << md5Map->size() << " entries." << std::endl;
    }
    else
    {
      std::cout << "Stage 3 is not run in fast and loose mode." << std::endl;
    }
  }        

  return 0;
}
