# ESP32 LLM Chat

一个将ESP32-S3接入大语言模型的演示项目，通过串口与用户交互，支持流式响应和历史对话管理。

![ESP32 LLM Chat Demo](https://github.com/user-attachments/assets/1319030d-b285-4c09-b04d-aa86d9e221fe)

## 项目概述

这是一个基于ESP32-S3的智能聊天设备项目，通过Wi-Fi连接到互联网，使用HTTP客户端与大语言模型API进行通信。项目支持流式响应、对话历史管理，并通过串口与用户进行交互。

## 主要特性

- ✅ **ESP32-S3支持**：针对ESP32-S3优化，使用N16R8最大带宽SPI和240MHz最大速度
- ✅ **大语言模型集成**：支持OpenAI兼容的API接口
- ✅ **流式响应**：实时接收和显示模型响应
- ✅ **对话历史**：自动管理最多20轮对话历史
- ✅ **串口交互**：通过UART串口与设备进行对话
- ✅ **Wi-Fi连接**：自动连接配置的Wi-Fi网络
- ✅ **可配置API**：支持自定义API URL、密钥和模型参数

## 硬件要求

- ESP32-S3开发板
- USB数据线（用于供电和串口通信）
- 稳定的Wi-Fi网络连接

## 软件要求

- ESP-IDF v5.0或更高版本
- Python 3.8+（用于构建工具）
- 串口终端软件（如PuTTY、minicom或VS Code串口监视器）

## 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/Dreams-Possible/esp_llm_chat.git
cd esp_llm_chat/esp_llm_chat
```

### 2. 配置项目

#### 配置Wi-Fi
编辑 `main/wifi.c` 文件，设置您的Wi-Fi SSID和密码：

```c
#define WIFI_SSID "您的Wi-Fi名称"
#define WIFI_PASS "您的Wi-Fi密码"
```

#### 配置API参数
编辑 `main/llm_chat.c` 文件，配置大语言模型API：

```c
#define API_URL "https://api.openai.com/v1"  // 或您的API端点
#define API_KEY "sk-your-api-key-here"      // 您的API密钥
#define API_MOD "gpt-3.5-turbo"             // 模型名称
#define SYSTEM_CHAT "You are a helpful assistant."  // 系统提示词
```

> **注意**：智谱清言提供免费API，可以前往官网申请使用。

### 3. 构建和烧录

```bash
# 设置ESP-IDF环境
get_idf

# 配置项目
idf.py set-target esp32s3
idf.py menuconfig

# 构建项目
idf.py build

# 烧录到设备
idf.py -p PORT flash  # 将PORT替换为您的串口，如COM3或/dev/ttyUSB0
```

### 4. 监控输出

```bash
idf.py -p PORT monitor
```

### 5. 开始对话

通过串口发送消息，设备将自动连接到Wi-Fi并调用大语言模型API，实时显示响应。

## 项目结构

```
esp_llm_chat/
├── CMakeLists.txt          # 项目CMake配置
├── sdkconfig              # 项目配置
├── sdkconfig.ci           # CI配置
├── pytest_hello_world.py  # Python测试脚本
├── main/                  # 主程序目录
│   ├── CMakeLists.txt     # 主程序CMake配置
│   ├── main.c             # 应用程序入口
│   ├── main.h             # 主程序头文件
│   ├── llm_chat.c         # LLM聊天核心逻辑
│   ├── llm_chat.h         # LLM聊天头文件
│   ├── uart.c             # 串口通信实现
│   ├── uart.h             # 串口通信头文件
│   ├── wifi.c             # Wi-Fi连接管理
│   └── wifi.h             # Wi-Fi头文件
└── README.md              # 项目说明文档
```

## 配置说明

### Wi-Fi配置
在 `main/wifi.c` 中配置：
- `WIFI_SSID`: Wi-Fi网络名称
- `WIFI_PASS`: Wi-Fi密码

### LLM API配置
在 `main/llm_chat.c` 中配置：
- `API_URL`: API端点URL
- `API_KEY`: API认证密钥
- `API_MOD`: 使用的模型名称
- `SYSTEM_CHAT`: 系统角色提示词
- `TEMPERATURE`: 模型温度参数（默认1.0）
- `HISTORY_MAX`: 最大历史对话轮数（默认20）
- `CHAT_MAX`: 聊天缓冲区大小（默认8KB）

### 串口配置
默认串口配置：
- 波特率：115200
- 数据位：8
- 停止位：1
- 无校验位

## 故障排除

### 1. 证书问题
如果遇到SSL证书验证失败，在menuconfig中启用：
```
Component config → ESP-TLS → Enable ESP-TLS security level configuration
```

然后启用：
```
Enable insecure options (DANGEROUS) → Allow potentially insecure options
```

### 2. Wi-Fi连接失败
- 检查Wi-Fi SSID和密码是否正确
- 确保设备在Wi-Fi信号范围内
- 查看串口输出中的错误信息

### 3. API调用失败
- 检查API密钥是否正确
- 验证API端点URL是否可访问
- 确认网络连接正常
- 查看HTTP响应状态码

### 4. 内存不足
- 确保使用ESP32-S3 N16R8配置
- 减少`HISTORY_MAX`值
- 减小`CHAT_MAX`缓冲区大小

### 5. 构建错误
```bash
# 清理构建缓存
idf.py fullclean

# 重新构建
idf.py build
```

## 性能优化

- **SPI配置**: 使用N16R8最大带宽SPI配置
- **CPU频率**: 配置为240MHz最大速度
- **内存管理**: 优化缓冲区大小，避免内存碎片
- **网络优化**: 使用持久连接，减少连接建立开销

## 扩展功能

### 支持其他大模型
项目使用OpenAI兼容的API接口，可以轻松扩展到支持：
- 智谱清言（GLM）
- 百度文心一言
- 阿里通义千问
- 其他兼容OpenAI API的模型

### 添加新功能
1. **语音输入/输出**: 集成麦克风和扬声器
2. **显示屏支持**: 添加OLED或LCD显示
3. **按钮控制**: 添加物理按钮进行控制
4. **OTA更新**: 支持无线固件更新
5. **多语言支持**: 添加多语言提示词

## 许可证

本项目基于MIT许可证开源 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 致谢

- [ESP-IDF](https://github.com/espressif/esp-idf) - Espressif IoT Development Framework
- [cJSON](https://github.com/DaveGamble/cJSON) - 轻量级JSON解析器
- 所有贡献者和用户

---

**注意**: 本项目为演示用途，请确保遵守相关API的使用条款和条件。对于生产环境使用，请进行充分的测试和安全评估。