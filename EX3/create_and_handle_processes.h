//includes functions related to the creation and handling of processes/threads.

#ifndef CREATE_AND_HANDLE_PROCESSES_H
#define CREATE_AND_HANDLE_PROCESSES_H

#include <windows.h>

DWORD CreateProcessSimpleMain(LPSTR command_line_arguments_to_run, int time_out_in_ms);

HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine,	LPVOID p_thread_parameters, LPDWORD p_thread_id);
void close_array_of_thread_handles(HANDLE* array_of_thread_pointers, int size_of_array);

char* concatenate_command_line_arguments_into_a_string(int num_of_arguments, char* arguments_array[]);

#endif