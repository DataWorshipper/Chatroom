all: server client

server: src/server.cpp
	g++ -Iinclude src/server.cpp -o server -pthread

client: src/client.cpp
	g++ -Iinclude src/client.cpp -o client -pthread

clean:
	rm -f server client