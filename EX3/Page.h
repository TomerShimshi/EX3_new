#pragma once
#ifndef Page
#include "HardCodedData.h"
#include <stdbool.h>
#include <Windows.h>
DWORD WINAPI Page_thread_func(LPVOID lpParam);
typedef struct
{
	int Frame_num;
	bool valid;
	int End_Time;

} Page_def;

typedef struct
{
	char line[Max_Size_of_Line];

} line_def;

typedef struct
{
	line_def* current_line;
	//Page_def* physycal_page;
	//Page_def* virtual_page;
	//int* clock;
	int* num_of_vir_pages;
	int* num_of_real_pages;

} pass_to_thread;

typedef enum
{
	THREAD__CODE_SUCCESS,
	THREAD__CODE_NULL_PTR,
} THREAD__return_code_t;

#endif