#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "read_msck_code.h"
#include "constants.h"
#include "error_handling.h"
#include "execution.h"


int readMachineCode(FILE* code_file, Memory* memory) {

	int bytes_for_data = 0, bytes_for_orders = 0, c_words_to_randomize = 0;	// Ilosc bajtow na sekcje danych, ilosc bajtow na sekcje rozkazow, ilosc slow z wartoscia niezdefiniowana.
	char* p = NULL;
	int* p_words_to_randomize = NULL;	// Tablica indeksow elementow niezdefiniowanych.

	countBytes(code_file, &bytes_for_data, &bytes_for_orders, &c_words_to_randomize, &p_words_to_randomize);	// Liczenie bajtow.
	
	p = setUpMemory(memory, bytes_for_data, bytes_for_orders);	// Alokacja pamieci na podstawie ilosci policzonych bajtow.
	
	interpretValues(code_file, p);	// Wczytanie wartosci z kodu do pamieci.

	randomizeUndefinedValues(memory, c_words_to_randomize, p_words_to_randomize);	// Losowanie wartosci niezdefiniowanych.

	return 1;
}

void countBytes(FILE* code_file, int* bytes_for_data, int* bytes_for_orders, int* n, int** p_words) {
	char instruction[MAX_MSC_INSTR_LENGTH];	// Ciag znakowy do pobrania instrukcji.
	char byte[3];	// Ciag znakowy do pomieszczenia bajtu w postaci szesnastkowej.
	int* p_buffor=NULL;	//	Tymczasowy bufor do realokacji pamieci.

	while (fgets(instruction, MAX_MSC_INSTR_LENGTH, code_file) != NULL) {	// Pobieraj kolejne linie do napotkania konca pliku.
		if (instruction[0] == '\n') break;	// Zakoncz czytanie sekcji danych gdy natrafisz na pusta linie.
		if (instruction[0] == '~') {		// Jezeli natrafisz na slowo niezdefiniowane, zapamietaj jego polozenie.
			(*n)++;
			p_buffor = realloc(*p_words, sizeof(int) * *n);
			if (p_buffor == NULL)
				handleError(error_execution_memory_allocation_fail, 0);
			*p_words = p_buffor;
			(*p_words)[*n - 1] = *bytes_for_data / BYTES_FOR_VALUE;
		}
		*bytes_for_data += BYTES_FOR_VALUE;	// Dodaj 4 bajty do licznika bajtow sekcji danych.
	}
	while (fscanf(code_file, "%s", byte) != EOF)	(*bytes_for_orders)++;	// Licz ciagi znakow oddzielone spacja do konca pliku. 
	rewind(code_file);	// Wroc do poczatku pliku by przygotowac go do interpretacji wartosci.
	return;
}

char* setUpMemory(Memory* memory, int bytes_for_data, int bytes_for_orders) {
	char* p = NULL;	// Tymczasowy wskaznik.

	p = malloc(sizeof(char) * (bytes_for_data + bytes_for_orders + 1));	// Rezerwuje ilosc bajtow potrzebnych na kod oraz dodatkowy bajt na oznaczenie konca kodu.
	if (p == NULL)	handleError(error_execution_memory_allocation_fail, 0);
	memory->operational_memory = (char*)p;															// Ustaw wskaznik na poczatek pamieci.
	memory->data_section = memory->operational_memory;												// Ustaw wskaznik na poczatek sekcji danych.
	memory->order_section = (char*)memory->operational_memory + bytes_for_data;						// Ustaw wskaznik na poczatek sekcji rozkazow.
	memory->end_of_program = (char*)memory->operational_memory + bytes_for_data + bytes_for_orders;	// Ustaw wskaznik na ostatni bajt w pamieci.

	return p;
}

void setupRegisters(Memory* memory) {
	int i;	// Iterator.

	// Rezerwacja pamieci dla rejestrow/
	for (i = 0; i < 16; i++) {
		memory->registers[i] = malloc(sizeof(int));
		if (memory->registers[i] == NULL) handleError(error_execution_memory_allocation_fail, 0);
		*(int*)memory->registers[i] = 0;
	}
	memory->status_register = malloc(sizeof(int) * 2);
	if (memory->status_register == NULL) handleError(error_execution_memory_allocation_fail, 0);
	memory->instruction_register = malloc(sizeof(int));
	if (memory->instruction_register == NULL) handleError(error_execution_memory_allocation_fail, 0);

	*((int*)memory->status_register) = 0;	// Wyzerowanie rejestru stanu
	*((int*)memory->status_register + 1) = (char*)memory->order_section - (char*)memory->operational_memory;	// Ustawienie adresu wykonywanego rozkazu na poczatek listy rozkazow.
	*(int*)memory->registers[REGISTER_WITH_ADDRESS_TO_ORDER_SECTION] = (int)memory->order_section - (int)memory->data_section;	// Ustawienie adresu sekcji rozkazow w rejestrze przeznaczonym do przechowywania adresu sekcji rozkazow.
	*(int*)memory->registers[REGISTER_WITH_ADDRESS_TO_DATA_SECTION] = 0;	// Ustawienie adresu sekcji danych w rejestrze przeznaczonym do przechowywania adresu sekcji danych.

	return;
}

void freeMemory(Memory* memory) {
	int i;	// Iterator.

	// Zwolnienie zarezerwowanej pamieci.	
	free(memory->operational_memory);
	free(memory->status_register);
	free(memory->instruction_register);
	for (i = 0; i < 16; i++)	free(memory->registers[i]);

	return;
}

void interpretValues(FILE* code_file, char* p) {
	char byte[3];	// Ciag znakowy do pomieszczenia bajtu w postaci szesnastkowej.

	while (fscanf(code_file, "%s", byte) != EOF) {	// Czytaj bajty oddzielone spacjami do konca pliku.
		if (byte[0] == '~');	// Pomin bajty niezdefiniowane.
		else {
			*p = xtoi(byte);	// Zapisz w pamieci bajt w postaci char.
		}
		p++;	// Przejscie do kolejnej komorki.
	}
	rewind(code_file); // Wroc do poczatku pliku by przygotowac do zapisywania wyniku.

	return;
}

void randomizeUndefinedValues(Memory* memory, int n, int* p_words) {
	int i;	// Iterator.

	srand(time(NULL));	// Ustal ziarno. 
	for (i = 0; i < n; i++) // Zapisz wylosowana liczbe do pamieci w kolejnych bajtach oznaczonych jako niezdefiniowane.
		storeValue(((int*)memory->data_section + p_words[i]), (rand() % (RANDOMISER_RANGE_UP + 1 - RANDOMISER_RANGE_LOW) + RANDOMISER_RANGE_LOW));

	return;
}

unsigned int xtoi(char* hex) {	// Algorytm do przeksztalcania liczby HEX w DEC.
	unsigned int dec = 0; // Liczba dziesietna.

	if ((hex[0] == '0') && ((hex[1] == 'x') || hex[1] == 'X'))	// Pominiecie ewentualnego prefixu 0x, 0X.
		hex += 2;
	while (*hex != '\0') {
		char c = toupper(*hex++);
		if ((c < '0') || (c > 'F') || ((c > '9') && (c < 'A'))) // Niedozwolone znaki, wiec koniec liczby.
			break;
		c -= '0'; // Uzyskaj cyfre.
		if (c > 9)	// Poza cyframi to przejdz na na odpowiedniki liter.
			c -= 7;
		dec = (dec << 4) + c;
	}
	return dec;
}