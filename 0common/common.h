#ifndef COMMON_H
#define COMMON_H

typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef unsigned char UCHAR;
typedef void (*GetSrcFileInfoFuncType)(char **fileName, int *rowNum, int *colNum);

#define SIZE_INT sizeof(int)
#define SIZE_CHAR sizeof(char)
#define SIZE_PTR sizeof(int *)
#define SIZE_DEFAULT 0

#define SCOPE_GLOBAL -1
#define SCOPE_MAIN_FUNC 0

#define SYM_TAB_INVALID_IND -1

#define REGISTER_ALLOC_IN_MEM -1

#define VAL_NAME_VOID "<void>"
#define VAL_NAME_INT "<int>"
#define VAL_NAME_CHAR "<char>"

#define ALIGNMENT_SIZE 4
#endif
