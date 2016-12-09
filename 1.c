#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#define tm_year tm_year+1900
#define tm_mon tm_mon+1
typedef struct{
	short boot;
}boot_block;

typedef struct{
	unsigned long inode_check[8];
	unsigned long data_check[16];
}super_block;

typedef struct inode_block{
	_Bool file_type; // 0:file , 1:directory
	long file_size;
	struct tm time;
	unsigned short db_n[3]; // 0:direct 1:single 2:double
}inode_block;

typedef struct data_block{
	char data[128];
	struct data_block *next;
}data_block;

typedef struct tree{
	char name[5];
	unsigned short inode;
	struct tree* child;
	struct tree* bro;
}tree;


boot_block bb; 

super_block sb;
inode_block *ib;
data_block *db;
data_block *idb;
data_block *head;
data_block *tail;

tree root;
tree *current=&root;

char location[5];
FILE *fs;

void mymkfs();
void mystate();
void mytree();
void myshowinode(char tmp[]);
void myshowblock(char tmp[]);
void mycpfrom(char tmp[]);
int input_prompt();

int new_tree(char name[],unsigned short inode); ///이미 있는 이름이면 0, 잘 만들었으면 1 리턴
tree* search_tree(char name[]);

unsigned short mk_inode(); ///사용가능하면서 가장 빠른 inode_B 넘버 찾기, 해당 넘버의 슈퍼블록 비트 1로 교체
unsigned short mk_data();  ///사용가능하면서 가장 빠른 data_B 넘버 찾기, 해당 넘버의 슈퍼블록 비트 1로 교체
unsigned short mk_single(unsigned short n); //(몇개 만들것인가)
unsigned short mk_double(unsigned short n);	//(몇개 만들것인가)

void rd_inode(unsigned short n);
void rd_data(unsigned short n);
unsigned short *rd_single(unsigned short n);
unsigned short *rd_double(unsigned short n);

void write_super();                  //
void write_inode(unsigned short n);  ////
void write_data(unsigned short n);   //// myfs에 작성해주기. (data_B or inode_B 넘버)

tree* search_tree(char name[]){ // 있으면 그 곳의 주솟값 리턴, 없으면 NULL 리턴.
	tree* check;
	check=current->child;
	while(check->bro!=NULL){
		if(!strcmp(name,check->name))
			return check;
		check=check->bro;
	}
	if(!strcmp(name,check->name))
		return check;
	else
		return NULL;
}
	
void mk_list(unsigned short inode){
	unsigned short* db_ns;
	unsigned short i=0;
	ib=(inode_block *)calloc(1,sizeof(inode_block));
	db=(data_block *)calloc(1,sizeof(data_block));
	rd_data(ib->db_n[0]);
	head=db;
	tail=db;
	head->next=NULL; db=NULL;
	if(ib->db[1]){
		i=0;
		db_ns=rd_single(ib->db[1]);
		while(db_ns[i]!=0){
			db=(data_block*)calloc(1,sizeof(data_block));
			rd_data(db_ns[i]);
			db->next=NULL;
			tail->next=db;
			tail=tail->next;
			db=NULL;
			i++;
		}
		free(db_ns);
	}
	if(ib->db[2]){
		i=0;
		db_ns=rd_double(db_ns[i]);
		while(db_ns[i]!=0){
			db=(data_block*)calloc(1,sizeof(data_block));
			rd_data(db_ns[i]);
			db->next=NULL;
			tail->next=db;
			tail=tail->next;
			db=NULL;
			i++;
		}
		free(db_ns);
	}
}
void mycat(char name[]){
	tree* target;

}
void rm_list(){
	data_block *cur;
	while(head!=NULL){
		cur=head;
		if(cur->next==NULL){
			free(cur);
			head=NULL;
		}
		else{
			while(cur->next!=tail)
				cur=cur->next;
			tail=cur;
			free(cur->next);
		}
	}
}


struct tm now_time(){
	struct tm* now;
	time_t ltime;
	ltime=time(NULL);
	now=localtime(&ltime);
	return *now;
}
void mytouch(char tmp[]){
	char name[5];
	int i=0;
	for(i=0;i<4;i++)
		name[i]=tmp[i];
}

		
void mycpfrom(char tmp[]){
	int i,j=0;
	char source_file[10];
	char my_dest_file[5],temp;
	long file_start,file_end;
	for(i=0;i<40;i++,j++){
		if(tmp[i]==' ') break;
		source_file[j]=tmp[i];
	}
	source_file[j]='\0';
	i++;
	for(j=0;j<4;i++,j++)
		my_dest_file[j]=tmp[i];
	my_dest_file[j]='\0';

	FILE *ifp;
	unsigned short n,q,r,inode_n,db_n;
	unsigned short *db_ns;

	if((ifp=fopen(source_file,"r"))==NULL){
		printf("mycpfrom : can not stat '%s' : no such file\n",source_file);
		return;
	}
	inode_n=mk_inode();
	new_tree(my_dest_file,inode_n); //같은 이름일 때 어떻게 해야할까?
	ib=(inode_block*)calloc(1,sizeof(inode_block));
	ib->file_type=0;
	file_start=ftell(ifp);
	fseek(ifp,0,SEEK_END);
	file_end=ftell(ifp);
	ib->file_size=(file_end-file_start);
	ib->time=now_time();
	q=ib->file_size/128;
	r=ib->file_size%128;
	rewind(ifp);
	if(r) n=q+1;
	if(!n){
		//touch(my_dest_file);
		return;
	}
	else{
		if(n>103){
			ib->db_n[2]=mk_double(n-103);
			db_ns=rd_double(ib->db_n[2]);
			j=0;
			while(*(db_ns+j)!=0){
				i=0;
				db=(data_block*)calloc(1,sizeof(data_block));
				rd_data(*(db_ns+j));
				while((fscanf(ifp,"%c",&temp))!=EOF && i<128)
					db->data[i++]=temp;
				write_data(*(db_ns+j));
				free(db);
				n--;
				j++;
			}
			free(db_ns);	
		}
		if(n>1){
			ib->db_n[1]=mk_single(n-1);
			db_ns=rd_single(ib->db_n[1]);
			j=0;
			while(*(db_ns+j)!=0){
				i=0;
				db=(data_block*)calloc(1,sizeof(data_block));
				rd_data(*(db_ns+j));
				while((fscanf(ifp,"%c",&temp))!=EOF && i<128)
					db->data[i++]=temp;
				write_data(*(db_ns+j));
				free(db);
				n--;
				j++;
			}	
			free(db_ns);
		}
		i=0;
		ib->db_n[0]=mk_data();
		db=(data_block*)calloc(1,sizeof(data_block));
		while((fscanf(ifp,"%c",&temp))!=EOF && i<128)
			db->data[i++]=temp;
		write_data(ib->db_n[0]);
		free(db);
	}
	write_inode(inode_n);
	write_super();
}

int main(){
	char tmp[10];

	root.bro=NULL; root.child=NULL;

	while(1){
		if((fs=fopen("myfs","rb+"))==NULL)
			printf("error : can not find 'myfs' file.\n");
		else{
			fseek(fs,sizeof(boot_block),SEEK_SET);
			fread(&sb,1,sizeof(super_block),fs);
			break;
		}
		scanf("%s",tmp);
		if(0==strcmp("mymkfs",tmp)){ 
			mymkfs();
			getchar();
			break;
		}
	}

	input_prompt();

	return 0;
}

int input_prompt(void )
{
	int i,j;
	char tmp[40];
	char order[13];
	char prompt[40];
	while(1)
	{
		prompt[0]=0;
		printf("[/%s ]$ ",location);
		scanf("%[^\n]",prompt);

		for(i=0;i<40;i++)
		{  
			if(prompt[i]==' ') break;
			order[i]=prompt[i];
		}
		order[i]='\0';
		i++;
		for(j=0;i<40;j++,i++)
			tmp[j]=prompt[i];

		if(0==strcmp("byebye",order)) exit(1);

		/*		else if(0==strcmp("mycat",order)) mycat(tmp);

		else if(0==strcmp("myshowfile",order)) myshowfile(tmp);

		else if(0==strcmp("mypwd",order)) mypwd(tmp);

		else if(0==strcmp("mycd",order)) mycd(tmp);

		else if(0==strcmp("mycp",order)) mycp(tmp);

		else if(0==strcmp("mycpto",order)) mycpto(tmp);

		else if(0==strcmp("myrmdir",order)) myrmdir(tmp);

		else if(0==strcmp("myrm",order)) myrm(tmp);

		else if(0==strcmp("mytouch",order)) mytouch(tmp);

		else if(0==strcmp("mymkdir",order)) mymkdir(tmp);
		*/
		else if(0==strcmp("mytree",order)) mytree(tmp);

		else if(0==strcmp("myshowinode",order)) myshowinode(tmp);

		else if(0==strcmp("myshowblock",order)) myshowblock(tmp);

		else if(0==strcmp("mystate",order)) mystate(tmp);

		else if(0==strcmp("mycpfrom",order)) mycpfrom(tmp);

		else system(prompt);

		while(getchar()!='\n');
	}
}
unsigned short mk_inode(){
	int i,j;
	unsigned long bit;
	unsigned short count=0;
	for(i=0;i<8;i++){
		bit=0x1;
		bit<<=63;
		if(sb.inode_check[i]){
			for(j=0;j<64;j++){
				if(bit & sb.inode_check[i]){
					count++;
					bit>>=1;
				}
				else{
					sb.inode_check[count/64]|=bit;
					break;
				}
			}
		}
		else{
			sb.inode_check[count/64]|=bit;
			break;
		}
	}
	write_super();
	return count; //꽉 차면 512
}
unsigned short mk_data(){
	int i,j;
	unsigned long bit,count=0;
	for(i=0;i<16;i++){
		bit=0x1;
		bit<<=63;
		if(sb.data_check[i]){
			for(j=0;j<64;j++){
				if(bit & sb.data_check[i]){
					count++;
					bit>>=1;
				}
				else{
					sb.data_check[count/64]|=bit;
					break;
				}
			}
		}
		else{
			sb.data_check[count/64]|=bit;
			break;
		}
	}
	write_super();
	return count; // 꽉 차면 1024
}
void mystate(){
	int i,j;
	unsigned long bit;
	unsigned short data_count=0,inode_count=0;
	for(i=0;i<8;i++){
		bit=0x1;
		bit<<=63;
		for(j=0;j<64;j++){
			if(bit & sb.inode_check[i])
				inode_count++;
			bit>>=1;
		}

	}
	for(i=0;i<16;i++){
		bit=0x1;
		bit<<=63;
		for(j=0;j<64;j++){
			if(bit & sb.data_check[i])
				data_count++;
			bit>>=1;
		}
	}
	printf("free inode block : %hu\n",512-inode_count);
	printf("free data block : %hu\n",1024-data_count);
}
void write_super(){
	fseek(fs,sizeof(boot_block),SEEK_SET);
	fwrite(&sb,sizeof(super_block),1,fs);
}	

void write_data(unsigned short n){
	int i;
	fseek(fs,sizeof(boot_block),SEEK_SET);
	fseek(fs,sizeof(super_block),SEEK_CUR);
	fseek(fs,sizeof(inode_block)*512,SEEK_CUR);
	for(i=0;i<n;i++){
		fseek(fs,128,SEEK_CUR);
	}
	fwrite(db,128,1,fs); 
}	
void write_inode(unsigned short n){
	int i;
	fseek(fs,sizeof(boot_block),SEEK_SET);
	fseek(fs,sizeof(super_block),SEEK_CUR);
	for(i=0;i<n;i++){
		fseek(fs,sizeof(inode_block),SEEK_CUR);
	}
	fwrite(ib,sizeof(inode_block),1,fs); 
}	
void mymkfs(){
	bb.boot=0;
	short i;

	sb.inode_check[0]=1;
	sb.inode_check[0]<<=63;

	sb.data_check[0]=1;
	sb.data_check[0]<<=63;

	ib=(inode_block*)calloc(1,sizeof(inode_block));
	db=(data_block*)calloc(1,sizeof(data_block));

	fs=fopen("myfs","wb+");
	fwrite(&bb,sizeof(boot_block),1,fs);
	fwrite(&sb,sizeof(super_block),1,fs);
	for(i=0;i<512;i++){
		fwrite(ib,sizeof(inode_block),1,fs);
	}
	for(i=0;i<1024;i++){
		fwrite(db,128,1,fs);
	}
	rewind(fs);
	free(ib);
	free(db);
}
void mytree(){
	tree *a,*children;
	a=root.child;
	printf("/\n");
	while(1){
		if(a==NULL) break;
		children=a->child;
		printf("--* %s\n",a->name);
		while(1){
			if(children==NULL) break;
			printf("-----* %s\n",children->name);
			children=children->bro;
		}
		a=a->bro;
	}
}
int new_tree(char name[],unsigned short inode){ ///이미 있는 이름이면 0, 잘 만들었으면 1 리턴
	tree *a,*check,*tmp;
	a=(tree*)calloc(1,sizeof(tree));
	a->child=NULL;
	a->bro=NULL;
	int i;
	strcpy(a->name,name);
	a->inode=inode;
	if(current->child==NULL){
		current->child=a;
	}
	else{
		check=current->child;
		while(1){
			if(check->bro==NULL){
				if(strcmp(check->name,a->name)>0){
					tmp=current->child;
					current->child=a;
					a->bro=tmp;
					return 1;
				}
				else if(strcmp(check->name,a->name)==0)
					return 0;
				else{
					check->bro=a;
					return 1;
				}
			}
			else{
				if(strcmp(check->bro->name,a->name)>0 && strcmp(check->name,a->name)<0){
					tmp=check->bro;
					check->bro=a;
					a->bro=tmp;
					return 1;
				}
			}
		check=check->bro;
		}	
	}
}

unsigned short mk_single(unsigned short n){ //(몇개 만들것인가)
	unsigned short bit,db_n,i,j=0;
	db=(data_block*)calloc(1,sizeof(data_block));
	for(i=0;i<n/4;i++){
		db_n=mk_data();
		bit=db_n&0x3fc;
		bit>>=2;
		db->data[j]|=bit;
		bit=db_n&0x3;
		bit<<=6;
		db->data[j+1]|=bit;

		db_n=mk_data();
		bit=db_n&0x3f0;
		bit>>=4;
		db->data[j+1]|=bit;
		bit=db_n&0xf;
		bit<<=4;
		db->data[j+2]|=bit;

		db_n=mk_data();
		bit=db_n&0x3c0;
		bit>>=6;
		db->data[j+2]|=bit;
		bit=db_n&0x3f;
		bit<<=2;
		db->data[j+3]|=bit;

		db_n=mk_data();
		bit=db_n&0x300;
		bit>>=8;
		db->data[j+3]|=bit;
		bit=db_n&0xff;
		db->data[j+4]|=bit; // 숫자 4개씩 규칙을 가짐.
		j+=5;
	}
	if(n%4>0){
		db_n=mk_data();
		bit=db_n&0x3fc;
		bit>>=2;
		db->data[j]|=bit;
		bit=db_n&0x3;
		bit<<=6;
		db->data[j+1]|=bit;
	}
		if(n%4>1){
			db_n=mk_data();
			bit=db_n&0x3f0;
			bit>>=4;
			db->data[j+1]|=bit;
			bit=db_n&0xf;
			bit<<=4;
			db->data[j+2]|=bit;
		}
		if(n%4>2){
			db_n=mk_data();
			bit=db_n&0x3c0;	
			bit>>=6;
			db->data[j+2]|=bit;
			bit=db_n&0x3f;
			bit<<=2;
			db->data[j+3]|=bit;
		}
		if(n%4>3){
			db_n=mk_data();
			bit=db_n&0x300;
			bit>>=8;
			db->data[j+3]|=bit;
			bit=db_n&0xff;
			db->data[j+4]|=bit;
		}
	db_n=mk_data();
	write_super();
	write_data(db_n);
	free(db);
	return db_n;
}
void rd_data(unsigned short n){
	db=(data_block *)calloc(1,sizeof(data_block));
	int i;
	fseek(fs,sizeof(boot_block),SEEK_SET);
	fseek(fs,sizeof(super_block),SEEK_CUR);
	fseek(fs,sizeof(inode_block)*512,SEEK_CUR);
	for(i=0;i<n;i++){
		fseek(fs,128,SEEK_CUR);
	}
	fread(db,1,128,fs);
}
void rd_inode(unsigned short n){
	int i;
	ib=(inode_block *)calloc(1,sizeof(inode_block));
	fseek(fs,sizeof(boot_block),SEEK_SET);
	fseek(fs,sizeof(super_block),SEEK_CUR);
	for(i=0;i<n;i++){
		fseek(fs,sizeof(inode_block),SEEK_CUR);
	}
	fread(ib,1,sizeof(inode_block),fs);
}
unsigned short *rd_single(unsigned short n){
	unsigned short bit,i=0,k=0;
	unsigned short *db_ns;
	db=(data_block*)calloc(1,sizeof(data_block));
	rd_data(n);
	db_ns=(unsigned short*)calloc(102,sizeof(unsigned short));
	while(1){
		bit=db->data[i+0]&0xff;	
		bit<<=2;
		db_ns[k]|=bit;
		bit=db->data[i+1]&0xc0;
		bit>>=6;
		db_ns[k]|=bit;
		if(db_ns[k]==0) break;
		k++;

		bit=db->data[i+1]&0x3f;
		bit<<=4;
		db_ns[k]|=bit;
		bit=db->data[i+2]&0xf0;
		bit>>=4;
		db_ns[k]|=bit;
		if(db_ns[k]==0) break;
		k++;

		bit=db->data[i+2]&0xf;
		bit<<=6;
		db_ns[k]|=bit;
		bit=db->data[i+3]&0xfc;
		bit>>=2;
		db_ns[k]|=bit;
		if(db_ns[k]==0) break;
		k++;

		bit=db->data[i+3]&0x3;			
		bit<<=8;
		db_ns[k]|=bit;
		bit=db->data[i+4]&0xff;
		db_ns[k]|=bit;
		if(db_ns[k]==0) break;
		k++;
		i+=5;
	}
	free(db);
	return db_ns;
}
void myshowinode(char tmp[]){
	unsigned short inode;
	char type[2][15]={"regular file","directory file"};
	ib=(inode_block*)calloc(1,sizeof(inode_block));
	inode=atoi(tmp);
	rd_inode(inode);
	printf("file type : %s\n",type[ib->file_type]);
	printf("file size : %ld\n",ib->file_size);
	printf("modified time : %d/%d/%d %d:%d:%d\n",ib->time.tm_year,ib->time.tm_mon,ib->time.tm_mday,ib->time.tm_hour,ib->time.tm_min,ib->time.tm_sec);
	printf("data block list : %hu",ib->db_n[0]);
	if(ib->db_n[1]) printf(", %hu",ib->db_n[1]);
	if(ib->db_n[2]) printf(", %hu",ib->db_n[2]);
	printf("\n");
	free(ib);
}
void myshowblock(char tmp[]){
	unsigned short data;
	db=(data_block*)calloc(1,sizeof(inode_block));
	data=atoi(tmp);
	rd_data(data);
	printf("%s\n",db->data);
	free(db);
}
unsigned short *rd_double(unsigned short n){
	unsigned short *single_ns,*doub_ns,*db_ns;
	int i=0,j=0,k=0;
	db_ns=(unsigned short*)calloc(102*102,sizeof(unsigned short));
	single_ns=rd_single(n);
	for(i=0;i<102;i++){
		if(*(single_ns+i)==0 || i==101){
			free(single_ns);
		   	break;
		}
		doub_ns=rd_single(*(single_ns+i));
		for(k=0;k<102;k++){
			if(*(doub_ns+k)==0 || k==101){
				free(doub_ns);
				break;
			}
			*(db_ns+j)=*(doub_ns+k);
			j++;
		}
	}
	return db_ns;
}
unsigned short mk_double(unsigned short n){	//(몇개 만들것인가)
	int i=0,j=0;
	unsigned short bit,db_n,db_ns[102];
	for(i=0;i<n/102;i++)
		db_ns[i]=mk_single(102);
	if(n%102)
		db_ns[i]=mk_single(n%102);
	db=(data_block*)calloc(1,sizeof(data_block));
	db_n=mk_data();
	while(1){
		if(db_ns[i]==0) break;
		bit=db_ns[i]&0x3fc;
		bit>>=2;
		db->data[j]|=bit;
		bit=db_ns[i]&0x3;
		bit<<=6;
		db->data[j+1]|=bit;
		i++;

		if(db_ns[i]==0 || i==101) break;
		bit=db_ns[i]&0x3f0;
		bit>>=4;
		db->data[j+1]|=bit;
		bit=db_ns[i]&0xf;
		bit<<=4;
		db->data[j+2]|=bit;
		i++;

		if(db_ns[i]==0) break;
		bit=db_ns[i]&0x3c0;
		bit>>=6;
		db->data[j+2]|=bit;
		bit=db_ns[i]&0x3f;
		bit<<=2;
		db->data[j+3]|=bit;
		i++;

		if(db_ns[i]==0) break;
		bit=db_ns[i]&0x300;
		bit>>=8;
		db->data[j+3]|=bit;
		bit=db_ns[i]&0xff;
		db->data[j+4]|=bit; // 숫자 4개씩 규칙을 가짐.
		i++;

		j+=5;
	}
	write_super();
	write_data(db_n);
	free(db);
	return db_n;
}
