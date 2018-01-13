all:main


main:main.c token.c scanner.c parser.c tiger_log.c absyn.c tiger_assert.c types.c semant.c escape.c temp.c tree.c
	g++ -g -c -Wno-write-strings  main.c
	g++ -g -c -Wno-write-strings scanner.c
	g++ -g -c -Wno-write-strings token.c
	g++ -g -c -Wno-write-strings parser.c
	g++ -g -c -Wno-write-strings tiger_log.c
	g++ -g -c -Wno-write-strings absyn.c
	g++ -g -c -Wno-write-strings tiger_assert.c
	g++ -g -c -Wno-write-strings types.c
	g++ -g -c -Wno-write-strings semant.c
	g++ -g -c -Wno-write-strings escape.c
	g++ -g -c -Wno-write-strings temp.c
	g++ -g -c -Wno-write-strings tree.c
	g++ main.o parser.o scanner.o token.o tiger_log.o absyn.o tiger_assert.o types.o semant.o escape.o temp.o tree.o -o main
clean:
	rm main 
	rm main.o 
	rm scanner.o
	rm token.o
	rm parser.o
	rm tiger_log.o
	rm absyn.o
	rm tiger_assert.o
	rm types.o
	rm semant.o
	rm escape.o
	rm temp.o
	rm tree.o

