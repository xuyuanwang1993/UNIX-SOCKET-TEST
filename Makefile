#Makefile
all:
	g++ -o unix_socket_test main.cpp unix_socket_helper.cpp
clean:
	-rm unix_socket_test
