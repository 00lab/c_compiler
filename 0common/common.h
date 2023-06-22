#ifndef COMMON_H
#define COMMON_H

typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef unsigned char UCHAR;
typedef void (*GetSrcFileInfoFuncType)(char **fileName, int *rowNum, int *colNum);

#define SIZE_INT sizeof(int)
#define SIZE_CHAR sizeof(char)
#define SIZE_DEFAULT 0

#endif
