#include"wifi.h"

//WiFi配置
#define SSID "北斗移动"
#define PASS "88888888"

//保存静态数据
typedef struct static_t
{
    uint8_t state;
    char ip[17];
}static_t;
static static_t*static_data=NULL;

//WiFi事件处理
static void event_hd(void*arg,esp_event_base_t event_base,int32_t event_id,void*event_data);
//WiFi初始化
uint8_t wifi_init();
//WiFi状态
uint8_t wifi_state();
//WiFiIP地址
char*wifi_ip();

//WiFi事件处理
static void event_hd(void*arg,esp_event_base_t event_base,int32_t event_id,void*event_data)
{
    //事件是WiFi事件，且是Station模式启动
    if(event_base==WIFI_EVENT&&event_id==WIFI_EVENT_STA_START)
    {
        //开始连接WiFi
        esp_wifi_connect();
        printf("wifi: event_hd: start connect wifi\n");
    }else
    //事件是WiFi事件，且事件是断开连接
    if(event_base==WIFI_EVENT&&event_id==WIFI_EVENT_STA_DISCONNECTED)
    {
        //更新WiFi状态为未连接
        static_data->state=0;
        //重新尝试连接WiFi
        esp_wifi_connect();
        printf("wifi: event_hd: retry connect wifi\n");
    }else
    //事件是IP事件，并且事件是获取到IP地址
    if(event_base==IP_EVENT&&event_id==IP_EVENT_STA_GOT_IP)
    {
        //更新WiFi状态为已连接
        static_data->state=1;
        //获取IP事件数据
        ip_event_got_ip_t*event=(ip_event_got_ip_t*)event_data;
        //更新WiFiIP地址
        memset(static_data->ip,0,17);
        snprintf(static_data->ip, 16, IPSTR, IP2STR(&event->ip_info.ip));
        printf("wifi: event_hd: get ip="IPSTR"\n",IP2STR(&event->ip_info.ip));
    }
}

//WiFi初始化
uint8_t wifi_init()
{
    //初始化静态缓冲区
    static_data=malloc(sizeof(static_t));
    if(!static_data)
    {
        //初始化静态缓冲区失败
        printf("wifi: wifi_init: memory out\n");
        return 1;
    }
    memset(static_data,0,17);
    esp_err_t ret=ESP_OK;
    //初始化NVS文件系统
    ret=nvs_flash_init();
    if(ret!=ESP_OK)
    {
        //格式化NVS文件系统
        ret=nvs_flash_erase();
        //重新初始化NVS文件系统
        ret=nvs_flash_init();
        if(ret!=ESP_OK)
        {
            //NVS文件系统初始化失败
            printf("wifi: wifi_init: init nvs fail\n");
            return 1;
        }
    }
    //初始化网络
    ret=esp_netif_init();
    //创建事件循环
    ret=esp_event_loop_create_default();
    //创建WiFi站点
    esp_netif_create_default_wifi_sta();
    //配置WiFi
    wifi_init_config_t cfg=WIFI_INIT_CONFIG_DEFAULT();
    //初始化WiFi
    ret=esp_wifi_init(&cfg);
    //注册WiFi事件和IP事件的处理函数
    ret=esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&event_hd,NULL,NULL);
    ret=esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&event_hd,NULL,NULL);
    //配置WiFi名称和密码
    wifi_config_t wifi_config={0};
    memcpy(wifi_config.sta.ssid,SSID,sizeof(SSID));
    memcpy(wifi_config.sta.password,PASS,sizeof(PASS));
    //设置WiFi模式为Station模式
    ret=esp_wifi_set_mode(WIFI_MODE_STA);
    //配置WiFi
    ret=esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    //启动WiFi
    ret=esp_wifi_start();
    if(ret!=ESP_OK)
    {
        //WiFi初始化失败
        printf("wifi: wifi_init: init station fail\n");
        return 1;
    }
    //WiF初始化成功
    printf("wifi: wifi_init: init wifi ok\n");
    return 0;
}

//WiFi状态
uint8_t wifi_state()
{
    if(static_data->state)
    {
        printf("wifi: wifi_state: connect\n");
        return 0;
    }else
    {
        printf("wifi: wifi_state: not connect\n");
        return 1;
    }
}

//WiFiIP地址
char*wifi_ip()
{
    if(wifi_state())
    {
        printf("wifi: wifi_ip: ip=%s\n",static_data->ip);
        return static_data->ip;
    }else
    {
        return NULL;
    }
}