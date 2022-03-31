#pragma once
#include "execution.h"
#include "GUI.h"

int arithmetic_operation(Memory*, GUIInterface*);	// Obsluzenie rozkazu operacji arytmetycznej.
int add(int, int, char*);							// Dodawanie.
int subtract(int, int, char*);						// Odejmowanie.
int multiply(int, int, char*);						// Mnozenie.
int divide(int, int, char*);						// Dzielenie.
