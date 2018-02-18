PROCESS ANALYZER
A menu-based process analysis tool for GNU/Linux
Written by Colin G. Powell
powelcol@oregonstate.edu

NOTE: Process Analyzer requires at least glibc 2.15 and Linux 3.2.
      Tested on Antergos Linux, kernel 4.15.3.1-ARCH.
ALSO NOTE: Process analyzer must be run as root ('sudo') to access all features.

To compile, enter "make" from the command line.
To run, enter "sudo ./proc-analyzer"

Menu Options:
[1]: View a list of running processes:
     -  Provides a list of PIDs, names, and paths for all 
        currently-running processes.
[2]: View a list of threads for a particular process:
     -  Supply a PID, program returns all threads associated with that PID.
[3]: View a list of shared objects for a particular process:
     -  Supply a PID, program returns a list of all shared objects loaded
        for that PID.
[4]: View a list of executable pages for a particular process: 
     -  Supply a PID, program returns a list of all executable pages 
        loaded into process virtual memory. List of pages includes 
        physical-memory page frame number of each virtual page. 
[5]: View process virtual memory:
     -  Supply a PID and a starting address; if the starting address
        is currently mapped in virtual memory, program will display up 
        to 4096 bytes of memory starting with the supplied address. If 
        the supplied address is not currently mapped, program returns
        an error message to the user. 
     -  User may continue viewing memory until the end of the currently
        mapped region is reached, or they choose to stop viewing memory
        by entering 'n' (for "no") when prompted.
[q]: Quit program.      
