#pragma once
#include "execution.h"
#include "GUI.h"

int jumpOrder(Memory*, GUIInterface*);		// Obsluzenie rozkazu skoku.
int jumpTo(Memory*, GUIInterface*, int*);	// Ustaw drugi bajt rejestru stanu na docelowa instrukcje.