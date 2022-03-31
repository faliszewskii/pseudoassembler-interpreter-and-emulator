#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "error_handling.h"
#include "interpret.h"
#include "read_file.h"

// TODO 
// Zoptymalizowanie kodu
// Rozdzielenie na funkcje
// Usuniecie DEBUG Dodaæ

int readPseudoassemblerCode(FILE* input_file, struct Row row_array[]) {

    char line_pointer[MAX_COMMAND_LENGTH];                  // Magazynuje ciag znakow calego wiersza
    char *token, *p;                                        // Bufor dla wycinkow ciagu znakow, pomocniczy wskaznik
    int it = 0, it2 = 0;                                    // Iteratory po ciagach znakowych
    int row_counter = -1, line = 0, argument_counter = 0;   // Liczniki (Uwzglednienie pierwszej inkrementacji)
    enum Stage stage = directives_stage;                    // Iterator etapu
    int parsing_error = 0;                                          // Czy wystapil blad w skladni




    while (fgets(line_pointer, MAX_COMMAND_LENGTH, input_file)) {                   // Pobiera kolejna linijke z pliku

        line++;
        row_counter++;
        argument_counter = 0;

        if (line_pointer[0] == '/')                                                 // Jezeli wiersz zaczyna sie od komentarza, opusc go i idz do kolejnego wiersza
            if (line_pointer[1] == '/') {
                continue;
            }
        if (line_pointer[0] == '\n') {                                              // Jezeli wiersz jest pusty, przejdz do nastepnego etapu i idz do kolejnego wiersza
            stage++;
            if (stage == end_of_program) {
                if (parsing_error) return 0;            // Jezeli wystapil jakikolwiek blad nie kontynuuj interpretacji
                return row_counter-1;                     // Drugi pusty wiersz oznacza koniec programu
            }
            continue;
        }

        token = strtok(line_pointer, " \t\n");
        if (token == NULL || (token[0] == '/' && token[1] == '/')) {
            parsing_error = handleError(error_parsing_forbidden_whitespaces, line);
            continue;
        }        

        if (!isspace(line_pointer[0])) {
            if (isdigit(token[0])) {                                                 // Etykieta zaczyna sie od cyfry
                parsing_error = handleError(error_parsing_label_starting_with_a_digit, line);
                continue;
            }
            strcpy(row_array[row_counter].label, token);
            token = strtok(NULL, " \t\n");     // Kolejny odcinek
            if (token == NULL || (token[0] == '/' && token[1] == '/')) {
                parsing_error = handleError(error_parsing_expected_instruction, line);
                continue;
            }
        }
        if (stage == directives_stage) {                                                                                        // Czy na etapie dyrektyw (Zapobiega interpretacji instrukcji w nie swoich etapach)
            if (isDirective(token)) {                                                                                           // Jezeli instrukcja jest dyrektywa
            
                strcpy(row_array[row_counter].instruction, token);                                                              // Przypisanie instrukcji do ostatniego Row
                token = strtok(NULL, " \t\n");                                                                                  // Kolejny odcinek
                if (token == NULL || (token[0] == '/' && token[1] == '/')) {
                    parsing_error = handleError(error_parsing_expected_values, line);    // Brak argumentow - blad
                    continue;
                }
                it = 0;                                                                                                         // Zresetowanie iteratorow
                it2 = 0;                                                                                                        //
                if (isdigit(token[0])) {                                                                                        // Liczba - rezerwacja wielu komorek
                    while (isdigit(token[it++]));                                                                               // Szukanie konca liczby
                    if (token[it - 1] != '*') {
                        parsing_error = handleError(error_parsing_expected_asterisk, line);                               // Sprawdzanie obecnosci gwiazdki. Przy braku - blad skladni
                        continue;
                    }
                    strncpy(row_array[row_counter].arguments[argument_counter], token, it - 1);                                 // Przypisanie liczby do wskazanego argumentu (1) ostatniego Row
                    argument_counter++;                                                                                         // Przejscie do kolejnego argumentu
                }
                it2 = it;                                                                                                       // Zresetowanie iteratorow
                p = &token[it];                                                                                                 // Ustalenie poczatku kolejnego elementu instrukcji
                while (isalpha(token[it])) it++;                                                                                // Szukanie konca typu
                strncpy(row_array[row_counter].arguments[argument_counter], p, it - it2);                                       // Przypisanie liczby do wskazanego argumentu (1 lub 2) ostatniego Row
                argument_counter++;                                                                                             // Przejscie do kolejnego argumentu
                if (strcmp(row_array[row_counter].instruction, "DC") == 0) {                                                    // Jezeli instrukcja jest dyrektywa DC to sprawdz nawiasy
                    if (token[it] != '(') {
                        parsing_error = handleError(error_parsing_expected_opening_bracket, line);                            // Sprawdzanie obecnosci nawiasu. Przy braku - blad skladni
                        continue;
                    }
                    it++;                                                                                                       // Ustawienie iteratora na poczatek liczby
                    it2 = it;                                                                                                   // Zresetowanie iteratorow
                    p = &token[it];                                                                                             // Ustalenie poczatku kolejnego elementu instrukcji 
                    if (p[0] == '-') it++;                                                                                      // Pomin sprawdzanie nastepnego warunku dla minusa
                    if (!isdigit(token[it])) {
                        parsing_error = handleError(error_parsing_wrong_values, line);                                     // Sprawdzanie czy w nawiasach jest liczba
                        continue;
                    }
                    while (isdigit(token[it]) || token[it] == '.') it++;                                                        // Szukanie konca liczby (Uwazglednienie liczb zmiennoprzecinkowych)
                    if (token[it] != ')') {
                        parsing_error = handleError(error_parsing_expected_closing_bracket, line);                            // Sprawdzanie obecnosci nawiasu. Przy braku - blad skladni
                        continue;
                    }
                    strncpy(row_array[row_counter].arguments[argument_counter], p, it - it2);                                   // Przypisanie liczby do wskazanego argumentu (2 lub 3) ostatniego Row 
                    if (token[it + 1] != '\0') {
                        parsing_error = handleError(error_parsing_wrong_values, line);                                   // Sprawdzanie czy cos znajduje sie po nawiasach. Jezeli tak - blad skladni
                        continue;
                    }
                }
                else                                                                                                            // Jest to dyrektywa DS. Wystarczy sprawdzic czy po typie nie ma innych znakow
                    if (token[it] != '\0') {
                        parsing_error = handleError(error_parsing_wrong_values, line);
                        continue;
                    }
            }
            else {                                                                                                                // Instrukcja nie jest dyrektywa
                parsing_error = handleError(error_parsing_unrecognised_instruction, line);
                continue;
            }
        }
        if (stage == orders_stage) {

            if (isOrderJump(token)) {           // Sprawdzanie skladni rozkazu skoku
                strcpy(row_array[row_counter].instruction, token);
                token = strtok(NULL, " \t\n");
                if (token == NULL || (token[0] == '/' && token[1] == '/')) {
                    parsing_error = handleError(error_parsing_expected_address, line);
                    continue;
                }
                if (isdigit(token[0])) {
                    it = 0;
                    it2 = 0;
                    while (isdigit(token[it])) it++;
                    if (token[it] != '(') {
                        parsing_error = handleError(error_parsing_expected_opening_bracket, line);
                        continue;
                    }
                    strncpy(row_array[row_counter].arguments[argument_counter], token, it);
                    it++;
                    argument_counter++;
                    it2 = it;
                    p = &token[it];
                    if (!isdigit(token[it])) {
                        parsing_error = handleError(error_parsing_wrong_address, line);
                        continue;
                    }
                    while (isdigit(token[it])) it++;
                    if (token[it] != ')') {
                        parsing_error = handleError(error_parsing_expected_closing_bracket, line);
                        continue;
                    }
                    strncpy(row_array[row_counter].arguments[argument_counter], p, it - it2);
                    if (token[it + 1] != '\0') {
                        parsing_error = handleError(error_parsing_wrong_address, line);
                        continue;
                    }
                }
                else {
                    strcpy(row_array[row_counter].arguments[argument_counter], token);
                }
            }
            else if (isOrderRM(token)) {        // Sprawdzanie skladni rozkazu rejestr - pamiec
                strcpy(row_array[row_counter].instruction, token);
                token = strtok(NULL, " \t\n");
                if (token == NULL || (token[0] == '/' && token[1] == '/') || !isdigit(token[0])) {
                    parsing_error = handleError(error_parsing_expected_register, line);
                    continue;
                }
                it = 0;
                it2 = 0;
                while (isdigit(token[it])) it++;
                if (token[it] != ',') {
                    parsing_error = handleError(error_parsing_expected_comma, line);
                    continue;
                }
                strncpy(row_array[row_counter].arguments[argument_counter], token, it);
                it++;
                argument_counter++;
                if (token[it] == '\0') {
                    token = strtok(NULL, " \t\n");
                    if (token == NULL || (token[0] == '/' && token[1] == '/')) {
                        parsing_error = handleError(error_parsing_expected_register, line);
                        continue;
                    }
                    it = 0;
                }
                it2 = it;
                p = &token[it];
                if (isdigit(token[it])) {
                    
                    while (isdigit(token[it])) it++;
                    if (token[it] != '(') {
                        parsing_error = handleError(error_parsing_expected_opening_bracket, line);
                        continue;
                    }
                    strncpy(row_array[row_counter].arguments[argument_counter], token, it - it2);
                    it++;
                    argument_counter++;
                    it2 = it;
                    p = &token[it];
                    if (!isdigit(token[it])) {
                        parsing_error = handleError(error_parsing_wrong_address, line);
                        continue;
                    }
                    while (isdigit(token[it++]));
                    if (token[it - 1] != ')') {
                        parsing_error = handleError(error_parsing_expected_closing_bracket, line);
                        continue;
                    }
                    strncpy(row_array[row_counter].arguments[argument_counter], p, it - it2 - 1);
                    if (token[it] != '\0') {
                        parsing_error = handleError(error_parsing_wrong_address, line);
                        continue;
                    }
                    
                }else{
                    while (isalpha(token[it])) it++;
                    strncpy(row_array[row_counter].arguments[argument_counter], p, it - it2);                    
                }
            }
            else if (isOrderRR(token)) {        // Sprawdzanie skladni rozkazu rejestr - rejestr
                strcpy(row_array[row_counter].instruction, token);
                token = strtok(NULL, " \t\n");
                if (token == NULL || (token[0] == '/' && token[1] == '/') || !isdigit(token[0])) {
                    parsing_error = handleError(error_parsing_expected_register, line);
                    continue;
                }
                it = 0;
                it2 = 0;
                while (isdigit(token[it])) it++;
                if (token[it] != ',') {
                    parsing_error = handleError(error_parsing_expected_comma, line);
                    continue;
                }
                strncpy(row_array[row_counter].arguments[argument_counter], token, it);
                it++;
                argument_counter++;
                if (token[it] == '\0') {
                    token = strtok(NULL, " \t\n");
                    if (token == NULL || (token[0] == '/' && token[1] == '/')) {
                        parsing_error = handleError(error_parsing_expected_register, line);
                        continue;
                    }
                    it = 0;
                }
                it2 = it;
                p = &token[it];
                if (!isdigit(token[it])) {
                    parsing_error = handleError(error_parsing_expected_register, line);
                    continue;
                }
                while (isdigit(token[it])) it++;
                strncpy(row_array[row_counter].arguments[argument_counter], p, it - it2);
                if (token[it] != '\0') {
                    parsing_error = handleError(error_parsing_wrong_register, line);
                    continue;
                }
                
            }
            else{
                parsing_error = handleError(error_parsing_unrecognised_instruction, line);
                continue;
            }

        }
        
        token = strtok(NULL, " \t\n");
        if (token != NULL && !(token[0] == '/' && token[1] == '/')) {
            parsing_error = handleError(error_parsing_too_many_arguments, line);
            continue;
        }
        
    } 

    if (parsing_error) return 0;
    return row_counter;

}
