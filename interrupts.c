#include "interrupts.h"

void initMemory(MemoryPartition partitions[])
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
void read_input_data(const char *filename, PCB process[], int *process_counter)
{
    FILE *file = fopen(filename, "r");
    if (file)
    {

        while (fscanf(file, "%d, %u, %u, %u, %u, %u\n",
                      &process[*process_counter].PID,
                      &process[*process_counter].memory_size,
                      &process[*process_counter].arrival_time,
                      &process[*process_counter].CPU_time,
                      &process[*process_counter].IO_frequency,
                      &process[*process_counter].IO_duration) == 6)
        {
            process[*process_counter].remaining_CPU_time = process[0].CPU_time;
            process[*process_counter].partition_number = FREE;
            process[*process_counter].state = NEW;
            (*process_counter)++;
        }
        fclose(file);
    }
    else
    {
        perror("Failed to load input file.\n");
    }
}
void printMemory(MemoryPartition partitions[])
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
void printProcess(PCB *process, int *process_counter)
{
    // Print the process

    printf("process Table:\n");
    printf("PID     | Size (MB) | Arrival Time | Total CPU Time | I/O Freq | I/O Duration | Partition Number | State\n");
    printf("-----------------------------------------------------------------------------------------------------------\n");
    for (unsigned int i = 0; i < *process_counter; i++)
    {
        printf("%7u | %7u | %12u | %10u | %10u | %10u | %10d | %10d\n",
               process[i].PID,
               process[i].memory_size,
               process[i].arrival_time,
               process[i].CPU_time,
               process[i].IO_frequency,
               process[i].IO_duration,
               process[i].partition_number,
               process[i].state);
    }
}
// allocate memory to process
int allocate_memory(PCB *process, MemoryPartition *partition)
{
    for (int i = 0; i < TOTAL_PARTITIONS - 1; i++)
    {
        if (process->memory_size <= partition[i].size)
        {
            if (process->memory_size >= partition[i+1].size && partition[i].occupiedBy == FREE)
            {
                partition[i].occupiedBy = process->PID;
                process->partition_number = i;
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
// Function to log state transitions
void log_transition(FILE *file, int time, int pid, const char *old_state, const char *new_state) {
    fprintf(file, "| %-17d | %-3d | %-9s | %-10s |\n", time, pid, old_state, new_state);
}
void create_execution_file(PCB *process){
    FILE *file = fopen("execution.txt", "w");

    // Print header
    fprintf(file, "+------------------------------------------------+\n");
    fprintf(file, "|Time of Transition | PID | Old State | New State |\n");
    fprintf(file, "+------------------------------------------------+\n");

    //log 
    fprintf(file, "| %-17d | %-3d | %-9d | %-10d |\n", process->arrival_time, process->PID, process->state, process->state+1);
    

    // Print footer
    fprintf(file, "+------------------------------------------------+\n");

    fclose(file);
}
// FCFS Scheduler
void fcfs_scheduler(PCB processes[], int *process_count, const char* output_file) {
    FILE* file = fopen(output_file, "w");
    if (!file) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "Time of Transition | PID | Old State | New State |\n");
    fprintf(file, "+------------------------------------------------+\n");

    int current_time = 0;

    for (int i = 0; i < *process_count; i++) {
        PCB* process = &processes[i];

        // Transition NEW -> READY       
        fprintf(file, "| %16d | %3d |       NEW |     READY |\n", current_time, process->PID);
        // strcpy(process->state, "READY");
        process->state = READY;


        // Transition READY -> RUNNING
        fprintf(file, "| %16d | %3d |     READY |   RUNNING |\n", current_time, process->PID);
        // strcpy(process->state, "RUNNING");
        process->state = RUNNING;

        while (process->remaining_CPU_time > 0) {
            // Simulate I/O if required
            if (process->IO_frequency > 0 && process->remaining_CPU_time > process->IO_frequency) {
                // Transition RUNNING -> WAITING
                current_time += process->IO_frequency;
                fprintf(file, "| %16d | %3d |   RUNNING |   WAITING |\n", current_time, process->PID);
                // strcpy(process->state, "WAITING");
                process->state = WAITING;
                process->remaining_CPU_time -= process->IO_frequency;

                // Transition WAITING -> READY
                current_time+= process->IO_duration;
                fprintf(file, "| %16d | %3d |   WAITING |     READY |\n", current_time, process->PID);
                // strcpy(process->state, "READY");
                process->state = READY;
                // current_time+= process->IO_duration;

                // Transition READY -> RUNNING
                fprintf(file, "| %16d | %3d |     READY |   RUNNING |\n", current_time, process->PID);
                process->state= RUNNING;
            }

            // Process runs for 1ms
            if(process->remaining_CPU_time < process->IO_frequency){
                process->remaining_CPU_time--;
                current_time++;
            }
            
        }

        // Transition RUNNING -> TERMINATE
        fprintf(file, "| %16d | %3d |   RUNNING | TERMINATE |\n", current_time, process->PID);
        // strcpy(process->state, "TERMINATE");
        process->state = TERMINATE;
    }

    fclose(file);
}
int main()
{
    MemoryPartition partitions[TOTAL_PARTITIONS];
    PCB processess[MAX_PROCESSESS];
    // initMemory(partitions);
    // printMemory(partitions);
    // first come first serve
    int process_count = 0;
    read_input_data("input_data.txt", processess, &process_count);
    fcfs_scheduler(processess,&process_count,"execution.txt");
    // printProcess(process, &process_count);
    return 1;
}