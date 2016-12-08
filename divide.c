#include <stdio.h>
#include <string.h>

int main(void)
{
	char a[20]="as/d/fa/sdf";
	char *b = strtok(a, "/");

	while( b != NULL)
	{
		printf("%s\n",b);
		b = strtok(NULL, "/");
	}

	return 0;
}
