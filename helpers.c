/*
 * Colin Powell
 * Process Analyzer
 * Helper Functions - Implementation File
 */

#include "helpers.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

/*
 * trim(): returns pointer to first non-whitespace char in string, 
 * and trims any whitespace (incl. newlines) from end of string.
 * NOTE: string must be null-terminated.
 * some of this adapted from: https://stackoverflow.com/questions/
 *      122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
 */

char* trim(char *st) {
    
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

char* get_field_val(char *line) {
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
 * is_hex(): returns TRUE if str points to a series of hex chars, FALSE otherwise
 */

int is_hex(char *st) {

    int len = strlen(st);
    int i = 0;
    char t;
    
    for( i = 0; i < len; i++) {
        t = toupper(st[i]);
        // if not a digit, and not an upper-case hex char:
        if((t < '0' || t > '9') && (t < 'A' || t > 'F')) {
           return FALSE; 
        }
    }

    return TRUE;
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


