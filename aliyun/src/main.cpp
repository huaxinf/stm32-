#include <Arduino.h>
#include <WiFi.h>
#include "PubSubClient.h"   ////A client library for MQTT messaging.
#include "string.h"
/* 连接WIFI SSID和密码 */
#define WIFI_SSID         "huaxinfeng"
#define WIFI_PASSWD       "12345678"

/* 设备的三元组信息*/
#define PRODUCT_KEY       "k190aUU5sNa"
#define DEVICE_NAME       "esp32_01"
#define DEVICE_SECRET     "264f732a3638d28479569ca4e18c084b"
#define REGION_ID         "cn-shanghai"

/* 线上环境域名和端口号，不需要改 */
#define MQTT_SERVER       PRODUCT_KEY".iot-as-mqtt."REGION_ID ".aliyuncs.com"
#define MQTT_PORT         1883
#define MQTT_USRNAME      "esp32_01&k190aUU5sNa"

#define CLIENT_ID         "k190aUU5sNa.esp32_01|securemode=2,signmethod=hmacsha256,timestamp=1715605236524|"
#define MQTT_PASSWD       "dd9ec1c5f3c3f5c47d1406c34e302bfef0fe003bac39c3ac2de189fba193fcfc"

//宏定义订阅主题
//#define ALINK_BODY_FORMAT         "{\"id\":\"wifi_01\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"
//#define ALINK_TOPIC_PROP_POST     "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"

///a1Buwr73pUI/${deviceName}/user/update    //注意把deviceName换成测试的设备名
#define   AliyunPublishTopic_user_update    "/k190aUU5sNa/esp32_01/user/ESP32"
#define   AliyunSubscribeTopic_user_get     "/sys/k190aUU5sNa/esp32_01/thing/service/property/set"
unsigned long lastMs = 0;

WiFiClient espClient;
PubSubClient  client(espClient);
/*变量声明*/
String inChar; //串口接收字符串
char enviroment_Data[6];//环境数据数组,存储收到的环境数据,水0，温1，光2，湿3，土4，CO2 5
//连接wifi
void wifiInit()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("WiFi not Connect");
    }
}
//mqtt连接
void mqttCheckConnect()
{
    while (!client.connected())
    {
        Serial.println("Connecting to MQTT Server ...");
        if(client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD))
        {
          Serial.println("MQTT Connected!");
        }
        else{
           Serial.print("MQTT Connect err:");
            Serial.println(client.state());
            delay(5000);
          }
        
    }
}
//mqtt向云端发送数据
static void mqtt_test_task()
{
    //整合数据
    char Water_Data[32];char temperature_Data[32];char Light_Data[32];char humidity_Data[32];char soil_Data[32];char CO2_Data[32];
    sprintf(Water_Data, "{\"Water\":%d}", enviroment_Data[0]);sprintf(temperature_Data, "{\"temperature\":%d}", enviroment_Data[1]);
    sprintf(Light_Data, "{\"Light\":%d}", enviroment_Data[2]);sprintf(humidity_Data, "{\"humidity\":%d}", enviroment_Data[3]);
    sprintf(soil_Data, "{\"soil\":%d}", enviroment_Data[4]);sprintf(CO2_Data, "{\"CO2\":%d}", enviroment_Data[5]);
    // 发布数据
    if(enviroment_Data[0]<100)client.publish(AliyunPublishTopic_user_update,Water_Data);
    if(enviroment_Data[1]<100)client.publish(AliyunPublishTopic_user_update,Light_Data);
    if(enviroment_Data[2]<100)client.publish(AliyunPublishTopic_user_update,humidity_Data);
    if(enviroment_Data[3]<100)client.publish(AliyunPublishTopic_user_update,temperature_Data);
    if(enviroment_Data[4]<100)client.publish(AliyunPublishTopic_user_update,soil_Data);
    if(enviroment_Data[5]<100)client.publish(AliyunPublishTopic_user_update,CO2_Data);
    Serial.println("Send Ok"); 
}
//mqtt数据重新连接
void reconnect() {
  // 循环重连直到连接成功
  while (!client.connected()) {
    if (client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD)) {
       Serial.println("MQTT Connected!");
      
    } else {
      Serial.println("MQTT NOT Connected!");
      delay(5000);
    }
  }
}
//串口信息处理函数
void Uart_Data_Solve()
{
  if (inChar.length() < 4)  {
    // 数据不足，无法解析
    Serial.println("Data is too short to parse.");
    return;
  }
  const char* all_data = inChar.c_str();
  String Command_String = inChar.substring(0, 3); // 提取命令部分;
   Serial.print("Command: ");
  Serial.println(Command_String);
  if(Command_String == "Wat")//储水量
  {
    enviroment_Data[0] = (all_data[3]-0x30)*10+(all_data[4]-0x30);
    if(enviroment_Data[0]>100){enviroment_Data[0]=enviroment_Data[0]/10;}
    Serial.println(enviroment_Data[0]+0);
  }
  if(Command_String == "Tem")//温度
  {
    enviroment_Data[1] = (all_data[3]-0x30)*10+(all_data[4]-0x30);
    if(enviroment_Data[1]>100){enviroment_Data[1]=enviroment_Data[1]/10;}
    Serial.println(enviroment_Data[1]+0);
  }   
  if(Command_String == "Lig")//光照
  {
    enviroment_Data[2] = (all_data[3]-0x30)*10+(all_data[4]-0x30);
    if(enviroment_Data[2]>100){enviroment_Data[2]=enviroment_Data[2]/10;}
    Serial.println(enviroment_Data[2]+0);
  }  
  if(Command_String == "Hum")//湿度
  {
    enviroment_Data[3] = (all_data[3]-0x30)*10+(all_data[4]-0x30);
    if(enviroment_Data[3]>100){enviroment_Data[3]=enviroment_Data[3]/10;}
    Serial.println(enviroment_Data[3]+0);
  }  
  if(Command_String == "Soi")//土壤含水量
  {
    enviroment_Data[4] = (all_data[3]-0x30)*10+(all_data[4]-0x30);
    if(enviroment_Data[4]>100){enviroment_Data[4]=enviroment_Data[4]/10;}
    Serial.println(enviroment_Data[4]+0);
  }    
  if(Command_String == "YHT")//二氧化碳含量
  {
    enviroment_Data[5] = (all_data[4]-0x30)*10+(all_data[4]-0x30);
    if(enviroment_Data[5]>100){enviroment_Data[5]=enviroment_Data[5]/10;}
    Serial.println(enviroment_Data[5]+0);
  } 
  //Serial.println(all_data[3]);Serial.println(all_data[4]);
}
//云端接收回调函数
void callback(char* topic, byte* payload, unsigned int length)
{
   // 将payload转换为字符串
  String payloadStr;
  char Control_Data[1];//控制命令信号
  char Ring_Data[1];//修改警戒数据
  for (int i = 0; i < length-6; i++) {
    payloadStr += (char)payload[i+2];
  }
  if(payloadStr == "Control")//判断云端传输的数据为命令
  {
    Control_Data[0] = (char)payload[11];
    Serial.println("Control");
    Serial.println(Control_Data[0]);
  }
  if(payloadStr == "ConData")//判断云端传输的数据为修改数据
  {
    Ring_Data[0] = (char)payload[11]*10+(char)payload[12];
    Serial.println("Change Data");
    Serial.println(Ring_Data[0]);
  }
  // 打印主题和消息
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  Serial.println(payloadStr);
  Serial.println("Data get ok");
  switch (Control_Data[0])
  {
  case '1' :
    Serial2.write(1);
    break;
  case '2' :
    Serial2.write(2);
    break;
  case '3' :
    Serial2.write(3);
    break;
  case '4' :
    Serial2.write(4);
    digitalWrite(27, 1);
    break;
  case '5' :
    Serial2.write(5);
    digitalWrite(27, 0); 
    break;
  case '6' :
    Serial2.write(6);
    break;
  case '7' :
    Serial2.write(7);
    break;
  default:
    break;
  }
}
//串口回调函数
void serialEvent() {
  while (Serial2.available()) { 
  // 读取串口数据
  inChar = Serial2.readString();
  
    Serial.print("Received: ");
    Serial.println(inChar);
    Uart_Data_Solve();
  }
}
//初始化
void setup()
{
  pinMode(27, OUTPUT);
  Serial.begin(115200,SERIAL_8N1);           //设置初始化串口0
	Serial2.begin(115200,SERIAL_8N1,16,17);  //初始化串口2，初始化参数可以去HardwareSerial.h文件中查看
	Serial2.onReceive(serialEvent);    //定义串口中断函数
  wifiInit();
  //设置回调函数
  client.setCallback(callback);
  client.setServer(MQTT_SERVER, MQTT_PORT);   /* 连接MQTT服务器 */
}
//uint16_t data;
//主循环
void loop()
{
  
  //如果断连就重连
  if (!client.connected()) {
    reconnect();
  }                     
    if ((millis() - lastMs)%5000 == 0)
    {
        mqttCheckConnect(); 
        //订阅
        client.subscribe(AliyunSubscribeTopic_user_get);
    }
     if (millis() - lastMs >= 5000)
    {
        lastMs = millis();
        mqttCheckConnect(); 
        /* 上报 */
        mqtt_test_task();
        
    }
    client.loop();
    //delay(2000);
} 