/**
 * Implementation of a Scheduler Simulato for SYSC 4001 Assignment 3
 * Dec 5th, 2024
 * Jordan Grewal, 101287828
 * Hadi Srour, 101200666
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

typedef struct
{
    unsigned int partitionNumber; // PID
    unsigned int size;            // Size of the partition
    int occupiedBy;               // PID of the occupied
} MemoryPartition;

typedef struct
{
    int PID;
    unsigned int CPU_time;               // Total CPU Time
    unsigned int IO_duration;            // Time to run I/O
    unsigned int IO_frequency;           // Process to perform I/O
    unsigned int remaining_CPU_time;     // How much time left in CPU for process
    unsigned int remaining_IO_duration;  // Time left in I/O
    unsigned int remaining_IO_frequency; // Time left for process to perfrom I/O
    unsigned int memory_size;            // Memory occupied
    unsigned int arrival_time;           // Arrival time
    int partition_number;                // To determine the occupied memory
} PCB;

typedef struct
{
    PCB pcb; //
    unsigned int arrival_time;

} PCBandArrival;

typedef struct
{
    int pid;            // process id
    int endTime;        // when process end
    int arrivalTime;    // when process was added to ready queue
    int turnaroundTime; // process turnaround time
    int waitingTime;    // Totol time the process waited in the ready queue
    int burstTime;
    double meanWaitTime;

} processMatrics;

#endif