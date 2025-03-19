#pragma once

//头文件
#include<stdio.h>
#include<string.h>
#include "driver/uart.h"

//初始化串口
uint8_t uart_init();
//读取串口数据
char*uart_read();