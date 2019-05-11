#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstdlib> 
#include <string.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

void print_error(std::string error_message){
	std::cout << error_message << std::endl;
	exit(1);
}

bool num_check(std::string s) {
	for (unsigned int i = 0; i < s.length(); i++) {
		if (s[i] != '1' && s[i] != '2' && s[i] != '3' && s[i] != '4' && s[i] != '5' && s[i] != '6' && 
			s[i] != '7' && s[i] != '8' && s[i] != '9' && s[i] != '0') return false;
	}
	return true;
}

int main(int argc, const char* argv[]) {
	if (argc != 2) {
		print_error("Error: required 1 int argument");
	}
	const char* tmp = argv[1];
	std::string line_check = tmp;
	if (!num_check(line_check)) {
		print_error("Error: argument must be int");
	}
	int num = atoi(tmp);
	unsigned char data[] = {0x55, 0x48, 0x89, 0xe5, 0x48, 0xc7, 0xc0, 0x04, 0x00, 0x00, 0x00,
							0x48, 0xf7, 0xe0, 0x90, 0x5d, 0xc3};
	data[7] = (char) num;
	if (creat("square", S_IRWXU) == -1) {
		print_error("Error, can't create file to copy machine code");
	}
	int f = open("square", O_RDWR);
	if (f == -1) {
		print_error("Error: can't open file to copy machine code");
	}
	if (write(f, data, sizeof(data)) == -1) {
		print_error("Error: can't write machine code to file");
	}
	if (close(f) == -1) {
		print_error("Error: can't close file");
	}
	int fd = open("square", O_RDONLY);
	if (fd == -1) {
		print_error("Error: can't open file to copy machine code");
	}
	void *addr;
	addr = mmap(NULL, sizeof(data), PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (addr == (void *) -1) {
		print_error("Error: can't allocate memory using mmap");
	}
	if (mprotect(addr, sizeof(data), PROT_EXEC) == -1) {
		print_error("Error: can't change memory prots using mprotect");
	}
    int (*func) (int) = (int (*) (int)) addr;
    std::cout << func(num) << std::endl;
    if (munmap(addr, sizeof(data)) == -1) {
    	print_error("Error: can't free allocated memory using munmap");
    }
    if (close(fd) == -1) {
		print_error("Error: can't close file");
	}
}