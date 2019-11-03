//Ryan McCrory
//Simple Web Server
//asgn1

#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>
#include <cstdint>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>

using namespace std;

//get method, copies a file from the server and puts it in the client directory
void get(int32_t cl, char *file_name, uint32_t bytes){
	//determine content length
	long length = 0;
	struct stat st;
	if (stat(file_name, &st)){
		length = 0;
	} else {
		length = st.st_size;
	}
	//open file if exists
	int32_t local_file = open(file_name, O_RDWR);
	//error if file does not exist
	if (local_file == -1){
		char not_found_header [55];
		sprintf(not_found_header, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
		send(cl, not_found_header, strlen(not_found_header), 0);
	}
	//send ok header
	char ok_header [55];
	sprintf(ok_header, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", length);
	send(cl, ok_header, strlen(ok_header), 0);
	//read/ write data
	char *buffer = (char *) malloc(bytes * sizeof(char));
	int32_t read_bytes = read(local_file, buffer, bytes);
	while (read_bytes > 0){
		send(cl, buffer, read_bytes, 0);
		read_bytes = read(local_file, buffer, bytes);
	}
	//free memory
	free(buffer);
}

//put method, copies a file from the client and puts it in the server directory
void put(int32_t cl, char *file_name, uint32_t bytes){
	//create buffer and recieve data from client
	char *buffer = (char *) malloc(bytes * sizeof(char));
	int32_t read_bytes = recv(cl, buffer, bytes, 0);
	//create new file with proper permissions
	int32_t new_file = open(file_name, O_CREAT | O_WRONLY, 0644);
	//send header
	char ok_header [55];
	sprintf(ok_header, "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n");
	send(cl, ok_header, strlen(ok_header), 0);
	//write to file
	while (read_bytes > 0){
		write(new_file, buffer, read_bytes);
		read_bytes = recv(cl, buffer, bytes, 0);
	}
	//close file and free memory
	close(new_file);
	free(buffer);
}

//parse method, argument is client file descriptor
void parse (int32_t cl){
	//read incoming data
	uint32_t bytes = 1024;
	char *buffer = (char *) malloc(bytes * sizeof(char));
	read(cl, buffer, bytes);
	//declare variables
	char *action = (char *) malloc(bytes * sizeof(char));
	char *file_name = (char *) malloc(bytes * sizeof(char));
	char *http = (char *) malloc(bytes * sizeof(char));
	//parse data using sscanf
	sscanf(buffer, "%s %s %s \n", action, file_name, http);
	//error checking for not HTTP/1.1
	if (strcmp(http, "HTTP/1.1") != 0){
		char bad_header [55];
		sprintf(bad_header, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
		send(cl, bad_header, strlen(bad_header), 0);
	}
	//error checking for bad resource/file_name: not 27 chars
	if (strlen(file_name) != 27){
		char bad_header [55];
		sprintf(bad_header, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
		send(cl, bad_header, strlen(bad_header), 0);
	}
	//check that resource is only ascii characters
	if (strspn(file_name, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_") != 27){
		char bad_header [55];
		sprintf(bad_header, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
		send(cl, bad_header, strlen(bad_header), 0);
	}
	//call put or get depending on what action is
	if (strcmp(action, "PUT") == 0){
		put(cl, file_name, bytes);
	} else if (strcmp(action, "GET") == 0){
		get(cl, file_name, bytes);
	} else {
		//if improper message, send error: not put or get
		char bad_header [55];
		sprintf(bad_header, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
		send(cl, bad_header, strlen(bad_header), 0);
	}
	//free memory
	free(buffer);
	free(action);
	free(file_name);
}

//listen to put and get requests on user specified server
int main (int argc, char *argv[]){
	//if no args
	if (argc == 1){
		cout << "error, no specified IP address" << "\n";
		exit(0);
	}
	//initialize hostname and port
	char * hostname;
	char * port = nullptr;
	//set port to 2nd arg if there is one, 80 if not
	if (argc == 2){
		sprintf(port, "80");
	} else {
		port = argv[2];
	}
	//set hostname to first argument
	hostname = argv[1];
	struct addrinfo *addrs, hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(hostname, port, &hints, &addrs);
	int main_socket = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
	int enable = 1;
	setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	bind(main_socket, addrs->ai_addr, addrs->ai_addrlen);
	listen(main_socket, 16);
	while (true){
		int32_t cl = accept(main_socket, NULL, NULL);
		//cl is now another socket that you can call
		//read/recv and write/send on to communicate with client
		//call parse with client file descriptor
		parse(cl);
	}
}
