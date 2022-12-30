#ifndef _GREP_H_
#define _GREP_H_

#define LINELENGTH 81

// #include <vector>
// #include <string>
// #include <utility>

//typedef std::pair<unsigned, std::string>  number_and_line;
typedef struct {
  int num;
  char* string;
} number_and_line;

/* Only process with rank 0 should read from the file, 
  * other processes must get their lines from rank 0 
  */
// void get_lines(std::vector<std::string> &input_string, 
//                 const std::string &file_name);
void get_lines(number_and_line* input_strings, char* file_name, int* local_lines_number, number_and_line* local_lines);

/* Differently from the example seen at lecture, the first input to this
  * function is a vector containing the portion of file that must be searched
  * by each process
  */
// void search_string(const std::vector<std::string> & input_strings,
//                     const std::string & search_string,
//                     lines_found &lines, unsigned &local_lines_number);
void search_string(number_and_line* input_strings, char* search_string, int local_lines_number, number_and_line* found_lines,int* found_elems);

/* Prints (preferrably to file) must be performed by rank 0 only, it is
  * fine to hard-code the file path for the result in this function */
// void print_result(const lines_found & lines, unsigned local_lines_number);
void print_result(number_and_line* lines, int found_elems);

#endif // GREP_H