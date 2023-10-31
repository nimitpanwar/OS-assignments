all:
	gcc  -m32 -no-pie -nostdlib -o fib fib.c
	gcc -m32 -o try1 try1.c
	./try1 fib

clean:
	-@rm -f fib try1