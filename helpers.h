/*
 * Colin Powell
 * Process Analyzer
 * Helper functions - header file
 */


#ifndef _PANALYZER
#define _PANALYZER

char* get_field_val(char *line);
char* trim(char *st);
int is_number(char *st);
int is_hex(char *st);
int read_bit(unsigned long long tgt, int bnum);

#endif
