
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
//#include "Page_Thread.h"


// Function Declarations -------------------------------------------------------

DWORD WINAPI Page_thread_func(LPVOID lpParam);


/*
need to create the following things:
-a function that reades the input file one row at a time and sends the data to the thred func
-a mutex for the output file
-a mutex for the virtual page table
-a mutex for the real page table
-a semaphore to count the vacent pages in the real table
*/

/*
* Logic
- go over evry line in the input file
-start a thread with this line :
	The thread needs to have:
	-check if the current data is in the real page table (the valid bit is equal to 1)
	if yes:
	check when will it finish and updte the end time in the end of usage
	updtade the output file
	if no:
	check if there is an open space in the pysychal page table:
	if yes add it to there and update the output file
	if no wait for an opening and the add it and update the output file

*/


// Variables -------------------------------------------------------------------
static HANDLE Output_File_mutex_handle = NULL;
static HANDLE num_of_threads_mutex_handle = NULL;
static HANDLE vir_pages_mutex_handle = NULL;
static HANDLE clock_mutex_handle = NULL;
static HANDLE real_pages_mutex_handle = NULL;
static HANDLE Input_File_mutex_handle = NULL;
static HANDLE vacent_pages_semaphore;
Page_def* vir_pages = NULL;
Page_def* real_pages = NULL;
int* clock = NULL;


int main(int argc, char* argv[])
{	
	float Virtual_bits = atoi(argv[2])-12;
	float physycal_bits = atoi(argv[3]) - 12;
	int* number_of_vir_pages = (int*)malloc(sizeof(int)); //pow(2.0, Virtual_bits);
	int* number_of_real_pages = (int*)malloc(sizeof(int)); //pow(2.0, physycal_bits);
	vir_pages = calloc(number_of_vir_pages, sizeof(Page_def));
	real_pages = calloc(number_of_real_pages, sizeof(Page_def));
	char* Path_to_input_file = argv[4];
	DWORD wait_code;
	int number_of_threds_working = 0;
	int current_row =0 ;
	int split_word[num_of_vars_in_row];
	int num_of_Comandes_to_do  = num_of_rows_in_a_file(Path_to_input_file);
	DWORD* p_thread_ids = calloc(num_of_Comandes_to_do, sizeof(DWORD));
	line_def* Line_buffers = calloc(num_of_Comandes_to_do, (Max_Size_of_Line * sizeof(char)));
	pass_to_thread* p_parameters_struct =  calloc(num_of_Comandes_to_do, ( sizeof(pass_to_thread)));
	clock = (int*)malloc(sizeof(int));

	if (clock == NULL || number_of_real_pages == NULL || number_of_vir_pages == NULL)
	{
		printf("Memory allocation to clock or page numbers failed in main!");
		exit(1);
	}
	*clock = -10;
	*number_of_real_pages = pow(2.0, physycal_bits);
	*number_of_vir_pages= pow(2.0, Virtual_bits);
	Line_buffers =  calloc(num_of_Comandes_to_do, sizeof(*Line_buffers));
	HANDLE* array_of_thread_pointers = calloc(num_of_Comandes_to_do, sizeof(HANDLE));

	if (vir_pages == NULL || real_pages == NULL || array_of_thread_pointers == NULL || Line_buffers == NULL || p_thread_ids == NULL || p_parameters_struct == NULL) {
		printf("Memory allocation to array of pages failed in main!");
		exit(1);
	}
	// init all the pages
	for (int i = 0; i < *number_of_real_pages; i++)
	{
		real_pages[i].valid = FALSE;
		real_pages[i].End_Time = NULL;
		real_pages[i].Frame_num = NULL;
	}
	for (int i = 0; i < *number_of_vir_pages; i++)
	{
		vir_pages[i].valid = FALSE;
		vir_pages[i].End_Time = NULL;
		vir_pages[i].Frame_num = NULL;
	}
	vacent_pages_semaphore = CreateSemaphore(
		NULL,	/* Default security attributes */
		number_of_real_pages,		/* Initial Count - all slots are empty */
		number_of_real_pages,		/* Maximum Count */
		NULL);  /* un-named */



	vir_pages_mutex_handle = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */

	real_pages_mutex_handle = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */

	clock_mutex_handle = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */

	Output_File_mutex_handle = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */

	Input_File_mutex_handle = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */

	if (Input_File_mutex_handle == NULL || Output_File_mutex_handle == NULL || vacent_pages_semaphore == NULL || clock_mutex_handle == NULL || real_pages_mutex_handle == NULL || vir_pages_mutex_handle == NULL) {
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
	

	for (int i = 0; i < num_of_Comandes_to_do; i++)
	{
		
		current_row = read_one_row(Path_to_input_file, &(Line_buffers[i]), current_row);
		p_parameters_struct[i].current_line = &Line_buffers[i];
		//p_parameters_struct[i].physycal_page = real_pages;
		//p_parameters_struct[i].virtual_page = vir_pages;
		//p_parameters_struct[i].clock = clock;
		p_parameters_struct[i].num_of_real_pages = number_of_real_pages;
		p_parameters_struct[i].num_of_vir_pages = number_of_vir_pages;


		//current_row = read_one_row(Path_to_input_file, line_buffer, current_row);
		//strcpy(Line_buffers[i], line_buffer);
		array_of_thread_pointers[i]= CreateThreadSimple(Page_thread_func, &p_parameters_struct[i], &(p_thread_ids[i]));
		//free(line_buffer);
	/* get the first token */
	}

	
	if (ReleaseMutex(Input_File_mutex_handle) == false)
	{
		const int error = GetLastError();
		printf("Error when realisng input file mutex error num: %d\n", error);
		exit(1);
	}

	//wait for the current batch of threads to finish working
	wait_code = WaitForMultipleObjects(num_of_Comandes_to_do, array_of_thread_pointers, TRUE, INFINITE);
	if (wait_code == WAIT_FAILED)
	{
		const int error = GetLastError();
		printf("Error when waiting for multiple threads, error code: %d\n", error);
		exit(1);
	}


	free(clock);
	free(p_thread_ids);
	free(array_of_thread_pointers);
	free(vir_pages);
	free(real_pages);
	free(Line_buffers);


}



//public function, the thread routine.
DWORD WINAPI Page_thread_func(LPVOID lpParam)
{
	if (NULL == lpParam)
	{
		return THREAD__CODE_NULL_PTR;
	}
	pass_to_thread* p_params;
	DWORD wait_res;
	
	
	p_params = (pass_to_thread*)lpParam;
	line_def* Line_buffer = p_params->current_line;
	printf("current thread line is num: %s\n", Line_buffer);
	int split_word[3];
	char* temp_char = strtok(Line_buffer, " ");
	split_word[0] = atoi(temp_char);


	for (int i = 1; i < num_of_vars_in_row; i++) {
		temp_char = strtok(NULL, " ");
		split_word[i] = atoi(temp_char);
	}
	int start_time = split_word[0];
	int frame_num = split_word[1] / size_of_page;
	int needed_time = split_word[2];

	

	//if (start_time >= *p_params->clock)
	if (start_time >= *clock)
	{
		// check if we can add a page to the physecal table
		wait_res = WaitForSingleObject(vacent_pages_semaphore, INFINITE); 
		if (wait_res != WAIT_OBJECT_0)
		{
			const int error = GetLastError();
			printf("Error when waiting for multiple vacent_pages_semaphore, error code: %d\n", error);
			exit(1);
		}
		printf("Thread %d: wait succeeded\n", GetCurrentThreadId());
		// check if we can edit the physcal page table
		wait_res = WaitForSingleObject(real_pages_mutex_handle, INFINITE);
		if (wait_res != WAIT_OBJECT_0)
		{
			const int error = GetLastError();
			printf("Error when waiting for multiple vacent_pages_semaphore, error code: %d\n", error);
		}
		// this is the critical zone
		// find the first free space and add the current addres to there
		for (int i = 0; i < *p_params->num_of_real_pages; i++)
		{
			if (real_pages[i].valid == FALSE)
			{
				real_pages[i].Frame_num = frame_num;
				real_pages[i].valid = 1;
				real_pages[i].End_Time = *clock + needed_time;
				break;
			}
		}

		if (ReleaseMutex(real_pages_mutex_handle) == false)
		{
			const int error = GetLastError();
			printf("Error when realisng real pages  mutex error num: %d\n", error);
			exit(1);
		}



	}

}

// ###### MAYBE USE THIS to split tha stuff######################

/*
	char* temp_char = strtok(Line_buffer, " ");
	split_word[0] = atoi(temp_char);


	for (int i = 1; i < num_of_vars_in_row; i++) {
		temp_char = strtok(NULL, " ");
		split_word[i] = atoi(temp_char);
	}

	*/