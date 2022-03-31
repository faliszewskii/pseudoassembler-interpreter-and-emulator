
#pragma once
#if defined( _MSC_VER )
#pragma warning(disable:4996)
#endif

#define MAX_FILE_NAME_LENGTH 50	// Maksymalna dlugosc sciezki wzglednej pliku wejsciowego
#define MAX_COMMAND_LENGTH 101  // Maksymalna dlugosc wiersza kodu Pseudoassemblera + '\0'
#define MAX_MSC_INSTR_LENGTH 13 // Maksymalna dlugosc wiersza kodu maszynowego
#define MAX_NAME_LENGTH 10		// Maksymalna dlugosc etykiety.
#define MAX_LENGTH 21			// Maksymalna dlugosc argumentow ("INTEGER(-2147483648)"+'\0')
#define BYTES_FOR_VALUE 4		// Ilosc bajtow przeznaczona na wartosci w pamieci
#define BYTES_FOR_RM_ORDER 4	// Ilosc bajtow przeznaczona na rozkazy rejstr-pamiec
#define BYTES_FOR_RR_ORDER 2	// Ilosc bajtow przeznaczona na rozkazy rejstr-rejstr
#define BYTES_FOR_LABEL 2		// Ilosc bajtow przeznaczona na etykiete
#define BYTES_FOR_REGISTER 0.5	// Ilosc bajtow przeznaczona na rejestr
#define REGISTER_WITH_ADDRESS_TO_DATA_SECTION 15	// Rejstr, w ktorym jest adres poczatku sekcji danych programu
#define REGISTER_WITH_ADDRESS_TO_ORDER_SECTION 14	// Rejstr, w ktorym jest adres poczatku sekcji rozkazow programu
#define LAST_BYTE_MASK 0xFF		// Maska wskazujaca na ostatni bajt
#define BYTE 8					// Ilosc bitow w bajcie
#define RANDOMISER_RANGE_LOW -RAND_MAX/2		// Zakres dolny losowania liczb 
#define RANDOMISER_RANGE_UP RAND_MAX/2		// Zakres gorny losowania liczb 


