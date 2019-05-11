#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/epoll.h>

int main(int argc, const char* argv[]) {
	if (argc != 2) {
		perror("Error: required 1 argument address");
		exit(EXIT_FAILURE);
	}
	char argument[1024];
	int sockfd, g_info, epoll_fd, epollctl;
	struct sockaddr_in servinfo;
	static struct epoll_event event, events[256];
	sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (sockfd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	servinfo.sin_family = AF_INET;
	servinfo.sin_port = htons(2593);
	servinfo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	int conn = connect(sockfd, (struct sockaddr *) &servinfo, sizeof(servinfo));
	bool if_connect = true;
	epoll_fd = epoll_create(256);
    if (epoll_fd == -1) {
        perror("epoll_create");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
	if ((conn == -1) && (errno == EINPROGRESS)) {
		if_connect = false;
	}
	event.events = EPOLLIN;
	event.data.fd = STDIN_FILENO;
	int in_epollctl = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
	if (in_epollctl == -1) {
		perror("epoll_ctl");
		close(sockfd);
		close(epoll_fd);
		exit(EXIT_FAILURE);
	}
	event.events = EPOLLIN;
	if (!if_connect) {
		event.events |= EPOLLOUT;
	}
	event.data.fd = sockfd;
	epollctl = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event);
	if (epollctl == -1) {
		perror("epoll_ctl");
		close(sockfd);
		close(epoll_fd);
		exit(EXIT_FAILURE);
	}
	bool f = true;
	while (f) {
		int wt;
		wt = epoll_wait(epoll_fd, events, 256, 100000);
		if (wt == -1) {
        	perror("epoll_wait");
        	close(sockfd);
        	close(epoll_fd);
        	exit(EXIT_FAILURE);
        }
        for (int i = 0; i < wt; i++) {
        	if (sockfd == events[i].data.fd && (events[i].events & EPOLLOUT) && !if_connect) {
        		int tmp = 0;
        		socklen_t len = sizeof(int);
        		int val = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &tmp, &len);
        		if (val != -1) {
        			if (tmp == 0) {
        				event.events = EPOLLIN;
        				event.data.fd = sockfd;
        				int n_epollctl = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sockfd, &event);
        				if (n_epollctl == -1) {
        					perror("epoll_ctl");
        					close(sockfd);
        					close(epoll_fd);
        					exit(EXIT_FAILURE);
        				}
        			} else {
        				perror("connection failed");
        				event.events = 0;
        				event.data.fd = sockfd;
        				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, &event);
        				close(sockfd);
        			}
        		}
        	} else {
        		if (events[i].data.fd == STDIN_FILENO) {
        			scanf("%s", argument);
        			std::string check = argument;
					if (check == "exit") {
						f = false;
						break;
					}
					ssize_t count = 0;
					while (count != strlen(argument)) {
						ssize_t tmp_count = send(sockfd, argument + count, strlen(argument) - count, 0);
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
        		} else {
        			if (events[i].data.fd == sockfd) {
        				char buf[1024];
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
        			}
        		}
        	}

        }
	}
    close(sockfd);
    close(epoll_fd);
    exit(EXIT_SUCCESS);
}