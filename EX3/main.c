
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
BOOL write_to_file(int num_of_real_pages, BOOL need_to_empty, int frame_num, int i);


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

static HANDLE Input_File_mutex_handle = NULL;
static HANDLE DB_mutex_handle = NULL;
static HANDLE vacent_pages_semaphore;
static HANDLE increase_clock_semaphore;
Page_def* vir_pages = NULL;
Page_def* real_pages = NULL;
int* clock = NULL; // this var will the current time
int* output_file_offset = NULL;

int* importent_times = NULL; // this array wil store all the importent times
int* num_of_times = NULL;



int main(int argc, char* argv[])
{	
	float Virtual_bits = atoi(argv[2])-12;
	float physycal_bits = atoi(argv[3]) - 12;
	int* number_of_vir_pages = (int*)malloc(sizeof(int)); //pow(2.0, Virtual_bits);
	int* number_of_real_pages = (int*)malloc(sizeof(int)); //pow(2.0, physycal_bits);
	
	char* Path_to_input_file = argv[4];
	DWORD wait_code;
	int number_of_threds_working = 0;
	int current_row =0 ;
	int split_word[num_of_vars_in_row];
	int num_of_Comandes_to_do  = num_of_rows_in_a_file(Path_to_input_file);
	DWORD* p_thread_ids = calloc(num_of_Comandes_to_do, sizeof(DWORD));
	line_def* Line_buffers = calloc(num_of_Comandes_to_do, (Max_Size_of_Line * sizeof(char)));
	pass_to_thread* p_parameters_struct =  calloc(num_of_Comandes_to_do, ( sizeof(pass_to_thread)));
	importent_times =(int*) calloc((num_of_Comandes_to_do), ( sizeof(int))); // evry thtead has a start time and an end time
	clock = (int*)malloc(sizeof(int));
	output_file_offset = (int*)malloc(sizeof(int));
	num_of_times = (int*)malloc(sizeof(int));
	
	

	DWORD wait_res;

	if (clock == NULL || number_of_real_pages == NULL || number_of_vir_pages == NULL || output_file_offset ==NULL || num_of_times ==NULL)
	{
		printf("Memory allocation to clock or page numbers failed in main!");
		exit(1);
	}
	*clock = 0;
	*num_of_times = 0;
	*output_file_offset = 0;
	*number_of_real_pages = pow(2.0, physycal_bits);
	*number_of_vir_pages= pow(2.0, Virtual_bits);
	vir_pages = calloc(*number_of_vir_pages, sizeof(Page_def));
	real_pages = calloc(*number_of_real_pages, sizeof(Page_def));
	Line_buffers =  calloc(num_of_Comandes_to_do, sizeof(*Line_buffers));
	HANDLE* array_of_thread_pointers = calloc(num_of_Comandes_to_do, sizeof(HANDLE));

	if (vir_pages == NULL || real_pages == NULL || array_of_thread_pointers == NULL || Line_buffers == NULL || p_thread_ids == NULL || p_parameters_struct == NULL || importent_times == NULL) {
		const int error = GetLastError();
		printf("Memory allocation to array of pages failed in main! error num %d", error);
		exit(1);
	}

	// init the times array
	for (int i = 0; i < ( num_of_Comandes_to_do); i++)
	{
		importent_times[i] = -1;
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
		*number_of_real_pages,		/*all slots are empty */
		*number_of_real_pages,		/* Maximum Count */
		NULL);  /* un-named */
	
	

	increase_clock_semaphore=CreateSemaphore(NULL, 0, 1, NULL);

	

	Input_File_mutex_handle = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */

	DB_mutex_handle = CreateMutex(
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL);  /* un-named */ 

	if (Input_File_mutex_handle == NULL || vacent_pages_semaphore == NULL ||  DB_mutex_handle == NULL || increase_clock_semaphore == NULL) {
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
		
		//current_row = read_one_row(Path_to_input_file, &(Line_buffers[i]), current_row);
		read_one_row(Path_to_input_file, &(Line_buffers[i]), i);
		p_parameters_struct[i].current_line = &Line_buffers[i];
		
		p_parameters_struct[i].num_of_real_pages = number_of_real_pages;
		p_parameters_struct[i].num_of_vir_pages = number_of_vir_pages;


		
		array_of_thread_pointers[i]= CreateThreadSimple(Page_thread_func, &p_parameters_struct[i], &(p_thread_ids[i]));
	
	}

	
	if (ReleaseMutex(Input_File_mutex_handle) == false)
	{
		const int error = GetLastError();
		printf("Error when realisng input file mutex error num: %d\n", error);
		exit(1);
	}

	// now we need to build a timer like mechanizem
	
	while (true)
	{
		wait_res = WaitForSingleObject(increase_clock_semaphore, INFINITE);
		if (wait_res != WAIT_OBJECT_0)
		{
			const int error = GetLastError();
			printf("Error when waiting for increase clock_semaphore, error code: %d\n", error);
			exit(1);
		}

		wait_res = WaitForSingleObject(DB_mutex_handle, INFINITE);
		if (wait_res != WAIT_OBJECT_0)
		{
			const int error = GetLastError();
			printf("Error when waiting for the DB Semaphore error code: %d\n", error);
			exit(1);
		}
		




		write_to_file(*number_of_real_pages, TRUE, 0, 0); //remove finished pages
		*clock = find_neareset(importent_times, *clock, *num_of_times);
		

		// exot the critical section
		if (ReleaseMutex(DB_mutex_handle) == false) // release the DB mutex
		{
			const int error = GetLastError();
			printf("Error when realisng real pages  mutex error num: %d\n", error);
			exit(1);
		}
		if (*clock == -1)
		{
			break;
		}

	}

	

	//wait for all the threads to finish working
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
	CloseHandle(DB_mutex_handle);
	CloseHandle(vacent_pages_semaphore);
	CloseHandle(Input_File_mutex_handle);
	


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
	BOOL release_res;
	LONG previous_count;
	bool need_to_wait = false;
	char* Line_To_Write = (char*)malloc(sizeof(char) * Max_Size_of_Line);
	char temp_str[Max_Size_of_Line];
	int num_of_bits_to_write = 0;// saves the number of bits yo write to the output file

	if (Line_To_Write == NULL)
	{
		printf("error alocating memory for the output file write string");
		exit(1);

	}
	

	
	int split_word[3];

	p_params = (pass_to_thread*)lpParam;
	line_def* Line_buffer = p_params->current_line;
	printf("current thread line is num: %s\n", Line_buffer);
	
	char* temp_char = strtok(Line_buffer, " ");
	split_word[0] = atoi(temp_char);


	for (int i = 1; i < num_of_vars_in_row; i++) {
		temp_char = strtok(NULL, " ");
		split_word[i] = atoi(temp_char);
	}
	int start_time = split_word[0];
	int frame_num = split_word[1] / size_of_page;
	int needed_time = split_word[2];

	wait_res = WaitForSingleObject(DB_mutex_handle, INFINITE);
	if (wait_res != WAIT_OBJECT_0)
	{
		const int error = GetLastError();
		printf("Error when waiting for the DB Semaphore error code: %d\n", error);
		exit(1);
	}

	// now we are in the safe zone we can add times to the time array
	*num_of_times = *num_of_times + 1;
	add_member(importent_times, start_time,*num_of_times);
	

	// after we added to the importent times array we can leave safe zone and release the mutex
	if (ReleaseMutex(DB_mutex_handle) == false) // release the DB mutex
	{
		const int error = GetLastError();
		printf("Error when realisng real pages  mutex error num: %d\n", error);
		exit(1);
	}

	while (start_time > *clock)
	{
		// wait for procces time
	}







	//this part is just for us to update the page table
	if (start_time <= *clock)
	{
		// first check if the corrent addres is already in the vir page table
		wait_res = WaitForSingleObject(DB_mutex_handle, INFINITE);
		if (wait_res != WAIT_OBJECT_0)
		{
			const int error = GetLastError();
			printf("Error when waiting for the DB Semaphore error code: %d\n", error);
			exit(1);
		}
		// now we can check the DB to see if we need to add the curreent page to the tabels
		if (vir_pages[frame_num].valid == TRUE)// meaning the current page is already in the DB
		{
			
			vir_pages[frame_num].End_Time = needed_time+ *clock; // update the end time
			real_pages[vir_pages[frame_num].Frame_num].End_Time = vir_pages[frame_num].End_Time; // update the end time in the real page as well

			find_neareset(importent_times, *clock, *num_of_times); // the clock is the closest to itself so it will remove it 
			add_member(importent_times, vir_pages[frame_num].End_Time, *num_of_times);
			printf("updated fram num: %d  to be %d \n", frame_num, vir_pages[frame_num].End_Time);
		}
		else // the page is not in the physycal page table
			// maybe add here clear the vir table
		{
			need_to_wait = true;
			write_to_file(*p_params->num_of_real_pages,TRUE,0,0);
							
			
		}
		// exit the critical section

		if (ReleaseMutex(DB_mutex_handle) == false) // release the DB mutex
		{
			const int error = GetLastError();
			printf("Error when realisng real pages  mutex error num: %d\n", error);
			exit(1);
		}


		if (need_to_wait== TRUE) // meaning the current addres is not in the page table
		{
			// check if we can add a page to the physecal table

			wait_res = WaitForSingleObject(vacent_pages_semaphore, INFINITE);
			if (wait_res != WAIT_OBJECT_0)
			{
				const int error = GetLastError();
				printf("Error when waiting for multiple vacent_pages_semaphore, error code: %d\n", error);
				exit(1);
			}
			
			//printf("*semaphore_count =: %d\n", *semaphore_count);
			//printf(" fram num: %d  acquired the semaphore my line starts with  %d \n", frame_num, start_time);
			// now need to check if can update all the data tabels
			wait_res = WaitForSingleObject(DB_mutex_handle, INFINITE);
			if (wait_res != WAIT_OBJECT_0)
			{
				const int error = GetLastError();
				printf("Error when waiting for the DB Semaphore error code: %d\n", error);
				exit(1);
			}
			// find the first free space and add the current addres to there
			int i = 0;
			for (i = 0; i < *p_params->num_of_real_pages; i++)
			{
				if (real_pages[i].valid == FALSE)
				{
					real_pages[i].Frame_num = frame_num;
					real_pages[i].valid = TRUE;
					real_pages[i].End_Time = *clock + needed_time;
					break;
				}
			}

			// now update the vir page table
			vir_pages[frame_num].Frame_num = i;
			vir_pages[frame_num].valid = TRUE;
			vir_pages[frame_num].End_Time = *clock + needed_time;

			// now we need to write it to the output file
			
			write_to_file(*p_params->num_of_real_pages, FALSE, frame_num, i);
			// finished the insertion of a new page now we release the mutex

			release_res = ReleaseSemaphore(
				increase_clock_semaphore,
				1, 		// Signal that exactly one cell was emptied
				&previous_count);
			if (release_res == FALSE) {
				const int error = GetLastError();
				printf("Error when realisng semaphore  mutex error num: %d\n", error);
				exit(1);
			}

			if (ReleaseMutex(DB_mutex_handle) == false) // release the DB mutex
			{
				const int error = GetLastError();
				printf("Error when realisng real pages  mutex error num: %d\n", error);
				exit(1);
			}







		}



	
	}
	printf(" line %d finished it therad   \n", start_time);

}


BOOL write_to_file(int num_of_real_pages, BOOL need_to_empty, int frame_num, int i )
{
	int j;
	LONG previous_count;
	BOOL release_res;
	char* Line_To_Write = (char*)malloc(sizeof(char) * Max_Size_of_Line);
	char temp_str[Max_Size_of_Line];
	if (Line_To_Write == NULL)
	{
		printf("error alocating memory for the output file write string");
		exit(1);

	}
	int things_to_write[2];// enter the waned ints to here
	int num_of_bits_to_write = 0;// saves the number of bits yo write to the output file
	bool foud_page_to_empty = FALSE;
	
	if (need_to_empty == TRUE)
	{
		for (j = 0;j < num_of_real_pages; j++)
		{
			if (*clock > real_pages[j].End_Time && real_pages[j].valid == TRUE)// meaning the current page finished its time
			{// we need to remove it from the table and updte the output file
				vir_pages[real_pages[j].Frame_num].valid = FALSE;
				real_pages[j].valid = FALSE;
				release_res = ReleaseSemaphore(
					vacent_pages_semaphore,
					1, 		// Signal that exactly one cell was emptied
					&previous_count);
				if (release_res == FALSE) {
					const int error = GetLastError();
					printf("Error when realisng semaphore  mutex error num: %d\n", error);
					exit(1);
				}
				
				//printf("*semaphore_count =: %d\n", *semaphore_count);
				// now we need to write it to the output file:
				things_to_write[0] = j;
				things_to_write[1] = real_pages[j].Frame_num;
				foud_page_to_empty = TRUE;
			
			}
		}
	}
	else
	{
		things_to_write[0] = i;
		things_to_write[1] = frame_num;
		
		
	}
	if (foud_page_to_empty == TRUE || need_to_empty ==FALSE)
	{ 

	num_of_bits_to_write = get_num_of_digits_in_an_int_number(*clock);
	sprintf(Line_To_Write, "%d", *clock);
	strcat(Line_To_Write, " ");

	sprintf(temp_str, "%d", things_to_write[1]);
	num_of_bits_to_write += get_num_of_digits_in_an_int_number(things_to_write[0]);


	strcat(Line_To_Write, temp_str);
	strcat(Line_To_Write, " ");
	sprintf(temp_str, "%d", things_to_write[0]);
	num_of_bits_to_write += get_num_of_digits_in_an_int_number(things_to_write[1]);
	strcat(Line_To_Write, temp_str);
	strcat(Line_To_Write, " ");
	if (need_to_empty == TRUE)
	{
		strcat(Line_To_Write, "E");
	}
	else
	{
		strcat(Line_To_Write, "P");
	}
	
	num_of_bits_to_write += 4;// added the spaces and char to the sum
	printf("wrote to output: %s\n", Line_To_Write);
	WinWriteToFile(Output_file_path, Line_To_Write, num_of_bits_to_write, *output_file_offset);
	*output_file_offset += num_of_bits_to_write;
	// start new line
	WinWriteToFile(Output_file_path, "\r\n", 4, *output_file_offset);
	*output_file_offset += 2;
	//return TRUE;
	}
	return TRUE;
}