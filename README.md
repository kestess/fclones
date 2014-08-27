### fclones

A duplicate file finder. I was given the specifications by a company that wanted the basic functionality in an hour as a preliminary test to go to an interview. Afterward I decided to use the specifications as the basis for this utility written in C++11 and Boost. It was completed (as much as it would be) in October 2013 and I never found a reason to polish it further. As far as I know it works, but if file duplication needed to be found on an enterprise basis I think I'd have a different solution that used the filesystem to store the hash - assuming there wasn't one I could use there already.

| option | description |
| ------ | ----------- |
| FUNCTEST | there is a functional test - "make test" and then run "fclones_test" - I never used Boost Test before this, but it appears I met the intent. |
| MINBYTES | Note: the parameters do not take "=" signs, so the format would be like "--minbytes 8192" |
| FASTANDLOOSE | used as an intermediate filtering step when not used as the final step |
| PERFTEST | gprof didn't appear to work on my Mac. I tried using Instruments and it was pretty and colorful, but I didn't see the tables I'm used to seeing. Added timing mechanism to baseline performance. Current output in uniprocessor form with (-p): |
| ESCAPISM | by default thanks to boost::filesystem output |
| NUMBERNICE | shows numbers in nice format |
| ISTHISTHINGON | with AL (add length), AB (add block) or AH (add hash) designators to indicate which stage the program is in |
| AREWETHEREYET | logging |

```
ivy> ./fclones -h
fclones - utility to find duplicate files based on contents.
Options:
  -a [ --arewethereyet ]  Provide progress indicators.
  -d [ --directory ] arg  Starting directory for scan. Could also be last 
                          unnamed parameter.
  -f [ --fastandloose ]   Minimal check of length and a few file blocks.
  -h [ --help ]           Show this message.
  -i [ --isthisthingon ]  Print file currently being processed.
  -m [ --minbytes ] arg   Minimum size of file to scan in bytes.
  -n [ --numbernice ]     Output in fixed format with units.
  -p [ --perftest ]       Output elapsed time for each stage.
  -t [ --threads ] arg    Number of threads.
```

Disk IO is easily the biggest problem when writing a tool like this.

Last touched in November of 2016

Added this to check git strangeness. Testing only.
