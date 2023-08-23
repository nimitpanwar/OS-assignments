#Compile the launch.c by linking it with the lib_simpleloader.so
all: 
	gcc -m32 -o launch launch.c -L. -l_simpleloader -Wl,-rpath=. -L../loader 
	./launch test/fib
#Provide the command for cleanup
