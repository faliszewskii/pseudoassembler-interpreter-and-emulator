#pragma once

int readPseudoassemblerCode(FILE*, Row**);							// Czyta plik wejsciowy i zapisuje kolejne wyrazy w tablicy struktur Row. Zwraca ilosc wierszy kodu.
int expandRowArray(Row**, int);										// Rozszerza tablice trzymajaca informacje o wierszach kodu Pseudoassemblera
int checkComment(char*);											// Zwraca wartosc logiczna czy linia jest komentarzem.
int checkLabel(Row[], int, char**, char*, int*);					// Czyta etykiete jezeli linia zaczyna sie od etykiety. Zwraca poprawnosc przeczytania.
int checkDirective(Row[], int, char**, int*, enum Stage);			// Czyta instrukcje dyrektywy. Zwraca poprawnosc przeczytania.
int checkOrder(Row[], int, char**, int*, enum Stage);				// Czyta instrukcje rozkazu. Zwraca poprawnosc przeczytania.

int readDirective(Row[], int, char**, int*);						// Czyta wiersz z dyrektyw¹. Zwraca wartosc logiczna czy wiersz jest poprawny.
int readOrderJump(Row[], int, char**, int*);						// Czyta wiersz z rozkazem skoku. Zwraca wartosc logiczna czy wiersz jest poprawny.
int readOrderRM(Row[], int, char**, int*);							// Czyta wiersz z rozkazem rejestr-pamiec. Zwraca wartosc logiczna czy wiersz jest poprawny.
int readOrderRR(Row[], int, char**, int*);							// Czyta wiersz z rozkazem rejestr-rejestr. Zwraca wartosc logiczna czy wiersz jest poprawny.
int readAddress(Row[], int, char**, int*, int*, int*, int*);		// Czyta adres zapisany w formie przesuniecie(rejestr). Zwraca wartosc logiczna poprawnosci ciagu.
int readFirstRegister(Row[], int, char**, int*, int*, int*, int*);	// Czyta pierwszy rejestr. Zwraca wartosc logiczna poprawnosci ciagu.
int readSecondRegister(Row[], int, char**, int*, int*, int*, int*);	// Czyta drugi rejestr. Zwraca wartosc logiczna poprawnosci ciagu.
