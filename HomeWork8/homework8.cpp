#include <iostream>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/mman.h>

std::string reg_lines[23] = {"REG_R8", "REG_R9", "REG_R10", "REG_R11", "REG_R12", "REG_R13", "REG_R14", "REG_R15", 
							 "REG_RDI", "REG_RSI", "REG_RBP", "REG_RBX", "REG_RDX", "REG_RAX", "REG_RCX", "REG_RSP", 
							 "REG_RIP", "REG_EFL", "RER_CSGSFS", "REG_ERR", "REG_TRAPNO", "REG_OLDMASK", "REG_CR2"};
jmp_buf jmp;

void mem_action(int sig, siginfo_t* sinfo, void* vcon) {
	if (sinfo->si_signo == SIGSEGV)
		longjmp(jmp, 1);
}

void my_action(int sig, siginfo_t* sinfo, void* vcon) {
	if (sinfo->si_signo == SIGSEGV) {
		std::cout << "SEG fault in " << sinfo->si_addr << " location" << "\n";
		ucontext_t* con = (ucontext_t*) vcon;
		std::cout << "Dumping registers" << "\n";
		for (int i = 0; i < NGREG; i++)
			std::cout << reg_lines[i] << " = " << con->uc_mcontext.gregs[i] << "\n";
		char* adr = (char*) sinfo->si_addr;
		adr = (char*) (adr - 5);
		std::cout << "Dumping memory" << "\n";
		for (int i = 0; i < 10; i++, adr++) {
			sigset_t sigset;
			sigemptyset(&sigset);
			sigaddset(&sigset, SIGSEGV);
			sigprocmask(SIG_UNBLOCK, &sigset, NULL);
			struct sigaction act;
			memset(&act, 0, sizeof(act));
			act.sa_flags = SA_SIGINFO;
			act.sa_sigaction = mem_action;
			act.sa_mask = sigset;
			int sig_act = sigaction(SIGSEGV, &act, NULL);
			if (sig_act < 0) {
				perror("memory sigaction");
				exit(EXIT_FAILURE);
			}
			if (setjmp(jmp) == 0) {
				int check = *(int*) adr;
				std::cout << "address = " << (ssize_t) adr << "\n";
			} else {
				std::cout << "bad address = " << (ssize_t) adr << "\n";
			}
		}
		exit(EXIT_SUCCESS);
	}
}

int main() {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = my_action;
	int sig_act = sigaction(SIGSEGV, &act, NULL);
	if (sig_act < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	char* line = "src";
	line[3] = 'q';
  	return 0;
}
