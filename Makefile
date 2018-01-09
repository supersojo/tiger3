all:main


main:main.c token.c scanner.c
	g++ -g -c main.c
	g++ -g -c scanner.c
	g++ -g -c token.c
	g++ -g -c parser.c
	g++ -g -c tiger_log.c
	g++ main.o parser.o scanner.o token.o tiger_log.o  -o main
clean:
	rm main 
	rm main.o 
	rm scanner.o
	rm token.o
	rm parser.o
	rm tiger_log.o

