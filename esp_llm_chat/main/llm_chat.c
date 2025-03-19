#include"llm_chat.h"

//配置
#define API_URL "https://"
#define API_KEY "sk-"
#define API_MOD "gpt-"
#define SYSTEM_CHAT "You are a helpful assistant."
#define TEMPERATURE "1.0"
#define HISTORY_MAX 20
#define CHAT_MAX 8*1024
#define err_printf printf

//定义
#define USER 0
#define ASSISTANT 1

//保存历史记录
typedef struct info_t
{
    char*assistant;
    char*user;
    struct info_t*next;
}info_t;
static info_t*info_root=NULL;

//保存静态数据
typedef struct static_t
{
    char http_tem[CHAT_MAX];
    char*line_tem;
    char*content_tem;
    char chat_tem[CHAT_MAX];
    info_t*info_top;
    llm_chat_t config;
}static_t;
static static_t*static_data=NULL;

//模型初始化
uint8_t llm_init(llm_chat_t config);
//发送请求
static esp_err_t send_request();
//接收响应
static esp_err_t receive_response(esp_http_client_event_t*event);
//获取一行
static uint8_t get_line();
//解析内容
static uint8_t get_content();
//添加历史记录
static void add_chat(uint8_t role,char*message);
//构建历史记录
static char*build_chat();
//调用模型
void llm_call(char*user);

//模型初始化
uint8_t llm_init(llm_chat_t config)
{
    //历史记录
    info_root=malloc(sizeof(info_t));
    if(!info_root)
    {
        err_printf("llm_chat: llm_init: memory out\n");
        return 1;
    }
    info_root->assistant=NULL;
    info_root->user=NULL;
    info_root->next=NULL;
    //静态数据
    static_data=malloc(sizeof(static_t));
    if(!static_data)
    {
        err_printf("llm_chat: llm_init: memory out\n");
        return 1;
    }
    memset(static_data,0,sizeof(static_t));
    static_data->line_tem=NULL;
    static_data->content_tem=NULL;
    static_data->info_top=info_root;
    static_data->config.callback=config.callback;
    err_printf("llm_chat: llm_init: init ok\n");
    return 0;
}

//发送请求
static esp_err_t send_request()
{
    //配置HTTP客户端
    esp_http_client_config_t config={0};
    config.url=API_URL"/chat/completions";
    config.event_handler=receive_response;
    //初始化HTTP客户端
    esp_http_client_handle_t client=esp_http_client_init(&config);
    //配置请求方法
    esp_http_client_set_method(client,HTTP_METHOD_POST);
    //配置请求头
    esp_http_client_set_header(client,"Content-Type","application/json");
    esp_http_client_set_header(client,"Authorization","Bearer "API_KEY);
    //构建数据
    char*chat_data=build_chat();
    //配置请求体
    esp_http_client_set_post_field(client,chat_data,strlen(chat_data));
    //发起POST请求
    esp_err_t ret=esp_http_client_perform(client);
    //请求失败
    if(ret!=ESP_OK)
    {
        err_printf("llm_chat: send_request: request fail\n");
        if(static_data->config.callback)
        {
            static_data->config.callback(LLM_TYPE_FAIL,"llm_chat: send_request: request fail");
        }
    }
    //释放HTTP客户端数据
    esp_http_client_cleanup(client);
    //释放数据
    free(chat_data);
    //请求成功
    return ret;
}

//接收响应
static esp_err_t receive_response(esp_http_client_event_t*event)
{
    switch(event->event_id)
    {
        //数据事件
        case HTTP_EVENT_ON_DATA:
            {
                //检查是否超出缓冲区
                if(strlen(static_data->http_tem)+event->data_len<CHAT_MAX)
                {
                    //拷贝到静态数据
                    memcpy(&static_data->http_tem[strlen(static_data->http_tem)],event->data,event->data_len);
                    while(get_line())
                    {
                        if(get_content())
                        {
                            if(static_data->config.callback)
                            {
                                static_data->config.callback(LLM_TYPE_STREAM,static_data->content_tem);
                            }
                        }
                    }
                }else
                {
                    err_printf("llm_chat: receive_response: buffer is full\n");
                }
            }
        break;
        //结束事件
        case HTTP_EVENT_ON_FINISH:
            add_chat(ASSISTANT,static_data->chat_tem);
            if(static_data->config.callback)
            {
                static_data->config.callback(LLM_TYPE_FINISH,static_data->chat_tem);
            }
        break;
        default:
        break;
    }
    return ESP_OK;
}

//获取一行
static uint8_t get_line()
{
    //定位换行符
    uint16_t num=0;
    for(uint16_t f=0;f<=strlen(static_data->http_tem);++f)
    {
        if(static_data->http_tem[f]=='\n')
        {
            num=f;
            break;
        }
    }
    //没有一行
    if(num==0&&static_data->http_tem[0]!='\n')
    {
        return 0;
    }
    //取出一行到静态数据
    if(static_data->line_tem)
    {
        free(static_data->line_tem);
        static_data->line_tem=NULL;
    }
    static_data->line_tem=malloc(num+1);
    if(!static_data->line_tem)
    {
        err_printf("llm_chat: get_line: memory out\n");
        return 0;
    }
    memset(static_data->line_tem,0,num+1);
    memcpy(static_data->line_tem,static_data->http_tem,num);
    //在静态数据中删除这一行
    uint16_t len=strlen(static_data->http_tem);
    uint16_t len_=strlen(&static_data->http_tem[num+strlen("\n\n")]);
    for(uint16_t f=0;f<len;++f)
    {
        if(f<len_)
        {
            static_data->http_tem[f]=static_data->http_tem[f+num+strlen("\n\n")];
        }else
        {
            static_data->http_tem[f]=0;
        }
    }
    //如果格式错误
    if(static_data->line_tem[strlen("data: ")-1]!='{'&&static_data->line_tem[strlen(static_data->line_tem)-1]!='}')
    {
        return 0;
    }
    return 1;
}

//解析内容
static uint8_t get_content()
{
    //解析JSON数据
    cJSON*json=cJSON_Parse(&static_data->line_tem[strlen("data: ")]);
    //解析失败
    if(json==NULL)
    {
        err_printf("llm_chat: get_content: json parse fail\n");
        return 0;
    }
    //提取内容
    cJSON*choices=cJSON_GetObjectItem(json,"choices");
    if(choices&&cJSON_IsArray(choices))
    {
        cJSON*first_choice=cJSON_GetArrayItem(choices,0);
        if(first_choice)
        {
            cJSON*message=cJSON_GetObjectItem(first_choice,"delta");
            if(message)
            {
                cJSON*content=cJSON_GetObjectItem(message,"content");
                //解析JSON数据成功
                if(content)
                {
                    //更新提取的内容到静态数据
                    if(static_data->content_tem)
                    {
                        free(static_data->content_tem);
                        static_data->content_tem=NULL;
                    }
                    uint16_t num=strlen(content->valuestring);
                    static_data->content_tem=malloc(num+1);
                    if(!static_data->content_tem)
                    {
                        err_printf("llm_chat: get_content: memory out\n");
                        return 0;
                    }
                    memcpy(static_data->content_tem,content->valuestring,num);
                    static_data->content_tem[num]=0;
                    //拼接
                    memcpy(&static_data->chat_tem[strlen(static_data->chat_tem)],content->valuestring,num);
                    return 1;
                }
            }
        }
    }
    //没有内容
    // err_printf("llm_chat: get_content: json no content\n");
    return 0;
}

//添加历史记录
static void add_chat(uint8_t role,char*message)
{
    static uint16_t count=0;
    //移动到最新记录
    info_t*llm_info=static_data->info_top;
    //添加用户记录
    if(role==0)
    {
        //释放上次的数据
        if(llm_info->user!=NULL)
        {
            free(llm_info->user);
            llm_info->user=NULL;
        }
        //新建用户记录
        if(llm_info->user==NULL)
        {
            char*user=malloc(strlen(message)+1);
            memcpy(user,message,strlen(message)+1);
            llm_info->user=user;
        }        
    }else
    //添加模型记录
    if(role==1)
    {
        //释放上次的数据
        if(llm_info->assistant!=NULL)
        {
            free(llm_info->assistant);
            llm_info->assistant=NULL;
        }
        //新建模型记录
        if(llm_info->assistant==NULL)
        {
            char*assistant=malloc(strlen(message)+1);
            memcpy(assistant,message,strlen(message)+1);
            llm_info->assistant=assistant;
        }        
    }
    //当前对话已记录
    if(llm_info->user!=NULL&&llm_info->assistant!=NULL)
    {
        info_t*llm_info_new=malloc(sizeof(info_t));
        //添加失败
        if(!llm_info_new)
        {
            err_printf("llm_chat: add_chat: add history fail\n");
            return;
        }
        //添加新记录
        llm_info->next=llm_info_new;
        llm_info_new->assistant=NULL;
        llm_info_new->user=NULL;
        llm_info_new->next=NULL;
        //更新最新记录
        static_data->info_top=llm_info_new;
        //删除最老的记录
        if(count<HISTORY_MAX)
        {
            ++count;
        }else
        {
            //备份老记录
            info_t*llm_info_old=info_root;
            //移动到下一个记录
            info_root=info_root->next;
            //释放老记录
            free(llm_info_old);
        }
    }
    return;
}

//构建历史记录
static char*build_chat()
{
    //创建JSON数据
    cJSON*json=cJSON_CreateObject();
    //构建模型信息
    cJSON_AddStringToObject(json,"model",API_MOD);
    //创建消息数据组
    cJSON*messages=cJSON_CreateArray();
    //构建角色信息
    cJSON*system_message=cJSON_CreateObject();
    cJSON_AddStringToObject(system_message,"role","system");
    cJSON_AddStringToObject(system_message,"content",SYSTEM_CHAT);
    cJSON_AddItemToArray(messages,system_message);
    //遍历全部记录
    info_t*llm_info=info_root;
    while(llm_info!=NULL)
    {
        //提取用户记录
        if(llm_info->user!=NULL)
        {
            //构建用户消息
            cJSON*user_message=cJSON_CreateObject();
            cJSON_AddStringToObject(user_message,"role","user");
            cJSON_AddStringToObject(user_message,"content",llm_info->user);
            cJSON_AddItemToArray(messages,user_message);
        }
        //提取模型记录
        if(llm_info->assistant!=NULL)
        {
            //构建模型消息
            cJSON*assistant_message = cJSON_CreateObject();
            cJSON_AddStringToObject(assistant_message,"role","assistant");
            cJSON_AddStringToObject(assistant_message,"content",llm_info->assistant);
            cJSON_AddItemToArray(messages,assistant_message);
        }
        //移动到下一个记录
        llm_info=llm_info->next;
    }
    //历史消息全部添加到JSON数据
    cJSON_AddItemToObject(json,"messages",messages);
    cJSON_AddTrueToObject(json,"stream");
    cJSON_AddStringToObject(json,"temperature",TEMPERATURE);
    //将JSON数据转化为字符串
    char*json_string=cJSON_PrintUnformatted(json);
    //释放JSON数据
    cJSON_Delete(json);
    return json_string;
}

//调用模型
void llm_call(char*user)
{
    memset(static_data->http_tem,0,CHAT_MAX);
    memset(static_data->chat_tem,0,CHAT_MAX);
    if(static_data->config.callback)
    {
        static_data->config.callback(LLM_TYPE_START,"llm_chat: llm_call: start");
    }
    //添加用户记录
    add_chat(USER,user);
    //发送请求
    send_request();
}
