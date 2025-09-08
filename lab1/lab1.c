// lab1_a.c
// CS 6233, Fall 2025 - Assignment 1
// Prints course banner, student name, and random number [0-149]

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    printf("Hello world! This is Introduction to Operating Systems, CS 6233, Fall 2025!\n");

    const char *first = "Hongdao";  
    const char *last  = "Meng";   

    srand((unsigned)time(NULL));
    int r = rand() % 150;

    printf("%s %s %d\n", first, last, r);
    return 0;
}
