/*
 * Colin Powell
 * 18/2/15
 * Process Analyzer: main 
 */

#include "helpers.h"

#define _LARGEFILE64_SOURCE     //enable 64-bit seeking for pagemap

#include <inttypes.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define MAX_NAME 1024
#define MAX_MAPS 2048
#define MAX_PATH 1024

typedef struct {
    char name[MAX_NAME];
    char cmdline[MAX_NAME];
    int pid;
} p_info;


typedef struct {
    char name[MAX_NAME]; 
    int tgid;
    int tid;
} t_info;

typedef struct {
    // start, end, perms, name
    unsigned long vm_start;
    unsigned long vm_end;
    char perms[8];
    char path[MAX_NAME];
} map_reg;

typedef struct {

    


} page_info;

/* USER INPUT */

void print_main_menu() ;
char get_user_choice(char *choices);
int get_numeric_input();

/* PROCESS FUNCTIONS */

int get_proc_info( char *path, p_info *pi);
int get_thread_info(char *path, t_info *ti);
int get_mapped_regions(int pid, map_reg *map);

int is_shared_obj(char *path);
int is_executable(map_reg *map);
int read_bit(unsigned long long tgt, int bnum);

int list_processes();
int list_threads(int pid);
void list_shared_objects(int pid);

int list_executable_pages(int pid);
void print_pageinfo(unsigned long addr, unsigned long long pmap_entry, char *path); 

/*
 * main(): driver for process analyzer functions
 */

int main(int argc, char **argv) {

    char user_choice;
    char choices[] = "12345q";
    int quit = FALSE;
    int pid = 0;

    printf("*****WELCOME TO PROCESS ANALYZER*****\n");

    while(!quit) {

        print_main_menu();
        user_choice = get_user_choice(choices);

        switch (user_choice){

            case '1':   list_processes();
                        break;

            case '2':   pid = get_numeric_input(); 
                        list_threads(pid);
                        break;

            case '3':   pid = get_numeric_input();
                        list_shared_objects(pid);
                        break;

            case '4':   pid = get_numeric_input();
                        list_executable_pages(pid);
                        break;

            case '5':   break;

            default:    quit = TRUE; 
        }

    }

    return 0;

}

/*
 * list_processes(): enumerates all processes.
 * -- returns TRUE if no read errors occured, FALSE otherwise.
 */

int list_processes() {

    int success = FALSE;
    p_info pi;

    struct dirent *entry = NULL;
    struct stat dir_stat;

    char temp[MAX_PATH];
    const char name[] = "/proc";
    
    DIR *dptr = opendir(name);
    if(dptr == NULL) {
        fprintf(stderr, "ERROR: could not open %s directory", name);
        return FALSE;
    }

    printf("----PROCESSES----\n");
    printf("%-8s%-32s%-64s\n", "PID", "NAME", "CMDLINE");
    
    while((entry = readdir(dptr)) != NULL) {

        // stat file
        stat(entry->d_name, &dir_stat);

        if(is_number(entry->d_name) && S_ISDIR(dir_stat.st_mode)){

            snprintf(temp, sizeof(temp), "%s/%s", name, entry->d_name);

            pi.pid = atoi(entry->d_name);
            success = get_proc_info(temp, &pi);
            printf("%-8d%-32s%-64s\n", pi.pid, pi.name, pi.cmdline);
        } 
    } 

    closedir(dptr);
    dptr = NULL;

    return success;

}

/*
 * list_threads(): output information about all threads associated with a particular pid
 */

int list_threads(int pid) {

    t_info ti;

    int success = FALSE;
    
    struct dirent *entry = NULL;
    struct stat dir_stat;

    char path[MAX_PATH] = {'\0'};
    sprintf(path, "/proc/%d/task", pid);

    char tmp[MAX_PATH] = {'\0'};

    DIR *dptr = opendir(path);
    if(dptr == NULL) {
        fprintf(stderr, "ERROR: could not open %s. Did you enter a valid PID?\n", path);
        return FALSE;
    }

    printf("----THREADS FOR PID: %d----\n", pid);
    printf("%-8s%-8s%-64s\n", "TID", "TGID", "NAME");
    while((entry = readdir(dptr)) != NULL) {
        stat(entry->d_name, &dir_stat);

        if(is_number(entry->d_name) && S_ISDIR(dir_stat.st_mode)) {
            snprintf(tmp, sizeof(tmp), "%s/%s", path, entry->d_name);

            ti.tid = atoi(entry->d_name);
            success = get_thread_info(tmp, &ti);
            printf("%-8d%-8d%-64s\n", ti.tid, ti.tgid, ti.name);
        }
    }

    return success;
}

/*
 * get_mapped_regions(): retrive information on all mapped virtual memory regions
 * in the process passed as an argument. 
 * Returns number of mapped regions read from /proc/pid/maps
 * NOTE: use of sscanf function to read hex memory values adapted from 
 */

int get_mapped_regions(int pid, map_reg *map) {
   
    char path[MAX_PATH] = {'\0'};
    snprintf(path, sizeof(path), "/proc/%d/maps", pid);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int i = 0;  // map array counter 
    int n;      // num. of values read on sscanf call

    FILE *fp = fopen(path, "r"); 
    if(fp == NULL) {
        fprintf(stderr, "ERROR: could not open %s. Did you enter a valid PID?\n", path);
        return FALSE;
    }
    while( (read = (getline(&line, &len, fp)) != -1)) {
        // process map entry
        // FORMAT: address perms offset dev inode pathname --> ignore offset dev inode
        
        n = sscanf(line, "%lX-%lX %s %*s %*s %*d %s", &map[i].vm_start, &map[i].vm_end, map[i].perms, map[i].path);
/*        
        if(n < 4 && map[i].path) {
            fprintf(stderr, "ERROR: could not read mapped region in %s\n", path);
        }
*/
        i++;
    }
/*
    printf("----ALL MAPPED REGIONS----\n");
    for(k = 0; k < i; k++) {
        printf("%lX -- %lX %s\n", map[k].vm_start, map[k].vm_end, map[k].path);
    }
*/
    free(line);
    fclose(fp);
    return i;

}

/*
 * list_shared_objects(): outputs a list of shared objects 
 */

void list_shared_objects(int pid) {

    map_reg maps[MAX_MAPS]; // list of all mapped regions
    memset(maps, '\0', sizeof(maps));

    // grab all mapped regions (will filter below)
    int n_maps = get_mapped_regions(pid, maps);

    char sh_objs[n_maps][MAX_PATH];
    memset(sh_objs, '\0', sizeof(sh_objs));

    int k;
    int j = 0;
    int n_shared = 0;   // num of shared objects

    // extract unique .so files from mapped regions
    for(k = 0; k < n_maps; k++) {
        j = 0;
        // if this path is not already in sh_objs, stop iterating
        while(j < n_shared) {
            if(strcmp(maps[k].path, sh_objs[j]) == 0) {
                break;
            }
            j++;
        }

        // if no duplicate and a .so file, place in sh_objs directory
        if(j >= n_shared && is_shared_obj(maps[k].path) ){ 
            strcpy(sh_objs[j], maps[k].path);
            n_shared++;
        }
    }

    printf("----SHARED OBJECTS----\n");
    for(k = 0; k < n_shared; k++) {
        printf("%s\n", sh_objs[k]);
    }

}

/*
 * is_shared_obj(): returns TRUE if string passed as an argument matches shared object file naming 
 * convention, and FALSE otherwise.
 * CONVENTION: starts with "lib", contains ".so" file extension, possibly followed by version number.
 */

int is_shared_obj(char *path) {

    int is_so = FALSE;
    
    // find filename in path (by finding last / char)
    if(strstr(path, ".so") != 0) {
        is_so = TRUE;
    }

    return is_so;
}

/*
 * list_executable_pages(): list information for all executable pages in a process
 * NOTE: useof 64-bit types and logic for determining offset adapted from:
 * eqware.net/Articles/CapturingProcessMemoryUsageUnderLinux/page-collect.c 
 */

int list_executable_pages(int pid) {

    int i = 0;  
    int j = 0;  // counters

    map_reg maps[MAX_MAPS];
    memset(maps, '\0', sizeof(maps));
    int n_maps = get_mapped_regions(pid, maps);

    char path[MAX_PATH] = {'\0'};
    snprintf(path, sizeof(path), "/proc/%d/pagemap", pid);
    
    unsigned long long pmap_entry;
    unsigned long addr;
    int num_pages;
    off64_t offset;
    ssize_t sz;

    // get index of pagemap entry from pos. in virtual memory
    // will be located at 64bits * num of pages to get to vm_start
    long idx;
  
    FILE *fp = fopen(path, "r");
    int fnum = fileno(fp);

    if(fp == NULL) {
        fprintf(stderr, "ERROR: unable to open file %s. Did you enter a valid PID?\n", path);    
        return FALSE;
    } 
            
    printf("%-24s%-16s%-16s%-64s\n", "VIRT. ADDRESS", "PAGEFRAME NO.", "EXCL. MAPPED", "PATH");

    for(i = 0; i < n_maps; i++) {
   
        // if mapped region is executable, read page entries for that mapped region 
        if(is_executable(&maps[i])) {

            // number of pages is (end addr - start addr) / size of a page
            num_pages = (maps[i].vm_end - maps[i].vm_start) / sysconf(_SC_PAGE_SIZE);
            addr = maps[i].vm_start;

            // starting index into pagemap
            idx = maps[i].vm_start / sysconf(_SC_PAGE_SIZE) * sizeof(unsigned long long);
            while( num_pages > 0) {
           
                offset = lseek64(fnum, idx, SEEK_SET);

                sz = read(fnum, &pmap_entry, sizeof(pmap_entry));
                if( sz < 0 ) {
                    fprintf(stderr, "ERROR: could not read pagemap entry at offset %llu", (unsigned long long)offset);
                }
    
                // if present (bit 63 set)
                if(read_bit(pmap_entry, 63))
                print_pageinfo(addr, pmap_entry, maps[i].path);
            
                // add 64 bits to index
                idx += sizeof(unsigned long long);

                addr += sysconf(_SC_PAGE_SIZE); 
                num_pages--;
            }
        }
    }    

    fclose(fp);
    return TRUE;
}

/*
 * is_executable(): returns TRUE if mapped region is executable, FALSE if not.
 */

int is_executable(map_reg *map) {

    if(strchr(map->perms, 'x') != NULL) return TRUE;
    return FALSE;

}

/*
 * read_bit(): reads the value of a specific bit, returns it
 * adapted from: stackoverflow.com/questions/
 *               2249731/how-do-i-get-bit-by-bit-data-from-an-integer-value-in-c
 */

int read_bit(unsigned long long tgt, int bnum) {

    // apply an AND mask to the target:
    //  - shift left bnum times to mask off bnum-th bit
    // then, shift right bnum times to get the bit-value we want

    // ex:      1100 1001 <-- we want bit 6
    // MASK:    0100 0000
    // RES:     0100 0000
    // SHR:     0000 0001

    return (tgt & ( 1 << bnum )) >> bnum;

}

/*
 * print_pageinfo(): prints content of a specific pagemap entry
 */

void print_pageinfo(unsigned long addr, unsigned long long pmap_entry, char *path) {

    // mask off all but last 
    unsigned long long pfn = pmap_entry & 0x7FFFFFFFFFFFFF;
    
    // print 16 hex chars, as long-long (64-bit) entry
    printf("%-24lX", addr);
    printf("%-16llu", pfn);
    printf("%-16d", read_bit(pmap_entry, 56));
    printf("%-64s", path);
    printf("\n");
    
/*
    printf("pagemap entry: %016llX\n", pmap_entry);
    printf("pfn: %llu\n", pmap_entry & 0x7FFFFFFFFFFFFF);
    printf("present: %d\n", read_bit(pmap_entry, 63));
    printf("swapped: %d\n", read_bit(pmap_entry, 62));
    printf("file page: %d\n", read_bit(pmap_entry, 61));
    printf("soft-dirty: %d\n", read_bit(pmap_entry, 55));
*/

}



/*
 * get_proc_info(): gets information about the process at the specified path, returns in a struct p_info.
 * NOTE: file reading logic adapted from getline() man pages, linux.die.net/man/3/getline
 */

int get_proc_info(char *path, p_info *pi) {

    char info_path[MAX_PATH] = {'\0'};
    strcpy(info_path, path);
    strcat(info_path, "/cmdline");

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // read command line info

    FILE *fp = fopen(info_path, "r");
    if(fp == NULL) {
        fprintf(stderr, "ERROR: could not open file %s\n", info_path);
        return FALSE;
    }
  
    // read command 
    while( (read = getline(&line, &len, fp)) != -1) {
        strcpy(pi->cmdline, line);
    }
    fclose(fp);

    strcpy(info_path, path);
    strcat(info_path, "/status");

    // open /proc/pid/status status for reading
    fp = fopen(info_path, "r");

    if(fp == NULL) {
        fprintf(stderr, "ERROR: could not open file %s\n", info_path);
        return FALSE;
    } 

    while( (read = getline(&line, &len, fp)) != -1) {

        // if we have matched name field
        if(strncmp("Name:", line, strlen("Name:")) == 0) {
            strcpy(pi->name, get_field_val(line)); 
            break;
        }
    }
    
    fclose(fp);

    // free read line per getline() specs
    free(line);
    return TRUE;

}

/*
 * get_thread_info(): reads /proc/[pid]/task/[tid], returns information about 
 * a process thread in a t_info struct
 * NOTE: file reading logic adapted from getline() man pages, linux.die.net/man/3/getline
 */

int get_thread_info(char *path, t_info *ti) {

    char info_path[MAX_PATH] = {'\0'};
    strcpy(info_path, path);
    strcat(info_path, "/status");

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE *fp = fopen(info_path, "r");
    if(fp == NULL) {
        fprintf(stderr, "ERROR: could not open file %s\n", info_path);
        return FALSE;
    }

    while( (read = getline(&line, &len, fp)) != -1) {
    
        // if we have matched name field
        if(strncmp("Name:", line, strlen("Name:")) == 0) {
            strcpy(ti->name, get_field_val(line));
        }

        // if we have matched thread group id field
        if(strncmp("Tgid:", line, strlen("Tgid:")) == 0) {
            ti->tgid =  atoi(get_field_val(line));
        }
    }

    fclose(fp);
    free(line);

    return TRUE;

}


/*
 * print_main_menu(): prints the program's main menu, incl. a list of user choices
 */

void print_main_menu() {

    printf("\nMAIN MENU:\n");
    printf("[1]: Enumerate all processes\n");
    printf("[2]: Enumerate all threads for a process\n");
    printf("[3]: Enumerate all shared objects (libraries) loaded for a process\n");
    printf("[4]: Show information for all executable pages for a process\n");
    printf("[5]: Read through memory of a process\n");
    printf("\nTo QUIT, enter \'q\'\n\n");
}

/*
 * get_user_input(): gets a character from user, returns it. 
 * -- parameters: choices, a string containing all valid character choices
 * NOTE: getline() usage follows Linux man pages, man7.org/linux/man-pages/man-3/getline.3.hmtl
 */

char get_user_choice(char *choices) {

    char user_choice;

    char *input = NULL;
    size_t len = 0;
    ssize_t nread; 
    int good_input = FALSE;

    while(!good_input) {
        printf("Enter your choice: ");
        nread = getline(&input, &len, stdin);

        // if char + newline was read, and char matches valid choice
        if(nread == 2 && strchr(choices, *input )) {
            good_input = TRUE;
        }
        else {
            printf("Invalid choice. Try again.\n");
        }
    } 

    user_choice = *input;
    free(input);

    return user_choice; 

}


/*
 * get_numeric_input(): gets an integral number from the user, validates.
 */

int get_numeric_input() {

    int num;
    char *input = NULL;
    size_t len = 0;
    int good_input = FALSE;
    int nread = 0;

    while(!good_input) {
        printf("Enter a PID: ");
        nread = getline(&input, &len, stdin);

        // if user entered more than '\n', and input is a number
        if(nread > 1 && is_number(trim(input))){
            good_input = TRUE;
        } 
        else {
            printf("PID must be a number. Try again.\n");
        }
    }

    num = atoi(input);

    free(input);
    return num;
}
