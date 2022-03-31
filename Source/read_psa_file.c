#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "error_handling.h"
#include "interpret.h"
#include "read_psa_file.h"

int readPseudoassemblerCode(FILE* input_file, Row** row_array) {
    char line_pointer[MAX_COMMAND_LENGTH] = "\0";   // Magazynuje ciag znakow calego wiersza
    int row_counter = -1;                           // Licznik (Uwzglednienie pierwszej inkrementacji)
    enum Stage stage = directives_stage;            // Iterator etapu
    char* token;                                    // Bufor dla wycinkow ciagu znakow
    int parsing_error = 0;                          // Flaga czy wystapil blad w skladni
   
    while (fgets(line_pointer, MAX_COMMAND_LENGTH, input_file)) {          // Pobiera kolejna linijke z pliku        
        row_counter++;      
        if (!expandRowArray(row_array, row_counter))                // Rozszerz tablice row_array
            handleError(error_parsing_memory_allocation_fail, 0);

        if (checkComment(line_pointer))                            // Jezeli wiersz zaczyna sie od komentarza, opusc go i idz do kolejnego wiersza 
            continue;
        if (line_pointer[0] == '\n') {                      // Jezeli wiersz jest pusty, przejdz do nastepnego etapu i idz do kolejnego wiersza
            stage++;
            if (stage == end_of_program) {                  // Drugi pusty wiersz oznacza koniec programu
                if (parsing_error) return 0;                // Jezeli wystapil jakikolwiek blad nie kontynuuj interpretacji
                return row_counter-1;                       
            }
            continue;
        }

        token = strtok(line_pointer, " \t\n");                                                  // Podziel wiersz wedlug bialych znakow
        if (token == NULL || (token[0] == '/' && token[1] == '/')) {                            // Jezeli token pusty oznacza to, ze przedtem byly w nim tylko biale znaki
            parsing_error = handleError(error_parsing_forbidden_whitespaces, row_counter + 1);
            continue;
        }        

        if (!checkLabel(*row_array, row_counter, &token, line_pointer, &parsing_error))
            continue;
        if (!checkDirective(*row_array, row_counter, &token, &parsing_error, stage))
            continue;
        if (!checkOrder(*row_array, row_counter, &token, &parsing_error, stage))
            continue;             
        
        token = strtok(NULL, " \t\n");
        if (token != NULL && !(token[0] == '/' && token[1] == '/')) {                           // Jezeli pozostalo cos w wierszu co nie jest komentarzem. Oznacza blad skladni
            parsing_error = handleError(error_parsing_too_many_arguments, row_counter + 1);            
        }
        
    } 

    if (parsing_error) return 0;        // Jezeli wystapil jakis blad w skladni, zwroc niepowodzenie czytania
    return row_counter;                 
}

int expandRowArray(Row** row_array, int row_counter) {
    Row* row_array_buffor;

    row_array_buffor = realloc(*row_array, sizeof(Row) * (row_counter + 1));
    if (row_array_buffor == NULL)
        return 0;
    *row_array = row_array_buffor;
    memset(&(*row_array)[row_counter], 0, sizeof(Row));

    return 1;
}

int checkComment(char* line_pointer) {

    if (line_pointer[0] == '/')
        return (line_pointer[1] == '/');
    return 0;
}

int checkLabel(Row row_array[], int row_counter, char** token, char* line_pointer, int* parsing_error) {

    if (!isspace(line_pointer[0])) {                                                        // Jezeli wiersz nie zaczyna sie od bialego znaku. Oznacza obecnosc etykiety
        if (isdigit((*token)[0])) {                                                            // Jezeli etykieta zaczyna sie od cyfry. Oznacza blad skladni
            *parsing_error = handleError(error_parsing_label_starting_with_a_digit, row_counter + 1);
            return 0;
        }
        strcpy(row_array[row_counter].label, *token);                                        // Zapisanie etykiety
        *token = strtok(NULL, " \t\n");
    }

    return 1;
}

int checkDirective(Row row_array[], int row_counter, char** token, int* parsing_error, enum Stage stage) {
    if (stage == directives_stage) {                                                        // Czy na etapie dyrektyw (Zapobiega interpretacji instrukcji w nie swoich etapach)
        if (isDirective(*token)) {                                                           // Jezeli instrukcja jest dyrektywa
            if (!readDirective(row_array, row_counter, token, parsing_error)) return 0;                                        // Czytaj dyrektywe. Jezeli niepowodzenie, ignoruj reszte wiersza
        }
        else {                                                                              // Instrukcja nie jest dyrektywa    
            *parsing_error = handleError(error_parsing_unrecognised_instruction, row_counter + 1);
            return 0;
        }
    }

    return 1;
}

int checkOrder(Row row_array[], int row_counter, char** token, int* parsing_error, enum Stage stage) {

    if (stage == orders_stage) {                                                            // Czy na etapie rozkazow

        if (isOrderJump(*token)) {                                                           // Jezeli instrukcja jest rozkazem skoku
            if (!readOrderJump(row_array, row_counter, token, parsing_error)) return 0;                                        // Czytaj rozkaz. Jezeli niepowodzenie, ignoruj reszte wiersza
        }
        else if (isOrderRM(*token)) {                                                        // Jezeli instrukcja jest rozkazem rejestr - pamiec
            if (!readOrderRM(row_array, row_counter, token, parsing_error)) return 0;                                          // Czytaj rozkaz. Jezeli niepowodzenie, ignoruj reszte wiersza
        }
        else if (isOrderRR(*token)) {                                                        // Jezeli instrukcja jest rozkazem rejestr - rejestr
            if (!readOrderRR(row_array, row_counter, token, parsing_error)) return 0;                                          // Czytaj rozkaz. Jezeli niepowodzenie, ignoruj reszte wiersza
        }
        else {                                                                               // Instrukcja nie jest rozkazem
            *parsing_error = handleError(error_parsing_unrecognised_instruction, row_counter + 1);            
            return 0;
        }
    }

    return 1;
}

int readDirective(Row row_array[], int row_counter, char** token, int* parsing_error)
{    
    char* p;
    int it, it2, argument_counter = 0;

    strcpy(row_array[row_counter].instruction, *token);                                                              // Przypisanie instrukcji do ostatniego Row
    *token = strtok(NULL, " \t\n");                                                                                  // Kolejny odcinek
    if (*token == NULL || ((*token)[0] == '/' && (*token)[1] == '/')) {
        *parsing_error = handleError(error_parsing_expected_values, row_counter + 1);                                // Brak argumentow - blad
        return 0;
    }
    it = 0;                                                                                                         // Zresetowanie iteratorow
    it2 = 0;                                                                                                        //
    if (isdigit((*token)[0])) {                                                                                        // Liczba - rezerwacja wielu komorek
        while (isdigit((*token)[it++]));                                                                               // Szukanie konca liczby
        if ((*token)[it - 1] != '*') {
            *parsing_error = handleError(error_parsing_expected_asterisk, row_counter + 1);                          // Sprawdzanie obecnosci gwiazdki. Przy braku - blad skladni
            return 0;
        }
        strncpy(row_array[row_counter].arguments[argument_counter], *token, it - 1);                                 // Przypisanie liczby do wskazanego argumentu (1) ostatniego Row
        argument_counter++;                                                                                         // Przejscie do kolejnego argumentu
    }
    it2 = it;                                                                                                       // Zresetowanie iteratorow
    p = &(*token)[it];                                                                                                 // Ustalenie poczatku kolejnego elementu instrukcji
    while (isalpha((*token)[it])) it++;                                                                                // Szukanie konca typu
    strncpy(row_array[row_counter].arguments[argument_counter], p, it - it2);                                       // Przypisanie liczby do wskazanego argumentu (1 lub 2) ostatniego Row
    argument_counter++;                                                                                             // Przejscie do kolejnego argumentu
    if (strcmp(row_array[row_counter].instruction, "DC") == 0) {                                                    // Jezeli instrukcja jest dyrektywa DC to sprawdz nawiasy        
        if ((*token)[it] != '(') {
            *parsing_error = handleError(error_parsing_expected_opening_bracket, row_counter + 1);                   // Sprawdzanie obecnosci nawiasu. Przy braku - blad skladni
            return 0;
        }
        it++;                                                                                                       // Ustawienie iteratora na poczatek liczby
        it2 = it;                                                                                                   // Zresetowanie iteratorow
        p = &(*token)[it];                                                                                             // Ustalenie poczatku kolejnego elementu instrukcji 
        if (p[0] == '-') it++;                                                                                      // Pomin sprawdzanie nastepnego warunku dla minusa
        if (!isdigit((*token)[it])) {
            *parsing_error = handleError(error_parsing_wrong_values, row_counter + 1);                               // Sprawdzanie czy w nawiasach jest liczba
            return 0;
        }
        while (isdigit((*token)[it]) || (*token)[it] == '.') it++;                                                        // Szukanie konca liczby (Uwazglednienie liczb zmiennoprzecinkowych)
        if ((*token)[it] != ')') {
            *parsing_error = handleError(error_parsing_expected_closing_bracket, row_counter + 1);                   // Sprawdzanie obecnosci nawiasu. Przy braku - blad skladni
            return 0;
        }
        strncpy(row_array[row_counter].arguments[argument_counter], p, it - it2);                                   // Przypisanie liczby do wskazanego argumentu (2 lub 3) ostatniego Row 
        if ((*token)[it + 1] != '\0') {
            *parsing_error = handleError(error_parsing_wrong_values, row_counter + 1);                               // Sprawdzanie czy cos znajduje sie po nawiasach. Jezeli tak - blad skladni
            return 0;
        }
    }
    else                                                                                                            // Jest to dyrektywa DS. Wystarczy sprawdzic czy po typie nie ma innych znakow
        if ((*token)[it] != '\0') {
            *parsing_error = handleError(error_parsing_wrong_values, row_counter + 1);
            return 0;
        }
    return 1;
}

int readOrderJump(Row row_array[], int row_counter, char** token, int* parsing_error)
{
    char* p;                                                                    // Wskaznik po elementach ciagu
    int it, it2, argument_counter = 0;                                          // Iteratory po ciagu znakow, argument_counter decyduje, do ktorej komorki wstawic fragment ciagu

    strcpy(row_array[row_counter].instruction, *token);                          // Zapisz rozkaz
    *token = strtok(NULL, " \t\n");
    if (*token == NULL || ((*token)[0] == '/' && token[1] == '/')) {                // Jezeli brak kolejnego argumentu. Oznacza blad.
        *parsing_error = handleError(error_parsing_expected_address, row_counter + 1);
        return 0;
    }
    if (isdigit((*token)[0]) || (*token)[0] == '-') {                                                    // Jezeli pierwszy znak jest cyfra. Oznacza, ze adres jest wyrazony poprzez [przesuniecie]([rejestr])
        it = 0;
        it2 = 0;
        if (!readAddress(row_array, row_counter, token, &it, &it2, &argument_counter, parsing_error))  return 0;   // Czytaj adres. Zwroc 0 jezeli niepowodzenie
    }
    else {                                                                      // Jezeli pierwszy znak nie jest cyfra. Oznacza, ze adres jest wyrazony poprzez etykiete
        strcpy(row_array[row_counter].arguments[argument_counter], *token);      // Zapisz argument (etykieta)
    }
    return 1;
}

int readOrderRM(Row row_array[], int row_counter, char** token, int* parsing_error)
{
    char* p;                                                                            // Wskaznik po elementach ciagu
    int it, it2, argument_counter = 0;                                                  // Iteratory po ciagu znakow, argument_counter decyduje, do ktorej komorki wstawic fragment ciagu

    strcpy(row_array[row_counter].instruction, *token);                                  // Zapisz rozkaz
    if (!readFirstRegister(row_array, row_counter, token, &it, &it2, &argument_counter, parsing_error))  return 0;         // Czytaj pierwszy rejestr
    it2 = it;
    p = &(*token)[it];    
    if (isdigit((*token)[it]) || (*token)[it] == '-') {                                                           // Jezeli znak jest cyfra. Oznacza, ze adres jest wyrazony poprzez [przesuniecie]([rejestr])
        if (!readAddress(row_array, row_counter, token, &it, &it2, &argument_counter, parsing_error))  return 0;           // Czytaj adres
    }
    else {                                                                              // Jezeli znak nie jest cyfra. Oznacza, ze adres jest wyrazony poprzez etykiete
        while ((*token)[it] != '\0') it++;                                    
        strncpy(row_array[row_counter].arguments[argument_counter], p, it - it2);       // Zapisz argument (etykieta)
    }
    return 1;
}

int readOrderRR(Row row_array[], int row_counter, char** token, int* parsing_error)
{
    char* p;                                                                            // Wskaznik po elementach ciagu
    int it, it2, argument_counter = 0;                                                  // Iteratory po ciagu znakow, argument_counter decyduje, do ktorej komorki wstawic fragment ciagu

    strcpy(row_array[row_counter].instruction, *token);                                  // Zapisz rozkaz
    if (!readFirstRegister(row_array, row_counter, token, &it, &it2, &argument_counter, parsing_error))  return 0;         //  Czytaj pierwszy rejestr
    if (!readSecondRegister(row_array, row_counter, token, &it, &it2, &argument_counter, parsing_error))  return 0;        //  Czytaj drugi rejestr
    return 1;
}

int readAddress(Row row_array[], int row_counter, char** token, int* it, int* it2, int* argument_counter, int* parsing_error)
{
    char* p;                                                                            // Wskaznik po elementach ciagu

    p = &(*token)[*it];          
    if ((*token)[*it] == '-') (*it)++;
    while (isdigit((*token)[*it])) (*it)++;                                                // Omin wszystkie cyfry przesuniecia
    if ((*token)[*it] != '(') {                                                            // Jezeli otwarcie nawiasu jest nieobecne. Oznacza blad skladni
        *parsing_error = handleError(error_parsing_expected_opening_bracket, row_counter + 1);
        return 0;
    }
    strncpy(row_array[row_counter].arguments[*argument_counter], p, *it - *it2);        // Zapisz argument (przesuniecie)
    (*it)++;
    (*argument_counter)++;
    *it2 = *it;
    p = &(*token)[*it];
    if (!isdigit((*token)[*it])) {                                                         // Jezeli rejestr nie zaczyna sie cyfra. Oznacza blad skladni
        *parsing_error = handleError(error_parsing_wrong_address, row_counter + 1);
        return 0;
    }
    while (isdigit((*token)[(*it)++]));                                                    // Omin wszystkie cyfry rejestru
    if ((*token)[*it - 1] != ')') {                                                        // Jezeli zamkniecie nawiasu jest nieobecne. Oznacza blad skladni
        *parsing_error = handleError(error_parsing_expected_closing_bracket, row_counter + 1);
        return 0;
    }
    strncpy(row_array[row_counter].arguments[*argument_counter], p, *it - *it2 - 1);    // Zapisz argument (rejestr)
    if ((*token)[*it] != '\0') {                                                           // Jezeli znaki po nawiasie. Oznacza blad skladni
        *parsing_error = handleError(error_parsing_wrong_address, row_counter + 1);
        return 0;
    }
    return 1;
}

int readFirstRegister(Row row_array[], int row_counter, char** token, int* it, int* it2, int* argument_counter, int* parsing_error) {
    int reg_num;

    *token = strtok(NULL, " \t\n");
    if (*token == NULL || ((*token)[0] == '/' && (*token)[1] == '/') || !isdigit((*token)[0])) {      // Jezeli brak kolejnego argumentu. Oznacza blad.
        *parsing_error = handleError(error_parsing_expected_register, row_counter + 1);
        return 0;
    }
    *it = 0;    
    while (isdigit((*token)[*it])) (*it)++;                                                     // Omin wszystkie cyfry numeru rejestru
    if ((*token)[*it] != ',') {                                                                 // Jezeli przecinek jest nieobecny. Oznacza blad skladni
        *parsing_error = handleError(error_parsing_expected_comma, row_counter + 1);
        return 0;
    }
    reg_num = atoi(*token);
    if (reg_num >15) {                                                           // Sprawdz czy poprawny rejestr
        *parsing_error = handleError(error_parsing_wrong_register, row_counter + 1);
        return 0;
    }
    strncpy(row_array[row_counter].arguments[*argument_counter], *token, *it);                  // Zapisz argument (rejestr)
    (*it)++;
    (*argument_counter)++;
    if ((*token)[*it] == '\0') {                                                                // Jezeli po przecinku wystapily spacje, przejdz do kolejnego tokena
        *token = strtok(NULL, " \t\n");
        if (*token == NULL || ((*token)[0] == '/' && (*token)[1] == '/')) {                     // Jezeli brak kolejnego argumentu. Oznacza blad
            *parsing_error = handleError(error_parsing_expected_address, row_counter + 1);
            return 0;
        }
        *it = 0;
    }
    return 1;
}

int readSecondRegister(Row row_array[], int row_counter, char** token, int* it, int* it2, int* argument_counter, int* parsing_error) {
    int reg_num;
    char* p;

    *it2 = *it;
    p = &(*token)[*it];
    if (!isdigit((*token)[*it])) {                                                          // Jezeli argument nie zaczyna sie od cyfry. Oznacza blad skladni
        *parsing_error = handleError(error_parsing_expected_register, row_counter + 1);
        return 0;
    }
    while (isdigit((*token)[*it])) (*it)++;                                                 // Omin wszystkie cyfry numeru rejestru
    reg_num = atoi(*token);
    if (reg_num >15) {                                                       // Sprawdz czy poprawny rejestr
        *parsing_error = handleError(error_parsing_wrong_register, row_counter + 1);
        return 0;
    }
    strncpy(row_array[row_counter].arguments[*argument_counter], p, *it - *it2);            // Zapisz argument (rejestr)
    if ((*token)[*it] != '\0') {                                                            // Jezeli znaki po cyfrach rejstru. Oznacza blad skladni
        *parsing_error = handleError(error_parsing_wrong_register, row_counter + 1);
        return 0;
    }
}
