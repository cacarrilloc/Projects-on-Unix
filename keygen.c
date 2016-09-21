/***************************************************************************
 ** Author:      Carlos Carrillo-Calderon                                  *
 ** Date:        08/07/16                                                  *
 ** Filename:    keygen.c                                                  *
 **                                                                        *
 ** Description: This program creates a key file of specified length. The  *
 **              characters in the file generated are any of the 27        *
 **              allowed characters, generated using the standard UNIX     *
 **              randomization methods. The last character keygen outputs  *
 **              should be a newline. All error text must be output to     *
 **              stderr.                                                   *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function Prototype.
int randInt(int min, int max);

int main(int argc, char *argv[]) {

    // Declare variables.
    int i, keyLength;
    time_t sysClock;
    char randLetter;

    // Check if there are enough arguments.
    if (argc < 2) {
        printf("Usage: keygen keyLength\n");
        exit(1);
    }
    // Get the same length as the plaintext for the key file.
    sscanf(argv[1], "%d", &keyLength);

    // Error checking.
    if (keyLength < 1) {
        printf("keygen: invalid keyLength\n");
        exit(1);
    }
    // Seed random number generator.
    srand((unsigned) time(&sysClock));

    // Produce random letters.
    for (i = 0; i < keyLength; i++) {
        
        // Call function to produce random letters.
        randLetter = (char) randInt(64, 90);

        // Space case.
        if (randLetter == '@') {
           randLetter = ' ';
        }
        printf("%c", randLetter);
    }
    printf("\n");

    return 0;
}

/*******************************************************
 * randInt(): Function to produce random letters.      *
 ******************************************************/
int randInt(int min, int max) {
    return rand() % (max - min + 1) + min;
}
