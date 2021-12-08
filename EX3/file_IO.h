//includes functions related to file handling

#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>
#include "create_and_handle_processes.h"
#include "Page.h"
#include "HardCodedData.h"
#include "Page_Thread.h"


int WinReadFromFile(char* pathToFile, char* buffer_for_bytes_read, int num_of_bytes_to_read, int read_from_offset);

int WinWriteToFile(char* pathToFile, char* stringToWrite, int StringLen, int write_from_offset);

int num_of_rows_in_a_file(char* pathToFile);

int return_numbers_of_chars_in_file(char * pathToFile);

int read_one_row(char* pathToFile, line_def* Line_To_Insert, int wanted_line);
//int read_one_row(char* pathToFile, char* Line_To_Insert, int wanted_line);

#endif

