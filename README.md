PURPOSE
----------------
This software performs dictionary matching in small space.  The underlying data structure is the compressed suffix tree.


INSTALLATION
----------------
1. Download and install SDSL, the Succinct Data Structures Library from https://github.com/simongog/sdsl
2. Compile the dictMatchCST.cpp file.

Compile in Linux using the following command:
g++ -O3 -DNDEBUG -funroll-loops -I${HOME}/include/ -L${HOME}/lib/ -o dictMatch dictMatchCST.cpp -lsdsl -ldivsufsort -ldivsufsort64

Run in Linux using the following command:
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:path_to_libdivsufsort_lib_directory ./dictMatch pats.txt text.txt 


The file CST_dt.cpp specifies the data structure for the CST and implementations of compressed suffix array and compressed LCP arrays to use as components.


INPUT FILES
----------------
Dictionary of new-line separated patterns
Text to search for patterns

NOTE: neither text nor any pattern should contain an end of line character or a #.  
These special characters are specified in the file CST_LMA.cpp and are easily modified.



AUTHORS
--------
--------
Shoshana Marcus
and 
Dina Sokol


WEB PAGE

--------
--------
http://www.sci.brooklyn.cuny.edu/~sokol/dictmatch.html


CONTACT

---------------
sokol@sci.brooklyn.cuny.edu

