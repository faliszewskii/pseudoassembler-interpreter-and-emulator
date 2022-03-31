#pragma once
//#include <curses.h>
#include "PDCurses\curses.h"
#include "execution.h"
#include "interpret.h"


typedef struct GUIInterface {
	int line_diff_bytes;
	int memory_cell;
	int register_1;
	int register_2;
	int step;
	int skip;
	int end;
} GUIInterface;

void setupWindow(FILE*, FILE*, char[], struct Memory*, GUIInterface*, struct Label[], int);	// Ustaw okna.
void initPDCurses();	// Zainicjuj funkcjonalnosc biblioteki PDCurses.
void calculateDecimalRepresentationLength(struct Memory*);							// Oblicz ile miejsca beda zajmowaly adresy bajtow.
void calculateContentProperties(FILE*, FILE*, struct Memory*, struct Label[], int);		// Oblicz wlasciwosci paneli zawartosci.
void initBoxWindows();										// Zainicjuj okna z ramkami.
void createBoxWindow(WINDOW**, int, int, int, int);			// Narysuj ramke.
void initPadWindows();										// Zainicjuj panele z zawartoscia.
void initTitleWindows();									// Zainicjuj okna z tytulami.
void printWelcomeCard(char[]);								// Wydrukuj komunikat powitalny na karcie naglowkowej.
void printStaticText(struct Memory*);						// Wydrukuj tekst niezmienny w symulacji.
void printPsaCode(FILE*);									// Wydrukuj kod Pseudoassemblera w panel psa_p.
void printMsckCode(FILE*);									// Wydrukuj kod maszynowy w panel msck_p.
void printLabels(struct Memory* , struct Label[]);			// Wydrukuj etykiety w panel labels_p.

int refreshGUI(struct Memory*, GUIInterface*);				// Odswiez ekran.
void refreshWindows(int[], int);							// Odswiez okna.
void refreshBoxWindows();									// Odswiez okna z ramkami.
void refreshPadWindows(int[], int);							// Odswiez panele z zawartoscia.
void refreshTitleWindows();									// Odswiez okna z tytulami.

void initGUII(GUIInterface*);								// Zainicjuj interfejs programu z GUI.
void resetGUII(GUIInterface*);								// Zresetuj interfejs programu z GUI dla nastepnej interacji.

int react(GUIInterface*, int[], int*, int*, int*);			// Odpowiedz na dzialanie uzytkownika.

void showRegisters(struct Memory*, GUIInterface*);			// Odswiez zawartosc i podswietlenie panelu rejestrow.
void showMemory(struct Memory*, GUIInterface*);				// Odswiez zawartosc i podswietlenie panelu komorek pamieci.
void showStatus(struct Memory*);							// Odswiez zawartosc panelu rejestru stanu.
void showSteps(int);										// Odswiez zawartosc panelu ilosci instrukcji na krok.
void calculateHighlight(struct Memory*, GUIInterface*, int*, int*);	// Oblicz roznice w liniach miedzy kolejnymi instrukcjami.
void highlightCode(struct Memory*, int, int);				// Odswiez podswietlenie kodu.
void showTitle(int);										// Odswiez zawartosci paneli tytulow.

void printOnCard(char*);									// Dodrukuj tekst na karte naglowkowa.
void clearCard();											// Wyczysc karte naglowkowa.


