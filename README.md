# Pseudoassembler Interpreter and Emulator


Pseudo-assembler interpreter & emulator in C language.

![image](https://user-images.githubusercontent.com/74872004/226138059-e298c365-6117-4072-9290-826998dd4df6.png)

1. **Information panel**. Contains instructions for navigating the interface and messages from the app

2. **Panel title underline**. Indicates the currently selected panel for interaction. You can move between ‘Pseudoassembler Code’, ‘Machine Code’, ‘Labels’, ‘Memory Cells’, and ‘ Instructions per step’ using the ‘q’ and ‘e’ keys.

3. **'Pseudoassembler Code' panel**. Contains the source code of the Pseudoassembler. When the source code is machine code, the panel is empty. It is possible to navigate through the contents of this panel using the ‘w’, ‘a’, ‘s’, ‘d’ keys.

4. **Highlighted Pseudoassembler code line**. It indicates the most recently executed instruction.

5. **Underlined Pseudoassembler code line**. It indicates the instruction that will be executed next.

6. **'Machine Code' panel**. Contains the machine code after interpretation, or from the source file. The numbers before the colon indicate the address of the first byte that the instruction occupies in memory. It is possible to navigate through the contents of this panel using the ‘w’, ‘s’ keys.

7. **Highlighted Machine Code line**. Indicates the most recently executed instruction. It corresponds to the same instruction from the Pseudoassembler code panel.

8. **Underlined Machine Code line**. Indicates the instruction that will be executed next. It corresponds to the same instruction from the Pseudoassembler code panel.

9. **'Labels' panel**. Contains a list of labels used in the Pseudoassembler code. The space between blocks of labels separates labels from the data section from labels from the instructions section. The numbers after the colon indicate the address of the first byte occupied in memory by the instruction to which the label points. The numbers after the vertical dash are, in turn, the hexadecimal and decimal representation of this address relative to the beginning of the section. It is possible to move through the contents of this panel using the ‘w’, ‘s’ keys.

10. **'Memory Cells' panel**. Contains a list of memory cells of the program execution simulation. The numbers before the colon indicate the address of the first byte of the cell. The numbers after the colon are the hexadecimal and decimal representation of the cell contents, respectively. It is possible to navigate through the contents of this panel using the ‘w’, ‘s’ keys.

11. **Memory cell highlight**. Indicates the memory cell that was used in the most recently executed instruction.

12. **'Instructions per step' panel**. Contains the number of instructions that will be executed in one step of the simulation. This number can be changed using the ‘a’ and ‘d’ keys.

13. **'Registers' panel**. Contains a list of 16 registers for simulating program execution.

14. **Register highlight**. Indicates the first register that was used in the last executed instruction.

15. **Grey register highlight**. Indicates the second register that was used in the most recently executed instruction.

16. **'Status register' panel**. It contains eight bytes of the program status register. The first four bytes indicate the sign of the last operation performed, this is expressed in words after the dash. The last four bytes indicate the address of the instruction to be executed next. The number after the dash is the decimal representation of this address.

17. **'Section address' panel**. This contains the decimal representation of the addresses of the data section and the command section respectively.
