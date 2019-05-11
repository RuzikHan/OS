#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <wait.h>

using namespace std;

struct linux_dirent {
	long d_ino;
	unsigned long d_off;
	unsigned short d_reclen; 
	char d_name[1024];
	char pad;
	char d_type;
};

unsigned int hardlink_num = -1;
int inod_num = -1, size_num = 0, size_mode = -2;
bool inod_flag = false, hardlink_flag = false, filename_flag = false, exec_flag = false;
const char* file_name;
const char* exec_path;

void bd(string path) {
	int fd, nread, bpos;
	char buf[1024], d_type;
    struct linux_dirent *d;
    fd = open(path.c_str(), O_RDONLY, O_DIRECTORY);
    if (fd == -1) {
    	perror(path.c_str());
    	return;
    }
    for (;;) {
    	nread = syscall(SYS_getdents, fd, buf, 1024);
    	if (nread == -1) {
    		perror(path.c_str());
    		return;
    	}
    	if (nread == 0)
    		break;
    	for (bpos = 0; bpos < nread;) {
    		d = (struct linux_dirent *) (buf + bpos);
    		d_type = *(buf + bpos + d->d_reclen - 1);
    		bpos += d->d_reclen;
    		struct stat filebuf;
    		int stat_flag = stat(path.c_str(), &filebuf);
    		if (stat_flag == -1) {
    			perror(path.c_str());
    			exit(EXIT_FAILURE);
    		}
    		bool flag = true;
    		if (inod_flag) {
    			if (inod_num != d->d_ino) flag = false;
    		}
    		if (filename_flag) {
    			string tmp0 = file_name;
    			string tmp1 = d->d_name;
    			if (tmp0 != tmp1) flag = false;
    		}
    		if (size_mode == -1) {
    			if (size_num <= filebuf.st_size) flag = false;
    		}
    		if (size_mode == 0) {
    			if (size_num != filebuf.st_size) flag = false;
    		}
    		if (size_mode == 1) {
    			if (size_num >= filebuf.st_size) flag = false;
    		}
    		if (hardlink_flag) {
    			if (hardlink_num != filebuf.st_nlink) flag = false;
    		}
    		string s = d->d_name;
    		if (s == ".") flag = false;
    		if (s == "..") flag = false;
    		if (exec_flag && flag) {
    			struct stat execbuf;
    			int exec_stat = stat(exec_path, &execbuf);
    			if (exec_stat == -1) {
    				perror(exec_path);
    				exit(EXIT_FAILURE);
    			}
    			if (!S_ISDIR(execbuf.st_mode)){
    				char* us[3];
    				string tmp = path;
    				tmp += '/';
    				tmp += d->d_name;
    				us[0] = (char*) exec_path;
    				us[1] = (char*) tmp.c_str();
    				us[2] = NULL;
    				const pid_t pid = fork();
    				if (pid == -1) {
    					perror(exec_path);
    					exit(EXIT_FAILURE);
    				}
    				int endcode = 0;
					if (!pid) {
    					endcode = execve(us[0], us, (char* const*) NULL);
    					if (endcode < 0) {
    						string exec_err = exec_path;
    						perror((exec_err + " " + tmp).c_str());
    					}
    					exit(endcode);
					} else {
						int currentwaitpid = waitpid(pid, &endcode, 0);
							while (currentwaitpid == !pid) {
								if (currentwaitpid == -1) {
									string exec_err = exec_path;
									perror((exec_err + " " + tmp).c_str());
								}
							currentwaitpid = waitpid(pid, &endcode, 0);
						}
					}
    			}
    		}
    		if (d_type == DT_DIR && s != "." && s != "..") {
    			string pt = path;
    			pt += '/';
    			pt += s;
    			bd(pt);
    		}
    		if (flag) cout << path << "/" << s << endl;
    	}
    }
    if (close(fd) < 0) {
    	perror(path.c_str());
    }
}

void print_error(string error_message){
	cout << error_message << endl;
	exit(1);
}

bool num_check(string s) {
	for (unsigned int i = 0; i < s.length(); i++) {
		if (s[i] != '1' && s[i] != '2' && s[i] != '3' && s[i] != '4' && s[i] != '5' && s[i] != '6' && 
			s[i] != '7' && s[i] != '8' && s[i] != '9' && s[i] != '0') return false;
	}
	return true;
}

int main(int argc, const char* argv[]){
	int fd, nread, bpos;
	char buf[1024], d_type;
    struct linux_dirent *d;
	int cur = 2;
	if (argc < 2) {
		print_error("Error: expected absolute path");
	}
	while (cur < argc) {
		string arg = argv[cur];
		if (arg == "-inum") {
			if (inod_flag) {
				print_error("Error: duplicate -inum argument");
			}
			if (cur == argc - 1) {
				print_error("Error: required num with -inum argument");
			}
			cur++;
			const char* tmp = argv[cur];
			string line_check = tmp;
			if (!num_check(line_check)) {
				print_error("Error: num with -inum argument must be a number");
			}
			inod_num = atoi(tmp);
			inod_flag = true;
			cur++;
		} else if (arg == "-name") {
			if (filename_flag) {
				print_error("Error: duplicate -name argument");
			}
			if (cur == argc - 1) {
				print_error("Error: required file name with -name argument");
			}
			cur++;
			file_name = argv[cur];
			filename_flag = true;
			cur++;
		} else if (arg == "-size") {
			if (size_mode != -2) {
				print_error("Error: duplicate -size argument");
			}
			if (cur == argc - 1) {
				print_error("Error: required size num with -size argument");
			}
			cur++;
			size_mode = -1;
			const char* tmp = argv[cur];
			string line_check = tmp;
			if (!num_check(line_check)) {
				print_error("Error: num with -size argument must be a number");
			}
			size_num = atoi(tmp);
			cur++;
		} else if (arg == "=size") {
			if (size_mode != -2) {
				print_error("Error: duplicate =size argument");
			}
			if (cur == argc - 1) {
				print_error("Error: required size num with =size argument");
			}
			cur++;
			size_mode = 0;
			const char* tmp = argv[cur];
			string line_check = tmp;
			if (!num_check(line_check)) {
				print_error("Error: num with =size argument must be a number");
			}
			size_num = atoi(tmp);
			cur++;
		} else if (arg == "+size") {
			if (size_mode != -2) {
				print_error("Error: duplicate +size argument");
			}
			if (cur == argc - 1) {
				print_error("Error: required size num with +size argument");
			}
			cur++;
			size_mode = 1;
			const char* tmp = argv[cur];
			string line_check = tmp;
			if (!num_check(line_check)) {
				print_error("Error: num with +size argument must be a number");
			}
			size_num = atoi(tmp);
			cur++;
		} else if (arg == "-nlinks") {
			if (hardlink_flag) {
				print_error("Error: duplicate -nlinks argument");
			}
			if (cur == argc - 1) {
				print_error("Error: required gardlink num with -nlinks argument");
			}
			cur++;
			const char* tmp = argv[cur];
			string line_check = tmp;
			if (!num_check(line_check)) {
				print_error("Error: num with -nlinks argument must be a number");
			}
			hardlink_num = atoi(tmp);
			hardlink_flag = true;
			cur++;
		} else if (arg == "-exec") {
			if (exec_flag) {
				print_error("Error: duplicate -exec argument");
			}
			if (cur == argc - 1) {
				print_error("Error: required exec path with -exec argument");
			}
			cur++;
			exec_path = argv[cur];
			exec_flag = true;
			cur++;
		} else {
			print_error("Error: bad arguments");
		}
	}
	fd = open(argc > 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY);
    if (fd == -1)
    	perror(argv[1]);
    for (;;) {
    	nread = syscall(SYS_getdents, fd, buf, 1024);
    	if (nread == -1) {
    		perror(argv[1]);
    		break;
    	}
    	if (nread == 0)
    		break;
    	for (bpos = 0; bpos < nread;) {
    		d = (struct linux_dirent *) (buf + bpos);
    		d_type = *(buf + bpos + d->d_reclen - 1);
    		bpos += d->d_reclen;
    		struct stat filebuf;
    		int stat_flag = stat(argv[1], &filebuf);
    		if (stat_flag == -1) {
    			perror(argv[1]);
    			exit(EXIT_FAILURE);
    		}
    		bool flag = true;
    		if (inod_flag) {
    			if (inod_num != d->d_ino) flag = false;
    		}
    		if (filename_flag) {
    			string tmp0 = file_name;
    			string tmp1 = d->d_name;
    			if (tmp0 != tmp1) flag = false;
    		}
    		if (size_mode == -1) {
    			if (size_num <= filebuf.st_size) flag = false;
    		}
    		if (size_mode == 0) {
    			if (size_num != filebuf.st_size) flag = false;
    		}
    		if (size_mode == 1) {
    			if (size_num >= filebuf.st_size) flag = false;
    		}
    		if (hardlink_flag) {
    			if (hardlink_num != filebuf.st_nlink) flag = false;
    		}
    		string s = d->d_name;
    		if (s == ".") flag = false;
    		if (s == "..") flag = false;
    		if (exec_flag && flag) {
    			struct stat execbuf;
    			int exec_stat = stat(exec_path, &execbuf);
    			if (exec_stat == -1) {
    				perror(exec_path);
    				exit(EXIT_FAILURE);
    			}
    			if (!S_ISDIR(execbuf.st_mode)){
    				char* us[3];
    				string tmp = argv[1];
    				tmp += '/';
    				tmp += d->d_name;
    				us[0] = (char*) exec_path;
    				us[1] = (char*) tmp.c_str();
    				us[2] = NULL;
    				const pid_t pid = fork();
    				if (pid == -1) {
    					perror(exec_path);
    					exit(EXIT_FAILURE);
    				}
    				int endcode = 0;
					if (!pid) {
    					endcode = execve(us[0], us, (char* const*) NULL);
    					if (endcode < 0) {
    						string exec_err = exec_path;
    						perror((exec_err + " " + tmp).c_str());
    					}
    					exit(endcode);
					} else {
						int currentwaitpid = waitpid(pid, &endcode, 0);
							while (currentwaitpid == !pid) {
								if (currentwaitpid == -1) {
									string exec_err = exec_path;
									perror((exec_err + " " + tmp).c_str());
								}
							currentwaitpid = waitpid(pid, &endcode, 0);
						}
					}
    			}
    		}
    		if (d_type == DT_DIR && s != "." && s != "..") {
    			string pt = argv[1];
    			pt += '/';
    			pt += s;
    			bd(pt);
    		}
    		if (flag) cout << argv[1] << "/" << s << endl;
    	}
    }
    if (close(fd) < 0) {
    	perror(argv[1]);
    }
}