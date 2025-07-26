#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <assert.h>
#include "../mytypes.h"
#include "../ioports.h"
#include "../queue/ollist_threads_rw.h"
#include "config_file.h"

static char open_br = '<';
static char close_br = '>';
static char open_br_slash[2] = "</";
static char nl = 0x0A;
static char tabx = 0x09;
static char first_line[] = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>";
static char table[6] = "Table\0";
static char tick = '\'';
static char space = 0x21;

/////////////////////////////////////////////////////////////////////////////
// CONFIG_FILE is the define used for compiling the list_db and init_db programs
// so (i/ol)(Load/Write)Config is used by sched/tasks etc
#ifndef CONFIG_FILE
/////////////////////////////////////////////////////////////////////////////
int olLoadConfig(char *filename, ollist_t *oll, size_t size,char *errmsg)
{
	char *fptr;
	int fp = -1;
	int i = 0;
	fptr = (char *)filename;
	UCHAR id;
	O_DATA o_data;

	fp = open((const char *)fptr, O_RDWR);
	if(fp < 0)
	{
		strcpy(errmsg,strerror(errno));
		close(fp);
		printf("%s  %s\n",errmsg,filename);
		return -2;
	}

	i = lseek(fp,0,SEEK_SET);
	i = 0;
	read(fp,&id,1);
	if(id != 0xAA)
	{
		strcpy(errmsg,"invalid file format - id is not 0x55\0");
		close(fp);
		return -1;
	}
	for(i = 0;i < NUM_PORT_BITS;i++)
	{
		read(fp,&o_data,sizeof(O_DATA));
		ollist_insert_data(i, oll, &o_data);
	}
//	printf("fp:%d  read: %d bytes in oLoadConfig\n",fp,i);
	close(fp);
	strcpy(errmsg,"Success\0");
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int olWriteConfig(char *filename,  ollist_t *oll, size_t size,char *errmsg)
{
	char *fptr;
	int fp = -1;
	int i,j,k;
	fptr = (char *)filename;
	O_DATA io;
	O_DATA *pio = &io;
	UCHAR id = 0xAA;

//#ifdef NOTARGET
	fp = open((const char *)fptr, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
//#else
//	fp = open((const char *)fptr, O_WRONLY | O_CREAT, 666);
//#endif
	if(fp < 0)
	{
		strcpy(errmsg,strerror(errno));
		close(fp);
		return -2;
	}

	j = 0;
//	printf("fp = %d\n",fp);
//	printf("seek=%lu\n",lseek(fp,0,SEEK_SET));
	i = lseek(fp,0,SEEK_SET);
	write(fp,&id,1);
	for(i = 0;i < size/sizeof(O_DATA);i++)
	{
		ollist_find_data(i,&pio,oll);
		j += write(fp,(const void*)pio,sizeof(O_DATA));

	}

	close(fp);
	strcpy(errmsg,"Success\0");
	return 0;
}
#endif
/////////////////////////////////////////////////////////////////////////////
int oLoadConfig(char *filename, O_DATA *curr_o_array,size_t size,char *errmsg)
{
	char *fptr;
	int fp = -1;
	int i = 0;
	fptr = (char *)filename;
	UCHAR id;

	fp = open((const char *)fptr, O_RDWR);
	if(fp < 0)
	{
		strcpy(errmsg,strerror(errno));
		close(fp);
		printf("%s  %s\n",errmsg,filename);
		return -2;
	}

	i = lseek(fp,0,SEEK_SET);
	i = 0;
	read(fp,&id,1);
	if(id != 0xAA)
	{
		strcpy(errmsg,"invalid file format - id is not 0x55\0");
		close(fp);
		return -1;
	}
	i = read(fp,(void*)curr_o_array,size);
//	printf("fp:%d  read: %d bytes in oLoadConfig\n",fp,i);
	close(fp);
	strcpy(errmsg,"Success\0");
	return 0;
}
///////////////////// Write/LoadConfig functions used by init/list_db start here (see make_db) ///////////////////////

/////////////////////////////////////////////////////////////////////////////
int oWriteConfig(char *filename, O_DATA *curr_o_array,size_t size,char *errmsg)
{
	char *fptr;
	int fp = -1;
	int i,j,k;
	fptr = (char *)filename;
	O_DATA io;
	O_DATA *pio = &io;
	O_DATA *curr_o_array2 = curr_o_array;
	UCHAR id = 0xAA;

//#ifdef NOTARGET
	fp = open((const char *)fptr, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
//#else
//	fp = open((const char *)fptr, O_WRONLY | O_CREAT, 666);
//#endif
	if(fp < 0)
	{
		strcpy(errmsg,strerror(errno));
		close(fp);
		printf("%s  %s\n",errmsg,filename);
		return -2;
	}

	j = 0;
//	printf("fp = %d\n",fp);
//	printf("seek=%lu\n",lseek(fp,0,SEEK_SET));
	i = lseek(fp,0,SEEK_SET);
	write(fp,&id,1);
	for(i = 0;i < size/sizeof(O_DATA);i++)
	{
//		memset(pio,0,sizeof(IO_DATA));
		pio = curr_o_array2;
		j += write(fp,(const void*)pio,sizeof(O_DATA));
		curr_o_array2++;
	}

	close(fp);
	strcpy(errmsg,"Success\0");
	return 0;
}
#ifndef CONFIG_FILE
/////////////////////////////////////////////////////////////////////////////
char *LoadParams(char *filename, PARAM_STRUCT *ps, char *password, char *errmsg)
{
	char *fptr;
	int fp = -1;
	int i = 0;
	fptr = (char *)filename;
	UCHAR id;
	char tpassword[PASSWORD_SIZE];
	memset(errmsg,0,sizeof(errmsg));

	fp = open((const char *)fptr, O_RDWR);
	if(fp < 0)
	{
		strcpy(errmsg,strerror(errno));
		close(fp);
		printf("%s  %s\n",errmsg,filename);
		memset(ps,0,sizeof(PARAM_STRUCT));
		i = WriteParams(filename, &ps, password, errmsg);
		//printf("created new config file\n");
		strcpy(errmsg,"created new config file\0");
		//printf("%s\n",errmsg);
		return (char *)0;
	}

	i = lseek(fp,0,SEEK_SET);
	i = 0;
	read(fp,&id,1);
	//printf("id: %02x\n",id);
	if(id != 0xAA)
	{
		close(fp);
		printf("bad file marker at begin\n");
		strcpy(errmsg,"bad file marker at begin\0");
		return (char *)0;
	}
	i = read(fp,(void*)ps,sizeof(PARAM_STRUCT));
	//printf("read param struct: %d\n",i);
	i = read(fp,(void*)&tpassword[0],PASSWORD_SIZE);
	//printf("read password: %d %s\n",i,tpassword);
//	printf("fp:%d  read: %d bytes in oLoadConfig\n",fp,i);
	read(fp,&id,1);
	if(id != 0x55)
	{
		close(fp);
		printf("bad file marker at end\n");
		strcpy(errmsg,"bad file marker at end\0");
		return (char *)0;
	}
	close(fp);
	strcpy(errmsg,"Success (loaded config file)\0");
	return tpassword;
}
///////////////////// Write/LoadConfig functions used by init/list_db start here (see make_db) ///////////////////////

/////////////////////////////////////////////////////////////////////////////
int WriteParams(char *filename, PARAM_STRUCT *ps, char *password, char *errmsg)
{
	char *fptr;
	int fp = -1;
	int i,j,k;
	fptr = (char *)filename;
	UCHAR id = 0xAA;

//#ifdef NOTARGET
	fp = open((const char *)fptr, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
//#else
//	fp = open((const char *)fptr, O_WRONLY | O_CREAT, 666);
//#endif
	if(fp < 0)
	{
		strcpy(errmsg,strerror(errno));
		close(fp);
		printf("%s  %s\n",errmsg,filename);
		return -2;
	}

	j = 0;
	write(fp,&id,1);
	write(fp,(const void*)ps,sizeof(PARAM_STRUCT));
	write(fp,(const void*)&password[0],PASSWORD_SIZE);
	id = 0x55;
	write(fp,&id,1);
	close(fp);
	strcpy(errmsg,"Success\0");
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int LoadSpecialInputFunctions(IP *ip, int no_current_ips)
{
	FILE *fp;
	int i = 0;
	int input_port, input_type;
	fp = fopen ("spec_cmds.txt", "r");

	while(fscanf(fp, "%d %d", &input_port, &input_type)!=EOF)
	{
		ip[no_current_ips + i].port = 0;
		ip[no_current_ips + i].input = input_port;
		ip[no_current_ips + i].function = input_type;
		i++;
	}
	fclose(fp);
	return i;
}
#endif
/////////////////////////////////////////////////////////////////////////////
int oWriteConfigXML(char *filename, O_DATA *curr_o_array,size_t size,char *errmsg)
{
	char *fptr;
	int fp = -1;
	int i,j,k;
	fptr = (char *)filename;
	O_DATA io;
	O_DATA *pio = &io;
	O_DATA *curr_o_array2 = curr_o_array;
	char labels[11][20] = {"O_DATA","label","port","onoff","input_port",
			"input_type","type","time_delay","time_left","pulse_time","reset"};
	char temp[5];
	char tempx[30];

//#ifdef NOTARGET
	fp = open((const char *)fptr, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
//#else
//	fp = open((const char *)fptr, O_WRONLY | O_CREAT, 666);
//#endif
	if(fp < 0)
	{
		strcpy(errmsg,strerror(errno));
		close(fp);
		printf("%s  %s\n",errmsg,filename);
		return -2;
	}

	j = 0;
//	printf("fp = %d\n",fp);
//	printf("seek=%lu\n",lseek(fp,0,SEEK_SET));
	i = lseek(fp,0,SEEK_SET);

	write(fp,(const void*)&first_line[0],strlen(first_line));
	write(fp,(const void*)&nl,1);
	write(fp,(const void*)&open_br,1);
	write(fp,(const void*)&table[0],strlen(table));
	write(fp,(const void*)&close_br,1);
	write(fp,(const void*)&nl,1);

	for(i = 0;i < size/sizeof(O_DATA);i++)
	{
//		memset(pio,0,sizeof(IO_DATA));
		pio = curr_o_array2;

		write(fp,(const void*)&tabx,1);
		write(fp,(const void*)&open_br,1);
		write(fp,(const void*)&labels[0],strlen(labels[0]));
		write(fp,(const void*)&close_br,1);
		write(fp,(const void*)&nl,1);

		for(j = 1;j < 11;j++)
		{
			write(fp,(const void*)&tabx,1);
			write(fp,(const void*)&tabx,1);
			write(fp,(const void*)&open_br,1);
			write(fp,(const void*)&labels[j],strlen(labels[j]));
			write(fp,(const void*)&close_br,1);

			switch(j)
			{
				case 1:
				sprintf(tempx,pio->label,strlen(pio->label));
				write(fp,(const void*)&tempx[0],strlen(tempx));
				break;
				case 2:
				sprintf(temp,"%d",pio->port);
				write(fp,(const void*)&temp[0],strlen(temp));
				break;
				case 3:
				sprintf(temp,"%d",pio->onoff);
				write(fp,(const void*)&temp[0],strlen(temp));
				break;
				case 4:
				sprintf(temp,"%d",pio->input_port);
				write(fp,(const void*)&temp[0],strlen(temp));
				break;
				case 5:
				sprintf(temp,"%d",pio->input_type);
				write(fp,(const void*)&temp[0],strlen(temp));
				break;
				case 6:
				sprintf(temp,"%d",pio->type);
				write(fp,(const void*)&temp[0],strlen(temp));
				break;
				case 7:
				sprintf(temp,"%d",pio->time_delay);
				write(fp,(const void*)&temp[0],strlen(temp));
				break;
				case 8:
				sprintf(temp,"%d",pio->time_left);
				write(fp,(const void*)&temp[0],strlen(temp));
				break;
				case 9:
				sprintf(temp,"%d",pio->pulse_time);
				write(fp,(const void*)&temp[0],strlen(temp));
				break;
				case 10:
				sprintf(temp,"%d",pio->reset);
				write(fp,(const void*)&temp[0],strlen(temp));
				break;
			}

			write(fp,(const void*)&open_br_slash,2);
			write(fp,(const void*)&labels[j],strlen(labels[j]));
			write(fp,(const void*)&close_br,1);
			write(fp,(const void*)&nl,1);
		}
		write(fp,(const void*)&tabx,1);
		write(fp,(const void*)&open_br_slash,2);
		write(fp,(const void*)&labels[0],strlen(labels[0]));
		write(fp,(const void*)&close_br,1);
		write(fp,(const void*)&nl,1);
		curr_o_array2++;
	}
//	write(fp,(const void*)&nl,1);
	write(fp,(const void*)&open_br_slash,2);
	write(fp,(const void*)&table[0],strlen(table));
	write(fp,(const void*)&close_br,1);
	write(fp,(const void*)&nl,1);

	close(fp);
	strcpy(errmsg,"Success\0");
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int GetFileFormat(char *filename)
{
	char *fptr;
	int fp = -1;
	int i = 0;
	fptr = (char *)filename;
	UCHAR id;
	char errmsg[40];
	fp = open((const char *)fptr, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
//	fp = open((const char *)fptr, O_RDONLY);
	if(fp < 0)
	{
		strcpy(errmsg,strerror(errno));
		close(fp);
		printf("%s  %s\n",errmsg,filename);
		return -2;
	}

	i = lseek(fp,0,SEEK_SET);
	i = 0;
	read(fp,&id,1);
//	printf("%x %s\n",id,filename);
	if(id == 0xAA)	// output file
		return 1;
	else return -1;
	close(fp);
	strcpy(errmsg,"Success\0");
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int getFileCreationTime(char *path,char *str)
{
// MM:DD-HH:MM:SS
    struct stat attr;
    stat(path, &attr);
    strncpy(str,ctime(&attr.st_mtime),24);
    *(str + 19) = 0;
    strcpy(str,str+4);
	return 0;
}
#if 0
/////////////////////////////////////////////////////////////////////////////
int LoadOdometer(char *filename, int *odo, char *errmsg)
{
	char *fptr;
	int fp = -1;
	int i = 0;
	fptr = (char *)filename;
	UCHAR id;

	fp = open((const char *)fptr, O_RDWR);
	if(fp < 0)
	{
		strcpy(errmsg,strerror(errno));
		close(fp);
#ifdef MAKE_TARGET
		printf("%s  %s\n",errmsg,filename);
#else
#ifndef MAKE_SIM
		mvprintw(LINES-2,20,"%s  %s   ",errmsg,filename);
		refresh();
#endif
#endif
		return -2;
	}

	i = lseek(fp,0,SEEK_SET);
	i = 0;
	i = read(fp,(void*)odo,sizeof(int));

	close(fp);
	strcpy(errmsg,"Success\0");
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
int WriteOdometer(char *filename, int *odo, char *errmsg)
{
	char *fptr;
	int fp = -1;
	int i,j,k;
	fptr = (char *)filename;

//#ifdef NOTARGET
	fp = open((const char *)fptr, O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
//	fp = open((const char *)fptr, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
//	fp = open((const char *)fptr, O_RDWR | O_TRUNC);
//#else
//	fp = open((const char *)fptr, O_WRONLY | O_CREAT, 666);
//#endif
	if(fp < 0)
	{
		strcpy(errmsg,strerror(errno));
		close(fp);

		printf("%s  %s\n",errmsg,filename);
		return -2;
	}

	j = 0;
//	printf("fp = %d\n",fp);
//	printf("seek=%lu\n",lseek(fp,0,SEEK_SET));
//	i = lseek(fp,0,SEEK_SET);
	write(fp,(const void*)odo,sizeof(int));

	close(fp);
	strcpy(errmsg,"Success\0");
	return 0;
}
#endif
