all:main


main:main.c token.c scanner.c
	g++ -c main.c
	g++ -c scanner.c
	g++ -c token.c
	g++ -c parser.c
	g++ -c tiger_log.c
	g++ main.o parser.o scanner.o token.o tiger_log.o  -o main
clean:
	rm main 
	rm main.o 
	rm scanner.o
	rm token.o
	rm parser.o
	rm tiger_log.o

