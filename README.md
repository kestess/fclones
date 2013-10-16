README

John Estess 10/11/2013

A company I will not name gave me the specifications for a file duplication utility and one hour to complete the required program. After looking at it and having a desire to create a correct, fast, and efficient version, I decided to implement the utility in C++ since that the language I've been using last and I need to continue to sharpen my skills since I haven't been using boost or C++11. It's obvious that I will need a few hashes and I really don't want to use libraries that aren't included with the language. I probably should be using python (I might have actually been able to finish the program in the allotted hour) or java (hashes and memory management), but I wanted the lowest amount of overhead because this program would have been extremely useful in the past. I've considered using C after relearning what really matters after the introduction of git (yet another program in C that is correct and used worldwide), but I wanted to concentrate on the core problem first. When reimplementing hashes sounds fun I may rewrite this...

The specifications are:

===============================================================================

Specifications
==============

Duplicate File Finder

[A] Minimum Capabilities

Implement a single-threaded in-memory command-line utility that scans a filesystem (assume for now it is on a local hard disk) and outputs a list of duplicated files. 
It should have the following capabilities:
1) There is one required parameter for the command line:  path. 
    All files under a specified start path (e.g C:\ or /home/users/anne) will be recursively checked by the utility
    (If you are unfamiliar with directory-enumeration APIs, it is OK to assume and use some function get_files_and_sizes(path) that returns a list of (pathname, size) pairs under the specified path. If you know the actual directory-enumeration APIs for your chosen language/OS, please use them)
2) The program's goal is to measure 'wasted' space due to duplicate files.
    Files are considered duplicates only if they contain the same stream of bytes when read using normal file open/read APIs
    Files can be duplicates even if their filenames differ, or if they have different timestamps or owners 
    (Ignore the issues of hard-linked or symbolic-linked files for now - assume those don't exist)
    (Ignore secondary data streams or extended attributes, resource forks etc on filesystems that support such things - assume those don't exist)
3) The program will print a list of files that have duplicates as follows:
    Each row of the list has the following entries, separated by commas (,)   Rows are separated by newlines (\n)
        Column 1 (SORT, DESCENDING): The amount of disk space that would be saved if all copies of the file were 'single-instanced' - for example hard-linked to a single copy of the data
        Column 2: The number of times this file is duplicated
        Column 3: The size of a single instance of the file
        Column 4: The list of filenames (including full path) for each duplicate. This list should be semicolon-separated
    Finally, the program shows how much space will be saved by de-duplicating all the files (e.g. sum of Column 1)
    All size numbers (file size, space saved etc) can be shown in units of bytes. Comma separation would be appreciated.
4) The program should be efficient
    Scan each file a minimal amount of times - approximately one scan per file is a good goal
    Use metadata checks where possible to rule out files that can't be the same (different file size) instead of scanning them all
5) The program should be accurate   
    Your solution should have no realistic chance of mistakenly considering two different files as duplicates...

All code should have simple unit tests.

You can assume that basic collection types (hashes, lists, queues, stacks) are available to you. If you use them, write a comment describing their behavior, or make it clear which one you are using (e.g  java.util.LinkedList). 

 
[B] OPTIONAL Extra credit / Feature Creep

If you have time, here are some optional "extra credit" features you can also choose to implement - upgrade your answer, impress us further and have fun!  We consider getting requirements 1-5 above correct to be more important than "extra credit" answers, so only take these on if you are confident that you have correctly implemented the minimum capabilities described above. 

Each list of suggestions is in no particular order. Use the 'shortname' tags here in your comments for the relevant code (e.g. // FUNCTEST) so we can easily see which of these you may have implemented.

Moderate options...

- FUNCTEST. A good functional test for the utility. This includes a way to mock/generate data sets and check the results. You will probably need to separate your code into a calculation part and an output part to make it more easily testable..
- MINBYTES. The minimum file size of interest can be specified as an OPTIONAL parameter to the program specified as --minbytes=<number>. If not specified, the default is 4096 bytes. Files smaller than this size would be ignored by this utility
- FASTANDLOOSE. Implement a --fastandloose option that can realistically have some false/missed duplicates, but is really fast yet better than just file size comparison (for example doing partial file compares rather than full file compares).
- PERFTEST. A performance test for the utility. Describe briefly in comments any profiling tools you would use and how with this test to identify bottlenecks
- ESCAPISM. Escape/Quote any separator characters in the output filenames (e..g "," and ";") so it's output can be parsed by other tools as a valid .CSV format file
- NUMBERNICE. All output sizes are expressed in a size-appropriate number with 1 decimal place, i.e. 6.2 GB, 302.1 MB, 45.5 KB etc  Support also TB and PB.
- ISTHISTHINGON. Show some progress indicator to the user as the scan goes on - current file being scanned
- AREWETHEREYET. Give an estimate of remaining time to completion, or at least % of 'work' done for some reasonably linear scale of work.

Challenging options...

- HARDLINKY. Do the right thing if files are hard-linked. Define in comments what you think 'the right thing' is and implement it. You may assume you have a function that can count and find all pathnames that hard link to a given pathname). 
- SYMLINKY. Do the right thing if files or folders are symbolically linked. Define in comments what you think 'the right thing' is and implement it
- NOSEEKS. If you had an api to get you the block ranges for a file, and an api to read those block ranges from disk, implement a mode that uses these to scan the system while minimizing disk seeks.
- UNSTABLE. The filesystem might be changing while the utility runs. Assume at the end of the run you can call a function that gives you a list of filenames under a path that changed since a specified point in time and the type of change (name, flag:deleted, flag:written, flag:created, flag:renamed, flag:isdirectory). Improve your solution to correct the results based on these changes without rescanning the entire disk. 
- HYDRA. This problem may involve a lot of asynchronous I/O and the disk subsystem may be a RAID-type system that can do a lot of concurrent I/O using many 'disk heads'. Use threads, or async I/O or other technique in order to have an implementation that exploits the potential performance of such a storage subsystem.

===============================================================================
