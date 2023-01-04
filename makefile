all: main

main: 
	gcc main.c -lncurses -o bin/main
	chmod +x bin/main
	./bin/main

clean:
	rm main
