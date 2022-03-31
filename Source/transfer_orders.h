#pragma once
#include "execution.h"
#include "GUI.h"

int transferOrder(Memory*, GUIInterface*);				// Obsluzenie rozkazu transferu danycyh.
int load(int*, int);									// Wczytaj wartosc.
int loadAddress(Memory*, GUIInterface*, int*, int*);	// Wczytaj adres komorki.
int store(Memory*, GUIInterface*, int*, int*);			// Zapisz wartosc w komorce.
void storeValue(char*, int);							// Zakodowanie wartosci jako ciag w pamieci.