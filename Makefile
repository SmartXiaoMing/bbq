bbq: bbq.c
	gcc -Wall -O6 -ansi -std=c99 -pthread -o bbq bbq.c

install: bbq
	sudo cp bbq /usr/bin/bbq

clean:
	rm bbq
