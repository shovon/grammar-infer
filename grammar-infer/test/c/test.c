#include <stdio.h>

int foo(int i) {
	return i + 1;
}

int main()
{

char c;
FILE* file;
file = fopen("gooby.txt", "r");

int i = 0;
c = fgetc(file); 
c = fgetc(file); 
c = fgetc(file); 
ungetc(c, file);
c = fgetc(file); 
foo(c);

return 0;
}

