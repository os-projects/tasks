CFLAGS = -std=c99 -Wall -pedantic
CXXFLAGS = -std=c++11 -Wall -pedantic


all: client poll select

client: client.c Makefile
	gcc $(CFLAGS) client.c -o client

poll: server_poll.C Makefile
	g++ $(CXXFLAGS) server_poll.C -o poll

select: server_select.C Makefile
	g++ $(CXXFLAGS) server_select.C -o select
