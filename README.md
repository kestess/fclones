README
======

John Estess 10/11/2013

A company I will not name gave me the specifications for a file duplication utility and one hour to complete the required program. After looking at it and having a desire to create a correct, fast, and efficient version, I decided to implement the utility in C++ since that the language I've been using last and I need to continue to sharpen my skills since I haven't been using boost or C++11. It's obvious that I will need a few hashes and I really don't want to use libraries that aren't included with the language. I probably should be using python (I might have actually been able to finish the program in the allotted hour) or java (hashes and memory management), but I wanted the lowest amount of overhead because this program would have been extremely useful in the past. I've considered using C after relearning what really matters after the introduction of git (yet another program in C that is correct and used worldwide), but I wanted to concentrate on the core problem first. When reimplementing hashes sounds fun I may rewrite this...


Post Analysis Notes
===================

To the team,

Another reason to use C or C++ (maybe even Go) was to give me room to incorporate a way to read blocks in order if that was possible. In the time I spent doing this I haven't found a way.

This isn't the most OO of programs. I was using C++ mostly for the unordered_multimap - iterations of adding to multimaps acted as filter.

gprof wasn't working on my Mac and I'm not in the same state as my Linux box (and it's unplugged). Since the process usually ran at 10% CPU and was nearly always in "stuck" in top, I'm guessing that my major time waster is disk IO. I've run this utility against 40,000 files many times and it usually runs in 3-5.5 minutes. After finding that retrieving and taking an MD5 on the first 4096 bytes has a total runtime as long as doing 8192 bytes, and that a block of 256 bytes actually increases run time, I still check the first 8K (by MD5 sum) as a "fast and dirty" check. Apparently Linux always grabs 2 blocks (8192 bytes) when requesting a block. I found that asking the program to seek and get blocks in the middle of the file would make the program run for hours. There might have been another bug at that time, but it didn't look promising. It's funny how many bugs you can discover when running a program against a million files.

It would be easy to do this on a filesystem that already calculates some sort of checksum.

If MD5 ever becomes a significant part of the runtime there may be asm commands to incorporate since some processors have native md5 functions.

I think the best possible improvement would be the result of finding a way to marshall the needed blocks and order them so the file system could perform file operations in order; however, I think the easiest thing to do at this point would be to see how threading would affect performance. This program could be easily (relatively) split up to work on the files separately.

I've run into situations where I could have used a utility like this. It could have been written in Python or Perl, but I was looking for performance, a demonstration of C++ and Boost (or at least the opportunity to learn a little bit), and the flexibility to use any API I would ever need. 

I should really delete the hashes before the program ends.

I met the specification listed in minimal capabilities. Some of the moderate optional capabilties were addressed:

FUNCTEST - there is a functional test - "make test" and then run "fclones_test" - I never used Boost Test before this, but it appears I met the intent.
MINBYTES - done. Note: the parameters do not take "=" signs, so the format would be like "--minbytes 8192"
FASTANDLOOSE - done. Used as an intermediate filtering step when not used as the final step.
PERFTEST - not done. gprof didn't appear to work on my Mac. I tried using Instruments and it was pretty and colorful, but I didn't see the tables I'm used to seeing.
ESCAPISM - done by default thanks to boost::filesystem output.
NUMBERNICE - done.
ISTHISTHINGON - done with AL (add length), AB (add block) or AH (add hash) designators to indicate which stage the program is in. 
AREWETHEREYET - done.

I didn't incorporate any of the challenging options. I figured my "hour" was up...

This is still very rough, but I think it was a good start to discover the major challenges to really solving this problem. Due to the use of hashes it should remain linear as batch size increases.

Thanks and regards,

John Estess


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

Performance on my home directory which is a mish-mash of directories from previous computers with many permissions problems:

A simple "time find . | wc -l" counts 387711 files with the following times:
real    0m58.388s
user    0m0.691s
sys     0m3.447s

20469  fclones      5.9       01:25.22 1    0    17   371+   336M+  2756K  337M+  353M+  2714M+ 20469 14909 stuck    501  608953+   95
20469  fclones      5.7       01:34.70 1    0    17   397+   362M+  2756K  363M+  379M+  2740M+ 20469 14909 stuck    501  615618+   95
20469  fclones      2.6       02:21.09 1    0    17   439    560M+  2756K  562M+  575M   2936M  20469 14909 stuck    501  666353+   95
20469  fclones      7.6       03:32.54 1    0    17   529    1157M+ 2756K  1158M+ 1169M  3530M  20469 14909 stuck    501  819109+   95
20469  fclones      3.6       03:38.98 1    0    17   542    1244M+ 2756K  1250M+ 1259M  3620M  20469 14909 stuck    501  842555+   95
20469  fclones      5.1       03:51.81 1    0    17   568    1424M+ 2756K  1429M+ 1439M  3800M  20469 14909 stuck    501  888526+   95


