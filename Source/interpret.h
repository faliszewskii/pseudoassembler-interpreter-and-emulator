
#pragma once
#include "constants.h"

typedef struct Row          // Reprezentuje jeden wiersz zapisany w pseudoassemblerze
{
    char label[MAX_NAME_LENGTH+1];
    char instruction[3];
    char arguments[3][MAX_LENGTH];
} Row;

typedef struct Label        // Reprezentuje etykiete
{
    char label_name[MAX_NAME_LENGTH+1];
    int reg;
    int label_interpretation;
} Label;

enum Stage {        // Wskazuje na etap podczas analizy programu
    directives_stage,
    orders_stage,
    end_of_program
};

FILE* interpret(FILE*, char[], Label**, int*);                  // Interpetuje kod zapisany w pseudoassemblerze. Zwraca plik z kodem maszynowym

int loadLabels(Row[], int*, Label**, int*);                     // Wczytuje do tablicy etykiety i je interpretuje. Zwraca wartosc logiczna czy pomyslnie

int expandLabelArray(Label**, int);                             // Rozszerza tablice trzymajaca informacje o etykietach

int interpretDirectives(FILE*, Row[], int*);                    // Wpisuje do pliku interpretacje sekcji danych. Zwraca wiersz, na ktorym zakonczyla sie sekcja danych

int interpretOrders(FILE*, Row[], int*, Label[], int*, int);    // Wpisuje do pliku interpretacje sekcji rozkazow. Zwraca wartosc logiczna czy pomyslnie

char* getOrderCode(char*);                                      // Zwraca kod rozkazu w stringu
Label* getLabel(Label[], int, char*);                           // Zwraca etykiete znajdywana poprzez nazwe

int isDirective(char*);     // Zwraca wartosc logiczna czy instrukcja jest dyrektywa
int isOrderRR(char*);       // Zwraca wartosc logiczna czy instrukcja jest rozkazem rejestr-rejestr
int isOrderRM(char*);       // Zwraca wartosc logiczna czy instrukcja jest rozkazem rejestr-pamiec
int isOrderJump(char*);     // Zwraca wartosc logiczna czy instrukcja jest rozkazem skoku

