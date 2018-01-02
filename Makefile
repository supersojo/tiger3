all:main.exe


main.exe:main.c token.c scanner.c
	cl /c /TP /EHsc main.c
	cl /c /TP /EHsc scanner.c
	cl /c /TP /EHsc token.c
	cl /c /TP /EHsc parser.c

	link /subsystem:console main.obj parser.obj scanner.obj token.obj /out:main.exe
clean:
	rm main.exe 
	rm main.obj 
	rm scanner.obj
	rm token.obj
	rm parser.obj

