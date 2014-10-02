.PHONY : trereo

DEBUG ?= 1
export DEBUG


.PHONY: build_env
build_env:
ifeq ($(DEBUG), 1)
	@echo "Compiling with Debugging Information"
else
	@echo "Compiling for Production"
endif

trereo : build_env
	gcc -c ../source/analyser.c ../source/utilities/prof_time.c ../libraries/delay/delay.c ../libraries/level/single.c ../libraries/pan/pan.c ../libraries/trereo/trereo.c -std=c99
	ar cr liblibrary.a analyser.o prof_time.o delay.o single.o pan.o trereo.o
	g++ -c ../applications/trereo/main.cpp -I/c/"Program Files (x86)"/SFML/include
	g++ main.o -L/c/Users/Mark/Desktop/Analyser/build -o Trereo -L/c/"Program Files (x86)"/SFML/lib -llibrary -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
	./trereo

.PHONY : histogram

histogram :
	gcc -c ../source/analyser.c ../source/utilities/prof_time.c ../libraries/delay/delay.c ../libraries/level/single.c ../libraries/pan/pan.c ../libraries/trereo/trereo.c -std=c99
	ar cr liblibrary.a analyser.o prof_time.o delay.o single.o pan.o trereo.o
	g++ -c ../applications/histogram/main.cpp -I/c/"Program Files (x86)"/SFML/include
	g++ main.o -L/c/Users/Mark/Desktop/Analyser/build -o Histogram -L/c/"Program Files (x86)"/SFML/lib -llibrary -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
	./histogram

waveform :
	gcc -c ../source/analyser.c ../source/utilities/prof_time.c ../libraries/delay/delay.c ../libraries/level/single.c ../libraries/pan/pan.c ../libraries/trereo/trereo.c -std=c99
	ar cr liblibrary.a analyser.o prof_time.o delay.o single.o pan.o trereo.o
	g++ -c ../applications/waveform/main.cpp -I/c/"Program Files (x86)"/SFML/include
	g++ main.o -L/c/Users/Mark/Desktop/Analyser/build -o Waveform -L/c/"Program Files (x86)"/SFML/lib -llibrary -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
	./waveform

spectrogram :
	gcc -c ../source/analyser.c ../source/utilities/prof_time.c ../libraries/delay/delay.c ../libraries/level/single.c ../libraries/pan/pan.c ../libraries/trereo/trereo.c -std=c99
	ar cr liblibrary.a analyser.o prof_time.o delay.o single.o pan.o trereo.o
	g++ -c ../applications/waveform/main.cpp -I/c/"Program Files (x86)"/SFML/include
	g++ main.o -L/c/Users/Mark/Desktop/Analyser/build -o Waveform -L/c/"Program Files (x86)"/SFML/lib -llibrary -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
	./waveform