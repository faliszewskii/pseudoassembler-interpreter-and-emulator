
// TODO Zmienic stringi przy wyjsciu na liczby typu 0xFDFD
//      "%X%X%X%04X\n" dla zer
// Zrobic row_array, row_counter, label_array i label_counter jako glabalna zmienna dla programu interpretera

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "error_handling.h"
#include "interpret.h"
#include "read_file.h"
#include "manage_file.h"

#pragma once
#if defined( _MSC_VER )
#pragma warning(disable:4996)
#endif

// MAIN
int main(int argc, char* argv[]) {

    int debug_mode = 0;
    FILE* executable_file = NULL, results;

    switch (argc) {
    case 4:
        if (strcmp(argv[3], "debug") == 0)
            debug_mode = 1;
        else {
            handleError(error_wrong_argument, 0);
        }
    case 3:
        if (strcmp(argv[2], "msck_code") == 0) {
            if (!loadFile(executable_file, argv[1]))
                handleError(error_wrong_file, 0);
            break;
        }
        else if (strcmp(argv[2], "psa_code") == 0);
        else {
            handleError(error_wrong_argument, 0);
        }
    case 2:
        executable_file = interpret(argv[1]);
        break;
    case 1:
        executable_file = interpret(NULL);
        break;
    default:
        handleError(error_wrong_amount_of_arguments, 0);
    }

    // result = run(executable_file, debug_mode)

   
   

    return 0;
}   
/*

int readFile(FILE* input_file, struct Row row_array[]) {

    char file_line[MAX_COMMAND_LENGTH];
    char* line_pointer = NULL, *p = NULL;

    int row_counter = 0, skip = 0;

    enum Phase phase = label_phase;


    while (line_pointer = fgets(file_line, MAX_COMMAND_LENGTH, input_file)) {         // Przejscie po pliku i zapisanie instrukcji w kolejnych rubrykach tablicy.
        skip = 0;                                                                       // Mowi petli ze powinna ominac cala linijke gdy napotka komentarz.
        phase = 0;                                                                      // Pozwala zidentyfikowac do ktorej rubrki wstawic slowo.
        if (strstr(line_pointer, "KONIEC...") != 0)    break;                           // Koniec kodu, zakoncz czytanie.
        for (; !isEnd(line_pointer) && !skip; line_pointer++) {                         // Rob dopoki nie natrafisz na koniec wiersza, pliku, badz komentarz.
            if (isComment(line_pointer)) {                                              // Wykrywa komentarz. (Jest gwarancja niewyjscia z tablicy; [i+1] moze byc co najwyzej '\0') TODO
                skip = 1;
                if (phase == 0) row_counter--;                                          // Rozpatruje przypadek gdy linijka zaczyna sie od komentarza.
            }

            else if (skipSpaces(&line_pointer)) {                                       // Napotkanie bialego znaku.
                phase++;                                                                // Skonczyla sie jedna czesc zaczyna druga.

            }
            else {
                switch (phase) {                                                    // Podjecie decyzji, do ktorej rubryki wstawic nastepujacy ciag znakow.
                case label_phase:                                                   // Etykiety
                    p = row_array[row_counter].label;                               //
                    break;                                                          //
                case order_code_phase:                                              // Dyrektywy lub rozkazy
                    p = row_array[row_counter].order;                               //
                    break;                                                          //
                case argument1_phase:                                               // Argument pierwszy
                    p = row_array[row_counter].argument1;                           //
                    break;                                                          //
                case argument2_phase:                                               // Argument drugi
                    p = row_array[row_counter].argument2;                           //
                    break;                                                          //
                case argument3_phase:                                               // Argument trzeci
                    p = row_array[row_counter].argument3;                           //
                    break;                                                          //
                default:
                    skip = 1;   //  TO DO ERROR HANDLING        Wyjscie poza ilosc slow
                    break;
                }
                if (skip)    break;
                for (; !isspace(*line_pointer) && *line_pointer != ',' && *line_pointer != '*' && *line_pointer != '(' && *line_pointer != ')' && j < MAX_NAME_LENGTH; line_pointer++, p++)        // Uzupelnianie odpiowiedniej komorki ciagiem znakow.    (TODO Uwzglednic przypadek wyjscia poza granice)
                    *p = *line_pointer;
                line_pointer--;                                                                         // Cofniecie ostatniej inkrementacji iteratora.
                *p = '\0';                                                                               // Dodanie znaku konca ciagu znakow
            }
        }
        row_counter++;          // Przejscie do kolejnego wiersza w tablicy
    }
    return row_counter;

}

int loadLabels(struct Row row_array[], int* row_counter, struct Label label_array[], int* label_counter){

    int i, j, n, byte_counter = 0;
    char str[MEMORY_CELL_LENGTH + 1];

    for (i = 0; i < *row_counter && strcmp(row_array[i].order, "\0") != 0; i++) {                                             // Rób do koñca pliku lub do napotkania pustego wiersza (Kazda lnijka kodu musi zawieraæ przynajmniej rozkaz; Koniec deklaracji)
        if (row_array[i].label[0] != '\0') {
            strcpy(label_array[*label_counter].label_name, row_array[i].label);                                                      // Skopiuj nazwe
            strcpy(label_array[*label_counter].label_interpretation, "f");                                                          // Dodaj adres sekcji pamieci
            strcat(label_array[*label_counter].label_interpretation, itoxStringWithZeros(str, byte_counter, BYTES_FOR_LABEL * 2));    // Dodaj adres na jaki wskazuje etykieta
            (*label_counter)++;
        }

        if (strcmp(row_array[i].order, "DC") == 0 || strcmp(row_array[i].order, "DS") == 0) {                           // Liczenie bitow
            n = 1;
            if (isdigit(row_array[i].argument1[0]))
                n = atoi(row_array[i].argument1);
            for (j = 0; j < n; j++)
                byte_counter += BYTES_FOR_VALUE;
        }
    }

    byte_counter = 0;                   // Rozpoczynanie liczenia bitow od poczatku dla sekcji rozkazow
    i++;

    for (; i < *row_counter; i++) {                                                                                          // Rób do konca listy

        if (row_array[i].label[0] != '\0') {
            strcpy(label_array[*label_counter].label_name, row_array[i].label);                                                      // Skopiuj nazwe
            strcpy(label_array[*label_counter].label_interpretation, "e");                                                          // Dodaj adres sekcji rozkazów
            strcat(label_array[*label_counter].label_interpretation, itoxStringWithZeros(str, byte_counter, BYTES_FOR_LABEL * 2));    // Dodaj adres na jaki wskazuje etykieta
            (*label_counter)++;
        }

        if (row_array[i].order[1] == 'R')  // Tymczasowe rozwiazanie (2 bajty zajmuja tylko rozkazy z rejestrami)
            byte_counter += BYTES_FOR_LABEL;
        else byte_counter += BYTES_FOR_VALUE;


    }   // TODO Uporoscic te dwie petle w jedna
    return 1;
}

int interpretDirectives(FILE* output_file, struct Row row_array[], int* row_counter) {

    int i, j, n;
    char* type = NULL, * value = NULL;
    char str[MEMORY_CELL_LENGTH + 1];

    for (i = 0; i < *row_counter && strcmp(row_array[i].order, "\0") != 0; i++) {

        n = 1;
        if (isdigit(row_array[i].argument1[0])) {
            n = atoi(row_array[i].argument1);
            type = row_array[i].argument2;
            value = row_array[i].argument3;
        }
        else {
            type = row_array[i].argument1;
            value = row_array[i].argument2;
        }

        if (strcmp(type, "INTEGER") == 0) {                   // Liczba jest liczba calkowita    
            if (strcmp(row_array[i].order, "DS") == 0)
                for (j = 0; j < n; j++) fprintf(output_file, "~~~~~~~~\n");
            else
                for (j = 0; j < n; j++) fprintf(output_file, strcat(itoxStringWithZeros(str, atoi(value), BYTES_FOR_VALUE * 2), "\n"));
        }
        else {
            // NIEOBSULGIWANY TYP ZMIENNYCH
        }
    }
    fprintf(output_file, "\n");
    return i;   // Zwraca wiersz, na ktorym zakonczyla sie sekcja danych

}

int interpretOrders(FILE* output_file, struct Row row_array[], int* row_counter, struct Label label_array[], int* label_counter, int i) {

    char str1[MAX_LENGTH];

    for (i += 1; i < *row_counter; i++) {
        fprintf(output_file, getOrderCode(row_array[i].order));

        if (!isdigit(row_array[i].argument1[0])) {
            str1[0] = '0';
            str1[1] = '\0';
            fprintf(output_file, strcat(str1, getLabelInterpretation(label_array, *label_counter, row_array[i].argument1)));
        }
        else {
            fprintf(output_file, itoxStringWithZeros(str1, atoi(row_array[i].argument1), BYTES_FOR_REGISTER * 2));

            if (!isdigit(row_array[i].argument2[0])) {
                fprintf(output_file, getLabelInterpretation(label_array, *label_counter, row_array[i].argument2));
            }
            else if (row_array[i].argument3[0] == '\0') {   // TODO Zrobic ze reaguje na rozkaz a nie na to czy jest w nawiasie cos
                fprintf(output_file, itoxStringWithZeros(str1, atoi(row_array[i].argument2), BYTES_FOR_REGISTER * 2));
            }
            else {
                fprintf(output_file, itoxStringWithZeros(str1, atoi(row_array[i].argument3), BYTES_FOR_REGISTER * 2));
                fprintf(output_file, itoxStringWithZeros(str1, atoi(row_array[i].argument2) * BYTES_FOR_VALUE, BYTES_FOR_LABEL * 2));
            }
        }
        fprintf(output_file, "\n");
    }
    return 1;
}

char* getOrderCode(char* order) {
    if (strcmp(order, "AR") == 0)  return "10";
    else if (strcmp(order, "A") == 0)  return "d1";
    else if (strcmp(order, "SR") == 0)  return "12";
    else if (strcmp(order, "S") == 0)  return "d3";
    else if (strcmp(order, "MR") == 0)  return "14";
    else if (strcmp(order, "M") == 0)  return "d5";
    else if (strcmp(order, "DR") == 0)  return "16";
    else if (strcmp(order, "D") == 0)  return "d7";
    else if (strcmp(order, "CR") == 0)  return "18";
    else if (strcmp(order, "C") == 0)  return "d9";
    else if (strcmp(order, "J") == 0)  return "e0";
    else if (strcmp(order, "JZ") == 0)  return "e1";
    else if (strcmp(order, "JP") == 0)  return "e2";
    else if (strcmp(order, "JN") == 0)  return "e3";
    else if (strcmp(order, "L") == 0)  return "f0";
    else if (strcmp(order, "LR") == 0)  return "f1";
    else if (strcmp(order, "LA") == 0)  return "f2";
    else if (strcmp(order, "ST") == 0)  return "f3";
    return "\0"; // TODO Nierozpoznany rozkaz
}

char* getLabelInterpretation(struct Label labels[], int length, char* name) {

    int i;

    for (i = 0; i <= length; i++)
        if (strcmp(labels[i].label_name, name) == 0)  return labels[i].label_interpretation;
    return "\0"; // TODO Nierozpoznana etykieta
}

*/
