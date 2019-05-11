#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main(int argc, const char* argv[]) {
	if (argc != 3) {
		perror("Error: required 2 arguments: address and line");
		exit(EXIT_FAILURE);
	}
	int sockfd, g_info;
	struct sockaddr_in servinfo;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	servinfo.sin_family = AF_INET;
	servinfo.sin_port = htons(2592);
	servinfo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	int conn = connect(sockfd, (struct sockaddr *) &servinfo, sizeof(servinfo));
	if (conn == -1) {
		perror("connect");
		exit(EXIT_FAILURE);
	}
	while (true) {
		std::string check = argv[2];
		if (check == "exit") break;
		char buf[1024];
		ssize_t count = 0;
		while (count != strlen(argv[2])) {
			ssize_t tmp_count = send(sockfd, argv[2] + count, strlen(argv[2]) - count, 0);
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
			spec_count = send(sockfd, symb, 1, 0);
		}
		ssize_t receive_count = 0;
		while (true) {
			ssize_t tmp_receive_count = recv(sockfd, buf + receive_count, 1024, 0);
			if (tmp_receive_count == -1) {
				perror("receive");
				exit(EXIT_FAILURE);
			}
			receive_count += tmp_receive_count;
			if (buf[receive_count - 1] == '\0') break;
		}
		buf[receive_count - 1] = '\0';
		printf("%s\n", buf);
		scanf("%s", argv[2]);
	}
	close(sockfd);
	exit(EXIT_SUCCESS);
}