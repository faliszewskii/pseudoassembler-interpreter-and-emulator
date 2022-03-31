#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "error_handling.h"
#include "interpret.h"
#include "read_psa_file.h"
#include "manage_file.h"
#include "execution.h"

int main(int argc, char* argv[]) {

    char debug_mode = 0;
    FILE* input_file = NULL, *executable_file = NULL, *results_file = NULL;
    char path[MAX_FILE_NAME_LENGTH] = "\0", file_name[MAX_FILE_NAME_LENGTH] = "\0"; // Sciezka do pliku. Sciezka do pliku z odcietym rozszerzeniem
    //struct Label label_array[MAX_LABELS] = { "\0", 0, 0 };  // Tablica etykiet. Wykorzystywana w intepretacji i symulacji programu
    struct Label* label_array = NULL;  // Tablica etykiet. Wykorzystywana w intepretacji i symulacji programu
    int label_counter = 0;

    if (argc >= 2)                  // Jezeli podano przynajmniej sciezke do pliku zrodlowego
        strcpy(path, argv[1]);      // Zapisz sciezke 
    if (!loadFile(&input_file, &path, "r"))    handleError(error_wrong_file, 0);               // Zaladuj plik wejsciowy
    trimFileExtension(file_name, path, strlen(path));    // Obetnij rozszerzenie pliku

    switch (argc) { 
    case 4:     // Jezeli 3 argument "debug" wlacz tryb debug i przejdz dalej
        if (strcmp(argv[3], "debug") == 0)             
            debug_mode = 1;
        else {
            handleError(error_wrong_argument, 0);
        }
    case 3:     // Jezeli 2 argument "msck_code" uruchom plik bez interpretacji
        if (strcmp(argv[2], "msck_code") == 0) {       
            executable_file = input_file;
            input_file = NULL;
            break;
        }
        else if (strcmp(argv[2], "psa_code") == 0);     // Jezeli 2 argument "psa_code" przejdz dalej
        else {
            handleError(error_wrong_argument, 0);
        }
    case 2:     // Jezeli zostal podany, zinterpretuj plik
    case 1:     // Jezeli brak argumentow, popros o sciezke do pliku i zinterpretuj plik
        executable_file = interpret(input_file, file_name, &label_array, &label_counter);              
        break;
    default:    // Inna ilosc argumentow - Wyswietl blad
        handleError(error_wrong_amount_of_arguments, 0);
    }
    execute(results_file, input_file, executable_file, path, file_name, label_array, label_counter,  debug_mode);   // Wykonaj symulacje programu
    return 0;
}