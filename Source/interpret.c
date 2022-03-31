
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "error_handling.h"
#include "interpret.h"
#include "manage_file.h"
#include "read_psa_file.h"

#pragma once
#if defined( _MSC_VER )
#pragma warning(disable:4996)
#endif

FILE* interpret(FILE* input_file, char file_name[], Label** label_array, int* label_counter) {
    FILE* output_file = NULL;
    struct Row* row_array = NULL;
    //struct Row row_array[MAX_ROWS] = { "\0", "\0", "\0", "\0" };
    //struct Label label_array[MAX_LABELS] = { "\0", 0, 0 };
    int i, j, n, row_counter = 0;
    char msck_file_name[MAX_FILE_NAME_LENGTH] = "\0";

    strcat(msck_file_name, file_name);
    strcat(msck_file_name, "_msck.txt");
    output_file = fopen(msck_file_name, "w+");                                            // Zaladuj plik wyjsciowy
    if (output_file == NULL)    handleError(error_unable_to_create_output_file, 0);     // Jezeli nie udalo sie otworzyc pliku wyjsciowego. Oznacza blad

    row_counter = readPseudoassemblerCode(input_file, &row_array);                       // Czytaj kod pseudoassemblera
    if (row_counter == 0)    handleError(error_pseudoassembler_code_reading_fail, 0);                   // Jezeli zwrocona wartosc 0. Oznacza blad czytania

    if(!loadLabels(row_array, &row_counter, label_array, label_counter))   handleError(error_load_labels_fail, 0);                         // Jezeli loadLabels zwrci falsz. Oznacza blad ladowania etykiet
    i = interpretDirectives(output_file, row_array, &row_counter);    
    if (i == -1) handleError(error_interpret_directives_fail, 0);                                                                            // Jezeli interpretDirectives zwroci falsz. Oznacza blad interpretacji dyrektyw
    if (!interpretOrders(output_file, row_array, &row_counter, *label_array, label_counter, i)) handleError(error_interpret_orders_fail, 0);// Jezeli interpretOrders zwroci falsz. Oznacza blad interpretacji rozkazow

    rewind(output_file);
    rewind(input_file);
    return output_file;     // Zwroc plik wyjsciowy
}

int loadLabels(Row row_array[], int* row_counter, Label** label_array, int* label_counter) {
    int i=0, j, n, byte_counter = 0, error=0;
    enum Stage stage = directives_stage;   

    while (row_array[i].instruction[0] == '\0') i++;                                    // Omin pierwsze puste wiersze tablicy row_array
    for (; i <= *row_counter; i++) {                                                    // Rob do konca pliku
        // Przechodzenie do etapu rozkazow
        if ( row_array[i].instruction[0] == '\0') {                                     // Jezeli natrafisz na linijke bez instrukcji to przejdz do etapu rozkazow ( Reszta pól te¿ jest pusta jesli przeszla readPseudoassemblerCode )
            stage = orders_stage;
            i++;                                                                        // Od razu przejdz do nastepnego wiersza
            byte_counter = 0;                                                           // Zacznij liczyc bity od poczatku przy sekcji rozkazow
        }
        // Zapisywanie etykiet
        if (row_array[i].label[0] != '\0') {                                                                                // Jezeli w wierszu znajduje sie etykieta
            expandLabelArray(label_array, label_counter);                                                                   // Rozszerz tablice label_array
            strcpy((*label_array)[*label_counter].label_name, row_array[i].label);                                          // Skopiuj nazwe do tablicy etykiet
            if (stage == directives_stage)   (*label_array)[*label_counter].reg = REGISTER_WITH_ADDRESS_TO_DATA_SECTION;    // Jezeli w etapie dyrektyw, dodaj adres sekcji pamieci 
            else if (stage == orders_stage) (*label_array)[*label_counter].reg = REGISTER_WITH_ADDRESS_TO_ORDER_SECTION;    // Jezeli w etapie rozkazow, dodaj adres sekcji rozkazow
            else return 0;
            (*label_array)[*label_counter].label_interpretation = byte_counter;                                             // Dodaj adres na jaki wskazuje etykieta
            (*label_counter)++;                                                                                             // Przejdz do nastepnego elementu label_array
        }
        // Liczenie bitow
        if (isDirective(row_array[i].instruction)) {                                            // Jezeli instrukcja jest dyrektywa to sprawdz ile komorek rezerwuje i tyle dodaj do bajtowego licznika
            n = 1;
            if (isdigit(row_array[i].arguments[0][0])) {
                n = atoi(row_array[i].arguments[0]);
            }

            for (j = 0; j < n; j++)
                byte_counter += BYTES_FOR_VALUE;
        }
        else if (isOrderRR(row_array[i].instruction))                                           // Jezeli instrukcja jest rozkazem rejestr-rejestr to zajmuje 2 bajty
            byte_counter += BYTES_FOR_RR_ORDER;
        else if (isOrderRM(row_array[i].instruction) || isOrderJump(row_array[i].instruction))  // Jezeli instrukcja jest rozkazem rejestr-pamiec badz rozkazem skok to zajmuje 4 bajty
            byte_counter += BYTES_FOR_RM_ORDER;
        else {                                                                                  // Jezeli instrukcja jest niezidentyfikowana, wyswietl blad
            handleError(error_parsing_wrong_instruction, i + 1);
            error = 1;
        }        
    }
    if (error)   return 0;
    return 1;
}

int expandLabelArray(Label** label_array, int label_counter) {
    Label* label_array_buffor;

    label_array_buffor = realloc(*label_array, sizeof(Label) * (label_counter + 1));
    if (label_array_buffor == NULL)
        return 0;
    *label_array = label_array_buffor;
    memset(&(*label_array)[label_counter], 0, sizeof(Label));

    return 1;
}

int interpretDirectives(FILE* output_file, Row row_array[], int* row_counter) {
    int i=0, j, k, n, value;
    int error = 0;
    char* type_str = NULL, * value_str = NULL;


    while (row_array[i].instruction[0] == '\0') i++;                                    // Omin pierwsze puste wiersze tablicy row_array
    if (!isDirective(row_array[i].instruction)) {                                    // Jezeli program bez rezerwacji pamieci
        fprintf(output_file, "\n");
        return i-1;
    }
    for (; i <= *row_counter && row_array[i].instruction[0] != '\0'; i++) {             // Rob do konca pliku badzs do znalezienia pustego wiersza

        n = 1;
        if (isdigit(row_array[i].arguments[0][0])) {                                    // Jezeli pierwszy argument zaczyna sie od cyfry. Oznacza, ze rezerwujemy ustalona liczbe komorek
            n = atoi(row_array[i].arguments[0]);                                        // Pierwszy argument zawiera ilosc komorek
            type_str = row_array[i].arguments[1];                                       // Drugi argument zawiera typ  danej
            value_str = row_array[i].arguments[2];                                      // Trzeci argument zawiera wartosc danej
        }
        else {                                                                          // Jezeli pierwszy argument nie zaczyna sie od cyfry. Oznacza, ze rezerwujemy pamiec bez podania ilosci komorek. Zakladana jedna.
            type_str = row_array[i].arguments[0];                                       // Pierwszy argument zawiera typ  danej
            value_str = row_array[i].arguments[1];                                      // Drugi argument zawiera wartosc danej
        }
        value = atoi(value_str);

        if (strcmp(type_str, "INTEGER") == 0) {                                         // Liczba jest liczba calkowita    
            if (strcmp(row_array[i].instruction, "DS") == 0)                            // Jezeli jest to dyrektywa DS
                for (j = 0; j < n; j++) fprintf(output_file, "~~ ~~ ~~ ~~\n");
            else {                                                                      // Jezeli jest to dyrektywa DC
                for (j = 0; j < n; j++) {                                                       
                    for (k = 3; k > 0; k--)                                             // Wypisz bajty po kolei
                        fprintf(output_file, "%02X ", (value >> k * BYTE) & LAST_BYTE_MASK);                    
                    fprintf(output_file, "%02X\n", value & LAST_BYTE_MASK);
                }
            }                
        }
        else {                                                                          // Jezeli inny typ niz INTEGER
            error = handleError(error_parsing_unrecognised_data_type, i + 1);             
        }
    }
    fprintf(output_file, "\n");                                                         // Wstawianie przerwy miedzy sekcja danych a rozkazow
    if (error)   return -1;    
    return i;                                                                           // Zwraca wiersz, na ktorym zakonczyla sie sekcja danych

}

int interpretOrders(FILE* output_file, Row row_array[], int* row_counter, Label label_array[], int* label_counter, int i) {
    int error = 0;
    struct Label *label;

    for (i += 1; i <= *row_counter; i++) {       
        if (getOrderCode(row_array[i].instruction) == NULL) {                                                                                                           // Blad jezeli nie znajdzie kodu rozkazu z listy
            error = handleError(error_parsing_wrong_instruction, i + 1);
            continue;
        }                                                                   
        fprintf(output_file, "%s ", getOrderCode(row_array[i].instruction));
        
        if (isOrderJump(row_array[i].instruction)) {                                                                                                                    // Jezeli rozkaz skoku            
            fprintf(output_file, "0");                                                                                                                                  // Wypisz 0 w miejscu przeznaczonym na pierwszy rejestr
            if (!isdigit(row_array[i].arguments[0][0]) && row_array[i].arguments[0][0] != '-') {                                                                                                               // Jezeli rozkaz skoku z etykieta
                label = getLabel(label_array, *label_counter, row_array[i].arguments[0]);
                if (label == NULL) {
                    error = handleError(error_parsing_unrecognised_label, i + 1);
                    continue;
                }
                fprintf(output_file, "%01X %02X %02X\n", label->reg, (label->label_interpretation >> BYTE & LAST_BYTE_MASK), label->label_interpretation & LAST_BYTE_MASK);// Wypisz intepretacje etykiety
            }
            else {                                                                                                                                                      // Jezeli rozkaz skoku z adresem
                fprintf(output_file, "%01X %02X %02X\n", atoi(row_array[i].arguments[1]), (atoi(row_array[i].arguments[0]) >> BYTE & LAST_BYTE_MASK), atoi(row_array[i].arguments[0]) & LAST_BYTE_MASK);// Wypisz adres
            }
        }
        else {                                                                                                                                                          // Jezeli nie rozkaz skoku (Rozkaz RM lub RR)
            fprintf(output_file, "%01X", atoi(row_array[i].arguments[0]));                                                                                              // Wypisz pierwszy rejestr
            
            if (!isdigit(row_array[i].arguments[1][0]) && row_array[i].arguments[1][0] != '-') {                                                                                                               // Jezeli etykieta
                label = getLabel(label_array, *label_counter, row_array[i].arguments[1]);
                if(label == NULL){
                    error = handleError(error_parsing_unrecognised_label, i + 1);
                    continue;
                }
                fprintf(output_file, "%01X %02X %02X\n", label->reg, (label->label_interpretation >> BYTE & LAST_BYTE_MASK), label->label_interpretation & LAST_BYTE_MASK);// Wypisz intepretacje etykiety
            }
            else if (row_array[i].arguments[2][0] == '\0') {                                                                                                            // Jezeli rejestr
                fprintf(output_file, "%01X\n", atoi(row_array[i].arguments[1]));                                                                                        // Wypisz rejestr
            }
            else {                                                                                                                                                      // Jezeli adres
                fprintf(output_file, "%01X %02X %02X\n", atoi(row_array[i].arguments[2]), (atoi(row_array[i].arguments[1]) >> BYTE & LAST_BYTE_MASK), atoi(row_array[i].arguments[1]) & LAST_BYTE_MASK);// Wypisz adres
            }
        }       
    }
    if(error) return 0;
    return 1;
}

char* getOrderCode(char* order) {

    if (strcmp(order, "AR") == 0)       return "10";
    else if (strcmp(order, "A") == 0)   return "D1";
    else if (strcmp(order, "SR") == 0)  return "12";
    else if (strcmp(order, "S") == 0)   return "D3";
    else if (strcmp(order, "MR") == 0)  return "14";
    else if (strcmp(order, "M") == 0)   return "D5";
    else if (strcmp(order, "DR") == 0)  return "16";
    else if (strcmp(order, "D") == 0)   return "D7";
    else if (strcmp(order, "CR") == 0)  return "18";
    else if (strcmp(order, "C") == 0)   return "D9";
    else if (strcmp(order, "J") == 0)   return "E0";
    else if (strcmp(order, "JZ") == 0)  return "E1";
    else if (strcmp(order, "JP") == 0)  return "E2";
    else if (strcmp(order, "JN") == 0)  return "E3";
    else if (strcmp(order, "L") == 0)   return "F0";
    else if (strcmp(order, "LR") == 0)  return "31";
    else if (strcmp(order, "LA") == 0)  return "F2";
    else if (strcmp(order, "ST") == 0)  return "F3";
    return NULL;
}

Label* getLabel(Label labels[], int length, char* name) {
    int i;

    for (i = 0; i <= length; i++)
        if (strcmp(labels[i].label_name, name) == 0)  return &(labels[i]);    
    return NULL;
}

int isDirective(char* str) {

    if (strcmp(str, "DC") == 0 || strcmp(str, "DS") == 0)   return 1;
    return 0;
}

int isOrderRR(char* str) {

    if (strcmp(str, "AR") == 0 ||
        strcmp(str, "SR") == 0 ||
        strcmp(str, "MR") == 0 ||
        strcmp(str, "DR") == 0 ||
        strcmp(str, "CR") == 0 ||
        strcmp(str, "LR") == 0) return 1;
    return 0;
}

int isOrderRM(char* str) {

    if (strcmp(str, "A") == 0 ||
        strcmp(str, "S") == 0 ||
        strcmp(str, "M") == 0 ||
        strcmp(str, "D") == 0 ||
        strcmp(str, "C") == 0 ||
        strcmp(str, "L") == 0 ||
        strcmp(str, "LA") == 0 ||
        strcmp(str, "ST") == 0) return 1;
    return 0;
}

int isOrderJump(char* str) {

    if (strcmp(str, "J") == 0 ||
        strcmp(str, "JZ") == 0 ||
        strcmp(str, "JP") == 0 ||
        strcmp(str, "JN") == 0) return 1;
    return 0;
}