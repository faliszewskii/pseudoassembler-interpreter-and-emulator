#include <stdio.h>
#include <stdlib.h>
#include "jump_order.h"
#include "constants.h"
#include "error_handling.h"
#include "execution.h"
#include "GUI.h"


int jumpOrder(Memory* memory, GUIInterface* guii) {
	int register_b_num; // Indeks rejestru.
	int* register_b; // Wskaznik na rejestr.
	char* status; // Wskaznik na rejestr stanu.

	register_b_num = *((char*)memory->instruction_register + 1) & 0b00001111;	// Pobranie indeksow rejestrow.
	guii->register_2 = register_b_num; // Zapisanie indeksu rejestru do interfejsu GUI.
	register_b = (int*)memory->registers[register_b_num];	// Pobranie rejsetru zawierajacego adres, od ktorego ma byc liczony offset.
	status = (char*)memory->status_register;	// Ustawienie wskaznika na rejestr stanu.

	switch ((*(char*)memory->instruction_register & 0b00001111)) {	// Rozpoznanie instrukcji skoku.
	case 0b00000000:		// Rozkaz J
		jumpTo(memory, guii, register_b);
		break;
	case 0b00000001:		// Rozkaz JZ
		if ((*status & 0b11) == 0b00)
			jumpTo(memory, guii, register_b);
		break;
	case 0b00000010:		// Rozkaz JP
		if ((*status & 0b11) == 0b01)
			jumpTo(memory, guii, register_b);
		break;
	case 0b00000011:		// Rozkaz JN
		if ((*status & 0b11) == 0b10)
			jumpTo(memory, guii, register_b);
		break;
	default:
		handleError(error_execution_unreadable_code, 0);
	}
	return 1;
}

int jumpTo(Memory* memory, GUIInterface* guii, int* address) {
	int dest;
	// Pobierz adres docelowy skoku
	dest = addOffset((char*)memory->order_section - (char*)memory->operational_memory, *address, loadOffset((char*)memory->instruction_register), (char*)memory->end_of_program - (char*)memory->operational_memory);
	if (dest == -1) { 
		*((char*)memory->status_register) = 0b11;
		return 0;
	}
	guii->line_diff_bytes += dest - *((int*)memory->status_register + 1); // Dodaj ró¿nicê adresów nowej instrukcji i 
	*((int*)memory->status_register + 1) = dest;
	return 1;
}
