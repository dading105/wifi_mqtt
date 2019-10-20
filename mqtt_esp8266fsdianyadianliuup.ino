//#include <kyzh_protocol.h>
#include"FS.h"
/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/
#define MAX_SRV_CLIENTS 3
#define TIMEOUT 300
uint8_t post;
uint8_t postok;
uint32_t hosttick;
uint8_t needApWifi=0;
uint8_t needUpWifi=0;
uint8_t lastDay=0;
uint16_t statusconfig=0;
uint8_t REGstep=0;
uint8_t REGsteppre=0;
uint8_t Relaystep=0;
uint8_t getstatusflag=0;
uint8_t getwifistr=0;
uint8_t getparaflag=0;
#define HEARTstatus     0x0001
#define HEARTTIME       (1000*60*10)//(1000*60*5)
int32_t hearttick=0;
int32_t httptick=0;
#define DLstatus        0x0002
#define DL1status       0x0004
#define DL2status       0x0008
#define DLTIME          (1000*60)//(1000*60*60*2)
int32_t dltick=0;
#define ALARMstatus     0x0010
#define ALARMTIME       (1000*60*1)//(1000*60*10)
#define RELAYstatus     0x0020
int32_t alarmtick=0;
#define WIFIstatus      0x0040
#define WIFITIME        (1000*60*60*12)//(1000*60*10)
#define PARAstatus      0x0080
#define PARATIME        (1000*5)
int32_t paratick=0;
int32_t wifitick=0;
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

uint32_t hotneed;
IPAddress local_IP(192, 168, 66, 4);  
IPAddress gateway(192, 168, 66, 4);  
IPAddress subnet(255, 255, 255, 0);  

WiFiServer server(8266);//你要的端口号，随意修改，范围0-65535
WiFiClient serverClients[MAX_SRV_CLIENTS];

WiFiServer serverhttp(85);
WiFiClient serverClientshttp[MAX_SRV_CLIENTS];

WiFiClient wificlient;

uint16_t timerclients[MAX_SRV_CLIENTS]={0,0,0};
uint32_t previoustimer[MAX_SRV_CLIENTS]={0,0,0};

char * str1;
char * str2;
char * str3;
char * str4;
char * str5;
char * str6;
char * str7;
// Update these with values suitable for your network.
char hot[32]="ky";
char ssid[128] = "JM2";//"TP-LINK_4DB6";//JM2";
char password[64] = "1122334455";
char mqtt_server[128] = "183.230.40.39";
char mqtt_port[32] = "6002";
char usermqtt[128]="123";
char productmqtt[128]="123";
char passwordmqtt[128]="123";

char weizhi[32]="1234";
char banben[16]="KY0402-0002";
char sendhttp[7][128];

const char command[][30]={{0xfe,0xfe,0x68,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0x68,0x13,0x00,0xDF,0x16},
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x11,0x04,0x33,0x33,0x33,0x33,0xad,0x16},
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x11,0x04,0x33,0x34,0x34,0x35,0xb1,0x16},
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x11,0x04,0x33,0x34,0x35,0x35,0xb2,0x16},
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x14,0x10,0x33,0x33,0x34,0x82,0x35,0x33,0x33,0x33,0x63,0x63,0x63,0x63,0x99,0xcc,0x33,0x32,0x30,0x16},//{0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x04,0x08,0x5b,0xf3,0x35,0x33,0x33,0x33,0x67,0x45,0xa0,0x16},//FE FE 68 AA AA AA AA AA AA 68 04 08 5B F3 35 33 33 33 67 45 A0 16 la
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x14,0x10,0x33,0x33,0x34,0x82,0x35,0x33,0x33,0x33,0x63,0x63,0x63,0x63,0xdd,0xee,0x33,0x32,0x96,0x16},//{0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x04,0x08,0x5b,0xf3,0x35,0x33,0x33,0x33,0xab,0x89,0x28,0x16},//FE FE 68 AA AA AA AA AA AA 68 04 08 5B F3 35 33 33 33 AB 89 28 16 he
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x14,0x10,0x33,0x33,0x34,0x82,0x35,0x33,0x33,0x33,0x63,0x63,0x63,0x63,0x88,0x66,0x33,0x32,0xb9,0x16},//{0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x04,0x08,0x5b,0xf3,0x35,0x33,0x33,0x33,0x88,0x66,0xe2,0x16},//FE FE 68 AA AA AA AA AA AA 68 04 08 5B F3 35 33 33 33 88 66 E2 16 1la
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x1c,0x10,0x35,0x33,0x33,0x33,0x63,0x63,0x63,0x63,0x4d,0x33,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0x9a,0x16},//{0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x04,0x08,0x5b,0xf3,0x35,0x33,0x33,0x33,0x99,0xcc,0x59,0x16},//FE FE 68 AA AA AA AA AA AA 68 04 08 5B F3 35 33 33 33 99 CC 59 16 1he
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x1c,0x10,0x35,0x33,0x33,0x33,0x63,0x63,0x63,0x63,0x4e,0x33,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0x9b,0x16},//{0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x04,0x08,0x5b,0xf3,0x35,0x33,0x33,0x33,0x55,0x44,0x8d,0x16},//FE FE 68 AA AA AA AA AA AA 68 04 08 5B F3 35 33 33 33 55 44 8D 16 2la
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x1c,0x10,0x35,0x33,0x33,0x33,0x63,0x63,0x63,0x63,0x4f,0x33,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0x9c,0x16},//{0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x04,0x08,0x5b,0xf3,0x35,0x33,0x33,0x33,0xbb,0xaa,0x59,0x16},//FE FE 68 AA AA AA AA AA AA 68 04 08 5B F3 35 33 33 33 BB AA 59 16 2he
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x01,0x02,0x53,0xf3,0x15,0x16},
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x11,0x04,0x32,0x38,0x33,0x37,0xb5,0x16},
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x01,0x02,0xaa,0xff,0x78,0x16},
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x01,0x02,0xab,0xff,0x79,0x16},
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x01,0x02,0xac,0xff,0x7a,0x16},
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x01,0x02,0xad,0xff,0x7b,0x16},
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x04,0x07,0xad,0xff,0x33,0x33,0x33,0x33,0x34,0x83,0x16},//FE FE 68 AA AA AA AA AA AA 68 04 07 AD FF 33 33 33 33 34 83 16 
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x01,0x02,0x52,0xf3,0x14,0x16},//FE FE 68 AA AA AA AA AA AA 68 01 02 52 F3 14 16 
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x04,0x09,0x83,0xff,0x33,0x33,0x33,0x33,0x33,0x58,0x33,0x00,0x16},//68 AA AA AA AA AA AA 68 04 09 83 FF 35 33 33 33 33 58 33 E7 16 
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x04,0x0c,0x94,0xff,0x33,0x33,0x33,0x33,0x33,0x58,0x33,0x33,0x83,0x33,0x00,0x16},//68 AA AA AA AA AA AA 68 04 0C 94 FF 33 33 33 33 33 58 33 33 83 33 E2 16 
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x01,0x02,0x72,0xe9,0x2a,0x16},//功率
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x01,0x02,0x92,0xe9,0x4a,0x16},//功率因素
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x01,0x02,0x52,0xe9,0x0a,0x16},//电压
                          {0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x01,0x02,0x62,0xe9,0x1a,0x16},//电流
                         };
const char commandsend[52]={0xFE,0xFE,0x68,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0x68,0x04,0x26,0xAA,0xFF,0x35,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0xCD,0x16};

char biaohao[14]="";
uint8_t temparray[256];
uint8_t touChuanarray[256];
uint16_t touChuanlen;
uint8_t getaddflag=0;
uint8_t getdlflag=0;
uint8_t encrypted_data_buf[64];
uint8_t parsed_data_buf[64];
uint32_t energyint[3];
float    energyfloat[3];
uint32_t paraint[4];
float    parafloat[4];
WiFiClient espClient;
PubSubClient client(espClient);

char msg[50];
char msgin[50];
//int value = 0;

String prepareHtmlPage()
{
  String htmlPage =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +  // the connection will be closed after completion of the response
            "Refresh: 120\r\n" +  // refresh the page automatically every 5 sec
            "\r\n" +
            "<!DOCTYPE HTML>" +
            "<html>" +
            "<form id=\"form1\" name=\"form1\" method=\"post\" action=\"/devices\">wifi:<input type=\"text\" name=\"wifi\" id=\"wifi\" value=\"\"/>password:<input type=\"text\" name=\"pass\" id=\"pass\" value=\"\"/>mqtt server:<input type=\"text\" name=\"mqtt\" id=\"mqtt\" value=\"\"/>port:<input type=\"text\" name=\"port\" id=\"port\" value=\"\"/>mqtt user:<input type=\"text\" name=\"user\" id=\"user\" value=\"\"/>product:<input type=\"text\" name=\"pd\" id=\"pd\" value=\"\"/>mqtt password:<input type=\"text\" name=\"address\" id=\"address\" value=\"\"/><input name=\"sub1\" type=\"submit\" value=\"ok\" /></form>" +
            "</html>" +
            "\r\n";
  return htmlPage;
}

String HtmlPageok()
{
  String htmlPage =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +  // the connection will be closed after completion of the response
            "Refresh: 120\r\n" +  // refresh the page automatically every 5 sec
            "\r\n" +
            "<!DOCTYPE HTML>" +
            "<html>" +
            "OK" +
            "</html>" +
            "\r\n";
  return htmlPage;
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  if (WiFi.status() != WL_CONNECTED) {
    delay(3000);
    Serial.print("wifi d.");
  }

  randomSeed(micros());

  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
uint16_t len;
  digitalWrite(BUILTIN_LED, LOW);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  //len = length-8;
  //kyzh_head_parse((uint8_s *)payload,(uint8_s *)parsed_data_buf,&len);
  //length = len;
  memcpy((uint8_t *)parsed_data_buf,(uint8_t *)payload,length);
  for (int i = 0; i < length; i++) {
    Serial.print((char)parsed_data_buf[i]);
    payload[i]=parsed_data_buf[i];
  }
  
  Serial.println();

  sprintf(msg,"%s%s","d/down/",biaohao);
  Serial.print("get mssage");
  Serial.print(msg);
  if(strcmp(topic,msg)==0) 
  {
    //Serial.print(msg);
    if((payload[0]==0x01)||(payload[0]==0x04))
    {
      if (payload[1]==0x01)
      {
      if((payload[2]==0x00)||(payload[2]==0x02))
      {
        //REGstep=0x01;
        REGsteppre=0x01;
      }
      }
      if(payload[1]==0x02)
      {
        needApWifi=1;
      }
      if(payload[1]==0x03)
      {
        needUpWifi=1;
      }
    }
    if(payload[0]==0x72)
    {
      if (payload[1]==0x6c)
      {
        Relaystep=payload[2]-0x30;
        statusconfig |=RELAYstatus;
      }
    }
    if((payload[0]==0x68)||(payload[0]==0xfe))
    {
      memcpy(touChuanarray,payload,length);
      touChuanlen=length;
      statusconfig |=PARAstatus;
    }
  }
  
/*
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
*/
  digitalWrite(BUILTIN_LED, HIGH);
}

void reconnect() {
  // Loop until we're reconnected
  if (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    //String clientId = "ESP8266Client-";
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    Serial.print(mqtt_server);
    if (client.connect(usermqtt, productmqtt, passwordmqtt)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(msgin);
      Serial.println(msgin);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
uint8_t k,n,j,i;
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(BUILTIN_LED, HIGH);
  delay(5000);
  Serial.begin(2400, SERIAL_8E1);//Serial.begin(115200);
  //sprintf (msg, "LED:%d", BUILTIN_LED);
  //Serial.print(msg);
  //setup_wifi();

  bool ok = SPIFFS.begin();
  if (ok) {
    Serial.println("ok");
    //检查文件是否存在
    bool exist = SPIFFS.exists("/config.bin");
    if (exist) {
      Serial.println("The file exists!"); 
      File f = SPIFFS.open("/config.bin", "r");
      if (!f) {
        // 在打开过程中出现问题f就会为空
        Serial.println("Some thing went wrong trying to open the file...");
      }
      else {
        int s = f.size();
        Serial.printf("Size=%d\r\n", s);
        //读取index.html的文本内容
        String data = f.readString();
        Serial.println(data);
        strcpy((char *)temparray,data.c_str());
        str1 = strtok((char *)temparray,",");
        str2 = strtok(NULL,",");
        str3 = strtok(NULL,",");
        str4 = strtok(NULL,",");
        str5 = strtok(NULL,",");
        str6 = strtok(NULL,",");
        str7 = strtok(NULL,",");
        strcpy(ssid,str1);
        strcpy(password,str2);
        strcpy(mqtt_server,str3);
        strcpy(mqtt_port,str4);
        strcpy(usermqtt,str5);
        strcpy(productmqtt,str6);
        strcpy(passwordmqtt,str7);
        Serial.println(ssid); 
        Serial.println(password); 
        Serial.println(mqtt_server); 
        Serial.println(mqtt_port); 
        Serial.println(usermqtt); 
        Serial.println(productmqtt); 
        Serial.println(passwordmqtt); 
        //关闭文件
        f.close();
      }
    }
    else {
      Serial.println("No such file found.");
    }
  }

  
hearttick=millis()-HEARTTIME;
REGstep = 0;
statusconfig=0;


  while(1)
    {
      getaddflag=0;
      for(k=0;k<14;k++)
      {
      Serial.write(command[0][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];
        Serial.readBytes(sbuf, counti);
        for(n=0;n<counti-7;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
          memcpy(temparray,&sbuf[n+1],6);
          getaddflag=1;
          break;
          }
      }
      
      if(getaddflag)
      break;
      /*
      rsttick++;
      if(rsttick>5)
      resetFunc();
      */
    }

    for(k=0;k<6;k++)
    {
      biaohao[k*2] = ((temparray[5-k]>>4)+0x30);
      biaohao[k*2+1] = ((temparray[5-k]&0x0f)+0x30);
    }
    biaohao[12]=0;
    biaohao[13]=0;
/*
    getwifistr=0;
    for(k=0;k<16;k++)
      {
      Serial.write(command[12][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              i=0;
              for(j=0;j<k-2;j++)
              {
              if((sbuf[n+12+(k-3-j)]-0x33)!=0)
              {
                temparray[i]=sbuf[n+12+(k-3-j)]-0x33;
                i++;
              }
              }
              temparray[i]=0;
              Serial.write((char *)temparray);
              str1 = strtok((char *)temparray,",");
              str2 = strtok(NULL,",");
              if((str1!=NULL)&&(str2!=NULL))
              {
                if(strcmp(ssid,str1)!=0)
                {
                strcpy(ssid,str1);
                getwifistr=1;
                }
                if(strcmp(password,str2)!=0)
                {
                strcpy(password,str2);
                getwifistr=1;
                }
                Serial.write(ssid);
                Serial.write(password);
                
                
              }
              break;
            }
          }
        }
      }

      for(k=0;k<16;k++)
      {
      Serial.write(command[13][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              i=0;
              for(j=0;j<k-2;j++)
              {
              if((sbuf[n+12+(k-3-j)]-0x33)!=0)
              {
                temparray[i]=sbuf[n+12+(k-3-j)]-0x33;
                i++;
              }
              }
              temparray[i]=0;
              str1 = strtok((char *)temparray,",");
              str2 = strtok(NULL,",");
              if((str1!=NULL)&&(str2!=NULL))
              {
                if(strcmp(mqtt_server,str1)!=0)
                {
                strcpy(mqtt_server,str1);
                getwifistr=2;
                }
                if(strcmp(mqtt_port,str2)!=0)
                {
                strcpy(mqtt_port,str2);
                getwifistr=2;
                }
                Serial.write(mqtt_server);
                Serial.write(mqtt_port);
                
              }
              break;
            }
          }
        }
      }
      
      for(k=0;k<16;k++)
      {
      Serial.write(command[14][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              i=0;
              for(j=0;j<k-2;j++)
              {
              if((sbuf[n+12+(k-3-j)]-0x33)!=0)
              {
                temparray[i]=sbuf[n+12+(k-3-j)]-0x33;
                i++;
              }
              }
              temparray[i]=0;
              str1 = strtok((char *)temparray,",");
              str2 = strtok(NULL,",");
              str3 = strtok(NULL,",");
              if((str1!=NULL)&&(str2!=NULL)&&(str3!=NULL))
              {
                if(strcmp(usermqtt,str1)!=0)
                {
                strcpy(usermqtt,str1);
                getwifistr=3;
                }
                if(strcmp(passwordmqtt,str2)!=0)
                {
                strcpy(passwordmqtt,str2);
                getwifistr=3;
                }
                if(strcmp(weizhi,str3)!=0)
                {
                strcpy(weizhi,str3);
                getwifistr=3;
                }
                Serial.write(usermqtt);
                Serial.write(passwordmqtt);
                Serial.write(weizhi);
                
              }
              break;
            }
          }
        }
      }

      for(k=0;k<16;k++)
      {
      Serial.write(command[15][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              REGstep=sbuf[n+12]-0x33;
              break;
            }
          }
        }
      }
*/
  uint16_t tport;
        j=strlen(mqtt_port);
        tport=0;
        for(i=0;i<j;i++)
        {
          tport=tport*10+(mqtt_port[i]-0x30);
        }
  //test
/*
  kyzh_enveloped_data((uint8_t *)biaohao,0x01,encrypted_data_buf,14);
  for(k=0;k<64;k++)
  {
    Serial.write(encrypted_data_buf[k]);
  }
*/

  setup_wifi();
  sprintf(msgin,"%s%s","d/down/",biaohao);

  client.setServer(mqtt_server, tport);
  client.setCallback(callback);

  

  hotneed=0;
}

void loop() {
uint8_t k,n,j,i,l,kk;
  if (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("wifi c.");
    hotneed++;
  }
  if (!client.connected()) {
    digitalWrite(BUILTIN_LED, LOW);
    reconnect();
    digitalWrite(BUILTIN_LED, HIGH);
    hotneed++;
  }
  else
  {
    hotneed=0;
  }

  if((hotneed>=10)||(needApWifi>0))
  {
    if(needApWifi>0)
    {
      needApWifi=0;
      temparray[0]=0x04;
      temparray[1]=0x02;
      temparray[2]=0x01;
      sprintf(msg,"%s%s","d/up/",biaohao);
      Serial.print((char *)temparray);
      clientpublish(msg, temparray, 3);
      needUpWifi=1;
    }
    hosttick = millis();
    while((millis()-hosttick)<1000*60*5)
    {
      WiFi.disconnect(true);

      delay(100);
      Serial.print("dis and host.");
      
    WiFi.softAPConfig(local_IP, gateway, subnet); 
    hot[0]='k';
    hot[1]='y';
    hot[2]=0; 
    sprintf(hot,"%s%s",hot,biaohao);
    WiFi.softAP(hot, "5544332211");
    
    server.begin();
    server.setNoDelay(true);  //加上后才正常些
    serverhttp.begin();
    serverhttp.setNoDelay(true);
    Serial.print("Soft-AP IP address = ");  
    Serial.println(WiFi.softAPIP());  
    Serial.print(hot);
    postok=0;

    while((millis()-hosttick)<1000*60*5)
    {
    for(i=0;i<MAX_SRV_CLIENTS;i++)
    {
      if (serverClients[i])
      {
        if (millis() - previoustimer[i] > 1000)  //1000ms
        {
        previoustimer[i] = millis();
        timerclients[i]++;
        if(timerclients[i]>=TIMEOUT)
        {
          timerclients[i]=0;
          serverClients[i].stop();
        }
        }
      }
    }
//http
    for(i=0;i<MAX_SRV_CLIENTS;i++)
    {
      if (serverClientshttp[i])
      {
        if (millis() - previoustimer[i] > 1000)  //1000ms
        {
        previoustimer[i] = millis();
        timerclients[i]++;
        if(timerclients[i]>=TIMEOUT)
        {
          timerclients[i]=0;
          serverClientshttp[i].stop();
        }
        }
      }
    }

    if (serverhttp.hasClient())
    {
        for (i = 0; i < MAX_SRV_CLIENTS; i++)
        {
            if (!serverClientshttp[i] || !serverClientshttp[i].connected())
            {
                if (serverClientshttp[i]) serverClientshttp[i].stop();//未联接,就释放
                serverClientshttp[i] = serverhttp.available();//分配新的
                continue;
            }
 
        }
        WiFiClient serverClienthttp = serverhttp.available();
        serverClienthttp.stop();
    }
    for (i = 0; i < MAX_SRV_CLIENTS; i++)
    {
        if (serverClientshttp[i])
        {
        Serial.println("\n[Client connected]");
        httptick = millis();
        post=0;
        while(serverClientshttp[i].connected())
        {
            //digitalWrite(LED, 0);//有链接存在,就一直长亮
            if((millis() - httptick)>60*1000)
            {
              break;
            }
            
            if (serverClientshttp[i].available())
            {
               timerclients[i]=0;
               String line = serverClientshttp[i].readStringUntil('\n');
               //Serial.print(line);
               // wait for end of client's request, that is marked with an empty line
               //ESP.wdtFeed();
               if (line[0]=='P' && line[1]=='O' && line[2]=='S' && line[3]=='T')
               {
                 post=1;
                 //Serial.println("[POST1]");
                 
               }
               if(post==0)
               {
               if (line.length() == 1 && line[0] == '\r')
               {

                 serverClientshttp[i].println(prepareHtmlPage());

                 //Serial.print(prepareHtmlPage());
                 break;
               }
               }
               else
               {
                 //Serial.println(line[0]);
                 if(line[0]=='w' && line[1]=='i'&& line[2]=='f' & line[3]=='i')
                 {
                   //Serial.println("P2");

                   for(j=0;j<250;j++)
                   {
                     //Serial.println(line[i]);
                     
                     if(line[j]=='o' && line[j+1]=='k')
                     {
                       n=0;k=0;l=0;postok=0;
                       for (k=0;k<j;k++)
                       {
                         if(line[k]=='=')
                         {
                           l=0;
                           while(line[k+l+1]!='&')
                           {
                             sendhttp[n][l]=line[k+l+1];
                             l++;
                           }
                           sendhttp[n][l]='\0';
                           n++;
                           if(n>=7)
                             break;
                         }
                       }
                       
                       Serial.println(sendhttp[0]);
                       Serial.println(sendhttp[1]);
                       Serial.println(sendhttp[2]);
                       Serial.println(sendhttp[3]);
                       Serial.println(sendhttp[4]);
                       Serial.println(sendhttp[5]);
                       Serial.println(sendhttp[6]);

    Serial.println("stwr");
    //检查文件是否存在
    bool exist = SPIFFS.exists("/config.bin");
    if (exist) {
      Serial.println("The file exists!"); 
      File f = SPIFFS.open("/config.bin", "w");
      if (!f) {
        // 在打开过程中出现问题f就会为空
        Serial.println("Some thing went wrong trying to open the file...");
      }
      else {

        strcpy(ssid,sendhttp[0]);
        strcpy(password,sendhttp[1]);
        strcpy(mqtt_server,sendhttp[2]);
        strcpy(mqtt_port,sendhttp[3]);
        strcpy(usermqtt,sendhttp[4]);
        strcpy(productmqtt,sendhttp[5]);
        strcpy(passwordmqtt,sendhttp[6]);
        Serial.println(ssid); 
        Serial.println(password); 
        Serial.println(mqtt_server); 
        Serial.println(mqtt_port); 
        Serial.println(usermqtt); 
        Serial.println(productmqtt); 
        Serial.println(passwordmqtt); 
        memset(temparray,0,127);
        strcat((char *)temparray,ssid);
        strcat((char *)temparray,",");
        strcat((char *)temparray,password);
        strcat((char *)temparray,",");
        strcat((char *)temparray,mqtt_server);
        strcat((char *)temparray,",");
        strcat((char *)temparray,mqtt_port);
        strcat((char *)temparray,",");
        strcat((char *)temparray,usermqtt);
        strcat((char *)temparray,",");
        strcat((char *)temparray,productmqtt);
        strcat((char *)temparray,",");
        strcat((char *)temparray,passwordmqtt);
        Serial.println((char *)temparray); 
        String data=(char *)temparray;
  if (f.print(data)) {
    Serial.println("F written");
  } else {
    Serial.println("F failed");
  }
 
        //关闭文件
        f.close();
        postok=4;
      }
    }
    else {
      Serial.println("No such file found.");
    }

                       /*
                       //sendhttp
                       memcpy(temparray,commandsend,52);
                       l=strlen(sendhttp[1]);
                       n=strlen(sendhttp[0]);
                       for(k=0;k<l;k++)
                       {
                         temparray[18+k]=temparray[18+k]+sendhttp[1][l-k-1];
                         
                       }
                       temparray[18+l] = temparray[18+l] + 0x2c;
                       for(k=0;k<n;k++)
                       {
                         temparray[18+l+k+1]=temparray[18+l+k+1]+sendhttp[0][n-k-1];
                       }
                       temparray[50]=0;
                       for(k=2;k<50;k++)
                       {
                         temparray[50]=temparray[50]+temparray[k];
                       }
                       for(k=0;k<52;k++)
                       {
                       Serial.write(temparray[k]);
                       }
                       delay(2000);

      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>11)
        {
        for(n=0;n<counti-11;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            if(sbuf[n+8]==0x84)
            {
            postok++;
            }
            else
            {

            }
          }
        }
      }
                       memcpy(temparray,commandsend,52);
                       temparray[12]=0xab;
                       l=strlen(sendhttp[3]);
                       n=strlen(sendhttp[2]);
                       for(k=0;k<l;k++)
                       {
                         temparray[18+k]=temparray[18+k]+sendhttp[3][l-k-1];
                         
                       }
                       temparray[18+l] = temparray [18+l] + 0x2c;
                       for(k=0;k<n;k++)
                       {
                         temparray[18+l+k+1]=temparray[18+l+k+1]+sendhttp[2][n-k-1];
                       }
                       temparray[50]=0;
                       for(k=2;k<50;k++)
                       {
                         temparray[50]=temparray[50]+temparray[k];
                       }
                       for(k=0;k<52;k++)
                       {
                       Serial.write(temparray[k]);
                       }
                       delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>11)
        {
        for(n=0;n<counti-11;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            if(sbuf[n+8]==0x84)
            {
            postok++;
            }
            else
            {

            }
          }
        }
      }

memcpy(temparray,commandsend,52);
                       temparray[12]=0xac;
                       temparray[11]=0x25;
                       kk=strlen(sendhttp[6]);
                       l=strlen(sendhttp[5]);
                       n=strlen(sendhttp[4]);
                       for(k=0;k<kk;k++)
                       {
                         temparray[18+k]=temparray[18+k]+sendhttp[6][kk-k-1];
                         
                       }
                       temparray[18+kk] = temparray[18+kk] + 0x2c;
                       for(k=0;k<l;k++)
                       {
                         temparray[18+kk+k+1]=temparray[18+kk+k+1]+sendhttp[5][l-k-1];
                       }
                       temparray[18+kk+l+1] = temparray[18+kk+l+1] + 0x2c;
                       for(k=0;k<n;k++)
                       {
                         temparray[18+kk+l+k+2]=temparray[18+kk+l+k+2]+sendhttp[4][n-k-1];
                       }
                       temparray[49]=0;
                       for(k=2;k<49;k++)
                       {
                         temparray[49]=temparray[49]+temparray[k];
                       }
                       temparray[50]=0x16;
                       for(k=0;k<51;k++)
                       {
                       Serial.write(temparray[k]);
                       }
                       delay(2000);

      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>11)
        {
        for(n=0;n<counti-11;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            if(sbuf[n+8]==0x84)
            {
            postok++;
            }
            else
            {

            }
          }
        }
      }
memcpy(temparray,command[16],21);//注册清零
temparray[18]=0x33;
temparray[19]=0x82;
for(k=0;k<21;k++)
      {
      Serial.write(temparray[k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>11)
        {
        for(n=0;n<counti-11;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            if(sbuf[n+8]==0x84)
            {
              postok++;
              break;
            }
          }
        }
      }
*/
                       //sendhttp end
                       Serial.print(line[j]);
                       Serial.print(line[j+1]);
                       if(postok==4)
                       serverClientshttp[i].println(HtmlPageok());
                       
                       break;
                     }
                   }
                   break;
                 }
               }
            }
        }
        delay(1);
        Serial.print("out");
        serverClientshttp[i].stop();
        Serial.println("[Client disonnected]");
        }
    }
//http
    if (server.hasClient())
    {
        for (i = 0; i < MAX_SRV_CLIENTS; i++)
        {
            if (!serverClients[i] || !serverClients[i].connected())
            {
                if (serverClients[i]) serverClients[i].stop();//未联接,就释放
                serverClients[i] = server.available();//分配新的
                continue;
            }
 
        }
        WiFiClient serverClient = server.available();
        serverClient.stop();
    }
    for (i = 0; i < MAX_SRV_CLIENTS; i++)
    {
        if (serverClients[i] && serverClients[i].connected())
        {
            //digitalWrite(LED, 0);//有链接存在,就一直长亮
 
            if (serverClients[i].available())
            {
                timerclients[i]=0;
                while (serverClients[i].available()) 
                    Serial.write(serverClients[i].read());
            }
        }
    }
    if (Serial.available())
    {
        size_t len = Serial.available();
        uint8_t sbuf[len];
        Serial.readBytes(sbuf, len);
        //push UART data to all connected telnet clients
        for (i = 0; i < MAX_SRV_CLIENTS; i++)
        {
            if (serverClients[i] && serverClients[i].connected())
            {
                serverClients[i].write(sbuf, len);  //向所有客户端发送数据
                
                delay(1);
            }
        }
    }

    }

    }
    hotneed=0;
    statusconfig |= WIFIstatus;
  }
  
  client.loop();
//check tick
  if(millis()-hearttick > HEARTTIME)
  {
    hearttick=millis();
    statusconfig |= HEARTstatus;
  }
  if(millis()-dltick > DLTIME)
  {
    dltick=millis();
    statusconfig |= DLstatus;
/*
    for(k=0;k<16;k++)
      {
      Serial.write(command[17][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              if((sbuf[n+13]!=lastDay)&&((sbuf[n+18]-0x33)==0x23))
              {
                lastDay = sbuf[n+13];
                statusconfig |= DLstatus;
              }
              break;
            }
          }
        }
      }
*/
  }
  if(millis()-alarmtick > ALARMTIME)
  {
    alarmtick=millis();
    statusconfig |= ALARMstatus;
  }
  if(millis()-wifitick > WIFITIME)
  {
    wifitick=millis();
    statusconfig |= WIFIstatus;
  }
  /*
  if(millis()-paratick > PARATIME)
  {
    paratick=millis();
    statusconfig |= PARAstatus;
  }
  */
//check status

  if ((statusconfig & WIFIstatus) || (needUpWifi>0)) {
    if(needUpWifi>0)
    {
      needUpWifi=0;
      temparray[0]=0x04;
      temparray[1]=0x03;
      temparray[2]=0x01;
      sprintf(msg,"%s%s","d/up/",biaohao);
      Serial.print((char *)temparray);
      clientpublish(msg, temparray, 3);
    }
    statusconfig &= ~WIFIstatus;
Serial.print(" wifiupdate. ");
    getwifistr=0;
/*
    for(k=0;k<16;k++)
      {
      Serial.write(command[12][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              i=0;
              for(j=0;j<k-2;j++)
              {
              if((sbuf[n+12+(k-3-j)]-0x33)!=0)
              {
                temparray[i]=sbuf[n+12+(k-3-j)]-0x33;
                i++;
              }
              }
              temparray[i]=0;
              str1 = strtok((char *)temparray,",");
              str2 = strtok(NULL,",");
              if((str1!=NULL)&&(str2!=NULL))
              {
                if(strcmp(ssid,str1)!=0)
                {
                strcpy(ssid,str1);
                getwifistr=1;
                }
                if(strcmp(password,str2)!=0)
                {
                strcpy(password,str2);
                getwifistr=1;
                }
                Serial.write(ssid);
                Serial.write(password);
                
                
              }
              break;
            }
          }
        }
      }

      for(k=0;k<16;k++)
      {
      Serial.write(command[13][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              i=0;
              for(j=0;j<k-2;j++)
              {
              if((sbuf[n+12+(k-3-j)]-0x33)!=0)
              {
                temparray[i]=sbuf[n+12+(k-3-j)]-0x33;
                i++;
              }
              }
              temparray[i]=0;
              str1 = strtok((char *)temparray,",");
              str2 = strtok(NULL,",");
              if((str1!=NULL)&&(str2!=NULL))
              {
                if(strcmp(mqtt_server,str1)!=0)
                {
                strcpy(mqtt_server,str1);
                getwifistr=2;
                }
                if(strcmp(mqtt_port,str2)!=0)
                {
                strcpy(mqtt_port,str2);
                getwifistr=2;
                }
                Serial.write(mqtt_server);
                Serial.write(mqtt_port);
                
              }
              break;
            }
          }
        }
      }
      
      for(k=0;k<16;k++)
      {
      Serial.write(command[14][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              i=0;
              for(j=0;j<k-2;j++)
              {
              if((sbuf[n+12+(k-3-j)]-0x33)!=0)
              {
                temparray[i]=sbuf[n+12+(k-3-j)]-0x33;
                i++;
              }
              }
              temparray[i]=0;
              str1 = strtok((char *)temparray,",");
              str2 = strtok(NULL,",");
              str3 = strtok(NULL,",");
              if((str1!=NULL)&&(str2!=NULL)&&(str3!=NULL))
              {
                if(strcmp(usermqtt,str1)!=0)
                {
                strcpy(usermqtt,str1);
                getwifistr=3;
                }
                if(strcmp(passwordmqtt,str2)!=0)
                {
                strcpy(passwordmqtt,str2);
                getwifistr=3;
                }
                if(strcmp(weizhi,str3)!=0)
                {
                strcpy(weizhi,str3);
                getwifistr=3;
                REGstep=0;
                REGsteppre=0;
                }
                Serial.write(usermqtt);
                Serial.write(passwordmqtt);
                Serial.write(weizhi);
              }
              break;
            }
          }
        }
      }
*/
    Serial.println("strdf");
    //检查文件是否存在
    bool exist = SPIFFS.exists("/config.bin");
    if (exist) {
      Serial.println("The file exists!"); 
      File f = SPIFFS.open("/config.bin", "r");
      if (!f) {
        // 在打开过程中出现问题f就会为空
        Serial.println("Some thing went wrong trying to open the file...");
      }
      else {
        int s = f.size();
        Serial.printf("Size=%d\r\n", s);
        //读取index.html的文本内容
        String data = f.readString();
        Serial.println(data);
        strcpy((char *)temparray,data.c_str());
        str1 = strtok((char *)temparray,",");
        str2 = strtok(NULL,",");
        str3 = strtok(NULL,",");
        str4 = strtok(NULL,",");
        str5 = strtok(NULL,",");
        str6 = strtok(NULL,",");
        str7 = strtok(NULL,",");
        strcpy(ssid,str1);
        strcpy(password,str2);
        strcpy(mqtt_server,str3);
        strcpy(mqtt_port,str4);
        strcpy(usermqtt,str5);
        strcpy(productmqtt,str6);
        strcpy(passwordmqtt,str7);
        Serial.println(ssid); 
        Serial.println(password); 
        Serial.println(mqtt_server); 
        Serial.println(mqtt_port); 
        Serial.println(usermqtt); 
        Serial.println(productmqtt); 
        Serial.println(passwordmqtt); 
        getwifistr=1;
        //关闭文件
        f.close();
      }
    }
    else {
      Serial.println("No such file found.");
    }
      
      if(getwifistr==1)
      {
        WiFi.disconnect(true);
        
        delay(100);
        Serial.print("c.");

        WiFi.begin(ssid, password);

        if (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }

        client.disconnect();
        uint16_t tport;
        j=strlen(mqtt_port);
        tport=0;
        for(i=0;i<j;i++)
        {
          tport=tport*10+(mqtt_port[i]-0x30);
        }
        Serial.print("mqtt.");
        client.setServer(mqtt_server, tport);
      }
  }

  if (statusconfig & HEARTstatus) {
    statusconfig &= ~HEARTstatus;
/*
    for(k=0;k<16;k++)
      {
      Serial.write(command[15][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              REGstep=sbuf[n+12]-0x33;
              break;
            }
          }
        }
      }
    
    if((REGstep!=0x01)&&(REGsteppre==1))
    {
    for(k=0;k<21;k++)
      {
      Serial.write(command[16][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>11)
        {
        for(n=0;n<counti-11;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            if(sbuf[n+8]==0x84)
            {
              REGstep=1;
              break;
            }
          }
        }
      }
    }
    
    if(REGstep!=0x01)
    {
    temparray[0]=0x04;
    temparray[1]=0x01;
    temparray[2]=0x00;
 //   temparray[3]=REGstep;
 //   temparray[4]=0x01;
    int tempk = atoi(weizhi);
    Serial.print(tempk);
    temparray[2]=tempk&0xff;
    tempk=tempk/256;
    temparray[3]=tempk&0xff;
    tempk=tempk/256;
    temparray[4]=tempk&0xff;
    tempk=tempk/256;
    temparray[5]=tempk&0xff;
    memcpy(&temparray[6],banben,11);
    //sprintf((char *)temparray,"%s%s%s",temparray,weizhi,"KY0402-0002");
    sprintf(msg,"%s%s","d/up/",biaohao);
    Serial.print((char *)temparray);
    clientpublish(msg, temparray, 17);

    Serial.print("Publish reg: ");
    Serial.print(REGstep+0x30);
    Serial.print(msg);
    }
*/
  }
  if (statusconfig & DLstatus)
  {
      statusconfig &= ~DLstatus;
      memset(temparray,0,17);

      getdlflag=0;
      for(k=0;k<18;k++)
      {
      Serial.write(command[1][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>4)
            {
              /*
              temparray[0]=0x04;
              temparray[1]=0x81;
              temparray[2]=0x0c;
              energyint[0]=0;
              for(j=0;j<k-2;j++)
              {
              temparray[3+j]=sbuf[n+12+(k-3-j)]-0x33;
              energyint[0]=energyint[0]*100+(temparray[3+j]-(temparray[3+j]>>4)*6);
              }
              energyfloat[0]=energyint[0]/100.0;
              //memcpy(&temparray[6],((unsigned char *)&energyfloat[0]),1);
              //memcpy(&temparray[5],((unsigned char *)&energyfloat[0])+1,1);
              //memcpy(&temparray[4],((unsigned char *)&energyfloat[0])+2,1);
              memcpy(&temparray[3],((unsigned char *)&energyfloat[0]),4);
              */
              temparray[0]=0x07;
              temparray[1]=0x00;
              temparray[2]=0x01;
              temparray[3]=0x00;
              temparray[4]=0x03;
              energyint[0]=0;
              for(j=0;j<k-4;j++)
              {
              temparray[5+j]=sbuf[n+14+(k-5-j)]-0x33;
              energyint[0]=energyint[0]*100+(temparray[5+j]-(temparray[5+j]>>4)*6);
              }
              energyfloat[0]=energyint[0]/100.0;
              //memcpy(&temparray[8],((unsigned char *)&energyfloat[0]),1);
              //memcpy(&temparray[7],((unsigned char *)&energyfloat[0])+1,1);
              //memcpy(&temparray[6],((unsigned char *)&energyfloat[0])+2,1);
              memcpy(&temparray[5],((unsigned char *)&energyfloat[0]),4);
              getdlflag=1;
              break;
            }
          }
        }
      }
      
      if(getdlflag==1)
      {
      for(k=0;k<18;k++)
      {
      Serial.write(command[2][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>4)
            {
              energyint[1]=0;
              for(j=0;j<k-4;j++)
              {
              temparray[9+j]=sbuf[n+14+(k-5-j)]-0x33;
              energyint[1]=energyint[1]*100+(temparray[9+j]-(temparray[9+j]>>4)*6);
              }
              energyfloat[1]=energyint[1]/10.0;
              memcpy(&temparray[9],((unsigned char *)&energyfloat[1]),4);
              getdlflag=2;
              break;
            }
          }
        }
      }
      /*
      for(k=0;k<16;k++)
      {
      Serial.write(command[2][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);

        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              temparray[0]=0x04;
              temparray[1]=0x81;
              temparray[2]=0x0c;
              energyint[1]=0;
              for(j=0;j<k-2;j++)
              {
              temparray[7+j]=sbuf[n+12+(k-3-j)]-0x33;
              energyint[1]=energyint[1]*100+(temparray[7+j]-(temparray[7+j]>>4)*6);
              }
              energyfloat[1]=energyint[1]/100.0;
              //memcpy(&temparray[10],((unsigned char *)&energyfloat[1]),1);
              //memcpy(&temparray[9],((unsigned char *)&energyfloat[1])+1,1);
              //memcpy(&temparray[8],((unsigned char *)&energyfloat[1])+2,1);
              memcpy(&temparray[7],((unsigned char *)&energyfloat[1]),4);
              getdlflag=2;
              break;
            }
          }
        }

      }
      */
      }
      if(getdlflag==2)
      {
      for(k=0;k<18;k++)
      {
      Serial.write(command[3][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>4)
            {
              energyint[2]=0;
              for(j=0;j<k-4;j++)
              {
              temparray[13+j]=sbuf[n+14+(k-5-j)]-0x33;
              energyint[2]=energyint[2]*100+(temparray[13+j]-(temparray[13+j]>>4)*6);
              }
              energyfloat[2]=energyint[2]/1000.0;
              memcpy(&temparray[13],((unsigned char *)&energyfloat[2]),4);
              getdlflag=3;
              break;
            }
          }
        }
      }
      /*
      for(k=0;k<16;k++)
      {
      Serial.write(command[3][k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);

        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              temparray[0]=0x04;
              temparray[1]=0x81;
              temparray[2]=0x0c;
              energyint[2]=0;
              for(j=0;j<k-2;j++)
              {
              temparray[11+j]=sbuf[n+12+(k-3-j)]-0x33;
              energyint[2]=energyint[2]*100+(temparray[11+j]-(temparray[11+j]>>4)*6);
              }
              energyfloat[2]=energyint[2]/100.0;
              //memcpy(&temparray[14],((unsigned char *)&energyfloat[2]),1);
              //memcpy(&temparray[13],((unsigned char *)&energyfloat[2])+1,1);
              //memcpy(&temparray[12],((unsigned char *)&energyfloat[2])+2,1);
              memcpy(&temparray[11],((unsigned char *)&energyfloat[2]),4);
              getdlflag=3;
              break;
            }
          }
        }
      }
      */
      }
      
      //if(getdlflag==3)
      if(getdlflag==3)
        {
          /*
          for(k=0;k<15;k++)
          {
          if(k>=3)
          temparray[k-1]=temparray[k];
          Serial.write(temparray[k]);
          }
          */
          //sprintf(msg,"%s%s","d/up/",biaohao);
          //clientpublish(msg, temparray, 14);
          sprintf(msg,"%s%s","d/dl/",biaohao);
          clientpublish(msg, temparray, 17);
          sprintf(msg,"$dp");
          //clientpublish(msg, temparray, 17);
          temparray[2]=1;
          temparray[4]=1;
          clientpublish(msg, temparray, 9);
          temparray[2]=2;
          memcpy(&temparray[5],&temparray[9],4);
          clientpublish(msg, temparray, 9);
          temparray[2]=3;
          memcpy(&temparray[5],&temparray[13],4);
          clientpublish(msg, temparray, 9);
          
          

          Serial.print("Publish energy: ");
          Serial.print(msg);
          
        }
  }

  if(statusconfig & RELAYstatus)
  {
    statusconfig &=~RELAYstatus;
    if((Relaystep>0)&&(Relaystep<=6))
    {
      memcpy(temparray,command[4+Relaystep-1],30);
      for(k=0;k<6;k++)
      {
        temparray[3+k]=(biaohao[11-k*2-1]-0x30)*16+(biaohao[11-k*2]-0x30);
      }
      temparray[28]=0;
      for(k=2;k<28;k++)
      {
        temparray[28]=temparray[28]+temparray[k];
      }
      for(k=0;k<30;k++)
      {
      Serial.write(temparray[k]);
      }
      delay(2000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>11)
        {
        for(n=0;n<counti-11;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            if((sbuf[n+8]==0x84)||(sbuf[n+8]==0x94)||(sbuf[n+8]==0x9c))
            {
              temparray[0]=0x72;
              temparray[1]=0x6c;
              temparray[2]=0x31;
              sprintf(msg,"%s%s","d/up/",biaohao);
              clientpublish(msg, temparray, 3);

              Serial.print("Publish relay: ");
              Serial.print(msg);
              for(k=0;k<3;k++)
              {
              Serial.write(temparray[k]);
              }
              break;
            }
            else
            {
              temparray[0]=0x72;
              temparray[1]=0x6c;
              temparray[2]=0x30;
              sprintf(msg,"%s%s","d/up/",biaohao);
              clientpublish(msg, temparray, 3);

              Serial.print("Publish relay: ");
              Serial.print(msg);
              for(k=0;k<3;k++)
              {
              Serial.write(temparray[k]);
              }
              break;
            }
          }
        }
      }
    Relaystep=0;
    }
    
  }

  if (statusconfig & PARAstatus)
  {
      statusconfig &= ~PARAstatus;
      getstatusflag=0;
    for(k=0;k<touChuanlen;k++)
      {
      Serial.write(touChuanarray[k]);
      }
    delay(2000);
    if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>11)
        {
        for(n=0;n<counti-11;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            getstatusflag=1;
            memcpy(temparray,sbuf,counti);
              sprintf(msg,"%s%s","d/up/",biaohao);
              clientpublish(msg, temparray, counti);
              Serial.print("Pubtou:");
              Serial.print(msg);
              break;
          }
        }
      }
/*
      memset(temparray,0,19);

      getparaflag=0;
      for(k=0;k<16;k++)
      {
      Serial.write(command[20][k]);
      }
      delay(1000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              temparray[0]=0x04;
              temparray[1]=0x83;
              temparray[2]=0x0c;
              paraint[0]=0;
              for(j=0;j<3;j++)
              {
              temparray[3+j]=sbuf[n+15+(2-j)]-0x33;
              paraint[0]=paraint[0]*100+(temparray[3+j]-(temparray[3+j]>>4)*6);
              }
              parafloat[0]=paraint[0]/10000.0;
              memcpy(&temparray[3],((unsigned char *)&parafloat[0]),4);
              paraint[1]=0;
              for(j=0;j<3;j++)
              {
              temparray[7+j]=sbuf[n+18+(2-j)]-0x33;
              paraint[1]=paraint[1]*100+(temparray[7+j]-(temparray[7+j]>>4)*6);
              }
              parafloat[1]=paraint[1]/10000.0;
              memcpy(&temparray[7],((unsigned char *)&parafloat[1]),4);
              getparaflag=1;
              break;
            }
          }
        }
      }
      if(getparaflag==1)
      {
      for(k=0;k<16;k++)
      {
      Serial.write(command[21][k]);
      }
      delay(1000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);

        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              temparray[0]=0x04;
              temparray[1]=0x83;
              temparray[2]=0x0c;
              paraint[2]=0;
              for(j=0;j<2;j++)
              {
              temparray[11+j]=sbuf[n+14+(1-j)]-0x33;
              paraint[2]=paraint[2]*100+(temparray[11+j]-(temparray[11+j]>>4)*6);
              }
              parafloat[2]=paraint[2]/1000.0;
              memcpy(&temparray[11],((unsigned char *)&parafloat[2]),4);

              paraint[3]=0;
              for(j=0;j<2;j++)
              {
              temparray[15+j]=sbuf[n+16+(1-j)]-0x33;
              paraint[3]=paraint[3]*100+(temparray[15+j]-(temparray[15+j]>>4)*6);
              }
              parafloat[3]=paraint[3]/1000.0;
              memcpy(&temparray[15],((unsigned char *)&parafloat[3]),4);
              
              getparaflag=2;
              break;
            }
          }
        }

      }
      }
      if(getparaflag==2)
      {
      for(k=0;k<16;k++)
      {
      Serial.write(command[17][k]);
      }
      delay(1000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              temparray[20]=7;
              temparray[19]=sbuf[n+15]-0x33;
              temparray[19]=temparray[19]-(temparray[19]>>4)*6;
              temparray[19]=temparray[19]+0xd0;
              temparray[21]=sbuf[n+14]-0x33;
              temparray[21]=temparray[21]-(temparray[21]>>4)*6;
              temparray[22]=sbuf[n+13]-0x33;
              temparray[22]=temparray[22]-(temparray[22]>>4)*6;
              temparray[23]=sbuf[n+18]-0x33;
              temparray[23]=temparray[23]-(temparray[23]>>4)*6;
              temparray[24]=sbuf[n+17]-0x33;
              temparray[24]=temparray[24]-(temparray[24]>>4)*6;
              temparray[25]=sbuf[n+16]-0x33;
              temparray[25]=temparray[25]-(temparray[25]>>4)*6;
              
              getparaflag=3;
              break;
            }
          }
        }
      }
      }
      
      if(getparaflag==3)
        {
          for(k=0;k<26;k++)
          {
          if(k>=3)
          temparray[k-1]=temparray[k];
          Serial.write(temparray[k]);
          }
          sprintf(msg,"%s%s","d/up/",biaohao);
          clientpublish(msg, temparray, 25);

          Serial.print("Publish para: ");
          Serial.print(msg);
          
        }
*/
  }
  
  if(statusconfig & ALARMstatus)
  {
    statusconfig &= ~ALARMstatus;
    
/*
    memset(temparray,0,5);

      getstatusflag=0;
      for(k=0;k<16;k++)
      {
      Serial.write(command[10][k]);
      }
      delay(1000);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);
        
        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              temparray[0]=0x04;
              temparray[1]=0x80;
              //temparray[2]=0x01;
              if((sbuf[n+12]-0x33)&0x01)
              {
                //temparray[4] |=0x04;
          temparray[2]=3;
          sprintf(msg,"%s%s","d/up/",biaohao);
          clientpublish(msg, temparray, 3);

          Serial.print("Publish alarm: ");
          Serial.print(msg);
          for(k=0;k<4;k++)
          {
          Serial.write(temparray[k]);
          }
              }
              if((sbuf[n+12]-0x33)&0x02)
              {
                //temparray[4] |=0x08;
          temparray[2]=4;
          sprintf(msg,"%s%s","d/up/",biaohao);
          clientpublish(msg, temparray, 3);

          Serial.print("Publish alarm: ");
          Serial.print(msg);
          for(k=0;k<4;k++)
          {
          Serial.write(temparray[k]);
          }
              }
              if((sbuf[n+12]-0x33)&0x04)
              {
                //temparray[4] |=0x10;
          temparray[2]=5;
          sprintf(msg,"%s%s","d/up/",biaohao);
          clientpublish(msg, temparray, 3);

          Serial.print("Publish alarm: ");
          Serial.print(msg);
          for(k=0;k<4;k++)
          {
          Serial.write(temparray[k]);
          }
              }
              if((sbuf[n+12]-0x33)&0x08)
              {
                //temparray[4] |=0x20;
          temparray[2]=6;
          sprintf(msg,"%s%s","d/up/",biaohao);
          clientpublish(msg, temparray, 3);

          Serial.print("Publish alarm: ");
          Serial.print(msg);
          for(k=0;k<4;k++)
          {
          Serial.write(temparray[k]);
          }
              }
              if((sbuf[n+12]-0x33)&0x10)
              {
                //temparray[4] |=0x40;
          temparray[2]=7;
          sprintf(msg,"%s%s","d/up/",biaohao);
          clientpublish(msg, temparray, 3);

          Serial.print("Publish alarm: ");
          Serial.print(msg);
          for(k=0;k<4;k++)
          {
          Serial.write(temparray[k]);
          }
              }
              if((sbuf[n+12]-0x33)&0x20)
              {
                //temparray[4] |=0x80;
          temparray[2]=8;
          sprintf(msg,"%s%s","d/up/",biaohao);
          clientpublish(msg, temparray, 3);

          Serial.print("Publish alarm: ");
          Serial.print(msg);
          for(k=0;k<4;k++)
          {
          Serial.write(temparray[k]);
          }
              }
              
              getstatusflag=1;
              
              break;
            }
          }
        }
      }
      //if(getstatusflag==1)
      {
      delay(500);
      while(Serial.read() >= 0){}
      for(k=0;k<18;k++)
      {
      Serial.write(command[11][k]);
      }
      delay(1200);
      if (Serial.available())//串口读取到的转发到wifi，因为串口是一位一位的发送所以在这里缓存完再发送
      {
        size_t counti = Serial.available();
        uint8_t sbuf[counti];

        Serial.readBytes(sbuf, counti);

        for(k=0;k<counti;k++)
          {
          Serial.write(sbuf[k]);
          }

        if(counti>12)
        {
        for(n=0;n<counti-12;n++)
          if((sbuf[n]==0x68)&&(sbuf[n+7]==0x68))
          {
            k=sbuf[n+9];
            if(k>2)
            {
              temparray[0]=0x04;
              temparray[1]=0x80;
              //temparray[2]=0x01;
              if((sbuf[n+14+6]-0x33)&0x20)
              {
                //temparray[3] |=0x01;
          temparray[2]=9;
          sprintf(msg,"%s%s","d/up/",biaohao);
          clientpublish(msg, temparray, 3);

          Serial.print("Publish alarm: ");
          Serial.print(msg);
          for(k=0;k<4;k++)
          {
          Serial.write(temparray[k]);
          }
              }
              if((sbuf[n+14+6]-0x33)&0x08)
              {
                //temparray[3] |=0x02;
          temparray[2]=10;
          sprintf(msg,"%s%s","d/up/",biaohao);
          clientpublish(msg, temparray, 3);

          Serial.print("Publish alarm: ");
          Serial.print(msg);
          for(k=0;k<4;k++)
          {
          Serial.write(temparray[k]);
          }
              }
              if((sbuf[n+14+8]-0x33)&0x20)
              {
                //temparray[3] |=0x04;
          temparray[2]=11;
          sprintf(msg,"%s%s","d/up/",biaohao);
          clientpublish(msg, temparray, 3);

          Serial.print("Publish alarm: ");
          Serial.print(msg);
          for(k=0;k<4;k++)
          {
          Serial.write(temparray[k]);
          }
              }
              if((sbuf[n+14+8]-0x33)&0x08)
              {
                //temparray[3] |=0x08;
          temparray[2]=12;
          temparray[3]=sbuf[n+14+8]-0x33;
          for(k=0;k<4;k++)
          {
          Serial.write(temparray[k]);
          }
          sprintf(msg,"%s%s","d/up/",biaohao);
          clientpublish(msg, temparray, 3);

          Serial.print("Publish alarm: ");
          Serial.print(msg);
          
              }
              
              getstatusflag=2;
              break;
            }
          }
        }

      }
      }
*/
      /*
      if(getstatusflag==2)
        {
          sprintf(msg,"%s%s","d/up/",biaohao);
          client.publish(msg, temparray, 5);

          Serial.print("Publish alarm: ");
          Serial.print(msg);
          for(k=0;k<5;k++)
          {
          Serial.write(temparray[k]);
          }
        }
      */
  }

/*
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
*/
}

/*
void clientpublish(char *msg,
                         uint8_s *data, uint16_s length)
{uint8_t k;
  kyzh_enveloped_data(data, 0x01,
                         encrypted_data_buf, length);
  client.publish(msg, encrypted_data_buf, length+8);
  Serial.print(length+8);
  Serial.print("<....");
  for(k=0;k<length+8;k++)
      {
      Serial.write(encrypted_data_buf[k]);
      }
  Serial.print("....>");
}
*/
void clientpublish(char *msg,
                         uint8_t *data, uint16_t length)
{uint8_t k;
//  kyzh_enveloped_data(data, 0x01,
//                         encrypted_data_buf, length);
  client.publish(msg, data, length);
  Serial.print(length);
  Serial.print("<....");
  for(k=0;k<length;k++)
      {
      Serial.write(data[k]);
      }
  Serial.print("....>");
}



