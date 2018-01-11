all:main


main:main.c token.c scanner.c tiger_log.c absyn.c tiger_assert.c
	g++ -g -c -Wno-write-strings  main.c
	g++ -g -c -Wno-write-strings scanner.c
	g++ -g -c -Wno-write-strings token.c
	g++ -g -c -Wno-write-strings parser.c
	g++ -g -c -Wno-write-strings tiger_log.c
	g++ -g -c -Wno-write-strings absyn.c
	g++ -g -c -Wno-write-strings tiger_assert.c
	g++ main.o parser.o scanner.o token.o tiger_log.o absyn.o tiger_assert.o  -o main
clean:
	rm main 
	rm main.o 
	rm scanner.o
	rm token.o
	rm parser.o
	rm tiger_log.o
	rm absyn.o
	rm tiger_assert.o

