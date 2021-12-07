
/*
Authors:

Amit Tzah 316062959
Tomer Shimshi 203200480

Project: Ex2


*/


#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <Windows.h>
#include <direct.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>



#include "create_and_handle_processes.h"
#include "Page.h"
#include "HardCodedData.h"


/*
need to create the following things:
-a function that reades the input file one row at a time and sends the data to the thred func
-a mutex for the output file
-a mutex for the virtual page table
-a mutex for the real page table
-a semaphore to count the vacent pages in the real table


*/

static HANDLE Output_File_mutex_handle = NULL;
static HANDLE Input_File_mutex_handle = NULL;
static HANDLE vacent_pages_semaphore;

int main(int argc, char* argv[])
{	
	float Virtual_bits = atoi(argv[2])-12;
	float physycal_bits = atoi(argv[3]) - 12;
	int number_of_vir_pages = pow(2.0, Virtual_bits);
	int number_of_real_pages = pow(2.0, physycal_bits);
	Page_def* vir_pages = calloc(number_of_vir_pages, sizeof(Page_def));
	Page_def* real_pages = calloc(number_of_real_pages, sizeof(Page_def));
	char* Path_to_input_file = argv[4];
	DWORD wait_code;
	int number_of_threds_working = 0;
	int current_row =0 ;
	int split_word[num_of_vars_in_row];
	
	char* Line_buffer= (char*)malloc(Max_Size_of_Line * sizeof(char));

	if (vir_pages == NULL || real_pages == NULL) {
		printf("Memory allocation to array of pages failed in main!");
		exit(1);
	}
	vacent_pages_semaphore = CreateSemaphore(
		NULL,	/* Default security attributes */
		number_of_real_pages,		/* Initial Count - all slots are empty */
		number_of_real_pages,		/* Maximum Count */
		NULL);  /* un-named */

	Output_File_mutex_handle = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */

	Input_File_mutex_handle = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */

	if (Input_File_mutex_handle == NULL || Output_File_mutex_handle == NULL || vacent_pages_semaphore == NULL) {
		const int error = GetLastError();
		printf("Memory allocation to mutex and semaphores failed in main! the error is %d\n", error);
		exit(1);
	}

	wait_code = WaitForSingleObject(Input_File_mutex_handle, INFINITE);
	if (WAIT_OBJECT_0 != wait_code)
	{

		const int error = GetLastError();
		printf("Error when waiting for mutex, error code: %d\n", error);
		exit(1);

	}
	/*
	* Critical Section
	* We can now safely access the shared resource.
	*/
	current_row = read_one_row(Path_to_input_file, Line_buffer, current_row);
	/* get the first token */
	char* temp_char = strtok(Line_buffer, " ");
	split_word[0] = atoi(temp_char);

	for (int i = 1; i < num_of_vars_in_row; i++) {
		temp_char = strtok(NULL, " ");
		split_word[i] = atoi(temp_char);
	}
	

	ReleaseMutex(Input_File_mutex_handle);

}
