/*
 * Colin Powell
 * 
 *  Process Analyzer
 */

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

typedef struct {
    char name[1024];
    char cmdline[1024];
    int pid;
} pinfo;

/* USER INPUT */

void print_main_menu();
char get_user_choice(char *choices);

/* PROCESS FUNCTIONS */

int list_processes();
int get_proc_info( char *path, pinfo *pi);

/* HELPERS */

char *get_field_val(char *line);
char* trim(char *st);
int is_number(char *st);

int main(int argc, char **argv) {

    char user_choice;
    char choices[] = "12345q";
    int quit = FALSE;

    while(!quit) {

        print_main_menu();
        user_choice = get_user_choice(choices);

        switch (user_choice){
            case '1':   list_processes();
                        break;
            default:    quit = TRUE; 
        }
    }
}

/*
 * list_processes(): enumerates all processes.
 * -- returns TRUE if no read errors occured, FALSE otherwise.
 */

int list_processes() {

    int success = FALSE;
    pinfo pi;

    struct dirent *entry = NULL;
    struct stat dir_stat;

    char temp[1024];
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

int list_threads(int pid) {

    int success = FALSE;
    
    struct dirent *entry = NULL;
    struct stat dir_stat;

    char path[1024] = {'\0'};
    sprintf(path, "/proc/%d/tasks", pid);

    DIR *dptr = opendir(path);
    if(dptr == NULL) {
        fprintf(stderr, "ERROR: could not open %s directory", path);
        return FALSE;
    }

    printf("----THREADS FOR PID: %d----\n", pid);
//    printf("%-8s%-32s%-64s\n", "TID", "NAME", "CMDLINE");
    while((entry = readdir(dptr)) != NULL) {
        stat(entry->d_name, &dir_stat);

        if(is_number(entry->d_name) && S_ISDIR(dir_stat.st_mode)) {
            // will be almost identical to process listing 
        }
    }

    return success;
}


/*
 * get_proc_info(): gets information about the process at the specified path, returns in a struct pinfo.
 * NOTE: file reading logic adapted from getline() man pages, linux.die.net/man/3/getline
 */

int get_proc_info(char *path, pinfo *pi) {

    char info_path[1024] = {'\0'};
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
 * lefttrim(): returns pointer to first non-whitespace char in string, 
 * and trims any whitespace (incl. newlines) from end of string.
 * NOTE: string must be null-terminated.
 * some of this adapted from: https://stackoverflow.com/questions/
 *      122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
 */

char *trim(char *st) {
    
    while(isspace((unsigned char)*st)) st++;
    char *end = st + strlen(st) - 1;

    // place null terminator at end 
    while((end > st) && isspace((unsigned char)*end)) end--;
    end = end + 1;
    *end = '\0';

    return st;
}

/*
 * get_field_val(): gets value of parameter in field marked with "fld_name"
 * - e.g., if field is "Name: foo", will return pointer to substring starting
 * - with 'f'.
 * NOTE: both parameters must be null-terminated strings.
 */

char *get_field_val(char *line) {
    line = strpbrk(line, ":") + 1;
    return trim(line);
}

/*
 * is_number(): returns TRUE if str points to a series of digit chars, FALSE otherwise
 */

int is_number(char *st) {

    int len = strlen(st);
    int i = 0;

    for( i = 0; i < len; i++) {
        if(st[i] < '0' || st[i] > '9') {
            return FALSE;
        }
    }

    return TRUE;

}

/*
 * prints the program's main menu, incl. a list of user choices
 */

void print_main_menu() {

    printf("*****WELCOME TO PROCESS ANALYZER*****\n");
    printf("\nMAIN MENU:\n");
    printf("[1]: Enumerate all processes\n");
    printf("[2]: Enumerate all threads for a process\n");
    printf("[3]: Enumerate all shared libraries loaded for a process\n");
    printf("[4]: Show all executable pages for a process\n");
    printf("[5]: Read through memory\n");

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
