#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "execution.h"
#include "constants.h"
#include "error_handling.h"
#include "arithmetic_operations.h"
#include "read_msck_code.h"
#include "GUI.h"

int execute(FILE* results_file, FILE* code_file, FILE* program_file, char file_path[], char file_name[], struct Label label_array[], int label_counter, char debug_mode) {
	int i;	// iterator
	Memory memory = { {NULL},NULL,NULL,NULL,NULL,NULL };	//	Wskazniki do pamieci wymaganej do symulacji wykonania programu
	GUIInterface guii;	// Interfejs GUI do interakcji z symulacja
	char var_file_name[MAX_FILE_NAME_LENGTH] = "\0";	// Kontener na nazwe pliku podczas tworzenia nowej nazwy pliku wyjsciowego

	if (!readMachineCode(program_file, &memory))				// Wczytaj kod maszynowy do pamieci.
		handleError(error_machine_code_reading_fail, 0);	

	strcat(var_file_name, file_name);
	strcat(var_file_name, "_var.txt");
	results_file = fopen(var_file_name, "w");
	if (!results_file)	handleError(error_unable_to_create_output_file, 0);

	setupRegisters(&memory);	// Przygotuj rejestry w pamieci do pracy
	if (debug_mode) setupWindow(code_file, program_file, file_path, &memory, &guii, label_array, label_counter);

	// Dopoki PC nie bedzie wskazywa na bajt konca kodu, wykonuj instrukcje
	while (*((int*)memory.status_register + 1) < (char*)memory.end_of_program - (char*)memory.operational_memory) {		
		
		if (debug_mode) {		// Tryb debug					
			if (!refreshGUI(&memory, &guii)) {	// Jezeli uzytkownik wcisnie klawisz wyjscia
				debug_mode = 0;
				break;	// Przerwij wykonanie symulacji
			}				
		}		
		memcpy(memory.instruction_register, (char*)memory.operational_memory + *((int*)memory.status_register + 1), 4);	// Przenies instrukcje do rejestru instrukcji
		pointToNextInstruction(&memory, &guii);
		executeInstruction(&memory, &guii);
		if (ifErrorStatus(&memory, &guii, &debug_mode))
			break;		
		if (debug_mode)	guii.step--;

	}
	if (debug_mode)	windowAfterEnd(&memory, &guii, var_file_name);
	saveResults(results_file, program_file, &memory);	// Zapisz wyniki do pliku _var
	printResults(&memory);	// Wydrukuj rejsetry i pamiec do konsoli

	freeMemory(&memory);	// Zwolnij pamiec operacyjna
	return 1;
}

void pointToNextInstruction(Memory* memory, GUIInterface* guii) {

	if ((*((char*)memory->instruction_register) & 0b11000000) == 0b00000000) {			// Dlugosc rozkazu - 2 bajty
		*((int*)memory->status_register + 1) += 2;
		guii->line_diff_bytes += 2;
	}
	else if ((*((char*)memory->instruction_register) & 0b11000000) == 0b11000000) {	// Dlugosc rozkazu - 4 bajty
		*((int*)memory->status_register + 1) += 4;
		guii->line_diff_bytes += 4;
	}
	else handleError(error_execution_unreadable_code, 0);

	return;
}

void executeInstruction(Memory* memory, GUIInterface* guii) {

	// Rozpoznanie instrukcji
	if ((*(char*)memory->instruction_register & 0b00110000) == 0b00010000)			// Grupa rozkazu - operacje arytmentyczne		
		arithmetic_operation(memory, guii);
	else if ((*(char*)memory->instruction_register & 0b00110000) == 0b00100000)	// Grupa rozkazu - instrukcje skoku
		jumpOrder(memory, guii);
	else if ((*(char*)memory->instruction_register & 0b00110000) == 0b00110000)	// Grupa rozkazu - zapisywanie wartosci
		transferOrder(memory, guii);
	else handleError(error_execution_unreadable_code, 0);

	return;
}

int ifErrorStatus(Memory* memory, GUIInterface* guii, char* debug_mode) {

	if (*((char*)memory->status_register) == 0b11) {	// Jezeli rejestr stanu 11 (Error)
		if (*debug_mode) {	// Jezeli tryb debug
			clearCard();	// Wypisz informacje o bledzie na karcie
			printOnCard("PSEUDOASSEMBLER INTERPRETER\nProhibited operation ( tried to access address out of bounds of a section, overflow or division by zero ).\n");
			printOnCard("Stopping simulation.\nUse 'q' and 'e' to navigate the headers. Use 'w', 'a', 's', 'd' to interact with the selected panel.\nPress 'x' to exit.\n");
			guii->end = 1;	// Ustaw flage zakonczenia symulacji na true
			refreshGUI(memory, guii);	// Ostatnia petla interakcji
			*debug_mode = 0;
		}
		else {
			printf("Prohibited operation (tried to access address out of bounds of a section, overflow or division by zero).\nStopping simulation...\n");
			system("pause");
		}
		return 1;
	}
	return 0;
}

void saveResults(FILE* results_file, FILE* code_file, Memory* memory) {
	char line[MAX_MSC_INSTR_LENGTH];
	char* p;
	int i, value;

	p = memory->data_section;
	while (fgets(line, MAX_MSC_INSTR_LENGTH, code_file) != NULL) {	// Pobieraj kolejne linie do napotkania konca pliku				
		if (line[0] == '~') {		// Jezeli natrafisz na slowo niezdefiniowane, zastap je nowa wartoscia
			value = loadValue(p);
			for (i = 3; i > 0; i--)                                             // Wypisz bajty po kolei
				fprintf(results_file, "%02X ", (value >> i * BYTE) & LAST_BYTE_MASK);
			fprintf(results_file, "%02X\n", value & LAST_BYTE_MASK);
		}
		else {
			fputs(line, results_file);	// W przeciwnym wypadku po prostu wypisz linie
		}
		if (line[0] == '\n')	break;	// Koniec sekcji danych
		p += BYTES_FOR_VALUE;	// wskaznik na kolejna komorke pamieci w sekcji danych
	}
	while (fgets(line, MAX_MSC_INSTR_LENGTH, code_file) != NULL) 	// Pobieraj kolejne linie do napotkania konca pliku		
		fputs(line, results_file); // Wypisz wszystkie linie sekcji rozkazow bez zmian

	return;
}

void windowAfterEnd(Memory* memory, GUIInterface* guii, char var_file_name[]) {

	clearCard();	// Wypisz informacje o poprawnym zakonczeniu wykanania symulacji
	printOnCard("PSEUDOASSEMBLER INTERPRETER\nSimulation completed successfully.\n\n");
	printOnCard("Use 'q' and 'e' to navigate the headers. Use 'w', 'a', 's', 'd' to interact with the selected panel.\nPress 'x' to exit.\n");
	guii->end = 1;	// Ustaw flage zakonczenia symulacji na true
	refreshGUI(memory, guii);	// Ostatnia petla interakcji	
	
	return;
}

void printResults(Memory* memory) {
	int i;
	char* p;

	printf("Registers:\n");
	for (i = 0; i < 16; i++) {
		p = (int*)memory->registers[i];
		printf("%2d : %02hhX %02hhX %02hhX %02hhX - %d\n", i, *p, *(p + 1), *(p + 2), *(p + 3), *(int*)p);
	}
	printf("Memory:\n");
	for (i = 0, p = memory->data_section; p < memory->order_section; i+=4, p += 4)
		printf("%d : %02hhX %02hhX %02hhX %02hhX - %d\n", i, *p, *(p + 1), *(p + 2), *(p + 3), loadValue((int*)p));

	system("pause");

	return;
}

short loadOffset(char* instruction) {	// Zwrocenie liczby reprezentujacej offset z dwoch ostatnich bajtow instrukcji
	short offset = 0;

	offset += (*(instruction + 2) & 0xff);
	offset <<= BYTE;
	offset += (*(instruction + 3) & 0xff);

	return offset;
}

int loadValue(char* instruction) {	// Zwrocenie liczby reprezentujacej wartosc w pamieci zawartej na czterech bajtach w danym adresie w pamieci
	int value = 0;

	value += (*(instruction) & 0xff);
	value <<= BYTE;
	value += (*(instruction + 1) & 0xff);
	value <<= BYTE;
	value += (*(instruction + 2) & 0xff);
	value <<= BYTE;
	value += (*(instruction + 3) & 0xff);
	return value;
}

int getValue(Memory* memory, GUIInterface* guii, int* p_address) {	// Zwrocenie liczby reprezentujacej wartosc w pamieci z adresu znajdujacego sie w rejestrze
	int address;

	address = addOffset(0 , *p_address, loadOffset((char*)memory->instruction_register), (char*)memory->order_section - (char*)memory->operational_memory );
	if (address == -1) {
		*((char*)memory->status_register) = 0b11;
		return 0;
	}
	guii->memory_cell = address/4;
	return loadValue((char*)memory->operational_memory + address);
}

int addOffset(int beginning, int address, int offset, int end) {	// Przesuniecie adresu o offset
	if (address + offset > end || address + offset < beginning) {		// Wyjscie poza pamiec		
		return -1;
	}
	return address + offset;
}
