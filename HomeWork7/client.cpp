#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/un.h>

#define MY_PATH "my_socket" 

int main(int argc, const char* argv[]) {
	char argument[1024];
	int sockfd, g_info, epoll_fd, epollctl;
	struct sockaddr_un servinfo;
	static struct epoll_event event, events[256];
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	servinfo.sun_family = AF_UNIX;
	strcpy(servinfo.sun_path, MY_PATH);
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
	int fdop = -1;
	std::cout << "Enter file name in first line" << "\n";
	std::cout << "After then you can write lines in file" << "\n";
	std::cout << "To stop writing type exit" << "\n";
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
						char symbf[1];
						symbf[0] = '\0';
						ssize_t spec_count = 0;
						while (spec_count != 1) {
							spec_count = send(sockfd, symbf, 1, 0);
						}
						f = false;
						break;
					}
        			if (fdop == -1) {
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
						int wr = write(fdop, argument, strlen(argument));
						if (wr == -1) {
							perror("write");	
							close(sockfd);
							exit(EXIT_FAILURE);
						}
					}
        		} else {
        			if (events[i].data.fd == sockfd) {
        				if (fdop == -1) {
							struct msghdr msg;
							memset(&msg, 0, sizeof(msg));
							char buf[CMSG_SPACE(sizeof(int))];
							msg.msg_control = buf;
							msg.msg_controllen = sizeof(buf);
							ssize_t r_c = recvmsg(sockfd, &msg, 0);
							if (r_c < 0) {
								perror("recvmsg");
								close(sockfd);
								exit(EXIT_FAILURE);
							}
							struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
							if (cmsg == NULL || cmsg->cmsg_type != SCM_RIGHTS) {
								perror("recvmsg");
								close(sockfd);
								exit(EXIT_FAILURE);
							}
							memcpy(&fdop, CMSG_DATA(cmsg), sizeof(int));
						}
        			}
        		}
        	}

        }
	}
    close(sockfd);
    close(epoll_fd);
    exit(EXIT_SUCCESS);
}