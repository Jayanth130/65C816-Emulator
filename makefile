all:
	g++ -std=c++26 main.cpp CPU.cpp mem.cpp -o emu816	
clean:
	rm -r emu816