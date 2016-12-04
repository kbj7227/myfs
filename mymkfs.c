#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define sbc 16
#define inoc 1552
#define inb 75 // 아이노드 한 블록의 비트수
#define datac 39952 //1552 + 75(아이노드한블록)*512
#define datab 

int main(void)
{
	FILE *fp;
	char fsn[10];
	int *a;
	int b;

	b=0;
	a = &b;
	
	printf("파일시스템을 만들어주세요:");
	scanf("%s", fsn);
	 if((fp = fopen(fsn,"rb"))!=NULL)
		{
			printf("error\n");
		}
	 else 
	    {
			fp = fopen(fsn,"wb");
		}
	

		


	
	
	return 0;
}



