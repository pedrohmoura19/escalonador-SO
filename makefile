all: escalonador teste15 teste30

escalonador: escalonador.c
	cc escalonador.c -o escalonador

teste15: teste15.c
	cc teste15.c -o teste15

teste30: teste30.c
	cc teste30.c -o teste30

run: escalonador
	./escalonador 

clean:
	rm -rf *.o *~ escalonador teste15 teste30