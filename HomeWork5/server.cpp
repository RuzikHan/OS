#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, const char* argv[]) {
	if (argc != 2) {
		perror("Error: required 1 argument address");
		exit(1);
	}
	int sockfd, clfd;
	char buf[1024];
	struct sockaddr_in servinfo;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
    	perror("socket");
    	exit(EXIT_FAILURE);
    }
    servinfo.sin_family = AF_INET;
    servinfo.sin_port = htons(2592);
    servinfo.sin_addr.s_addr = htonl(INADDR_ANY);
    int bnd = bind(sockfd, (struct sockaddr *) &servinfo, sizeof(servinfo));
    if (bnd == -1) {
    	perror("bind");
    	exit(EXIT_FAILURE);
    }
    int list = listen(sockfd, 1);
    if (list == -1) {
    	perror("listen");
    	exit(EXIT_FAILURE);
    }
    std::cout << "Server: waiting for connection" << std::endl;
    while (true) {
    	clfd = accept(sockfd, NULL, NULL);
    	if (clfd == -1) {
    		perror("accept");
    		exit(EXIT_FAILURE);
    	}
    	while (true) {
    		ssize_t rec_count = 0;
    		while (true) {
    			ssize_t tmp_rec_count = recv(clfd, buf + rec_count, 1024, 0);
    			if (tmp_rec_count <= 0) break;
    			rec_count += tmp_rec_count;
    			if (buf[rec_count - 1] == '\0') break;
    		}
    		if (rec_count <= 0) break;
    		buf[rec_count - 1] = '\0';
    		strcat(buf, ", nice to meet you");
    		ssize_t count = 0;
    		while (count != strlen(buf)) {
    			ssize_t tmp_count = send(clfd, buf + count, strlen(buf) - count, 0);
    			if (tmp_count == -1) {
    				perror("send");
    				exit(EXIT_FAILURE);
    			}
    			count += tmp_count;
    		}
    		char symb[1];
    		symb[0] = '\0';
    		ssize_t spec_count = 0;
    		while (spec_count != 1) {
    			spec_count = send(clfd, symb, 1, 0);
    		}
    	}
    	close(clfd);
    }
}