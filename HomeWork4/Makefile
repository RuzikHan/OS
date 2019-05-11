all:homework4.c libfirst_lib.a libsecond_lib.so libthird_lib.so
	gcc -c homework4.c
	gcc homework4.o -L. -lfirst_lib -Wl,-rpath="." -L. -lsecond_lib -ldl -o a.out
libfirst_lib.a:first_lib.c
	gcc -c first_lib.c
	ar rc libfirst_lib.a first_lib.o
libsecond_lib.so:second_lib.c
	gcc -fPIC -c second_lib.c
	gcc -shared -o libsecond_lib.so second_lib.o
libthird_lib.so:third_lib.c
	gcc -fPIC -c third_lib.c
	gcc -shared -o libthird_lib.so third_lib.o
clean:
	rm *.a *.o *.so a.out