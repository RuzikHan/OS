#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/un.h>

#define MY_PATH "my_socket"

int main(int argc, const char* argv[]) {
	int sockfd, clfd, epoll_fd, epollctl;
    int files[1024];
    for (int i = 0; i < 1024; i++)
        files[i] = -1;
	char buf[1024];
	struct sockaddr_un servinfo;
    static struct epoll_event event, events[256];
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
    	perror("socket");
    	exit(EXIT_FAILURE);
    }
    servinfo.sun_family = AF_UNIX;
    strcpy(servinfo.sun_path, MY_PATH);
    int ttt = 1;
    int setinfo = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &ttt, sizeof (ttt));
    if (setinfo == -1) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    unlink(MY_PATH);
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
                struct sockaddr_un client_info;
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
                if (rec_count == 1 && buf[0] == '\0' && files[i] != -1) {
                    close(files[i]);
                    files[i] = -1;
                } else {
                    buf[rec_count - 1] = '\0';
                    int fdop = open(buf, O_RDONLY | O_WRONLY | O_CREAT, 00777);
                    if (fdop < 0) {
                        perror("open");
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }
                    files[i] = fdop;
                    struct msghdr msg = {0};
                    struct cmsghdr *cmsg;
                    int *fdptr;
                    char iobuf[1];
                    struct iovec iov;
                    iov.iov_base = iobuf;
                    iov.iov_len = sizeof(iobuf);
                    union {
                    char buf[CMSG_SPACE(sizeof(fdop))];
                    struct cmsghdr align;
                    } st;
                    msg.msg_iov = &iov;
                    msg.msg_iovlen = 1;
                    msg.msg_control = st.buf;
                    msg.msg_controllen = sizeof(st.buf);
                    cmsg = CMSG_FIRSTHDR(&msg);
                    cmsg->cmsg_level = SOL_SOCKET;
                    cmsg->cmsg_type = SCM_RIGHTS;
                    cmsg->cmsg_len = CMSG_LEN(sizeof(fdop));
                    fdptr = (int *) CMSG_DATA(cmsg);
                    memcpy(fdptr, &fdop, sizeof(fdop));
                    int s_c = sendmsg(clfd, &msg, 0);
                    if (s_c == -1) {
                        perror("sendmsg");
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }
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