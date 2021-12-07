//includes functions related to file handling

#define _CRT_SECURE_NO_WARNINGS

#include "file_IO.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <Windows.h>


//based on the example from :
/*
https://riptutorial.com/winapi/example/5736/create-a-file-and-write-to-it

*/

/// <summary>
/// 
/// <Function to read from a given file with given offset>
/// <param name="pathToFile"></path to file to be read >
/// <param name="buffer_for_bytes_read"></the string buffer to which the bytes are read. Should have space for  Null terminator>
/// <param name="num_of_bytes_to_read"></Number of bytes that will be read from offset>
/// <param name="read_from_offset"></offset from which to start reading>
/// <returns></returns the number of bytes that were read>
/// 


int WinReadFromFile(char* pathToFile, char* buffer_for_bytes_read, int num_of_bytes_to_read, int read_from_offset  )
{
	

	OVERLAPPED offset_bytes;
	DWORD num_of_bytes_read;
	LPDWORD  num_of_bytes_read_pointer = &num_of_bytes_read;

	offset_bytes.Offset = read_from_offset;
	offset_bytes.OffsetHigh = 0;
	offset_bytes.hEvent = 0;



	// Open a handle to the file
	HANDLE hFile = CreateFileA(

		pathToFile,    //(LPCTSTR)(pathToFile)
		GENERIC_READ,        // Desired access
		NULL,
		NULL, // Security attributes
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		// Share mode
		NULL
	);                  


	if (hFile == INVALID_HANDLE_VALUE)
	{
		const int error = GetLastError();
		printf("Error opening file %s!. Error Code: %d\n", pathToFile, error);
		exit(1);
	}

	int res;
	res = ReadFile(hFile, buffer_for_bytes_read, num_of_bytes_to_read, num_of_bytes_read_pointer, &offset_bytes);

	buffer_for_bytes_read[num_of_bytes_to_read] = '\0'; //null terminate the array of bytes read, to get a string.

	if (FALSE == res)
	{
		const int error = GetLastError();
		printf("Error reading file!. Error Code: %d\n", error);
		exit(1);
	}
	CloseHandle(hFile);

	return num_of_bytes_read;
}


//write to a file using the win API lib
//based on the example from :
/* 
https://riptutorial.com/winapi/example/5736/create-a-file-and-write-to-it 

*/ 

	/// <summary>
	/// 
	/// </Generic write to file from the given offset. StringToWrite is written to pathToFile>
	/// <param name="pathToFile"></Path of the file to be written into.>
	/// <param name="stringToWrite"></Null terminated>
	/// <param name="StringLen"></Length of string to be written>
	/// <param name="write_from_offset"></Offset from which to start overwriting>
	/// <returns></returns the number of bytes that were written.>
	
	int WinWriteToFile(char* pathToFile, char* stringToWrite, int StringLen, int write_from_offset)
{

		OVERLAPPED offset_bytes;

		offset_bytes.Offset = write_from_offset;
		offset_bytes.OffsetHigh = 0;
		offset_bytes.hEvent = 0;

		// Open a handle to the file
	HANDLE hFile = CreateFileA(
		pathToFile,   //(LPCTSTR)(pathToFile)
		GENERIC_WRITE,        // Desired access
		NULL,
		NULL, // Security attributes
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);


	if (hFile == INVALID_HANDLE_VALUE)
	{
		const int error = GetLastError();
		printf("WinWriteToFile Error opening file %s!. Error Code: %d\n", pathToFile, error);
		exit(1);
	}


	DWORD bytesWritten;
	int res;

	res= WriteFile(
		hFile,            // Handle to the file
		stringToWrite,  // Buffer to write
		StringLen,   // Buffer size
		&bytesWritten,    // Bytes written
		&offset_bytes);         // Overlapped

	if (FALSE == res)
	{
		const int error = GetLastError();
		printf("Error reading file!. Error Code: %d\n", error);
		exit(1);
	}

	 // Close the handle once we don't need it.
	CloseHandle(hFile);

	return bytesWritten;
}

	

	

	// Based on:
	/*
	https://www.geeksforgeeks.org/c-program-to-count-the-number-of-characters-in-a-file/
	https://www.educative.io/edpresso/what-is-fopens-in-c
	*/


	/// <summary>
	/// 
	/// </returns number of characters in file>
	/// <param name="pathToFile"></a path to file location>
	/// <returns></number of characters in file>
	
	int return_numbers_of_chars_in_file(char* pathToFile) {


		FILE* fp;

		// Declaring a variable of type errno_t to store the return value of fopen_s
		errno_t error_code;
	
		//Opening file in r mode
		error_code = fopen_s(&fp, pathToFile , "r");

		if (error_code != 0) {
			printf("Error! Failed to open file in r mode!\n");
			exit(1);
		}
		else{
		// Character counter (result)
		int count = 0;

		// To store a character read from file
		char c=0;

		// Extract characters from file
		// and store in character c
		for (c = getc(fp); c != EOF; c = getc(fp))

			// Increment count for this character
			count = count + 1;

		// Close the file
		fclose(fp);

		return count;
		}

}

	/*
	 Returns the num of rows in a given text file.
	based on https://stackoverflow.com/questions/12733105/c-function-that-counts-lines-in-file
	*/

	int num_of_rows_in_a_file(char* pathToFile)
	{
		FILE* fp = fopen(pathToFile, "r");
		if (fp == NULL)
		{
			printf("Error opening file to get the number of rows!");
			exit(1);
		}
		int count = 0;
		int ch = 0;
		while (!feof(fp))
		{
			ch = fgetc(fp);
			if (ch == '\n')
			{
				count++;
			}
		}
		fclose(fp);
		return count;
	}

	


