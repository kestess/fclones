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
PERFTEST - somewhat - gprof didn't appear to work on my Mac. I tried using Instruments and it was pretty and colorful, but I didn't see the tables I'm used to seeing. Added timing mechanism to baseline performance. Current output in uniprocessor form with (-p):

    Total space saved after deleting duplicate files over 4096 long: 550.8 MB
    Total number of files that are copies (excluding one considered an original): 5524
    Stage 1 completed in 0.45 seconds. Inserted 18083 entries.
    Stage 2 completed in 1.39 seconds. Inserted 13503 entries.
    Stage 3 completed in 12.12 seconds. Inserted 10709 entries.

    Note: I have a boost folder and a boost_old folder that provide many copies. Also, my mysql installation dir is symlinked and provides an excellent 
          excuse to fix the symlink problem. UPDATE - fixed half the symlink problem - the dirs. New times (excludes the mysql symlink)

    Total space saved after deleting duplicate files over 4096 long: 90.4 MB
    Total number of files that are copies (excluding one considered an original): 3061
    Stage 1 completed in 0.38 seconds. Inserted 15568 entries.
    Stage 2 completed in 1.00 seconds. Inserted 9656 entries.
    Stage 3 completed in 2.10 seconds. Inserted 5865 entries.

    UPDATE - removed all symlinks:

    Total space saved after deleting duplicate files over 4096 long: 69.7 MB
    Total number of files that are copies (excluding one considered an original): 3053
    Stage 1 completed in 0.48 seconds. Inserted 15554 entries.
    Stage 2 completed in 27.86 seconds. Inserted 9639 entries.
    Stage 3 completed in 1.97 seconds. Inserted 5853 entries.

ESCAPISM - done by default thanks to boost::filesystem output.
NUMBERNICE - done.
ISTHISTHINGON - done with AL (add length), AB (add block) or AH (add hash) designators to indicate which stage the program is in. 
AREWETHEREYET - done.

I didn't incorporate any of the challenging options. I figured my "hour" was up...

This is still very rough, but I think it was a good start to discover the major challenges to really solving this problem. Due to the use of hashes it should remain linear as batch size increases.

Thanks and regards,

John Estess


The specifications are:

Deleted.

Notes:
======

Total space saved after deleting duplicate files over 0 long: 78.9 MB
Total number of files that are copies (excluding one considered an original): 9163
Stage 1 completed in 0.55 seconds. Inserted 37746 entries.
Stage 2 completed in 67.56 seconds. Inserted 31401 entries.
Stage 3 completed in 2.62 seconds. Inserted 17506 entries.

Total space saved after deleting duplicate files over 0 long: 78.9 MB
Total number of files that are copies (excluding one considered an original): 9163
Stage 1 completed in 0.56 seconds. Inserted 37746 entries.
Stage 2 completed in 93.89 seconds. Inserted 31401 entries.
Stage 3 completed in 2.69 seconds. Inserted 17506 entries.

It's obvious the files are paged in before stage 3, but it's still a baseline.



After not going back to disk for files that are less than 8 Kb long:
====================================================================
(data finding all files - not using 4096 limit)

Total space saved after deleting duplicate files over 0 long: 78.9 MB
Total number of files that are copies (excluding one considered an original): 9163
Stage 1 completed in 0.54 seconds. Inserted 37746 entries.
Stage 2 completed in 2.80 seconds. Inserted 31401 entries.
Stage 3 completed in 2.25 seconds. Inserted 17500 entries.

Total space saved after deleting duplicate files over 0 long: 78.9 MB
Total number of files that are copies (excluding one considered an original): 9163
Stage 1 completed in 0.54 seconds. Inserted 37746 entries.
Stage 2 completed in 68.21 seconds. Inserted 31401 entries.
Stage 3 completed in 2.35 seconds. Inserted 17500 entries.

Over a 10% decrease...not great, but not bad.

I had a six more files, but they are gone now. Reverted the code and still had 17500 files. Mysql?
Testing on live data is great, but it can drive you crazy.

Before thread-safe blockMap insert:
./fclones -m 0 -p /usr/local
-Stage 1 of 3 (L): Inserted 18083 entries in 0.446768 seconds.
-Stage 2 of 3 (B): Inserted 13503 entries in 35.1348 seconds.
-Stage 3 of 3 (H): Inserted 10709 entries in 12.085 seconds.

After running a few times to make sure /usr/local is mostly in memory using "./fclones -a -p /usr/local"
(I'm only testing the mutex penalty at the moment)

Stage 1 completed in 0.48 seconds. Inserted 15554 entries.
Stage 2 completed in 1.50 seconds. Inserted 9639 entries.
Stage 3 completed in 1.66 seconds. Inserted 5847 entries.

Stage 1 completed in 0.49 seconds. Inserted 15554 entries.
Stage 2 completed in 1.70 seconds. Inserted 9639 entries.
Stage 3 completed in 1.61 seconds. Inserted 5847 entries.

There doesn't seem to be a mutex penalty, but the compiler might have noticed the program
was still multi-threaded:

Stage 1 completed in 0.48 seconds. Inserted 15554 entries.
Stage 2 completed in 1.24 seconds. Inserted 9639 entries.
Stage 3 completed in 1.64 seconds. Inserted 5847 entries.

// After threading stage 2 four ways (my hardware concurrency)
Total space saved after deleting duplicate files over 4096 long: 3.5 MB
Total number of files that are copies (excluding one considered an original): 344
Stage 1 completed in 0.48 seconds. Inserted 15554 entries.
Stage 2 completed in 0.74 seconds. Inserted 9639 entries.
Stage 3 completed in 0.08 seconds. Inserted 648 entries.  // Oops!!!

running just the one thread works:
Total space saved after deleting duplicate files over 4096 long: 69.7 MB
Total number of files that are copies (excluding one considered an original): 3053
Stage 1 completed in 0.48 seconds. Inserted 15554 entries.
Stage 2 completed in 2.36 seconds. Inserted 9639 entries.
Stage 3 completed in 5.06 seconds. Inserted 5847 entries.

4 threads (unfortunately md5 is not thread safe so the whole thing is mutexed...):
Stage 1 completed in 0.51 seconds. Inserted 15554 entries.
Stage 2 completed in 0.96 seconds. Inserted 9639 entries.
Stage 3 completed in 1.66 seconds. Inserted 5847 entries.

Total space saved after deleting duplicate files over 4096 long: 69.7 MB
Total number of files that are copies (excluding one considered an original): 3053
Stage 1 completed in 0.50 seconds. Inserted 15554 entries.
Stage 2 completed in 0.97 seconds. Inserted 9639 entries.
Stage 3 completed in 1.67 seconds. Inserted 5847 entries.

Both stage 2 and 3 are now threaded. MD5 is not.

Total space saved after deleting duplicate files over 4096 long: 8.2 MB
Total number of files that are copies (excluding one considered an original): 1364
Stage 1 completed in 0.49 seconds. Inserted 15554 entries.
Stage 2 completed in 0.94 seconds. Inserted 9639 entries.
Stage 3 completed in 0.77 seconds. Inserted 5847 entries.

Wow - and that was a bomb

Stage 1 completed in 0.48 seconds. Inserted 15554 entries.
Stage 2 completed in 0.94 seconds. Inserted 9639 entries.
Stage 3 completed in 1.39 seconds. Inserted 5847 entries.

Back to single threaded (num_threads = 1)

Stage 1 completed in 0.51 seconds. Inserted 15554 entries.
Stage 2 completed in 1.21 seconds. Inserted 9639 entries.
Stage 3 completed in 1.56 seconds. Inserted 5847 entries.

Stage 1 completed in 0.52 seconds. Inserted 15554 entries.
Stage 2 completed in 1.59 seconds. Inserted 9639 entries.
Stage 3 completed in 2.86 seconds. Inserted 5847 entries.


Total space saved after deleting duplicate files over 0 long: 78.9 MB
Total number of files that are copies (excluding one considered an original): 9163
Stage 1 completed in 0.54 seconds. Inserted 37746 entries.
Stage 2 completed in 2.80 seconds. Inserted 31401 entries.
Stage 3 completed in 2.25 seconds. Inserted 17500 entries.


Forcing one processor:
Total space saved after deleting duplicate files over 0 long: 78.9 MB
Total number of files that are copies (excluding one considered an original): 9163
Stage 1 completed in 0.54 seconds. Inserted 37746 entries.
Stage 2 completed in 2.80 seconds. Inserted 31401 entries.
Stage 3 completed in 2.21 seconds. Inserted 17500 entries.

FOUR
====
Total space saved after deleting duplicate files over 0 long: 78.9 MB
Total number of files that are copies (excluding one considered an original): 9163
Stage 1 completed in 0.54 seconds. Inserted 37746 entries.
Stage 2 completed in 2.77 seconds. Inserted 31401 entries.
Stage 3 completed in 1.56 seconds. Inserted 17500 entries.

THREE
=====
"Saved Space","Number Of Duplicates","File Size","List of Names"
Total space saved after deleting duplicate files over 0 long: 78.9 MB
Total number of files that are copies (excluding one considered an original): 9163
Stage 1 completed in 0.55 seconds. Inserted 37746 entries.
Stage 2 completed in 2.82 seconds. Inserted 31401 entries.
Stage 3 completed in 1.63 seconds. Inserted 17500 entries.
