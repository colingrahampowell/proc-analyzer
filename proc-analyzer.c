/*
 * Colin Powell
 * 
 *  Process Analyzer
 */

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
    int pid;

} pinfo;

int lefttrim(char *st);
int get_proc_info( char *path, pinfo *pi);
int is_number(char *st);

int main(int argc, char **argv) {

    printf("Listing all PIDs...\n");

    pinfo pi;

    struct dirent *entry = NULL;
    struct stat dir_stat;

    char temp[1024];
    char name[8] = {'\0'};
    strcpy(name, "/proc");
    
    DIR *dptr = opendir(name);
    
    while((entry = readdir(dptr)) != NULL) {

        // stat file
        stat(entry->d_name, &dir_stat);

        // if filename is a number, read further
//        strcpy(temp, entry->d_name);
       
        if(is_number(entry->d_name) && S_ISDIR(dir_stat.st_mode)){

            snprintf(temp, sizeof(temp), "%s/%s", name, entry->d_name);
            printf("found process id: %s\n", temp);

            get_proc_info(temp, &pi);


        } 
    } 

    closedir(dptr);
    dptr = NULL;

    return 0;
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
 * get_proc_info(): gets information about the process at the specified path, returns in a struct pinfo.
 * NOTE: file reading logic adapted from getline() man pages, linux.die.net/man/3/getline
 */

int get_proc_info( char *path, pinfo *pi) {

    char statuspath[1024] = {'\0'}; 
  
    strcpy(statuspath, path);
    strcat(statuspath, "/status");

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // open /proc/pid/status status for reading
    FILE *fp = fopen(statuspath, "r");

    if(fp == NULL) {
        fprintf(stderr, "ERROR: could not open file %s\n", statuspath);
        return FALSE;
    } 

    printf("getting information for process...\n");

    while( (read = getline(&line, &len, fp)) != -1) {

        // if we have matched name field
        if(strncmp("Name:", line, strlen("Name:")) == 0) {

            // trim off "Name:" along with any whitespace
//            strcpy(pi->name, lefttrim( strcspn(line, ":") ));

            // copy into "name" field of our process struct
            printf("%s", line);
        }
        // otherwise, if we have matched Pid field
        else if(strncmp("Pid:", line, strlen("pid:")) == 0){
            printf("%s", line);
        }
    }
    
    fclose(fp);

    return TRUE;

}

/*
 * lefttrim(): returns number of whitespace characters read  
 */

int lefttrim(char *st) {

    int i = 0;

    while( strcspn(st, " \t") ) {
        st++;
        i++;
    }
    
    return i;
}
