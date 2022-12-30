# University project - Parallel Grep

Implementation of a parallel version of the grep command, using MPI. <br>
This implementation exploits MPI's different workers to divide the search among the available processors.

## Compile and run
To compile, type in the terminal: <br>
mpicc grep-main.c -o grep_main <br><br>

Then, to run it, type: <br>
mpirun -np N grep_main word_to_search input_file <br><br>

- N: how many processes you want to use (max: your machine's number of processors)
- word_to_search: the word you are searching for in the text
- input_file: file to search the word into
