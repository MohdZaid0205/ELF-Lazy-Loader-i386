#Create 32-bit executable for fib.c by using the gcc flags as mentioned in the PDF
all:
	gcc -m32 -no-pie -nostdlib -o fib fib.c
	gcc -m32 -no-pie -nostdlib -o fac fac.c
	gcc -m32 -no-pie -nostdlib -o var var.c
#Provide the command for cleanup
clean:
	-@rm -f fib
	-@rm -f fac
	-@rm -f var
