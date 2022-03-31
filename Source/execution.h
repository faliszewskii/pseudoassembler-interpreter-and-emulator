#pragma once
#include "GUI.h"

typedef struct Memory {
	void* registers[16];		// Tablica wskaznikow do rejestrow.
	void* status_register;		// 2 slowa na adres rozkazu i status.
	void* program_counter;		// Wskaznik na pierwszy bajt kolejnego rozkazu.
	void* operational_memory;	// Wzkaznik na pierwszy bajt pamieci operacyjnej programu.
	void* data_section;			// Wzkaznik na pierwszy bajt sekcji danych w pamieci operacyjnej programu.
	void* order_section;		// Wzkaznik na pierwszy bajt sekcji rozkazow w pamieci operacyjnej programu.
	void* end_of_program;		// Wzkaznik na ostatni bajt pamieci operacyjnej programu.
	void* instruction_register;	// Wskaznik na rejestr przetrzymujacy aktualnie wykonywana instrukcje.
} Memory;

int execute(FILE*, FILE*, FILE*, char[], char[], struct Label[], int, char);	// Obsluguje symulacje programu.
void pointToNextInstruction(Memory*, struct GUIInterface*);				// Ustawia drugi bajt rejestru stanu na nowa instrukcje.
void executeInstruction(Memory*, struct GUIInterface*);					// Wykonuje instrukcje z rejestru instrukcji.
int ifErrorStatus(Memory*, struct GUIInterface*, char*);					// Obsluzenie wartosci 11 w pierwszym bajcie rejetru stanu.
void windowAfterEnd(Memory*, struct GUIInterface*, char[]);				// Wyswietlenie komunikatu o poprawnym ukonczeniu symulacji.

void saveResults(FILE*, FILE*, Memory*);		// Zapisanie wyniku w pliku "*_var.txt".
void printResults(Memory*);						// Wypisanie w konsoli stanu rejestrow i komorek pamieci.

short int loadOffset(char*);							// Interpretacji wartosci offsetu.
int loadValue(char*);									// Interpretacja wartosci komorki.
int getValue(Memory*, struct GUIInterface*, int*);		// Pobranie wartosci z komorki o danym adresie.
int addOffset(int, int, int, int);						// Uzglednienie offsetu.