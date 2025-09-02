#Create 32-bit executable for fib.c by using the gcc flags as mentioned in the PDF
all:
	gcc -m32 -no-pie -fno-pic -nostdlib -o fib fib.c
	gcc -m32 -no-pie -fno-pic -nostdlib -o fac fac.c
	gcc -m32 -no-pie -fno-pic -nostdlib -o var var.c
	gcc -m32 -no-pie -fno-pic -nostdlib -o str str.c
#Provide the command for cleanup
clean:
	-@rm -f fib
	-@rm -f fac
	-@rm -f var
	-@rm -f str
