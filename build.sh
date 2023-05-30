#!/bin/bash

set -e
BASE_PATH=$(cd "$(dirname $0)"; pwd)

# 用于定义支持的构建选项
_COUNT=0
define_option() {
    local var=$1
    local default=$2
    local opt=$3
    local flag=$4
    local intro=$5

    # 记录共有几个选项，用于判断超过设置的max_num数量后，显示帮助信息时换行
    _COUNT=$((${_COUNT} + 1))

    local max_num=4
    local mod_max=$((${_COUNT} % ${max_num}))
    local intro_space="    " # 用于显示帮助的选项介绍信息时，换行能开头对齐
    local flag_space="              " # 用于显示帮助信息时，换行能开头对齐
    local flag2=`echo ${flag} | awk '{print $1}'`
    # 设置全局变量的值，下划线开始的变量将在全局使用
    if [[ "X${var}" != "X" ]] && [[ "X${default}" != "X" ]]; then
        eval "${var}=${default}"
    fi
    # 用于提取输入的构建选项
    _OPTS="${_OPTS}${opt}"
    # 用于显示选项的详细介绍
    _INTROS="${_INTROS}${intro_space}${flag2} ${intro}\n"
    # 用于显示帮助信息bash build.sh [-h] [-d] [-j[n]] [-c]，超过设定的max_num值可自动换行
    if [[ "X${mod_max}" = "X1" ]] && [[ "X${mod_max}" != "X${_COUNT}" ]]; then
        _FLAGS="${_FLAGS}\n${flag_space}[${flag}]"
    else
        _FLAGS="${_FLAGS} [${flag}]"
    fi
}
# define options
define_option ""            ""            "h"      "-h"            "Print help info"
define_option DEBUG_MODE    "off"         "d"      "-d"            "Enable debug mode, default off"
define_option THREAD_NUM    4             "j:"     "-j[n]"         "Set the threads when building (Default: -j4)"
define_option CLEAN         "off"         "c"      "-c"            "clean build result, rm build* and output*"

help_func() {
    printf "Help info:\nbash build.sh${_FLAGS}\n\n"
    printf "Build options:\n${_INTROS}"
}

check_func()
{
  if [[ "X$1" != "Xon" && "X$1" != "Xoff" ]]; then
    echo "Invalid value $1 for option -$2"
    help_func
    exit 1
  fi
}

parser_options()
{
    # Process the options
    while getopts "${_OPTS}" opt
    do
        OPTARGRAW=${OPTARG}
        OPTARG=$(echo ${OPTARG} | tr '[A-Z]' '[a-z]')
        case "${opt}" in
            h)
                check_func
                exit 0
                ;;
            d)
                DEBUG_MODE="on"
                ;;
            j)
                THREAD_NUM=${OPTARG}
                ;;
            c)
                CLEAN="on"
                ;;
            *)
                echo "Unknown option ${opt}!"
                check_func
                exit 1;
        esac
    done
}
parser_options "$@"

if [[ "X${CLEAN}" = "Xon" ]]; then
    rm -rf ${BASE_PATH}/build ${BASE_PATH}/output ${BASE_PATH}/build_debug ${BASE_PATH}/output_debug
    echo "build.sh clean successful"
    exit 0
fi

if [[ "X${DEBUG_MODE}" = "Xon" ]]; then
    BUILD_PATH="${BASE_PATH}/build_debug/"
else
    BUILD_PATH="${BASE_PATH}/build/"
fi

OUTPUT_PATH="${BASE_PATH}/output/"
if [[ "X${DEBUG_MODE}" = "Xon" ]]; then
    OUTPUT_PATH="${BASE_PATH}/output_debug/"
fi


echo "build start"

mkdir -pv ${BUILD_PATH}
mkdir -pv ${OUTPUT_PATH}

if [[ "X${DEBUG_MODE}" = "Xon" ]]; then
    CMAKE_ARGS="${CMAKE_ARGS} -DCMAKE_BUILD_TYPE=Debug"
fi

cd ${BUILD_PATH}
echo "run cmd: cmake ${BASE_PATH} ${CMAKE_ARGS}"
cmake ${BASE_PATH} ${CMAKE_ARGS}
echo "build: make -j${THREAD_NUM}"
make "-j${THREAD_NUM}"
make install
echo "build end"
