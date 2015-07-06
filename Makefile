all:
	gcc -Wall -D_GNU_SOURCE -fpic -shared -o pinthread.so pinthread.c -ldl -lpthread
	gcc -Wall -lpthread dotprod_mutex.c -o dotprod_mutex

install:
	install -m 0644 pinthread.so /usr/lib
	ldconfig

clean:
	rm -f pinthread.so
	rm -f dotprod_mutex
