# 6C5816 Emulator
A WDC 65C816 emulator written in C++ with support for compiler-generated C programs, interactive debugging, and UART-based I/O.

## Requirements
Input files must be in **S28 (Motorola S-record, 24-bit)** format. If you're assembling with 64tass, generate this with:
 
```
64tass --m65816 --s-record -o file_name.s28 file_name.asm
```

## Usage (Order doesn't matter)

```
./emu816 <file.s28> [-s] [-d] [-m <bytes> <address>]
```
### Modes
 
**Run mode** (default)
- Executes the program to completion (or until `STP`) and shows final output.
- Supports memory viewing:
```
  -m <bytes_to_read> <address>
```

**Step mode** (`-s`)
- Prints a full trace of every instruction executed.

### Decimal mode (`-d`)
Available in both Run and Step mode. 
Displays the `A`, `X`, and `Y` registers in decimal instead of hex. All other output (addresses, memory dumps, etc.) is unaffected.

## I/O
 
### 1.WDM instruction
Special WDM commands are supported :
 
| Byte | Function |
|------|----------|
| `$00` | Print a single character |
| `$01` | Print a null-terminated string |
| `$02` | Read a character into `$FFDC` |
| `$FE` | Trigger IRQ (executes the IRQ handler) |
| `$FF` | Trigger NMI |

For `$00`/`$01`, a 3-byte address must follow the instruction.
Example (64tass):
```asm
.byte $00, $80, $00   ; little-endian -> bank:addr = 00:8000
```
### 2.UART terminal
Memory-mapped UART emulation:
 
| Address | Function |
|---------|----------|
| `$FFD6` | Write a byte to transmit over UART |
| `$FFD7` | Read UART status |

### Halting
 
Execution stops when the `STP` opcode is encountered.

## Development Notes

- To execute C programs, the examples in this repository are compiled using the **Calypsi C Compiler** targeting the WDC 65C816 processor.
- A utility is provided to extract the compiled program's entry point and patch the boot ROM. The call to this utility is commented out by default and should be enabled when compiling C programs.
- **64tass** is used to assemble the boot ROM and generate the final **Motorola S28** image loaded by the emulator.

## Example

An example showing the complete workflow for compiling and running a C program is available in the
[`Examples`](examples) directory.

## Credits

- [`Andrew Jacobs`](https://github.com/andrew-jacobs) 65C816 Emulator — studied as a learning reference for understanding the processor architecture and validating     instruction behavior.
- Western Design Center (WDC) — creator of the W65C816 processor and official programming manuals used as the primary architectural reference.
- All implementation in this repository was written independently.
  
## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
