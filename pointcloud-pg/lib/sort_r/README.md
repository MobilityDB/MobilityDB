The following README.md and sort_r.h files have been imported from the [noporpoise/sort_r Github project](https://github.com/noporpoise/sort_r) in order to provide a portable reentrant version of qsort.


sort_r
======

Isaac Turner 2013  
Portable qsort_r / qsort_s  
Discussion here: http://stackoverflow.com/questions/4300896/how-portable-is-the-re-entrant-qsort-r-function-compared-to-qsort  
License: Public Domain - use as you wish, no warranty

[![Build Status](https://travis-ci.org/noporpoise/sort_r.png?branch=master)](https://travis-ci.org/noporpoise/sort_r)

About
-----

If you want to qsort() an array with a comparison operator that takes parameters
you need to use global variables to pass those parameters (not possible when
writing multithreaded code), or use qsort_r/qsort_s which are not portable
(there are separate GNU/BSD/Windows versions and they all take different arguments).

So I wrote a portable qsort_r/qsort_s called sort_r():

    void sort_r(void *base, size_t nel, size_t width,
                int (*compar)(const void *a1, const void *a2, void *aarg),
                void *arg);

`base` is the array to be sorted
`nel` is the number of elements in the array
`width` is the size in bytes of each element of the array
`compar` is the comparison function
`arg` is a pointer to be passed to the comparison function

Using sort_r
------------

Add `#include "sort_r.h"` to the top of your code. Then copy sort_r.h into your
working directory, or add -I path/to/sort_r to your compile arguments.

Build Example
-------------

Compile example code (`example.c`) with:

    make

To build using nested functions and qsort instead of qsort_r use

    make NESTED_QSORT=1

Nested functions are not permitted under ISO C, they are a GCC extension.

License
-------

Public Domain. Use as you wish. No warranty. There may be bugs.

