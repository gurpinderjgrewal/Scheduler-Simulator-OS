/**
 * Implementation of a Scheduler Simulato for SYSC 4001 Assignment 3
 * Dec 5th, 2024
 * Jordan Grewal, 101287828
 * Hadi Srour, 101200666
 */
#include "interrupts__101287828_101200666.h"
// Global variables
long int simTime = 0;
PCB readyQue[MAX_PROCESSESS];
int readyQueSize = 0;
PCB waitingQue[MAX_PROCESSESS];
int waitingQueSize = 0;
int processTerminated = 0;
int number_of_process;

char result[256];
int values[] = {-1, -1, -1, -1, -1, -1};

// For memory
int total_free_memory = TOTAL_MEMORY;
int useable_free_memory = TOTAL_MEMORY;
int memory_usd = 0;
FILE *file_mem;

PCB running;
// PCB processess[MAX_PROCESSESS];
PCBandArrival newArr[MAX_PROCESSESS];

MemoryPartition partitions[TOTAL_PARTITIONS];

processMatrics metrics[MAX_PROCESSESS];
int index_of_pid = 0;

// initialize functions here
void initMemory()
{
    // Initialize the memory
    unsigned int size[TOTAL_PARTITIONS] = {40, 25, 15, 10, 8, 2};
    for (unsigned int i = 0; i < TOTAL_PARTITIONS; i++)
    {
        partitions[i].partitionNumber = i + 1;
        partitions[i].size = size[i];
        partitions[i].occupiedBy = FREE;
    }
}
void initMetrics()
{
    for (int i = 0; i < number_of_process; i++)
    {
        metrics[i].pid = newArr[i].pcb.PID;
        metrics[i].arrivalTime = 0;
        metrics[i].endTime = 0;
        metrics[i].turnaroundTime = 0;
        metrics[i].waitingTime = 0;
        metrics[i].burstTime = 0;
        metrics[i].meanWaitTime = 0.0;
    }
}

// Read input file and load into newArr array of PCB structure
// return number of process in input file
int read_input_data(const char *filename)
{
    int process_counter = 0;
    FILE *file = fopen(filename, "r");
    if (file)
    {

        while (fscanf(file, "%d, %u, %u, %u, %u, %u\n",
                      &newArr[process_counter].pcb.PID,
                      &newArr[process_counter].pcb.memory_size,
                      &newArr[process_counter].pcb.arrival_time,
                      &newArr[process_counter].pcb.CPU_time,
                      &newArr[process_counter].pcb.IO_frequency,
                      &newArr[process_counter].pcb.IO_duration) == 6)
        {
            newArr[process_counter].pcb.remaining_CPU_time = newArr[0].pcb.CPU_time;
            newArr[process_counter].pcb.remaining_IO_frequency = newArr[0].pcb.IO_frequency;
            newArr[process_counter].pcb.remaining_IO_duration = newArr[0].pcb.IO_duration;
            newArr[process_counter].pcb.partition_number = FREE;

            process_counter++;
        }
        fclose(file);
    }
    else
    {
        perror("Failed to load input file.\n");
    }

    return process_counter;
}

// This function is used to convert array of integer values to string
void arrayToString(int array[], int length, char *output, int outputSize)
{
    int offset = 0;
    for (int i = 0; i < length; i++)
    {
        // Append the current value to the output string
        offset += snprintf(output + offset, outputSize - offset, "%d", array[i]);

        // Add a comma if it's not the last element
        if (i < length - 1)
        {
            offset += snprintf(output + offset, outputSize - offset, ", ");
        }

        // Prevent buffer overflow
        if (offset >= outputSize)
            break;
    }
}

void printMemory()
{
    // Print the memory

    printf("Partition Table:\n");
    printf("Partition Number | Size (MB) | Occupied By (PID)\n");
    printf("-------------------------------------------------\n");
    for (unsigned int i = 0; i < TOTAL_PARTITIONS; i++)
    {
        printf("%16u | %9u | %15d\n",
               partitions[i].partitionNumber,
               partitions[i].size,
               partitions[i].occupiedBy);
    }
}
// Function to print process with its PCB values
void printProcess()
{
    // Print the process

    printf("process Table:\n");
    printf("PID     | Size (MB) | Arrival Time | Total CPU Time | I/O Freq  | I/O Duration | Partition Number\n");
    printf("-------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < number_of_process; i++)
    {
        printf("%7u | %8u | %12u | %15u | %10u | %13u | %16d\n",
               newArr[i].pcb.PID,
               newArr[i].pcb.memory_size,
               newArr[i].pcb.arrival_time,
               newArr[i].pcb.CPU_time,
               newArr[i].pcb.IO_frequency,
               newArr[i].pcb.IO_duration,
               newArr[i].pcb.partition_number);
        /* code */
    }
}

// find function to return the index of process
int findMetric(int id)
{
    for (int i = 0; i < number_of_process; i++)
    {
        if (metrics[i].pid == id)
        {
            return i;
        }
    }
    exit(0);
}
// Function to find index at which the partition is occupied
int findInPartition(int id)
{
    for (int i = 0; i < TOTAL_PARTITIONS; i++)
    {
        if (partitions[i].occupiedBy == id)
        {
            return i;
        }
    }
    exit(0);
}

// allocate memory to process
int allocate_memory(PCB *process)
{
    int bestIndex = -1;
    int minSizeDifference = TOTAL_PARTITIONS + 1;
    // Case 1:
    for (int i = 0; i < TOTAL_PARTITIONS; i++)
    {
        if (partitions[i].occupiedBy == FREE && partitions[i].size >= process->memory_size)
        {
            int sizeDifference = partitions[i].size - process->memory_size;

            if (sizeDifference <= minSizeDifference)
            {
                minSizeDifference = sizeDifference;
                bestIndex = i;
            }
        }
    }
    // Case 2:
    if (bestIndex == -1)
    {
        for (int i = 0; i < TOTAL_PARTITIONS; i++)
        {
            if (partitions[i].occupiedBy == FREE && partitions[i].size >= process->memory_size)
            {
                int sizeDifference = partitions[i].size - process->memory_size;

                if (sizeDifference >= minSizeDifference)
                {
                    minSizeDifference = sizeDifference;
                    bestIndex = i;
                }
            }
        }
    }
    // Case 3:
    if (bestIndex == -1)
    {
        // printf("Cant allocate memory");
        return 0;
    }

    partitions[bestIndex].occupiedBy = process->PID;
    process->partition_number = bestIndex + 1;
    memory_usd += process->memory_size;
    total_free_memory -= process->memory_size;
    useable_free_memory -= partitions[bestIndex].size;
    return bestIndex;
}
// free memory from process
int deallocate_memory(PCB *process)
{
    int index = findInPartition(process->PID);
    if (index >= 0 && index < TOTAL_PARTITIONS)
    {
        if (partitions[index].occupiedBy != FREE)
        {

            partitions[index].occupiedBy = FREE;
            process->partition_number = -1;
            memory_usd -= process->memory_size;
            total_free_memory += process->memory_size;
            useable_free_memory += partitions[index].size;
            return index;
        }
    }
    return -1;
}
void printMetrics()
{
    printf("------------------------------------------------------------------------------------------\n");
    printf("PID  | Arrival Time | End Time | Turnaround Time | Wait Time | Burst Time | Mean Wait Time\n");
    printf("------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < number_of_process; i++)
    {
        printf("%-4d | %-12d | %-8d | %-15d | %-10d | %-10d | %-14.2lf\n",
               metrics[i].pid,
               metrics[i].arrivalTime,
               metrics[i].endTime,
               metrics[i].turnaroundTime,
               metrics[i].waitingTime,
               metrics[i].burstTime,
               metrics[i].meanWaitTime);
        /* code */
    }
    printf("------------------------------------------------------------------------------------------\n");
}

// Function to log state transitions
void log_transition(FILE *file, int time, int pid, const char *old_state, const char *new_state)
{
    fprintf(file, "| %-17d | %-3d | %-9s | %-10s |\n", time, pid, old_state, new_state);
}
// Function to log memory transitions
void log_memory(FILE *file, int time, int memory_used, char *result, int total_free_memory, int usable_free_memory)
{
    fprintf(file_mem, "| %-13d | %-11d | %-25s | %-18d | %-18d |\n",
            time, memory_used, result, total_free_memory, usable_free_memory);
    if (memory_usd == 0)
    {
        fprintf(file_mem, "+--------------------------------------------------------------------------------------------------+\n");

        // Close the file_mem
        if (number_of_process < 0)
        {
            fclose(file_mem);
        }
    }
}
void printStateChange(int id, char msg[])
{
    printf("%ldms ID-%d: %s\n", simTime, id, msg);
}

// FCFS simulation
void simulation(PCBandArrival processes[], int counter, FILE *file)
{
    PCBandArrival temp[MAX_PROCESSESS];
    memcpy(temp, processes, MAX_PROCESSESS * sizeof(PCBandArrival));
    int cpu_busy = 0; // cpu free
    int process_count = counter;
    int arrivalSize = 0;
    int t = 0;
    while (processTerminated < counter)
    {
        if (arrivalSize == 0)
        {
            for (int i = 0; i < process_count; i++)
            {

                if (temp[i].pcb.arrival_time <= simTime)
                {
                    readyQue[readyQueSize] = temp[i].pcb;

                    // log transition change
                    // printStateChange(readyQue[readyQueSize].PID, "NEW      ---> READY");
                    log_transition(file, simTime, readyQue[readyQueSize].PID, "NEW", "READY");

                    // case 1:
                    if (i < process_count - 1)
                    {
                        for (int a = i; a < process_count - 1; a++)
                        {
                            temp[a] = temp[a + 1];
                        }

                        process_count--;
                        i--;
                    }
                    // case 2: i is at end
                    else
                    {
                        process_count--;
                        if (process_count == 0)
                        {
                            arrivalSize = 1;
                        }
                    }

                    t = findMetric(readyQue[readyQueSize].PID);
                    // log memory change
                    int bestIndex = allocate_memory(&processes[t].pcb);
                    values[bestIndex] = processes[t].pcb.PID;
                    arrayToString(values, TOTAL_PARTITIONS, result, sizeof(result));

                    log_memory(file_mem, simTime, memory_usd, result, total_free_memory, useable_free_memory);

                    // metrics log
                    metrics[t].arrivalTime = simTime;
                    readyQueSize++;
                }
            }
        }

        if (cpu_busy == 0) // if cpu is free
        {
            if (readyQueSize > 0)
            {
                running = readyQue[0];

                for (int i = 0; i < readyQueSize - 1; i++) // if there is more than 1 process in ready que
                {
                    readyQue[i] = readyQue[i + 1];
                }
                readyQueSize--;
                // log transition change
                // printStateChange(running.PID, "READY      ---> RUNNING");
                log_transition(file, simTime, running.PID, "READY", "RUNNING");

                cpu_busy = 1;
            }
        }
        simTime++;
        // if cpu is busy
        if (cpu_busy)
        {
            running.remaining_CPU_time--;
            running.remaining_IO_frequency--;
            // check if process is done
            if (running.remaining_CPU_time <= 0)
            {
                // update metric
                index_of_pid = findMetric(running.PID);
                metrics[index_of_pid].burstTime++;
                metrics[index_of_pid].endTime = simTime;
                metrics[index_of_pid].turnaroundTime = simTime - metrics[index_of_pid].arrivalTime;
                metrics[index_of_pid].meanWaitTime = metrics[index_of_pid].waitingTime / metrics[index_of_pid].burstTime;

                // log transition change
                // printStateChange(running.PID, "RUNNING      ---> TERMINATE");
                log_transition(file, simTime, running.PID, "RUNNING", "TERMINATE");
                t = findMetric(running.PID);

                // log memory transition
                int bestIndex = deallocate_memory(&processes[t].pcb);
                values[bestIndex] = partitions[bestIndex].occupiedBy;
                arrayToString(values, TOTAL_PARTITIONS, result, sizeof(result));
                log_memory(file_mem, simTime, memory_usd, result, total_free_memory, useable_free_memory);

                // Free CPU and increment finished process
                cpu_busy = 0;
                processTerminated++;
            }
            // if process is not done, it will go to IO
            else if (running.remaining_IO_frequency <= 0)
            {
                // update metric
                index_of_pid = findMetric(running.PID);
                metrics[index_of_pid].burstTime++;

                // log transition change
                // printStateChange(running.PID, "RUNNING      ---> WAITING");
                log_transition(file, simTime, running.PID, "RUNNING", "WAITING");
                running.remaining_IO_frequency = running.IO_frequency;
                waitingQue[waitingQueSize] = running;
                waitingQueSize++;
                cpu_busy = 0;
            }
        }
        // process to do IO
        for (int i = 0; i < waitingQueSize; i++)
        {

            // check if it is done with IO
            if (waitingQue[i].remaining_IO_duration == 0)
            {
                waitingQue[i].remaining_IO_duration = waitingQue[i].IO_duration;
                readyQue[readyQueSize] = waitingQue[i];
                readyQueSize++;
                // printStateChange(waitingQue[i].PID, "WAITING      ---> READY");
                log_transition(file, simTime, waitingQue[i].PID, "WAITING", "READY");

                // case 1: i is at start or middle
                if (i < waitingQueSize - 1)
                {
                    for (int a = i; a < waitingQueSize - 1; a++)
                    {
                        waitingQue[a] = waitingQue[a + 1];
                    }
                    waitingQueSize--;
                }
                // case 2: i is at end
                else
                {
                    waitingQueSize--;
                }
            }
            waitingQue[i].remaining_IO_duration--;
        }
        // for process waiting in ready queue
        for (int i = 0; i < readyQueSize; i++)
        {
            index_of_pid = findMetric(readyQue[i].PID);
            metrics[index_of_pid].waitingTime++;
        }
    }
}

// Function to create memory_partition.txt file
void create_mem_partition_file()
{
    file_mem = fopen("memory_status_101287828_101200666.txt", "w");
    // Print table header to the file
    fprintf(file_mem, "+--------------------------------------------------------------------------------------------------+\n");
    fprintf(file_mem, "| Time of Event | Memory Used | Partitions State          | Total Free Memory | Usable Free Memory |\n");
    fprintf(file_mem, "+--------------------------------------------------------------------------------------------------+\n");

    fprintf(file_mem, "| %-13d | %-11d | %-25s | %-18d | %-18d |\n",
            0, 0, result, total_free_memory, useable_free_memory);
}

// Function to create execution.txt file
void create_execution_file(const char *filename, PCBandArrival processes[], int counter)
{

    FILE *file = fopen(filename, "w");

    // Print header
    fprintf(file, "+--------------------------------------------------+\n");
    fprintf(file, "|Time of Transition | PID | Old State | New State |\n");
    fprintf(file, "+--------------------------------------------------+\n");

    // Run simulation and log each transition
    simulation(processes, counter, file);

    // Print footer
    fprintf(file, "+--------------------------------------------------+\n");

    fclose(file);
}

// function to print final stats in terminal
void printFinalStat(double wait_time, double tnt_time)
{
    for (int i = 0; i < number_of_process; i++)
    {
        wait_time += metrics[i].waitingTime;
        tnt_time += metrics[i].turnaroundTime;
    }
    wait_time = wait_time / number_of_process;
    tnt_time = tnt_time / number_of_process;

    printf("\n Average Process Wait Time: %0.2lfms\n Average Turnaround Time: %0.2lfms\n Average Throughput: %0.2lf\n ",
           wait_time, tnt_time, (100 / tnt_time));
}

int main(int argc, char *argv[])
{
    int no_process_left = 0;
    int cpu_busy = 0;
    double avrg_wait_time = 0.0;
    double avrg_tnt_time = 0.0;
    const char *filename = argv[1];
    const char *scheduler = argv[2];

    srand(time(NULL));
    if (argc < 3)
    {
        // check for incorrect usage of argv
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // const char *filename = argv[1];
    int number1 = -1;

    // Validate scheduler parameter
    if (strcmp(scheduler, "FCFS") != 0 && strcmp(scheduler, "RR") != 0 && strcmp(scheduler, "SJF") != 0)
    {
        fprintf(stderr, "Invalid scheduler type. Supported schedulers are: FCFS, RR, SJF\n");
        return 1;
    }
    // extracts the number from provided filename in argv
    if (strncmp(filename, "input_data_", 11) == 0 && strcmp(filename + strlen(filename) - 4, ".txt") == 0)
    {
        number1 = atoi(filename + 11); // stores number from filename as integer
    }

    if (number1 == -1)
    {
        // checks for incorrect filename format
        fprintf(stderr, "Invalid filename format. Expected format: trace<number>.txt\n");
        return 1;
    }

  

    PCBandArrival *temp = newArr;
    initMemory();
   
    // first come first serve
    arrayToString(values, TOTAL_PARTITIONS, result, sizeof(result));
    // load all process from .txt file into newArr array
    number_of_process = read_input_data(filename);
    initMetrics();

    if (strcmp(scheduler, "FCFS") == 0)
    {
        create_mem_partition_file();
        create_execution_file("execution_101287828_101200666.txt", newArr, number_of_process);
        printf("+-----------------------------------------------------------------+\n");
        printf("\n------------------- Printing of Metrics ---------------------------\n");
        printMetrics();
        printf("\n------------------------ Final Stats -----------------------------\n");
        printFinalStat(avrg_wait_time, avrg_tnt_time);
    }

    return 0;
}