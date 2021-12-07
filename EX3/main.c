
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



#include "create_and_handle_processes.h"
#include "Page.h"


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
	DWORD wait_code;

	if (vir_pages == NULL || real_pages == NULL) {
		printf("Memory allocation to array of pages failed in main!");
		exit(1);
	}
	vacent_pages_semaphore = CreateSemaphore(
		NULL,	/* Default security attributes */
		number_of_real_pages,		/* Initial Count - all slots are empty */
		1,		/* Maximum Count */
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
		printf("Memory allocation to mutex and semaphores failed in main!");
		exit(1);
	}

	wait_code = WaitForSingleObject(Input_File_mutex_handle, INFINITE);
	if (WAIT_OBJECT_0 != wait_code)
	{

		const int error = GetLastError();
		printf("Error when waiting for mutex, error code: %d\n", error);
		exit(1);

	}
}
