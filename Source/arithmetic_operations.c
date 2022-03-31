#include <stdio.h>
#include <stdlib.h>
#include "arithmetic_operations.h"
#include "constants.h"
#include "error_handling.h"
#include "execution.h"
#include "GUI.h"

int arithmetic_operation(Memory* memory, GUIInterface* guii) {

	int result = 0;	// Wynik operacji.
	int register_a_num, register_b_num;	// Indeksy rejstrow.
	int* register_a, * register_b; // wskazniki na rejestry, na ktorych dziala operacja.
	char *status;	// wskaznik na rejestr stanu.

	register_a_num = (*((char*)memory->instruction_register + 1) & 0b11110000) >> 4; // Pobranie indeksow rejestrow.
	register_b_num = *((char*)memory->instruction_register + 1) & 0b00001111;
	guii->register_1 = register_a_num;	// Zapisanie indeksow rejestrow do interfejsu GUI.
	guii->register_2 = register_b_num;
	register_a = (int*)memory->registers[register_a_num];	// Pobranie wskaznikow na rejstry.
	register_b = (int*)memory->registers[register_b_num];

	status = (char*)memory->status_register;	// Ustawienie wskaznika na rejestr stanu.

	switch ((*(char*)memory->instruction_register & 0b00001111)) {	// Rozpoznanie operacji arytmetycznej.
	case 0b00000000:		// Operacja AR			
		*register_a = result = add(*register_a, *register_b, status);	// Wykonanie operacji i jednoczesne przypisanie jej do rejestru.
		break;
	case 0b00000001:		// Operacja A
		*register_a = result = add(*register_a, getValue(memory, guii, register_b), status);
		break;
	case 0b00000010:		// Operacja SR
		*register_a = result = subtract(*register_a, *register_b, status);
		break;
	case 0b00000011:		// Operacja S
		*register_a = result = subtract(*register_a, getValue(memory, guii, register_b), status);
		break;
	case 0b00000100:		// Operacja MR
		*register_a = result = multiply(*register_a, *register_b, status);
		break;
	case 0b00000101:		// Operacja M
		*register_a = result = multiply(*register_a, getValue(memory, guii, register_b), status);
		break;
	case 0b00000110:		// Operacja DR
		*register_a = result = divide(*register_a, *register_b, status);
		break;
	case 0b00000111:		// Operacja D
		*register_a = result = divide(*register_a, getValue(memory, guii, register_b), status);
		break;
	case 0b00001000:		// Operacja CR
		result = subtract(*register_a, *register_b, status);	// Wykonanie operacji odejmowania bez przypisania wartosci do rejestru.
		break;
	case 0b00001001:		// Operacja C
		result = subtract(*register_a, getValue(memory, guii, register_b), status);
		break;
	default:
		handleError(error_execution_unreadable_code, 0);
	}
	//	Ustalenie wyniku operacji.
	if (*status == 0b11)	return 1;
	if (result > 0)
		*status = 0b01;
	else if (result == 0)
		*status = 0b00;
	else
		*status = 0b10;
	
	return 1;
}

int add(int a, int b, char* status) {	// Operacja dodawania.
	if ((a > 0 && b > INT_MAX - a) || (a < 0 && b < INT_MIN - a)) {	// Sprawdzanie overflow.
		*status = 0b11;
		return 0;
	}
	return a + b;
}

int subtract(int a, int b, char* status) {	// Operacja odejmowania.
	if ((a > 0 && b < INT_MIN + a) || (a < 0 && b-1 > INT_MAX + a))	{ // Sprawdzanie overflow.
		*status = 0b11;
		return 0;
	}
	return a - b;
}

int multiply(int a, int b, char* status) {	// Operacja mnozenia.
	if ((b != 0 && (a > INT_MAX / b || a < INT_MIN / b)) || (b == -1 && a == INT_MIN) || (a == -1 && b == INT_MIN)) { // Sprawdzanie overflow.
		*status = 0b11;
		return 0;
	}
	return a * b;
}

int divide(int a, int b, char* status) {	// Operacja dzielenia.
	if ((a == -1 && b == INT_MIN) || (b == -1 && a == INT_MIN) || b == 0) {// Sprawdzanie overflow. Sprawdzanie dzielenia przez zero.
		*status = 0b11;
		return 0;
	}
	return a / b;
}