# Anton Stengel & Jake Intrater
# makefile for directoryfiletrees part 3
# COS217

all: ft

# builds final tests
ft: ft_client.o ft.o nodeDir.o nodeFile.o dynarray.o
	gcc217 -g ft.o ft_client.o nodeDir.o nodeFile.o dynarray.o -o ft

# builds intermidiaries
ft_client.o: ft_client.c ft.h
	gcc217 -g -c ft_client.c

ft.o: ft.c ft.h a4def.h dynarray.h nodeFile.h nodeDir.h
	gcc217 -g -c ft.c

nodeDir.o: nodeDir.c nodeDir.h nodeFile.h
	gcc217 -g -c nodeDir.c
	
nodeFile.o: nodeFile.c nodeFile.h nodeDir.h
	gcc217 -g -c nodeFile.c

dynarray.o: dynarray.c dynarray.h
	gcc217 -g -c dynarray.c




