#pragma once

int loadFile(FILE**, char[], char*);            // Zajmuje sie obsluga pliku wejsciowego. Zwraca wartosc logiczna czy plik zostal poprawnie wczytany
void trimPathName(char[], char[], int);			// Obcina sciezke oraz rozszerzenie nazwy pliku. Jezeli 
