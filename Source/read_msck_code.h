#pragma once
#include "execution.h"

int readMachineCode(FILE*, Memory*);							// Obsluguje czytanie kodu maszynowego i rezerwacje pamieci na symulacje

void countBytes(FILE*, int*, int*, int*, int**);				// Liczenie bajtow, ktore trzeba azarezerwowac dla symulacji
char* setUpMemory(Memory*, int, int);							// Rezerwacja odpowiedniej ilosci pamieci oraz ustalenie wartosci poczatkowych
void freeMemory(Memory*);										// Zwolnienie zarezerwowanej pamieci
void setupRegisters(Memory*);									// Rezerwacja pamieci dla rejestrow
void interpretValues(FILE*, char*);								// Zinterpretuj wartosci w kodzie maszynowym na ciag bitowy
void randomizeUndefinedValues(Memory*, int, int*);				// Ustaw niezdefiniowane wartosci w kodzie maszynowym na wartosci losowe

unsigned int xtoi(char*);	// Konwersja z systemu szesnastkowego na dziesietny