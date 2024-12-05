/**
 * Implementation of a Scheduler Simulato for SYSC 4001 Assignment 3
 * Dec 5th, 2024
 * Jordan Grewal, 101287828
 * Hadi Srour, 101200666
 */
#include "interrupts.h"
// Global variables
long int simTime = 0;
PCB readyQue[MAX_PROCESSESS];
int readyQueSize = 0;
PCB waitingQue[MAX_PROCESSESS];
int waitingQueSize = 0;
int processTerminated = 0;
int number_of_process;
// int newArr = 1;
PCB running;
// PCB processess[MAX_PROCESSESS];
PCBandArrival newArr[MAX_PROCESSESS];
PCBandArrival memory_update[MAX_PROCESSESS];
int memory_update_size=0;

MemoryPartition partitions[TOTAL_PARTITIONS];

processMatrics metrics[MAX_PROCESSESS];
int index_of_pid = 0;

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
void printProcess(PCBandArrival process[], int *process_counter)
{
    // Print the process

    printf("process Table:\n");
    printf("PID     | Size (MB) | Arrival Time | Total CPU Time | I/O Freq  | I/O Duration | Partition Number\n");
    printf("-------------------------------------------------------------------------------------------------\n");
    for (unsigned int i = 0; i < *process_counter; i++)
    {
        printf("%7u | %8u | %12u | %15u | %10u | %13u | %16d\n",
               process[i].pcb.PID,
               process[i].pcb.memory_size,
               process[i].pcb.arrival_time,
               process[i].pcb.CPU_time,
               process[i].pcb.IO_frequency,
               process[i].pcb.IO_duration,
               process[i].pcb.partition_number);
    }
}
// allocate memory to process
int allocate_memory(PCBandArrival *process)
{
    for (int i = 0; i < TOTAL_PARTITIONS - 1; i++)
    {
        if (process->pcb.memory_size <= partitions[i].size)
        {
            if (process->pcb.memory_size >= partitions[i + 1].size && partitions[i].occupiedBy == FREE)
            {
                partitions[i].occupiedBy = process->pcb.PID;
                process->pcb.partition_number = i+1;
                return 1;
            }
        }
    }
    return 0;
}
// free memory from process
int deallocate_memory(PCB *process)
{

    return 0;
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

        /* code */
    }
}

int findMetric(int id)
{
    for (int i = 0; i < number_of_process; i++)
    {
        if (metrics[i].pid == id)
        {
            return i;
        }
        /* code */
    }
    exit(0);
}
// Function to log state transitions
void log_transition(FILE *file, int time, int pid, const char *old_state, const char *new_state)
{
    fprintf(file, "| %-17d | %-3d | %-9s | %-10s |\n", time, pid, old_state, new_state);
}

void printStateChange(int id, char msg[])
{
    printf("%ldms ID-%d: %s\n", simTime, id, msg);
}
void simulation(PCBandArrival processes[], int counter, FILE *file)
{

    _Bool cpu_busy = 0; // cpu free
    int process_count = counter;
    int arrivalSize = 0;


    while (processTerminated < counter)
    {
        if (arrivalSize == 0)
        {
            for (int i = 0; i < process_count; i++)
            {

                if (newArr[i].pcb.arrival_time <= simTime)
                {
                    readyQue[readyQueSize] = processes[i].pcb;

                    printStateChange(readyQue[readyQueSize].PID, "NEW      ---> READY");
                    log_transition(file, simTime, readyQue[readyQueSize].PID, "NEW", "READY");
                    
                    // allocate_memory(&processes[i]);
                    // printProcess(processes,&number_of_process);
                    if (i < process_count - 1)
                    {
                        for (int a = i; a < process_count - 1; a++)
                        {
                            newArr[a] = newArr[a + 1];
                        }
                        // dec size of newArr, dec i so as we check the new value we shifted into i
                        process_count--;
                        i--;
                    }
                    // case 2: i is at end
                    else
                    {
                        process_count--;
                        // check if empty
                        if (process_count == 0)
                        {
                            arrivalSize = 1;
                        }
                    }
                    index_of_pid = findMetric(readyQue[readyQueSize].PID);
                    metrics[index_of_pid].arrivalTime = simTime;
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
                printStateChange(running.PID, "READY      ---> RUNNING");
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

                // log
                printStateChange(running.PID, "RUNNING      ---> TERMINATE");
                log_transition(file, simTime, running.PID, "RUNNING", "TERMINATE");
                

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

                printStateChange(running.PID, "RUNNING      ---> WAITING");
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
            //  printf("IO Remaining: %u\n",waitingQue[i].remaining_IO_duration);
            // check if it is done with IO
            if (waitingQue[i].remaining_IO_duration == 0)
            {
                waitingQue[i].remaining_IO_duration = waitingQue[i].IO_duration;
                readyQue[readyQueSize] = waitingQue[i];
                readyQueSize++;
                printStateChange(waitingQue[i].PID, "WAITING      ---> READY");
                log_transition(file, simTime, waitingQue[i].PID, "WAITING", "READY");
                // case 1: i is at start or middle
                if (i < waitingQueSize - 1)
                {
                    for (int a = i; a < waitingQueSize - 1; a++)
                    {
                        waitingQue[a] = waitingQue[a + 1];
                    }
                    // dec size of newArr, dec i so as we check the new value we shifted into i
                    waitingQueSize--;
                }
                // case 2: i is at end
                else
                {
                    waitingQueSize--;
                }
                /* code */
            }
            waitingQue[i].remaining_IO_duration--;
        }
        // for process waiting in ready queue
        for (int i = 0; i < readyQueSize; i++)
        {
            index_of_pid = findMetric(readyQue[i].PID);
            metrics[index_of_pid].waitingTime++;
            /* code */
        }
    }
}
void create_mem_partition_file(){
    FILE *file = fopen("memory_partition.txt", "w");
     // Print table header to the file
    fprintf(file, "+------------------------------------------------------------------------------------------+\n");
    fprintf(file, "| Time of Event | Memory Used | Partitions State  | Total Free Memory | Usable Free Memory |\n");
    fprintf(file, "+------------------------------------------------------------------------------------------+\n");

    // Print table footer to the file
    fprintf(file, "+------------------------------------------------------------------------------------------+\n");

    // Close the file
    fclose(file);

    printf("Data successfully written to memory_events.txt.\n");

}
void create_execution_file(PCBandArrival processes[], int counter)
{
    FILE *file = fopen("execution.txt", "w");

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
void printFinalStat(double wait_time, double tnt_time){
    for (int i = 0; i < number_of_process; i++)
    {  
        wait_time+= metrics[i].waitingTime;
        tnt_time+= metrics[i].turnaroundTime;

        /* code */
    }
    wait_time = wait_time/number_of_process;
    tnt_time = tnt_time/number_of_process;

    printf("\n Average Process Wait Time: %0.2lfms\n Average Turnaround Time: %0.2lfms\n Average Throughput: %0.2lf\n ",
    wait_time,tnt_time,(100/tnt_time));
    
}
int main()
{
    _Bool no_process_left = 0;
    _Bool cpu_busy = 0;
    double avrg_wait_time=0.0;
    double avrg_tnt_time = 0.0;
    // PCBandArrival *temp = newArr;
    initMemory();
    printMemory();
    // first come first serve

    // load all process from .txt file into newArr array
    number_of_process = read_input_data("input_data_2.txt");
    initMetrics();
    printProcess(newArr, &number_of_process);
    printf("+-----------------------------------------------------------------+\n");
    printf("------------------- Start of simulation ---------------------------\n");
    printf("+-----------------------------------------------------------------+\n");
    create_execution_file(newArr, number_of_process);
    printf("+-----------------------------------------------------------------+\n");
    printf("--------------------- Simulation Over -----------------------------\n");
    // printProcess(newArr, &number_of_process);
    printf("+-----------------------------------------------------------------+\n");
    printf("\n------------------- Printing of Metrics ---------------------------\n");
    printMetrics();
    printf("\n------------------------ Final Stats -----------------------------\n");
    printFinalStat(avrg_wait_time,avrg_tnt_time);

  
    
    return 1;
}