
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

int abs(int x);
int inc(int x);
int dec(int x);

int square(int x);
int cube(int x);

int main() {
	printf("%s\n", "Static library check");
	int x = inc(1);
	printf("%s%d\n", "inc(1) = ", x);
	x = dec(x);
	printf("%s%d\n", "dec(2) = ", x);
	x = abs(-5);
	printf("%s%d\n", "abs(-5) = ", x);
	printf("%s\n", "Dynamic library check 1");
	x = square(3);
	printf("%s%d\n", "square(3) = ", x);
	x = cube(7);
	printf("%s%d\n", "cube(7) = ", x);
	printf("%s\n", "Dynamic library check 2");
	void *tmp;
	tmp = dlopen("libthird_lib.so", RTLD_LAZY);
	if (tmp == NULL) {
		dlerror();
		exit(EXIT_FAILURE);
	}
	void *first_func;
	first_func = dlsym(tmp, "add");
	if (first_func == NULL) {
		dlerror();
		exit(EXIT_FAILURE);
	}
	int (*add) (int, int) = (int (*) (int, int)) first_func;
	void *second_func;
	second_func = dlsym(tmp, "multi");
	if (second_func == NULL) {
		dlerror();
		exit(EXIT_FAILURE);
	}
	int (*multi) (int, int) = (int (*) (int, int)) second_func;
	x = add(15, 7);
	printf("%s%d\n", "add: 15 + 7 = ", x);
	x = multi(6, 8);
	printf("%s%d\n", "multi: 6 * 8 = ", x);
	if (dlclose(tmp) != 0) {
		dlerror();
	}
}