#pragma once

//头文件
#include<stdio.h>
#include<string.h>
#include"esp_err.h"
#include"nvs_flash.h"
#include"esp_wifi.h"

//WiFi初始化
uint8_t wifi_init();
//WiFi状态
uint8_t wifi_state();
//WiFiIP地址
char*wifi_ip();