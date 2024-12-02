/**
 * Interrupt simulator header file for SYSC 4001 Assignment 2
 * November 1st, 2024
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#define TOTAL_PARTITIONS 6
#define FREE -1 // define free space 
#define MAX_PROCESSESS 100
enum ProcessState{
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATE

};
typedef struct {
    unsigned int partitionNumber; // PID
    unsigned int size; //Size of the partition
    int occupiedBy; //PID of the occupied
} MemoryPartition;

typedef struct {
    char program_name[20];
    unsigned int size;
} ExternalFile;

typedef struct {
    int PID;
    unsigned int CPU_time;
    unsigned int IO_duration;
    unsigned int IO_frequency;
    unsigned int remaining_CPU_time;  
    unsigned int memory_size;
    unsigned int arrival_time;
    int partition_number; 
    enum ProcessState state;
} PCB;




/**
 * Function to simulate a fork() system call
 * 
 */
void forkSyscall(int parent_pid, int dur);

/**
 * Function to simulate exec syscall
 */
void execSyscall(int pid, const char* filename, int dur);

/**
 * Logs system status after fork or exec syscalls.
 * Stores information in system_status.txt
 */
void logSystemStatus();

/**
 * Function to setup pcb tables, memory partitions, external files, etc.
 * Runs at start of program
 */
void initalizeSystem();

/**
 * This function reads the vector_table.txt file, storing it's contents
 * in the global array vectorTable.
 */
void read_vector_table();

/**
 * This function sets up the output file, overwriting any content
 * at the start of the program, ensuring that only new information is stored
 * in a "fresh" file.
 */
void setupOutputFile();

/**
 * This function is just a random number generator for use throughout the program
 * generates random number from 1-3
 * 
 * @return random number from 1 to 3
 */
int rand_num();

/**
 * This function stores the output/simulation text
 * into the execution.txt file.
 * Uses the outputText and outputCounter global variables.
 * resets outputText and outputCounter when complete.
 */
void store_output();

/**
 * This function procedurally "prints" out each line of execution into an array of strings
 * (outputText). if the outputText array is full, it calls store_output() to store the contents
 * of outputText. increases the timer based on duration of each call.
 * 
 * @param duration the duration of the call
 * @param message the message/description of the call
 */
void printer(int duration, char* message);

/**
 * This function is the ISR routine for SYSCALLs.
 * It takes in the information provided by the trace file,
 * "calling" the appropriate SYSCALL with information from the vectorTable.
 * 
 * @param num the position of the vector table provided by the trace file.
 * @param dur the duration of the event provided by the trace file
 */
void SYSCALL_ISR(int num, int dur, int type, char* program);

/**
 * This function is the ISR routine for END_IO.
 * It works the same as SYSCALL_ISR(), but with appropriate activities
 * for the END_IO routine.
 * 
 * @param num the position of the vector table provided by the trace file.
 * @param dur the duration of the event provided by the trace file
 */
void ENDIO_ISR(int num, int dur);

/**
 * This function parses information taken from the trace file, 
 * calling ISR routines if needed, and printing other lines directly using the 
 * printer() function. 
 * 
 * @param p1 The Message of an event from trace file
 * @param p2 If present, the position of a ISR Routine from trace file
 * @param p3 The duration of an event from trace file
 */
void executeLine(char *p1, char *p2, char *p3);

/**
 * main loop of program
 * Opens and reads information from the trace file.
 * Goes line by line, parsing information and calling executeLine() to execute each line as
 * they are read.
 * 
 * @param fileName The file name of the trace text file.
 */
void executionLoop(char *fileName);

#endif