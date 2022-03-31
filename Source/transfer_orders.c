#include <stdio.h>
#include <stdlib.h>
#include "transfer_orders.h"
#include "constants.h"
#include "error_handling.h"
#include "execution.h"
#include "GUI.h"

int transferOrder(Memory* memory, GUIInterface* guii) {
	int register_a_num, register_b_num;	// Indeksy rejstrow, na ktorych dziala operacja
	int* register_a, * register_b; // wskazniki na rejestry, na ktorych dziala operacja.

	register_a_num = (*((char*)memory->instruction_register + 1) & 0b11110000) >> 4;	// Pobranie indeksow rejestrow
	register_b_num = *((char*)memory->instruction_register + 1) & 0b00001111;
	guii->register_1 = register_a_num;	// Zapisanie indeksow rejestrow do interfejsu GUI
	guii->register_2 = register_b_num;
	register_a = (int*)memory->registers[register_a_num];	// Pobranie wskaznikow na rejstry.
	register_b = (int*)memory->registers[register_b_num];

	switch ((*(char*)memory->instruction_register & 0b00001111)) {	// Rozpoznanie operacji.
	case 0b00000000:		// Rozkaz L
		load(register_a, getValue(memory, guii, register_b));
		break;
	case 0b00000001:		// Rozkaz LR
		load(register_a, *register_b);
		break;
	case 0b00000010:		// Rozkaz LA
		loadAddress(memory, guii, register_a, register_b);
		break;
	case 0b00000011:		// Rozkaz ST
		store(memory, guii, register_a, register_b);
		break;
	default:
		handleError(error_execution_unreadable_code, 0);
	}
	return 1;
}

int load(int* dest, int value) {	// Przypisuje wskaznikowi wartosc.

	*dest = value;
	return value;
}

int loadAddress(Memory* memory, GUIInterface* guii, int* dest, int* p_address) { // Przypisuje wskaznikowi adres .
	int address;
	address = addOffset(0, *p_address, loadOffset((char*)memory->instruction_register), (char*)memory->order_section - (char*)memory->operational_memory);
	if (address == -1) {
		*((char*)memory->status_register) = 0b11;
		return 0;
	}
	guii->memory_cell = address/4;
	*dest = address;
	return 1;
}

int store(Memory* memory, GUIInterface* guii, int* src, int* p_address) {	// Zapisuje w pamieci wartosc ze wskaznika.
	int address;
	address = addOffset(0 , *p_address, loadOffset((char*)memory->instruction_register), (char*)memory->order_section - (char*)memory->operational_memory);
	if (address == -1) {
		*((char*)memory->status_register) = 0b11;
		return 0;
	}
	guii->memory_cell = address/4;
	storeValue(address + (char*)memory->operational_memory, *src);
	return 1;
}

void storeValue(char* dest, int value) { // Zapisanie liczby int jako wartosc reprezentujaca ja w pamieci.

	*(int*)dest = 0;
	*(dest + 3) = value & 0xff;
	value >>= BYTE;
	*(dest + 2) += value & 0xff;
	value >>= BYTE;
	*(dest + 1) += value & 0xff;
	value >>= BYTE;
	*(dest) += value & 0xff;

	return;
}
