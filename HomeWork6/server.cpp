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
		exit(1);
	}
	int sockfd, clfd, epoll_fd, epollctl;
	char buf[1024];
	struct sockaddr_in servinfo;
    static struct epoll_event event, events[256];
    sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd == -1) {
    	perror("socket");
    	exit(EXIT_FAILURE);
    }
    int ttt = 1;
    int setinfo = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &ttt, sizeof (ttt));
    if (setinfo == -1) {
    	perror("setsockopt");
    	close(sockfd);
    	exit(EXIT_FAILURE);
    }
    servinfo.sin_family = AF_INET;
    servinfo.sin_port = htons(2593);
    servinfo.sin_addr.s_addr = INADDR_ANY;
    int bnd = bind(sockfd, (struct sockaddr *) &servinfo, sizeof(servinfo));
    if (bnd == -1) {
    	perror("bind");
    	close(sockfd);
    	exit(EXIT_FAILURE);
    }
    int list = listen(sockfd, 1);
    if (list == -1) {
    	perror("listen");
    	close(sockfd);
    	exit(EXIT_FAILURE);
    }
    epoll_fd = epoll_create(256);
    if (epoll_fd == -1) {
        perror("epoll_create");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    event.data.fd = sockfd;
    event.events = EPOLLIN;
    epollctl = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event);
    if (epollctl == -1) {
        perror("epoll_ctl");
        close(sockfd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Server: waiting for connection" << std::endl;
    while (true) {
        int wt;
        wt = epoll_wait(epoll_fd, events, 256, 100000);
        if (wt == -1) {
        	perror("epoll_wait");
        	close(sockfd);
        	close(epoll_fd);
        	exit(EXIT_FAILURE);
        }
        for (int i = 0; i < wt; i++) {
            if (sockfd == events[i].data.fd) {
                struct sockaddr_in client_info;
                socklen_t client_len;
                client_len = sizeof(client_info);
                clfd = accept(sockfd, (struct sockaddr *) &client_info, &client_len);
                if (clfd == -1) {
                	if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) continue; else {
                    	perror("accept");
                    	break;
                	}
                }
                static struct epoll_event cl_event;
                cl_event.data.fd = clfd;
                cl_event.events = EPOLLIN | EPOLLRDHUP;
                int cl_ctl = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clfd, &cl_event);
                if (cl_ctl == -1) {
                    perror("epoll_ctl");
                    break;
                }
            } else if (events[i].events == EPOLLIN) {
            	clfd = events[i].data.fd;
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
            } else {
                close(events[i].data.fd);                
            }
        }
    }
    close(sockfd);
    close(epoll_fd);
    exit(EXIT_SUCCESS);
}