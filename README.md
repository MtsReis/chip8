CHIP-8 Emulator/Interpreter
---
Implemented with the original CHIP-8 specs in mind.\
As the options to enable/disable some quirks from variants are added, implementations for S-CHIP and XO-CHIP might be included.

Compiled and tested on Linux only. Has SDL2 as dependency.

### Building
Just run `make` while in the project directory.

### Running
`chip8 PATH [--freq <int>] [--sound <double>] [--bg "#RRGGBB"] [--fg "#RRGGBB"]`

note: after running `make` the binary file is saved in the `bin/` directory.\
Example:
`bin/chip8 examples/tingo.ch8 --freq -1 --bg "#000015" --fg "#FFD077" --sound 170`

### Sound
A sine wave sound is played for the buzzer. You can adjust its frequency with the ``--sound <double>`` flag.

### Keys
This is the CHIP-8 keypad layout on a QWERTY keyboard:

```
     QWERTY                      CHIP-8
[1] [2] [3] [4]             [1] [2] [3] [C]
[Q] [W] [E] [R]    <===>    [4] [5] [6] [D]
[A] [S] [D] [F]             [7] [8] [9] [E]
[Z] [X] [C] [V]             [A] [0] [B] [F]
```
---
![image](https://github.com/user-attachments/assets/00f44eb4-8b0c-4b12-9141-289d24041067)

![image](https://github.com/user-attachments/assets/071ab88e-759d-4898-93c4-8a1e30f0bafa)

![image](https://github.com/user-attachments/assets/2297e778-aa5e-49d8-b00b-df83e68c14de)

