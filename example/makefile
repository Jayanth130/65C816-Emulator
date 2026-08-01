<<<<<<< HEAD
ACT=boot.asm
SR=boot.s28
SRC=kernel.c
OBJ=kernel.o
BIN=kernel.bin
RAW=kernel_raw.bin
SCM=kernel.scm

all: compile link extract assemble 

compile:
	cc65816 --target SNES -c -o $(OBJ) $(SRC)

link: compile
	ln65816 --target SNES --cstartup none -o $(BIN) $(SCM) $(OBJ) --list-file kernel.lst

extract: link
	gcc extraction.c -o extract
	./extract

assemble: extract
	64tass --m65816 --s-record -o $(SR) $(ACT)
	
clean:
	rm -f $(OBJ) $(BIN) $(RAW) $(SR) output.txt extract
=======
all:
	g++ -std=c++26 main.cpp CPU.cpp mem.cpp -o emu816	
clean:
	rm -r emu816
>>>>>>> 6a6145fd489df071cddd432563ff192298f1a5cb
