#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <wait.h>
#include <cstring>
#include <errno.h>
#include <cstdlib>

int main() {
	while (true) {
		char c;
		char args[255][255];
		int curLine = 0, curSymb = 0;
		c = getchar();
		while (c != EOF) {
			if (c == '\n') {
				args[curLine][curSymb] = '\0';
				curLine++;
				curSymb = 0;
				break;
			} else if (c == ' ') {			
				args[curLine][curSymb] = '\0';
				curLine++;
				curSymb = 0;
			} else {
				args[curLine][curSymb] = c;
				curSymb++;
			}
			c = getchar();
		}
		if (c == EOF) {
			return 1;
		}
		char* us[256];
		us[curLine] = NULL;
		for (int i = 0; i < curLine; i++) 
			us[i] = args[i];
		if (strcmp(us[0], "exit") == 0) {
			std::cout << "End of the program" << std::endl;
			return 0;
		}
		const pid_t pid = fork();
		if (pid == -1) {
			std::cout << "Sorry, an error occurred in fork process" << std::endl;
			return 1;
		}
		int endcode = 0;
		if (!pid) {
			std::cout << "Executable program: " << us[0] << std::endl;
			endcode = execve(us[0], us, (char* const*)NULL);
			switch (errno) {
				case E2BIG: std::cout << strerror << std::endl; break;
				case EACCES: std::cout << "Permission is denied" << std::endl; break;
				case EAGAIN: std::cout << "Reach resourse limit" << std::endl; break;
				case EFAULT: std::cout << "filename outside your accessible address space" << std::endl; break;
				case EINVAL: std::cout << "An ELF executable had more than one PT_INTERP segment" << std::endl; break;
				case EIO: std::cout << "An I/O error occurred" << std::endl; break;
				case EISDIR: std::cout << "An ELF interpreter was a directory" << std::endl; break;
				case ELIBBAD: std::cout << "An ELF interpreter was not in a recognized format" << std::endl; break;
				case ELOOP: std::cout << "Too many symbolic links in resolving filename" << std::endl; break;
				case EMFILE: std::cout << "Reach limit on the number of open file descriptors" << std::endl; break;
				case ENAMETOOLONG: std::cout << "filename too long" << std::endl; break;
				case ENFILE: std::cout << "Reach limit on the total number of open files" << std::endl; break;
				case ENOENT: std::cout << strerror << std::endl; break;
				case ENOEXEC: std::cout << "An executable is not in a recognized format" << std::endl; break;
				case ENOMEM: std::cout << "Insufficient kernel memory was available" << std::endl; break;
				case ENOTDIR: std::cout << "A component of the path prefix of filename is not a directory" << std::endl; break;
				case EPERM: std::cout << "The filesystem is mounted nosuid" << std::endl; break;
				case ETXTBSY: std::cout << "Executable was open for writing by one or more processes" << std::endl; break;
			}
			exit(endcode);
		} else {
			int currentwaitpid = waitpid(pid, &endcode, 0);
			while (currentwaitpid == !pid) {
				if (currentwaitpid == -1) {
					std::cout << "Sorry, an error occurred in waitpid process" << std::endl;
					return 1;
				}
				currentwaitpid = waitpid(pid, &endcode, 0);
			};
			switch (errno) {
				case ECHILD: std::cout << "The calling process does not have any unwaited-for children" << std::endl; break;
				case EINTR: std::cout << "The process specified by pid does not exist" << std::endl; break;
				case EINVAL: std::cout << "WNOHANG was no set or SIGCHLD was caught" << std::endl; break;
			}
			std::cout << "End of execution process with code " << WEXITSTATUS(endcode) << std::endl;
		}
	}
}
