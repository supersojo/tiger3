all:main


main:main.c token.c scanner.c
	g++ -c main.c
	g++ -c scanner.c
	g++ -c token.c
	g++ -c parser.c
	g++ main.o parser.o scanner.o token.o -o main
clean:
	rm main 
	rm main.o 
	rm scanner.o
	rm token.o
	rm parser.o

