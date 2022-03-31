#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "execution.h"
#include "constants.h"
#include "error_handling.h"
#include "arithmetic_operations.h"


//	Opsiac uklad sterujacy (Te dodatkowe rejstry w sensie)

//	TODO
//	Ogarnac dzialanie rejestru stanu
//  Test na overflow
//	Uzupelnic operacje arytmetyczne r-r
//	Zaprojektowac operacje arytmetyczne r-p
//	Zaprojektowac operacje arytmetyczne porownania
//	Rozkazy zapisywania	
//  Funkcja RozkazRM(*funkcjaDzialania)

// ZMIENNE GLOBALNE ZAMIENIC NA LOKALNES (W strukture np)
Memory memory = {
	{NULL},		// Tablica wskaznikow do rejestrow
	NULL,		// 2 slowa na rozkaz i status
	NULL,		// Wskaznik na pierwszy bajt kolejnego rozkazu
	NULL,	// Wzkaznik na pierwszy bajt pamieci operacyjnej programu
	NULL,			// Wzkaznik na pierwszy bajt sekcji danych w pamieci operacyjnej programu
	NULL,			// Wzkaznik na pierwszy bajt sekcji rozkazow w pamieci operacyjnej programu
	NULL		// Wzkaznik na ostatni bajt pamieci operacyjnej programu
};
//*instruction_register = NULL;

int run(FILE* results, FILE* code_file, char debug_mode){

	int i;

	if (!readMachineCode(code_file))//, &operational_memory, &data_section, &order_section))
		handleError(error_machine_code_reading_fail, 0);

	// Funkcja Setup registers
	for (i = 0; i < 16; i++)	memory.registers[i] = malloc(sizeof(int));
	memory.status_register = malloc(sizeof(int) * 2);
	*((int*)memory.status_register + 1) = 0;
	memory.program_counter = memory.order_section;
	*(int*)memory.registers[REGISTER_WITH_ADDRESS_TO_ORDER_SECTION] = (int)memory.order_section;
	*(int*)memory.registers[REGISTER_WITH_ADDRESS_TO_DATA_SECTION] = (int)memory.data_section;
	
	while (memory.program_counter < memory.end_of_program) {
		memcpy(memory.status_register, memory.program_counter, 4);
		printf("Rozkaz: %x\n", *(int*)memory.status_register);

		if ((*((char*)memory.status_register) & 0b11000000) == 0b00000000)			// Dlugosc rozkazu - 2 bajty
			(char*)memory.program_counter += 2;
		else if ((*((char*)memory.status_register) & 0b11000000) == 0b11000000)	// Dlugosc rozkazu - 4 bajty
			(char*)memory.program_counter += 4;
		else exit(EXIT_FAILURE);	// TODO Obsluzenie bledu

		if ((*(char*)memory.status_register & 0b00110000) == 0b00010000)			// Grupa rozkazu - operacje arytmentyczne
		{
			arithmetic_operation();
			printf("Znak wyniku operacji: %x\n", *((int*)memory.status_register + 1));
		}
		else if ((*(char*)memory.status_register & 0b00110000) == 0b00100000)	// Grupa rozkazu - instrukcje skoku
			jumpOrder();
		else if ((*(char*)memory.status_register & 0b00110000) == 0b00110000)	// Grupa rozkazu - zapisywanie wartosci
			transferOrder();
		
		printf("Rejestr 0: %d\n", *(int*)memory.registers[0]);
	}

	printf("Rejestry:\n");
	for (int i = 0; i < 14; i++) {
		printf("%d: %d\n", i, *(int*)memory.registers[i]);
	}
	for(char* p = memory.data_section;p< memory.order_section;p+=4)
		printf("%hhx %hhx %hhx %hhx\n",*p,*(p+1),*(p+2),*(p+3));

	free(memory.operational_memory);
	free(memory.status_register);
	for (i = 0; i < 14; i++)	free(memory.registers[i]);

	return 1;
}



int readMachineCode(FILE* code_file) { //, char** operational_memory, char** data_section_pointer, char** order_section_pointer) {

	int bytes_for_data = 0, bytes_for_orders = 0, i;
	char instruction[MAX_MSC_INSTR_LENGTH], *token, byte[3];
	char num;
	char* p = NULL;	
	int iterator_do_losowania_liczb = 0;
	int* p_words_to_randomize = NULL;
	int c_words_to_randomize = 0;

	srand(time(NULL));

	while (fgets(instruction, MAX_MSC_INSTR_LENGTH, code_file) != NULL) {
		if (instruction[0] == '\n') break;
		if (instruction[0] == '~') {
			c_words_to_randomize++;
			p_words_to_randomize = realloc(p_words_to_randomize, sizeof(int) * c_words_to_randomize);
			p_words_to_randomize[c_words_to_randomize - 1] = bytes_for_data / BYTES_FOR_VALUE;
		}
		bytes_for_data += BYTES_FOR_VALUE;
	}
	while (fscanf(code_file, "%s", byte) != EOF)	bytes_for_orders++;

	p = malloc(sizeof(char) * (bytes_for_data + bytes_for_orders + 1));	// Rezerwuje ilosc bajtow potrzebnych na kod oraz dodatkowy bajt na oznaczenie konca kodu
	memory.operational_memory = (char*)p;
	memory.data_section = memory.operational_memory;
	memory.order_section = (char*)memory.operational_memory + bytes_for_data;
	memory.end_of_program = (char*)memory.operational_memory + bytes_for_data + bytes_for_orders;

	rewind(code_file);
	while (fscanf(code_file, "%s", byte) != EOF) {
		if (byte[0] == '~') {/*printf("Puste: ")*/	// Zostaw smieci albo wygeneruj losowa		SPYTAC SIE!
			if ((iterator_do_losowania_liczb % 4) == 2) *p = rand() % 6;// TODO Ulpeszyc losowanie liczb
			else if ((iterator_do_losowania_liczb % 4) == 3) *p = rand() % 256+0; else *p = 0; // TODO Ulpeszyc losowanie liczb
		}
		else {
			*p = xtoi(byte);
		}
		//printf("%d\t%s\n", (*p), byte);
		p++;
		iterator_do_losowania_liczb++;
	}
	

	for (i = 0; i < c_words_to_randomize; i++) {
		//storeValue(((int*)memory.data_section + p_words_to_randomize[i]), (rand() % (RANDOMISER_RANGE_UP + 1 - RANDOMISER_RANGE_LOW) + RANDOMISER_RANGE_LOW));
		printf("%d\n", loadValue((int*)memory.data_section + p_words_to_randomize[i]));
	}


	return 1;
}

unsigned int xtoi(char* hexstring)
{
	unsigned int	i = 0;

	if ((*hexstring == '0') && ((*(hexstring + 1) == 'x') || *(hexstring + 1) == 'X'))
		hexstring += 2;
	while (*hexstring) {
		char c = toupper(*hexstring++);
		if ((c < '0') || (c > 'F') || ((c > '9') && (c < 'A')))
			break;
		c -= '0';
		if (c > 9)
			c -= 7;
		i = (i << 4) + c;
	}
	return i;
}

unsigned int loadOffset(char* instruction) {
	unsigned int offset = 0;

	offset += (*(instruction + 2) & 0xff);
	offset <<= BYTE;
	offset += (*(instruction + 3) & 0xff);

	/**(int*)status_register = 0;	
	*(int*)status_register += *program_counter;
	*(int*)status_register <<= BYTE;
	*(int*)status_register += *(program_counter + 1);
	*(int*)status_register <<= BYTE;
	*(int*)status_register += *(program_counter + 2);
	*(int*)status_register <<= BYTE;
	*(int*)status_register += *(program_counter + 3);
	*/
	return offset;
}

int loadValue(char* instruction) {
	unsigned int value = 0;

	value += (*(instruction) & 0xff);
	value <<= BYTE;
	value += (*(instruction + 1) & 0xff);
	value <<= BYTE;
	value += (*(instruction + 2) & 0xff);
	value <<= BYTE;
	value += (*(instruction + 3) & 0xff);
	return value;
}

void storeValue(char* dest, int value) {

	*(dest + 3) = value & 0xff;
	printf("%x ", value & 0xff);
	value >>= BYTE;
	*(dest + 2) += value & 0xff;
	printf("%x ", value & 0xff);
	value >>= BYTE;
	*(dest + 1) += value & 0xff;
	printf("%x ", value & 0xff);
	value >>= BYTE;
	*(dest) += value & 0xff;
	printf("%x\n", value & 0xff);
	return;
}


int jumpOrder() {
	int* b;
	unsigned int offset;

	b = (int*)memory.registers[*((char*)memory.status_register + 1) & 0b00001111];
	offset = loadOffset((char*)memory.status_register);

	switch ((*(char*)memory.status_register & 0b00001111)) {
	case 0b00000000:		// Rozkaz J
		jumpTo(b,offset);
		break;
	case 0b00000001:		// Rozkaz JZ
		if ((*((int*)memory.status_register + 1) & 0b11) == 0b00)
			jumpTo(b, offset);
		break;
	case 0b00000010:		// Rozkaz JP
		if ((*((int*)memory.status_register + 1) & 0b11) == 0b01)
			jumpTo(b, offset);
		break;
	case 0b00000011:		// Rozkaz JN
		if ((*((int*)memory.status_register + 1) & 0b11) == 0b10)
			jumpTo(b, offset);
		break;
	default:	// TODO Nieobslugiwany kod operacji
		;
	}
	return 1;
}

int jumpTo(int* address, unsigned int offset) {

	char* dest;
	if ((dest = fetchAddress((int*)*address, offset, memory.end_of_program)) == NULL)	return 0;
	memory.program_counter = dest;
	printf("Skok do instrukcji na bajcie: %d\n", offset );
	return 1;
}

int transferOrder() {
	int result;
	unsigned int offset;
	int *a, *b;

	offset = loadOffset((char*)memory.status_register);
	a = (int*)memory.registers[(*((char*)memory.status_register + 1) & 0b11110000) >> 4];
	b = (int*)memory.registers[*((char*)memory.status_register + 1) & 0b00001111];

	switch ((*(char*)memory.status_register & 0b00001111)) {
	case 0b00000000:		// Rozkaz L
		*a = executeRMOrder(a, b, offset, load);
		break;
	case 0b00000001:		// Rozkaz LR
		load(a, *b);
		break;
	case 0b00000010:		// Rozkaz LA
		loadAddress(a, b, offset);
		break;
	case 0b00000011:		// Rozkaz ST
		store(a, b, offset);
		break;
	default:	// TODO Nieobslugiwany kod operacji
		;
	}
	return 1;
}

int load(int* dest, int value) {

	*dest = value;
	printf("Loaded: %d\n", value);
	return value;
}

int loadAddress(int* a, int* p_address, unsigned int offset) {
	//int* b;
	*a = (int*)fetchAddress((int*)*p_address, offset, memory.order_section);	// TODO Obsluga wykroczenia z danych
	return 1;
}

int store(int* a, int* p_address, unsigned int offset) {
	char* b;
	b = fetchAddress((int*)*p_address, offset, memory.order_section);	// TODO sprawdzenie wyjscia z adresow
	storeValue(b, *a);
	printf("Stored: %d, /t%d\n", *a, loadValue(b));
	return 1;
}

int arithmetic_operation() {

	int result=0;
	int* a, * b;
	unsigned int offset;

	a = (int*)memory.registers[(*((char*)memory.status_register + 1) & 0b11110000) >> 4];
	b = (int*)memory.registers[*((char*)memory.status_register + 1) & 0b00001111];

	if ((*(char*)memory.status_register & 0b00001000) == 0b00000000) {			// Rozkaz jest z grupy rozkazow z zapisem wyniku
		switch ((*(char*)memory.status_register & 0b00001111)) {
		case 0b00000000:		// Operacja AR			
			result = add(a, *b/*, (int*)status_register + 1*/);
			break;
		case 0b00000001:		// Operacja A
			result = executeRMOrder(a, b, loadOffset((char*)memory.status_register), add);
			break;
		case 0b00000010:		// Operacja SR
			result = subtract(a, *b);
			break;
		case 0b00000011:		// Operacja S
			result = executeRMOrder(a, b, loadOffset((char*)memory.status_register), subtract);
			break;
		case 0b00000100:		// Operacja MR
			result = multiply(a, *b);
			break;
		case 0b00000101:		// Operacja M
			result = executeRMOrder(a, b, loadOffset((char*)memory.status_register), multiply);
			break;
		case 0b00000110:		// Operacja DR
			result = divide(a, *b);
			break;
		case 0b00000111:		// Operacja D
			result = executeRMOrder(a, b, loadOffset((char*)memory.status_register), divide);
			break;
		default:
			;//TODO Chyba usunac bo nie ma innej mozliwosci
		}
		*a = result;
	}
	else {
		switch ((*(char*)memory.status_register & 0b00001111)) {
		case 0b00001000:		// Operacja CR
			result = subtract(a, *b);
			break;
		case 0b00001001:		// Operacja C
			result = executeRMOrder(a, b, loadOffset((char*)memory.status_register), subtract);
			break;		
		default:
			;//TODO Error Nieobs³ugiwany kod operacji
		}
	}
	if(result > 0)
		*((int*)memory.status_register + 1) = 0b01;
	else if(result == 0)
		*((int*)memory.status_register + 1) = 0b00;
	else
		*((int*)memory.status_register + 1) = 0b10;

	return 1;
}

int add(int* a, int b) {
	int result;
	result = *a + b;
	if ((*a ^ b) > 0 && (result ^ *a) < 0) {	// Sprawdza roznice znakow miedzy liczbami. Jezeli suma liczb jest innego znaku, niz liczby, ktore maja taki sam znak, to overflow
		*((int*)memory.status_register + 1) = 0b11;
		return 0;
	}
	printf("%d + %d = %d\n", *a, b, result);
	return result;
}

int subtract(int* a, int b) {
	//	TODO Dodac handling overflow
	printf("%d - %d = %d\n", *a, b, *a-b);
	return *a - b;
}

int multiply(int* a, int b) {
	//	TODO Dodac handling overflow
	printf("%d * %d = %d\n", *a, b, *a * b);
	return *a * b;
}

int divide(int* a, int b) {
	//	TODO Dodac handling overflow
	if (b == 0 /* || Warunek overflow?? */) {
		*((int*)memory.status_register + 1) = 0b11;
		return 0;
	}
	printf("%d / %d = %d\n", *a, b, *a / b);
	return *a / b;
}

int executeRMOrder(int* a, int* p_address, unsigned int offset, int operation()) {
	int result, * b;

	b = (int*)fetchAddress((int*)*p_address, offset, memory.order_section);	// Uzaleznic trzeci argument od czegos, bo teraz pierwszy nie ma sensu
	if (b == NULL) 		return 0;
	result = operation(a, loadValue(b));
	return result;
}

char* fetchAddress(char* beginning, unsigned int offset, char* end) {
	if (beginning + offset > end || beginning + offset < beginning) {		// Wyjscie poza pamiec	
		*((int*)memory.status_register + 1) = 0b11; // TODO Jest nadpisywana gdzies!
		return NULL;
	}
	printf("\nOffset: %d\n", offset);
	return beginning + offset;
}

//void refresh(){	// 3 Etap
//}