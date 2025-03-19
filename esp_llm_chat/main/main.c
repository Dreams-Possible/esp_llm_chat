#include"main.h"

//输入缓冲区大小
#define INPUT 1*1024

//模型回调函数
void llm_cb(uint8_t state,char*data)
{
    switch(state)
    {
        case LLM_TYPE_START:
            printf("[ASSISTANT]: \n");
        break;
        case LLM_TYPE_STREAM:
            printf("%s",data);
        break;
        case LLM_TYPE_FINISH:
            printf("\n[OVER]\n");
        break;
        default:
            printf("[FAIL]\n");
        break;
    }
    return;
}

void app_main()
{
    //WiFi初始化
    wifi_init();

    //初始化串口
    uart_init();

    //模型初始化
    llm_chat_t llm_config={0};
    llm_config.callback=llm_cb;
    llm_init(llm_config);

    //等待WiFi连接
    while(wifi_state())
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf("main: no wifi\n");
    }

    //建立输入缓冲区
    char*user=malloc(INPUT);
    memset(user,0,INPUT);

    while(1)
    {
        //从串口获取消息
        snprintf(user,INPUT,uart_read());
        printf("[USER]: \n%s\n",user);
        //调用模型
        llm_call(user);
    }
}
