# Pseudoassembler Interpreter and Emulator


Pseudo-assembler interpreter & emulator in C language.

![image](https://user-images.githubusercontent.com/74872004/226138059-e298c365-6117-4072-9290-826998dd4df6.png)

1. **Panel informacji**. Zawiera instrukcje nawigacji po interfejsie oraz komunikaty od
programu.

2. **Podkreślenie tytułu panelu**. Oznacza aktualnie wybrany panel do interakcji. Można się
przemieszczać między „Pseudoassembler Code”, „Machine Code”, „Labels”, „Memory
Cells”, a „ Instructions per step” za pomocą klawiszy ‘q’ i ‘e’.

3. **Panel „Pseudoassembler Code”**. Zawiera źródłowy kod Pseudoassemblera. W
przypadku gdy kodem źródłowym jest kod maszynowy, panel jest pusty. Możliwe jest
przemieszczanie się po zawartości tego panelu za pomocą klawiszy ‘w’, ‘a’, ‘s’, ‘d’.

4. **Podświetlenie linii kodu Pseudoassemblera**. Oznacza ono ostatnio wykonaną
instrukcję.

5. **Podkreślenie linii kodu Pseudoassemblera**. Oznacza instrukcję, która zostanie
wykonana jako następna.

6. **Panel „Machine Code”**. Zawiera kod maszynowy po zinterpretowaniu, bądź z pliku
źródłowego. Liczby przed dwukropkiem oznaczają adres pierwszego bajtu, który
zajmuje w pamięci dana instrukcja. Możliwe jest przemieszczanie się po zawartości
tego panelu za pomocą klawiszy ‘w’, ‘s’.

7. **Podświetlenie linii kodu maszynowego**. Oznacza ostatnio wykonaną instrukcję.
Odpowiada ona tej samej instrukcji z panelu kodu Pseudoassemblera.

8. **Podkreślenie linii kodu maszynowego**. Oznacza instrukcję, która zostanie wykonana
jako następna. Odpowiada ona tej samej instrukcji z panelu kodu Pseudoassemblera.

9. **Panel „Labels”**. Zawiera listę etykiet wykorzystanych w kodzie Pseudoassemblera.
Odstęp między blokami etykiet oddziela etykiety z sekcji danych od etykiet z sekcji
rozkazów. Liczby po dwukropku oznaczają adres pierwszego bajtu, który zajmuje w
pamięci instrukcja, na którą wskazuje etykieta. Liczby po kresce pionowej są kolejno
reprezentacją szesnastkową i dziesiętną tego adresu względnego wobec początku
sekcji. Możliwe jest przemieszczanie się po zawartości tego panelu za pomocą klawiszy
‘w’, ‘s’.

10. **Panel „Memory Cells”**. Zawiera listę komórek pamięci symulacji wykonania programu.
Liczby przed dwukropkiem oznaczają adres pierwszego bajtu komórki. Liczby po
dwukropku to kolejno reprezentacja szesnastkowa i dziesiętna zawartości komórki.
Możliwe jest przemieszczanie się po zawartości tego panelu za pomocą klawiszy ‘w’,
‘s’.

11. **Podświetlenie komórki pamięci**. Wskazuje komórkę pamięci, która została użyta w
ostatnio wykonanej instrukcji.

12. **Panel „Instructions per step”**. Zawiera liczbę instrukcji jaka zostanie wykonana w
jednym kroku symulacji. Można tą liczbę zmienić za pomocą klawiszy ‘a’ i ‘d’.

13. **Panel „Registers”**. Zawiera listę 16 rejestrów symulacji wykonania programu.

14. **Podświetlenie rejestru**. Wskazuje pierwszy rejestr, który został użyty w ostatnio
wykonanej instrukcji.

15. **Szare podświetlenie rejestru**. Wskazuje drugi rejestr, który został użyty w ostatnio
wykonanej instrukcji.

16. **Panel „Status register”**. Zawiera osiem bajtów rejestru stanu programu. Pierwsze
cztery bajty oznaczają znak ostatniej wykonanej operacji, jest to wyrażone słownie po
myślniku. Ostatnie cztery bajty oznaczają adres instrukcji, która ma być wykonana jako
następna. Liczba po myślniku jest reprezentacją dziesiętną tego adresu.

17. **Panel adresów sekcji**. Zawiera on reprezentację dziesiętną adresów kolejno sekcji
danych oraz sekcji rozkazów. 
