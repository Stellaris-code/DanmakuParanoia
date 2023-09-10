#include "fs.h"

#include <stdlib.h>
#include <stdio.h>
#include "sys/alloc.h"

char *read_file(const char *path, unsigned long* len)
{
    FILE *f=fopen(path,"rb");
    if(f == NULL) { return NULL; };
    fseek(f,0,SEEK_END);
    *len=ftell(f);
    fseek(f,0,SEEK_SET);
    char *data=(char*)danpa_alloc(*len+1+4096); // extra space to be safe regarding SIMD operations
    fread(data,1,*len,f);
    data[*len]='\0';
    fclose(f);

    return data;
}
