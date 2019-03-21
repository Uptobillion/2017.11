//mini板拷入代码方法：需要在烧录的时候，提示upload（上传）时，手动按一下板子上的RESET按钮复位
//输出位置状态格式:'S'+libfloor+'A'+libroom+'A''+libdesk+'A''+s（有、无人状态）+'B'，例如:S00A0000A0000A0B
#define libfloor "03"//楼层2位
#define libroom "0001"//房间4位
#define libdesk "0001"//座位4位
#define ssid "Jesse Wang"//WIFI's SSID
#define pwd "00000001"//WIFI密码
#define severaddress "115.159.65.240"//服务器地址
#define severport "8081"//服务器端口
#define mi 3//有无人算法权重 有人的最小权重 周期=mi*randr1（秒）
#define ma 12//有无人算法权重 有人的最大权重 周期=ma*randr1（秒）
#define q1 360//连接检测周期数 周期=q1*randr1（秒）
#define distance1 50//距离判定触发距离 单位：厘米
#define randr1 5000//运行周期 单位：毫秒 最小值：80
//esp8266的CH_PD和vcc连接3.3v esp&HCSR04的gnd连gnd
#define rxpin 2//2 接收无线数据引脚 连接esp8266Tx引脚
#define txpin 3//3 发送无线数据引脚 连接esp8266Rx引脚
#define inputpin 4//4引脚连接HCSR04传感器Echo
#define outputpin 5//5引脚连接HCSR04传感器Trig
#define controlpin 6//6引脚连接HCSR04传感器Vcc
#define ledtestpin 13//测试引脚 判定为有人时亮起
#include <SoftwareSerial.h>
SoftwareSerial mySerial(rxpin, txpin);
int ok = 0;
int randr = randr1 - 80;//
int avb;//最近一次接收端口字符数
int i;//有无人算法权重
int distance;//前方物体距离
int q = q1;
char s = '0';//有、无人状态
char val = '0';//本机运行状态
char val1 = '0';
char val2 = '0';
void connectwifi();//连接wifi&服务器函数
void send2wifi(char b);//发送位置&状态函数
void dis();//测距函数
void lop();//有、无人判断算法
void a();//总控函数
void setup() {
  Serial.begin(9600);
  pinMode(ledtestpin, OUTPUT);
  pinMode(controlpin, OUTPUT);
  pinMode(inputpin, INPUT);
  pinMode(outputpin, OUTPUT);
  mySerial.begin(115200);
  //connectwifi();
  delay(6000);
  for (; mySerial.available() > 0;) {//清除缓存
    val2 = mySerial.read();
  }
  send2wifi(s);
  mySerial.print("\n");
}
void loop() {
  a();
}
void a() {
  if (q == q1) {
    mySerial.print("ONLINE");
    mySerial.print("\n");
    q = 0;
  }
  delay(randr);
  avb = mySerial.available();
  //mySerial.print("available:");
  //mySerial.print(avb);
  //mySerial.print("\n");
  for (; mySerial.available() > 0;) {
    val1 = mySerial.read();
    if (val1 == '1' || val1 == '0') {
      val = val1;
      //mySerial.print("Our device has been connected\n");
      ok = 1;
    }
  }

  if (q == 0 && ok == 0) {
    //mySerial.print("Our device has been Unconnected\n");
    //if (avb < 1) {
    connectwifi();
    send2wifi(s);
    mySerial.print("\n");
    q = q1 - 1;
    //}
  }
  ok = 0;
  if (val == '1') {
    //mySerial.print("Device running");
    //mySerial.print("\n");
    dis();
    lop();
  } else {
    if (val == '0') {
      //mySerial.print("Device has stoped");
      //mySerial.print("\n");
      s = '0';
      i = 0;
    }
  }
  q++;
}
void dis() {
  digitalWrite(controlpin, HIGH);
  delay(80);
  digitalWrite(outputpin, LOW); // 使发出发出超声波信号接口低电平2μs
  delayMicroseconds(2);
  digitalWrite(outputpin, HIGH); // 使发出发出超声波信号接口高电平10μs，这里是至少10μs
  delayMicroseconds(10);
  digitalWrite(outputpin, LOW); // 保持发出超声波信号接口低电平
  distance = pulseIn(inputpin, HIGH); // 读出脉冲时间
  digitalWrite(controlpin, LOW);
  distance = distance / 58; // 将脉冲时间转化为距离（单位：厘米）
  //mySerial.print("The distance is:"); //输出距离值
  //mySerial.print(distance); //输出距离值
  //mySerial.print("\n"); //输出距离值
}
void lop() {
  if (distance <= distance1) {
    if (s == '1') {
      if (i < ma) {
        i++;
      }
    }
    else {
      i++;
      if (i >= mi) {
        s = '1';
        i = ma;
        digitalWrite(ledtestpin, HIGH); //发送坐标 发送“有人”
        send2wifi(s);
        mySerial.print("\n");
      }
    }
  }
  else {
    if (i > 0) {
      i--;
      if (s == '1') {
        if (i == 0) {
          s = '0';
          digitalWrite(ledtestpin, LOW); //发送坐标 发送“无人”
          send2wifi(s);
          mySerial.print("\n");
        }
      }
    }
  }
}
/*void connectwifi() {
  delay(50);
  mySerial.print("+++");//退出透传模式 进入指令模式
  delay(18);
  mySerial.print("AT+CWJAP=\"");//连接wifi 1
  mySerial.print(ssid);
  mySerial.print("\",\"");
  mySerial.print(pwd);
  mySerial.print("\"\r\n");//连接wifi 2
  for (; mySerial.available() < 1;) {
  }
  for (; mySerial.available() > 0;) {//清除缓存
    val2 = mySerial.read();
  }
  mySerial.print("AT+SAVETRANSLINK=1,\"");//连接服务器&端口 1
  mySerial.print(severaddress);
  mySerial.print("\",");
  mySerial.print(severport);
  mySerial.print(",\"TCP\"\r\n");//连接服务器&端口 2
  for (; mySerial.available() < 1;) {
  }
  for (; mySerial.available() > 0;) {//清除缓存
    val2 = mySerial.read();
  }
  mySerial.print("AT+RST\r\n");//开启透传
  delay(5000);
  for (; mySerial.available() < 1;) {//清除缓存
  }
  for (; mySerial.available() > 0;) {//清除缓存
    val2 = mySerial.read();
  }
  }*/
void connectwifi() {
  delay(50);
  mySerial.print("+++");
  delay(18);
  mySerial.print("AT+CWJAP=\"");
  mySerial.print(ssid);
  mySerial.print("\",\"");
  mySerial.print(pwd);
  mySerial.print("\"\r\n");
  delay(10000);
  mySerial.print("AT+SAVETRANSLINK=1,\"");
  mySerial.print(severaddress);
  mySerial.print("\",");
  mySerial.print(severport);
  mySerial.print(",\"TCP\"\r\n");
  delay(1000);
  mySerial.print("AT+RST\r\n");
  delay(15000);
  for (; mySerial.available() < 1;) {//清除缓存
  }
  for (; mySerial.available() > 0;) {//清除缓存
    val2 = mySerial.read();
  }
}
void send2wifi(char b) {
  mySerial.print('S');
  mySerial.print(libfloor);
  mySerial.print('A');
  mySerial.print(libroom);
  mySerial.print('A');
  mySerial.print(libdesk);
  mySerial.print('A');
  mySerial.print(b);
  mySerial.print('B');
}
