#include <stdio.h>
#include <stdlib.h>
#include "constants.h"

int loadFile(FILE** input_file, char path[], char* argument) {
    int c;  
    
    do {                                                    // Pros o sciezke do pliku do momentu gdy jest ona poprawna
        system("cls");
        if (path[0] == '\0') {
            printf("Enter the path to the file to load:\n");
            while ((c = getchar()) != '\n')
                strcat(path, &c);
        }

        *input_file = fopen(path, argument);
        if (*input_file == NULL) {
            perror("Error");
            printf("Press any key to continue...");
            getch();
            memset(path, 0, sizeof(path));
        }
        
    } while (*input_file == NULL);

    if ((*input_file) == NULL) return 0;                        // Zwroc falsz gdy nie udalo sie wczytac pliku
    return 1;

}

void trimFileExtension(char path_name[], char path[], int n) {
    int i = 0;
    for (i = n-1; i >=0 && path[i] != '.'; i--);
    strncat( path_name, path, i);
    return;
}
