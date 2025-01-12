# FIGSEARCH

2nd college assignment,
A program that scans a 2d array of "pixels" from a text file and has functions that find the largest vertical line, horizontal line or the largest square and prints their coordinates to the console.
Written for linux only (windows text files have \n\r instead of \n),

Note: the correct format for the file is first 2 numbers are the size of the "image" the rest of them are either 1 (full pixel) or 0 (empty pixel) and every number is split from the others by one whitespace character,

## Compilation:
To compile this project run this command,

```
$ gcc -std=c11 -Wall -Wextra -Werror figsearch.c -o figsearch
```

## How to run:
To run this project you can use all of these commands,

```
$ ./figsearch --help
```
--help: prints out all commands implemented, prints out "Valid" or "Invalid" depending on the result
```
$ ./figsearch test *FILEPATH*
```
test: tests if a given file has the proper formatting
```
$ ./figsearch hline *FILEPATH*
```
hline: finds the first longest horizontal line in the file and prints out its start and end coords
```
$ ./figsearch vline *FILEPATH*
```
vline: finds the first longest vertical line in the file and prints out its start and end coords
```
$ ./figsearch square *FILEPATH*
```
square: finds the first largest square in the file and prints out its start and end coords
