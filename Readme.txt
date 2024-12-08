#########################################################
#	    ____  _________    ____     __  _________	#
#	   / __ \/ ____/   |  / __ \   /  |/  / ____/	#
#	  / /_/ / __/ / /| | / / / /  / /|_/ / __/   	#
#	 / _, _/ /___/ ___ |/ /_/ /  / /  / / /___   	#
#	/_/ |_/_____/_/  |_/_____/  /_/  /_/_____/   	#
							
#########################################################                                       

Assignment 3 SYSC 4001

Authors:
- [Jordan Grewal]
- [Hadi Srour]

----------------------------------------------------------

Input format for FCFS scheduling algorithim with IO

----------------------------------------------------------
The input format is as follows: ./sim input_data_1.txt FCFS 
same goes for input_data_n input files.

The input file data is as such:
	---------------------------------------------------------------------------------------
	|  PID  | Memory Size |   ArivalTime  |  CPU Total Time |  ioFrequency  |  ioDuration  | 
	---------------------------------------------------------------------------------------
each line represent these following PCB paramters.

- PID: Process ID
- Memory Size: Memory size of the process
- Arrival Time: Time at which the process arrives
- CPU Total Time: Total CPU time required by the process
- ioFrequency: Frequency of IO operations
- ioDuration: Duration of each IO operation


    NOTE for input file:
        -The paramters of each line should not exccede more than 6
        - Memory size of Process should not be more than 100
        - All the parameter values should be positive integer 


---------------------------------------------------------------------------------------
Example: 
    15, 10, 0, 25, 11, 3

---------------------------------------------------------------------------------------
How to compile 

gcc -g interrupts.c -o sim
./sim input_data_1.txt FCFS
./sim input_data_2.txt FCFS

After the simulation is performed this will be the result:
---------------------------------------------------------------------------------------
            OUTPUT HERE: 


content of execution.txt file will be as such:
    +--------------------------------------------------+
    |Time of Transition | PID | Old State | New State |
    +--------------------------------------------------+
    | 0                 | 15  | NEW       | READY      |
    | 0                 | 15  | READY     | RUNNING    |
    | 11                | 15  | RUNNING   | WAITING    |
    | 14                | 15  | WAITING   | READY      |
    | 14                | 15  | READY     | RUNNING    |
    | 25                | 15  | RUNNING   | WAITING    |
    | 28                | 15  | WAITING   | READY      |
    | 28                | 15  | READY     | RUNNING    |
    | 31                | 15  | RUNNING   | TERMINATE  |
    +--------------------------------------------------+

content of memory_status.txt file will be:

    +--------------------------------------------------------------------------------------------------+
    | Time of Event | Memory Used | Partitions State          | Total Free Memory | Usable Free Memory |
    +--------------------------------------------------------------------------------------------------+
    | 0             | 0           | -1, -1, -1, -1, -1, -1    | 100                | 100                |
    | 0             | 10          | -1, -1, -1, 15, -1, -1    | 90                 | 90                 |
    | 31            | 0           | -1, -1, -1, -1, -1, -1    | 100                | 100                |
    +--------------------------------------------------------------------------------------------------+



---------------------------------------------------------------------------------------

        NOTE:
        - The memory is divided into 6 partitions, each of size 40, 25, 15, 10, 8, 2 units.
        - The memory is initially empty.
        - The memory is managed using a simple first-fit allocation strategy.
        - The memory is deallocated when a process terminates.
        - The memory usage is tracked at each event (process arrival, process termination, etc.).
        - The memory usage is reported in the memory_status.txt file




