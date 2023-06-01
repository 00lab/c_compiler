# c_compiler
一个C语言的编译器练习项目

## 正常编译运行
- 编译运行

./build.sh

- 清空编译

./build.sh -c

- 运行
export LD_LIBRARY_PATH=~/c_compiler/output/lib:$LD_LIBRARY_PATH

./output/bin/lexer_bin

## 调试

./build.sh -d

gdb ./output/bin/lexer_bin

start test.c

n

## 环境
- 已测试环境：

gcc version 7.5.0 (Ubuntu 7.5.0-3ubuntu1~18.04)

Target: x86_64-linux-gnu

cmake version 3.22.0
