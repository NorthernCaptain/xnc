#include "globals.h"

char fname[L_MAXPATH];
int realsiz;

#define PROGRAM_VERSION  "1.0.5"

void do_convert(char *buf,int siz,char* realname)
{
    char *b;
    int i;
    time_t t;
    time(&t);
    FILE* fp=fopen(fname,"w");
    if(fp==NULL)
    {
	fprintf(stderr,"Error: can't create output file '%s'\n",fname);
	exit(255);
    }
    b=strchr(fname,'.');
    if(b) *b=0;
    fprintf(fp,"/*\n"
	    " * This file was generated automatically from data source\n"
	    " * Source: %s\n"
	    " * Size:   %d\n"
	    " * Date:   %s"
	    " * Convertor: data2c version "PROGRAM_VERSION"\n"
	    " */\n",realname,siz,ctime(&t));
    fprintf(fp,"int %s_chr_size=%d;\n",fname,realsiz);
    fprintf(fp,"char %s_chr[]={\n",fname);
    for(i=0;i<siz-1;i++)
    {
	fprintf(fp,"0x%02X,",(int)buf[i] & 0xff);
	if((i+1)%20==0)
	    fprintf(fp,"\n");
    }
    fprintf(fp,"0x%02X};\n",(int)buf[i] & 0xff);
    fclose(fp);
}


int main(int argc, char **argv)
{
    char *buf;
    int fp,siz;
    int startoff=0,endoff,bytes=0;
    char *b;

    if(argc<2)
    {
	printf("Data files to C converter. (c) Leo'98 v"
	       PROGRAM_VERSION
	       "\nUsage: %s data.file [startoff] [endoff] [+bytes]\n",argv[0]);
	return 0;
    }
    fp=open(argv[1], O_RDONLY);
    if(fp==-1)
    {
	fprintf(stderr,"Error: Can't open '%s' file for input\n",argv[1]);
	return 255;
    }
    siz=lseek(fp,0l,SEEK_END);
    realsiz=siz;
    endoff=siz-1;
    lseek(fp,0l,SEEK_SET);
    if(argc>2)
    {
	if(argv[2][0]=='+')
	    sscanf(argv[2]+1,"%d",&bytes);
	else
	    sscanf(argv[2],"%d",&startoff);
	if(startoff>=siz)
        {
	    fprintf(stderr,"Warning: Start offset > filesize -> startoff will be set to 0\n");
	    startoff=0;
        }
    }        
    if(argc>3)
    {
	if(argv[3][0]=='+')
	    sscanf(argv[3]+1,"%d",&bytes);
	else
	    sscanf(argv[3],"%d",&endoff);
	if(endoff>=siz+bytes)
        {
	    fprintf(stderr,"Warning: End offset > filesize -> endoff will be set to Max\n");
	    endoff=siz-1;
        }
    }        
    if(argc>4 && argv[4][0]=='+')
        sscanf(argv[4]+1,"%d",&bytes);
    buf=new char[endoff-startoff+1+bytes];
    memset(buf,0,endoff-startoff+1+bytes);
    lseek(fp,startoff,SEEK_SET);
    read(fp,buf,endoff-startoff+1);
    close(fp);
    endoff+=bytes;
    strcpy(fname,argv[1]);
    b=strrchr(fname,'.');
    if(b) *b=0;
    strcat(fname,".c");
    fprintf(stderr,"Converting '%s' -> '%s'\n",
	    argv[1],fname);
    do_convert(buf,endoff-startoff+1,argv[1]);
    delete buf;
    return 0;
}
