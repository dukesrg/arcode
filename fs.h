#ifndef _FS_H
#define _FS_H

typedef struct file_s {
    int s;
    int pos;
    int size;
} FILE;

#define FILE_R 0x1
#define FILE_W 0x6

int(*IFile_Open)(FILE *this, const short *path, int flags) = (void *)0x0022FE08;
int(*IFile_Read)(FILE *this, unsigned int *read, void *buffer, unsigned int size) = (void *)0x001686DC;
int(*IFile_Write)(FILE *this, unsigned int *written, void *src, unsigned int len) = (void *)0x00168764;

#endif /* _FS_H */
