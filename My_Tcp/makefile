.PHONY:all
all:server client server_pro

server_pro:server_pro.c
	gcc -o $@ $^

server:server.c
	gcc -o $@ $^

client:client.c
	gcc -o $@ $^

.PHONY:clean

clean:
	rm server client server_pro
