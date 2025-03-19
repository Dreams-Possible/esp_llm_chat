#pragma once

//头文件
#include<stdio.h>
#include<string.h>
#include"esp_err.h"
#include"esp_http_client.h"
#include"cJSON.h"

//响应类型
#define LLM_TYPE_START 0
#define LLM_TYPE_STREAM 1
#define LLM_TYPE_FINISH 2
#define LLM_TYPE_FAIL 3

//配置信息
typedef struct llm_chat_t
{
    void(*callback)(uint8_t,char*);
}llm_chat_t;

//模型初始化
uint8_t llm_init(llm_chat_t config);
//调用模型
void llm_call(char*user);
