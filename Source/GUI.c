#include <stdio.h>
#include <stdlib.h>
//#include <curses.h>
#include "PDCurses\curses.h"
#include "GUI.h"
#include "execution.h"
#include "error_handling.h"
#include "constants.h"
#include "interpret.h"
#include "math.h"

//	Zmienne obslugujace okna oraz ich wlasciwosci sa na tyle intensywnie wykorzystywane we wszystkich funkcjach tego pliku, ze zdecydowalem,
//	ze bardziej czytelnie bedzie ustanowienie ich zmiennymi globalnymi dla tego pliku.
//  W przecinym wypadku za kazdym razem musialbym odnosic sie do nich przez nazwe struktury, w ktorej bym je zemiescil co znaczaco wydluzyloby kod.
WINDOW *psa_w, *msck_w, *memory_w, *register_w, *status_w, *labels_w, *steps_w, *sections_w, *card_w;	// Okna, ktore wyswietlaja ramke.
WINDOW *psa_p, *msck_p, *memory_p, *register_p, *status_p, *labels_p, *steps_p, *sections_p, *card_p;	// Wirtualne okna, ktore zawieraja zawartosc okna i ich fragment jest renderowany w srodku ramek.
WINDOW *psa_t, *msck_t, *memory_t, *register_t, *status_t, *labels_t, *steps_t;							// Okna, ktore wyswietlaja tytuly rubryk.

int max_lines[4] = { 0, 0, 0 , 0};				// Wysokosc wirtualnego okna kolejno: psa_p, msck_p, label_p, memory_p.
int titlebar_length = 1;						// Wysokosc tytulow.
int begin_y = 10,			begin_x = 0;			// Pocztek ekranu symulacji.
int psa_length = 31,		psa_width = 48;			// Parametry okna psa_w.
int msck_length = 31,		msck_width = 18;		// Parametry okna msck_w.
int labels_length = 31,		labels_width = 28;		// Parametry okna labels_w.
int memory_length = 31,		memory_width = 31;		// Parametry okna memory_w.
int steps_length = 3,		steps_width = 33;		// Parametry okna steps_w.
int register_length = 18,	register_width = 33;	// Parametry okna register_w.
int status_length = 4,		status_width = 33;		// Parametry okna status_w.
int sections_length = 4,	sections_width = 33;	// Parametry okna sections_w.
int psa_order_begin = -1, msck_order_begin = -1;		// Linia, od ktorej rozpoczynaja sekcje rozkazow w kodzie psa i msck .
int program_length_dec_rep = 0, memory_length_dec_rep = 0, offset_length_dec_rep = 0;	// Ilosc znakow jakie zajmuja indeksy bajtow w kodzie.

// Setup Window

void setupWindow(FILE* code_file, FILE* program_file, char file_path[], Memory* memory, GUIInterface* guii , struct Label label_array[], int label_counter) {
	int terminal_y, terminal_x;

	initPDCurses();		// Przejdz procedure inicjalizacji potrzebnych ustawien biblioteki PDCurses.
	initGUII(guii);		// Ustaw poczatkowe wartosci intefejsu GUI.

	calculateDecimalRepresentationLength(memory);		// Oblicz ile znakow beda zajmowaly dziesietne reprezentacje indeksow bajtow.

	// Dopasowanie terminalu do wielkosci okien.
	terminal_y = begin_y + register_length + 3 * titlebar_length + status_length + sections_length + steps_length;
	terminal_x = begin_x + psa_width + msck_width + labels_width + memory_width + register_width;
	resize_term(terminal_y, terminal_x);

	// Obliczenie wysokosci wymaganej do zmieszczenia zawartosci kodu psa, msck, etykiet oraz pamieci oraz linii, od ktorych rozpoczynaja sie rozkazy.
	calculateContentProperties(code_file, program_file, memory, label_array, label_counter);
	
	initBoxWindows();	// Zainicjuj okna z ramkami.
	initPadWindows();	// Zainicjuj okna z zawartoscia.
	initTitleWindows(); // Zainicjuj okna z tytulami.
	
	printWelcomeCard(file_path);		// Wczytaj tekst powitalny do karty nag³ówkowej.
	printStaticText(memory);			// Wczytaj tekst, ktory sie w czasie symulacji nie zmienia.
	printPsaCode(code_file);			// Wczytaj kod pseudoassemblera do psa_p.
	printMsckCode(program_file);		// Wczytaj kod maszynowy do msck_p.
	printLabels(memory, label_array);	// Wczytaj etykiety do labels_p.

	return;
}

void initPDCurses() {

	initscr();						// Zainicjuj okno PDCurses.
	cbreak();						// Wylacz buforowanie znakow.
	noecho();						// Nie wyswietlaj wprowadzanych znakow.
	start_color();					// Wlacz kolory.
	init_color(8, 600, 600, 600);	// Stworz kolor szary.
	init_pair(1, 8, COLOR_BLACK);	// Stworz pare tlo szare, czarne znaki.

	return;
}

void calculateDecimalRepresentationLength(Memory* memory) {

	if ((char*)memory->end_of_program - (char*)memory->operational_memory != 0) {	// Dziedzina log10(abs(x)).
		program_length_dec_rep = floor(log10(abs((char*)memory->end_of_program - 2 - (char*)memory->operational_memory))) + 1; // Liczenie bajtow od konca do poczatku programu.
		memory_length_dec_rep = floor(log10(abs((char*)memory->order_section - (char*)memory->operational_memory))) + 1; // Liczenie bajtow od konca sekcji danych do poczatku programu.
		offset_length_dec_rep = fmax(memory_length_dec_rep, floor(log10(abs((char*)memory->end_of_program - 2 - (char*)memory->order_section))) + 1);	// Najwieksza z dlugosci rep sekcji rozkazow i sekcji danych.
	}
	msck_width += program_length_dec_rep;				// Uwzglednienie szerokosci zajmowanej przez indeksy bajtow.
	labels_width += program_length_dec_rep + offset_length_dec_rep;
	memory_width += program_length_dec_rep;

	return;
}

void calculateContentProperties(FILE* code_file, FILE* program_file, Memory* memory, struct Label label_array[], int label_counter) {
	int i;
	char code_file_buffor[MAX_COMMAND_LENGTH], program_file_buffor[MAX_MSC_INSTR_LENGTH];

	if (code_file != NULL) {	// Jezeli symulujemy wykonanie programu z kodu pseudoassemblera.
		while (fgets(code_file_buffor, MAX_COMMAND_LENGTH, code_file) != NULL) {	// Jezeli istenieje kolejna linia kodu psa.
			max_lines[0]++;	// Zinkrementuj liczbe linii kodu psa.
			if (code_file_buffor[0] == '\n' && psa_order_begin < 0)	// Jezeli natrafisz na pusta linie oznacza to, ze jest to pocztek sekcji rozkazow w psa.
				psa_order_begin = max_lines[0];
		}
		max_lines[0] += 2;	// Zapas.
		rewind(code_file);
	}
	while ((fgets(program_file_buffor, MAX_MSC_INSTR_LENGTH, program_file)) != NULL) { // Jezeli istenieje kolejna linia kodu msck.
		max_lines[1]++; // Zinkrementuj liczbe linii kodu msck.
		if (program_file_buffor[0] == '\n' && msck_order_begin < 0)	// Jezeli natrafisz na pusta linie oznacza to, ze jest to pocztek sekcji rozkazow w msck.
			msck_order_begin = max_lines[1];
	}
	max_lines[1] += 2;	// Zapas.
	rewind(program_file);
	for (i = 0; i < label_counter && label_array[i].label_name[0] != '\0'; i++) { // Jezeli istenieje kolejna etykieta.
		max_lines[2]++; // Zinkrementuj liczbe linii listy etykiet.
	}
	max_lines[2] += 2 + 1;	// Zapas + przerwa dla oddzielenia sekcji.
	max_lines[3] = ((int*)memory->order_section - (int*)memory->data_section + 1) + 1;	// Ilosc slow w sekcji danych + zapas.

	return;
}

void initBoxWindows() {
	int cursor_x = begin_x, cursor_y = 0;

	createBoxWindow(&card_w, begin_y, COLS, cursor_y, cursor_x);
	cursor_y += begin_y + titlebar_length;
	createBoxWindow(&psa_w, psa_length, psa_width, cursor_y , cursor_x);
	cursor_x += psa_width;
	createBoxWindow(&msck_w, msck_length, msck_width, cursor_y, cursor_x);
	cursor_x += msck_width;
	createBoxWindow(&labels_w, labels_length, labels_width, cursor_y, cursor_x);
	cursor_x += labels_width;
	createBoxWindow(&memory_w, memory_length, memory_width, cursor_y, cursor_x);
	cursor_x += memory_width;
	createBoxWindow(&steps_w, steps_length, steps_width, cursor_y, cursor_x);
	cursor_y += titlebar_length + steps_length;
	createBoxWindow(&register_w, register_length, register_width, cursor_y, cursor_x);
	cursor_y += titlebar_length + register_length;
	createBoxWindow(&status_w, status_length, status_width, cursor_y, cursor_x);
	cursor_y += status_length;
	createBoxWindow(&sections_w, sections_length, sections_width, cursor_y, cursor_x);

	return;
}

void createBoxWindow(WINDOW** win, int height, int width, int starty, int startx)
{
	*win = newwin(height, width, starty, startx);
	box(*win, 0, 0);

	return;
}

void initPadWindows() {

	card_p = newpad(begin_y - 2, COLS - begin_x - 3);
	psa_p = newpad(max_lines[0], MAX_COMMAND_LENGTH);
	msck_p = newpad(max_lines[1], msck_width - 3);
	labels_p = newpad(max_lines[2], labels_width - 3);
	memory_p = newpad(max_lines[3], memory_width - 3);
	steps_p = newpad(1, steps_width - 3);
	register_p = newpad(register_length - 2, register_width - 3);
	status_p = newpad(status_length - 2, status_width - 3);
	sections_p = newpad(sections_length - 2, sections_width - 3);

	return;
}

void initTitleWindows() {
	int cursor_x = begin_x, cursor_y = begin_y;

	psa_t = newwin(1, psa_width, cursor_y, cursor_x);
	cursor_x += psa_width;
	msck_t = newwin(1, msck_width, cursor_y, cursor_x);
	cursor_x += msck_width;
	labels_t = newwin(1, labels_width, cursor_y, cursor_x);
	cursor_x += labels_width;
	memory_t = newwin(1, memory_width, cursor_y, cursor_x);
	cursor_x += memory_width;
	steps_t = newwin(1, steps_width, cursor_y, cursor_x);
	cursor_y += titlebar_length + steps_length;
	register_t = newwin(1, register_width, cursor_y, cursor_x);
	cursor_y += titlebar_length + register_length;
	status_t = newwin(1, status_width, cursor_y, cursor_x);

	return;
}

void printWelcomeCard(char file_path[]) {

	wprintw(card_p, "PSEUDOASSEMBLER INTERPRETER\n");
	wprintw(card_p, "Simulating the execution of the program from \"%s\"\n\n", file_path);
	wprintw(card_p, "Use 'q' and 'e' to navigate the headers. Use 'w', 'a', 's', 'd' to interact with the selected panel.\n");
	wprintw(card_p, "Press 'f' to advance the simulation by a step. Press 'c' to complete the simulation. Press 'x' to exit.\n");
	wprintw(card_p, "Highlighted lines indicate the instruction executed in the last step and it's result. Underlined line indicate the next instruction to be executed.\n");
	wprintw(card_p, "Second register in the instruction can be distinguished by grey colour.\n");
	wprintw(card_p, "Numbers after pipe in \"Labels\" panel represent label's address relative to the beginning of the section.");

	return;
}

void printStaticText(Memory* memory) {

	mvwprintw(psa_t, 0, (psa_width - strlen("Pseudoassembler code")) / 2, "Pseudoassembler code");	// Wysrodkuj tytul.
	mvwprintw(msck_t, 0, (msck_width - strlen("Machine code")) / 2, "Machine code");
	mvwprintw(labels_t, 0, (labels_width - strlen("Labels")) / 2, "Labels");
	mvwprintw(memory_t, 0, (memory_width - strlen("Memory cells")) / 2, "Memory cells");
	mvwprintw(steps_t, 0, (steps_width - strlen("Instructions per step")) / 2, "Instructions per step");
	mvwprintw(register_t, 0, (register_width - strlen("Registers")) / 2, "Registers");
	mvwprintw(status_t, 0, (status_width - strlen("Status register")) / 2, "Status register");
	wprintw(sections_p, "Data section:  %d\n", (char*)memory->data_section - (char*)memory->operational_memory);
	wprintw(sections_p, "Order section: %d\n", (char*)memory->order_section - (char*)memory->operational_memory);

	return;
}

void printPsaCode(FILE* code_file) {
	char code_file_buffor[MAX_COMMAND_LENGTH];

	if (code_file != NULL) {
		while ((fgets(code_file_buffor, MAX_COMMAND_LENGTH, code_file)) != NULL) {
			wprintw(psa_p, "%s", code_file_buffor);
		}
		rewind(code_file);
	}

	return;
}

void printMsckCode(FILE* program_file) {
	char program_file_buffor[MAX_MSC_INSTR_LENGTH];
	int d = 0;	// Licznik indeksu bajtow.

	while ((fgets(program_file_buffor, MAX_MSC_INSTR_LENGTH, program_file)) != NULL) {
		if (strlen(program_file_buffor) > 6) {	// Jezeli linijka zawiera instrukcje cztero-bajtowa.
			wprintw(msck_p, "%*d : ", program_length_dec_rep, d);
			d += 4;	// Dodaj do licznika indeksu bajtow 4.
		}
		else if (strlen(program_file_buffor) > 1) {	// Jezeli linijka zawiera instrukcje dwu-bajtowa.
			wprintw(msck_p, "%*d : ", program_length_dec_rep, d);
			d += 2;	// Dodaj do licznika indeksu bajtow 2.
		}
		wprintw(msck_p, "%s", program_file_buffor);
	}
	rewind(program_file);

	return;
}

void printLabels(Memory* memory, struct Label label_array[]) {
	int i, a, b;
	struct Label* label;

	for (i = 0; i < max_lines[2] - 3; i++) {
		label = &label_array[i];
		if (i != 0)	// Zabezpiecznie przed wyjsciem z tablicy.
			if (label->reg != label_array[i - 1].reg) // Jezeli zmiana sekcji, dodaj jedna pusta linie.
				wprintw(labels_p, "\n");
		a = label->label_interpretation;
		// Zubezwzglednienie adresu, na ktory wskazuje etykieta.
		b = a + ((label->reg == REGISTER_WITH_ADDRESS_TO_ORDER_SECTION) ? (char*)memory->order_section : (char*)memory->data_section) - (char*)memory->operational_memory;
		wprintw(labels_p, "%-10s : %*d | %02hhX %02hhX - %*d\n", label->label_name, program_length_dec_rep, b, a >> 8, a, offset_length_dec_rep, a);
	}

	return;
}

// Refresh GUI

int refreshGUI(Memory* memory, GUIInterface* guii) {

	static int active_row[4] = { 0 }, active_column = 0, active_psa_column = 0; // Zmienne przechowujace aktualny stan nawigacji po programie uzytkownika.
	static int active_code_line = 0, next_code_line = 0;	// linijka kodu do podswietlenia w kodzie jako wykonana oraz nastepna do wykonania. 
	static int instructions_per_step = 1;	// Zmienna ustala ilosc instrukcji wykonanych w jednym kroku.

	calculateHighlight(memory, guii, &active_code_line, &next_code_line);	// Obliczenie aktywnych linii kodu w nastepnym kroku korzystajac z interfejsu GUII.
	if (!guii->skip || guii->end) {	// Jezeli uzytkownik nie wlaczyl trybu ukonczenia symulacji lub jest na koncu wykonania symulacji.
		highlightCode(memory, active_code_line, next_code_line);	// Funkcja odpowiadajaca za rzeczywiste podswietlenie aktywnych i zgaszenie poprzednich linii kodu na ekranie.
		showRegisters(memory, guii);	// Zaktualizuj informacje okna rejstrow.
		showMemory(memory, guii);		// Zaktualizuj informacje okna pamieci.
		showStatus(memory);				// Zaktualizuj informacje okna rejsetru stanu.
		// Petla interakcji uzytkownika z programem miedzy wykonaniem krokow.
		while ((guii->step <= 0 && guii->skip != 1) || guii->end) {	// Wyjdz gdy uzytkownik zazyczy wykonanie kolejnego kroku, badz ukonczenie symulacji lub gdy symulacja sie skonczyla.
			showTitle(active_column);						// Zaktualizuj podswietlenie tytulow.
			showSteps(instructions_per_step);				// Zaktualizuj informacje okna ilosci instrukcji na krok.
			refreshWindows(active_row, active_psa_column);	// Odswiez okna w terminalu.
			// Reaguj na wprowadzane znaki z klawiatury.
			if (react(guii, active_row, &active_psa_column, &active_column, &instructions_per_step) == 0) {	
				endwin();
				return 0;	// Uzytkownik zazyczyl wyjscie z symulacji bez jej ukonczenia.
			}
		}
	}
	resetGUII(guii);
	return 1;
}

void refreshWindows(int active_row[], int active_psa_column) {

	refreshBoxWindows();
	refreshPadWindows(active_row, active_psa_column);
	refreshTitleWindows();
	refresh();

	return;
}

void refreshBoxWindows() {

	wrefresh(card_w);
	wrefresh(psa_w);
	wrefresh(msck_w);
	wrefresh(memory_w);
	wrefresh(steps_w);
	wrefresh(register_w);
	wrefresh(status_w);
	wrefresh(sections_w);
	wrefresh(labels_w);
	return;
}

void refreshPadWindows(int active_row[], int active_psa_column) {
	int cursor_x, cursor_y;

	cursor_x = begin_x;
	cursor_y = begin_y + titlebar_length + 1;
	prefresh(card_p, 0, 0, 1, 2, begin_y - 2, COLS - begin_x - 3);
	prefresh(psa_p, active_row[0], active_psa_column, cursor_y, cursor_x + 2, cursor_y + psa_length - 3, cursor_x + psa_width - 3);
	cursor_x += psa_width;
	prefresh(msck_p, active_row[1], 0, cursor_y, cursor_x + 2, cursor_y + msck_length - 3, cursor_x + msck_width - 1);
	cursor_x += msck_width;
	prefresh(labels_p, active_row[2], 0, cursor_y, cursor_x + 2, cursor_y + labels_length - 3, cursor_x + labels_width - 1);
	cursor_x += labels_width;
	prefresh(memory_p, active_row[3], 0, cursor_y, cursor_x + 2, cursor_y + memory_length - 3, cursor_x + memory_width - 1);
	cursor_x += memory_width;
	prefresh(steps_p, 0, 0, cursor_y, cursor_x + 2, cursor_y + steps_length - 3, cursor_x + steps_width - 1);
	cursor_y += steps_length + titlebar_length;
	prefresh(register_p, 0, 0, cursor_y, cursor_x + 2, cursor_y + register_length - 3, cursor_x + register_width - 1);
	cursor_y += titlebar_length + register_length;
	prefresh(status_p, 0, 0, cursor_y, cursor_x + 2, cursor_y + status_length - 3, cursor_x + status_width - 1);
	cursor_y += status_length;
	prefresh(sections_p, 0, 0, cursor_y, cursor_x + 2, cursor_y + sections_length - 3, cursor_x + sections_width - 1);

	return;
}

void refreshTitleWindows() {

	wrefresh(psa_t);
	wrefresh(msck_t);
	wrefresh(memory_t);
	wrefresh(steps_t);
	wrefresh(register_t);
	wrefresh(status_t);
	wrefresh(labels_t);

	return;
}

// GUI

void initGUII(GUIInterface* guii) {

	guii->line_diff_bytes = 0;
	guii->memory_cell = -1;
	guii->register_1 = -1;
	guii->register_2 = -1;
	guii->step = 0;
	guii->skip = 0;
	guii->end = 0;

	return;
}

void resetGUII(GUIInterface* guii) {
	guii->memory_cell = -1; 
	guii->register_1 = -1;
	guii->register_2 = -1;

	return;
}

// React

int react(GUIInterface* guii, int active_row[], int* active_psa_column, int* active_column, int* instructions_per_step) {
	char ch;
	
	ch = getch();
	switch (ch) {
	case 'x':
		return 0;
	case 'w':
		if (*active_column < 4)	// Scrollowalny panel.
			if (active_row[*active_column] > 0)	// Nie zescrollowany do samej gory.
				active_row[*active_column]--;	
		break;
	case 'a':
		if (*active_column == 0 && *active_psa_column > 0)	// Panel kodu psa i nie zescrollowany do samej lewej.
			(*active_psa_column)--;
		else if (*active_column == 4 && *instructions_per_step > 1) // Panel instrukcji na krok i minimum 2 instrukcje na krok.
			(*instructions_per_step)--;
		break;
	case 's':
		if (*active_column < 4)	// Scrollowalny panel.
			if (active_row[*active_column] < max_lines[*active_column] - 2) // Nie zescrollowany do samej gory.
				active_row[*active_column]++;
		break;
	case 'd':
		if (*active_column == 0 && *active_psa_column < MAX_COMMAND_LENGTH)	// Panel kodu psa i nie zescrollowany do samej prawej.
			(*active_psa_column)++;
		else if (*active_column == 4)	// Panel instrukcji.
			(*instructions_per_step)++;
		break;
	case 'q':
		if (*active_column > 0)	// Niegraniczny, lewy tytu³.
			(*active_column)--;
		break;
	case 'e':
		if (*active_column < 4)	// Niegraniczny, prawy tytu³.
			(*active_column)++;
		break;
	case 'f':
		guii->step = *instructions_per_step;	// Wykonaj dana ilosc instrukcji.
		break;
	case 'c':
		guii->skip = 1;	// Wykonaj symulacje do konca.
		break;
	}
	return 1;
}

// Show

void showRegisters(Memory* memory, GUIInterface* guii) {
	int i = 0;
	char* reg;
	
	wclear(register_p);
	for (i = 0; i < 16; i++) {
		reg = (char*)memory->registers[i];		// wskaznik na pierwszy bajt rejestru
		mvwprintw(register_p, i, 0, "%2d: %02hhX %02hhX %02hhX %02hhX - %11d\n", i, *(reg+3), *(reg + 2), *(reg + 1), *reg, *(int*)reg);
	}
	mvwchgat(register_p, guii->register_1, 0, register_width - 4, A_STANDOUT, 0, NULL);
	mvwchgat(register_p, guii->register_2, 0, register_width - 4, A_STANDOUT | COLOR_PAIR(1), 0, NULL);

	return;
}

void showMemory(Memory* memory, GUIInterface* guii) {
	void* p;
	int i = 0;

	wclear(memory_p);
	// wskaznik na pierwszy bajt sekcji danych
	for (i = 0, p = memory->data_section; i < max_lines[3] && p < memory->order_section; i++, ((int*)p)++) {
		wprintw(memory_p, "%*d: %02hhX %02hhX %02hhX %02hhX - ", memory_length_dec_rep, i*4, *(char*)p, *((char*)p + 1), *((char*)p + 2), *((char*)p + 3));
		wprintw(memory_p, "%11d\n", loadValue((char*)p));
	}		
	mvwchgat(memory_p, guii->memory_cell, 0 , memory_width - 4, A_STANDOUT, 0, NULL);

	return;
}

void showStatus(Memory* memory) {
	char* p;
	int i = 0;
	wclear(status_p);
	p = (char*)memory->status_register;	// wskaznik na pierwszy bajt pierwszego bajtu rejstru stanu 
	wprintw(status_p, "%*s%02hhX %02hhX %02hhX %02hhX - ", (status_width - 2 - 12 - 11) / 2,"",*(p + 3), *(p + 2), *(p + 1), *p);
	wprintw(status_p, "%s\n", (*p == 0) ? "ZERO" : (*p == 1) ? "POSITIVE" : (*p == 2) ? "NEGATIVE" : "ERROR");
	p = (char*)((int*)memory->status_register + 1);	// wskaznik na drugi bajt pierwszego bajtu rejstru stanu 
	for (i = 0; i < (status_width - 2 - 12 - 11) / 2; i++)	wprintw(status_p, " ");
	wprintw(status_p, "%02hhX %02hhX %02hhX %02hhX - %d", *(p + 3), *(p + 2), *(p + 1), *p, *(int*)p);

	return;
}

void showSteps(int instructions_per_step) {

	wclear(steps_p);
	wprintw(steps_p, "<%*d ", steps_width/2 + (int)(floor(log10(abs(instructions_per_step))) + 1) - 3, instructions_per_step);	// Wysrodkuj liczbe
	wprintw(steps_p, "%*s>", steps_width/2 - (int)(floor(log10(abs(instructions_per_step))) + 1) - 3, "");

	return;
}

void calculateHighlight(Memory* memory, GUIInterface* guii, int* active_code_line, int* next_code_line) {
	int d = guii->line_diff_bytes;

	*active_code_line = *next_code_line;
	while (d != 0) {	// Dopoki roznica miedzy aktualna a docelowa instrukcja nie jest 0.
		if (d > 0) {	// Skok do przodu.
			if ((char)mvwinch(msck_p, msck_order_begin + *next_code_line, msck_width - 6) != ' ')	// Jezeli instrukcja jest cztero-bajtowa to powinna w tym miejscu znak.
				d -= 4;
			else	// w przeciwnym wypadku instrukcja dwubajtowa.
				d -= 2;
			(*next_code_line)++;
		}
		else { // d < 0
			if ((char)mvwinch(msck_p, msck_order_begin + *next_code_line - 1, msck_width - 6) != ' ') // Jezeli instrukcja jest cztero-bajtowa to powinna miec w tym miejscu znak.
				d += 4;
			else	// w przeciwnym wypadku instrukcja dwubajtowa.
				d += 2;
			(*next_code_line)--;
		}
	}
	guii->line_diff_bytes = 0;	// Zaktualizuj interfejs.

	return;
}

void highlightCode(Memory* memory, int active_code_line, int next_code_line) {
	static int last_active_code_line = -1, last_next_code_line = -1;

	mvwchgat(psa_p, psa_order_begin + last_active_code_line, 0, MAX_COMMAND_LENGTH, A_NORMAL, 0, NULL);	// Zgas poprzednie podswietlenie
	mvwchgat(msck_p, msck_order_begin + last_active_code_line, 0, msck_width - 4, A_NORMAL, 0, NULL);
	mvwchgat(psa_p, psa_order_begin + last_next_code_line, 0, MAX_COMMAND_LENGTH, A_NORMAL, 0, NULL);
	mvwchgat(msck_p, msck_order_begin + last_next_code_line, 0, msck_width - 4, A_NORMAL, 0, NULL);

	mvwchgat(psa_p, psa_order_begin + active_code_line, 0, MAX_COMMAND_LENGTH, A_STANDOUT, 0, NULL);	// Podswietl aktualne instrukcje
	mvwchgat(msck_p, msck_order_begin + active_code_line, 0, msck_width - 4, A_STANDOUT, 0, NULL);
	mvwchgat(psa_p, psa_order_begin + next_code_line, 0, MAX_COMMAND_LENGTH, A_UNDERLINE, 0, NULL);
	mvwchgat(msck_p, msck_order_begin + next_code_line, 0, msck_width - 4, A_UNDERLINE, 0, NULL);

	last_active_code_line = active_code_line;	// Przygotuj linie do zgaszenia
	last_next_code_line = next_code_line;

	return;
}

void showTitle(int active_column) {	
	
	mvwchgat(psa_t, 0, 0, psa_width, A_NORMAL, 0, NULL);	// Zresetuj podswietlenia tytulow
	mvwchgat(msck_t, 0, 0, msck_width, A_NORMAL, 0, NULL);
	mvwchgat(labels_t, 0, 0, labels_width, A_NORMAL, 0, NULL);
	mvwchgat(memory_t, 0, 0, memory_width, A_NORMAL, 0, NULL);
	mvwchgat(steps_t, 0, 0, steps_width, A_NORMAL, 0, NULL);

	switch (active_column) {	// Podswietl aktywny tytul
	case 0:
		mvwchgat(psa_t, 0, 0, psa_width, A_UNDERLINE, 0, NULL);
		break;
	case 1:
		mvwchgat(msck_t, 0, 0, msck_width, A_UNDERLINE, 0, NULL);
		break;
	case 2:
		mvwchgat(labels_t, 0, 0, labels_width, A_UNDERLINE, 0, NULL);
		break;
	case 3:
		mvwchgat(memory_t, 0, 0, memory_width, A_UNDERLINE, 0, NULL);
		break;
	case 4:
		mvwchgat(steps_t, 0, 0, steps_width, A_UNDERLINE, 0, NULL);
		break;
	}

	return;
}

// Card

void printOnCard( char* text) {

	wprintw(card_p, text);

	return;
}

void clearCard() {

	wclear(card_p);

	return;
}