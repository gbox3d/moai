#include <Arduino.h>
#include <TaskScheduler.h>
#include <WiFiManager.h> // WiFiManager 라이브러리를 포함
#include <AsyncUDP.h>

#include <tonkey.hpp>

#include "config.hpp"

#include "moai_imu.hpp"

#if defined(LOLIN_D32)

const int ledPins[] = {4, 5};
const int analogPins[] = {34, 35};
const int buttonPins[] = {19, 23};

#elif defined(SEED_XIAO_ESP32C3)
const int ledPins[] = {D10, D9};
const int analogPins[] = {D0, D1};
const int buttonPins[] = {D8, D2};

#elif defined(LOLIN_D32_LITE)
const int ledPins[] = {4, 5};
const int analogPins[] = {34, 35};
const int buttonPins[] = {19, 23};

#endif

const int setupButtonPin = 0;
const int triggerButtonPin = 1;

const int statusLedPin = 0;
const int motorLedPin = 1;
const int batteryAnalogPin = 0;

String strTitleMsg = "it is MOAI-C3 (ARHS) revision 1";

String strHelpMsg = "command list\n\
help : show this message\n\
led on|off|toggle|pwm <index> <value> : control led\n\
button <index> : read button\n\
analog <index> : read analog\n\
config mNumber <value> : set mNumber\n\
config mMsg <value> : set mMsg\n\
save : save config\n\
load : load config\n\
clear : clear config\n\
reboot : reboot esp32\n\
print : print config\n\
";

CCongifData g_Config;
tonkey g_MainParser;
Scheduler runner;

bool bVerbose = false;

// long total_Elapsed = 0;
// long average_Elapsed = 0;
// long average_Count = 0;

//--------udp network code
AsyncUDP udp;
String udpAddress;
int udpPort = 9250;

struct S_Udp_IMU_RawData_Packet
{
  uint32_t checkCode;
  uint8_t cmd;
  uint8_t parm[3];
  float aX;
  float aY;
  float aZ;
  float gX;
  float gY;
  float gZ;
  float mX;
  float mY;
  float mZ;
  float extra;
  float battery;

  float pitch;
  float roll;
  float yaw;

  uint16_t dev_id;
  uint16_t fire_count;

  //quaternion
  float qw;
  float qx;
  float qy;
  float qz;

};

static S_Udp_IMU_RawData_Packet packet;
static float imudata[10];

String processCommand(String _strLine)
{
  _strLine.trim();
  g_MainParser.parse(_strLine);

  if (g_MainParser.getTokenCount() > 0)
  {
    String _result = "OK";

    String cmd = g_MainParser.getToken(0);
    

    if (cmd == "help")
    {
      _result = strTitleMsg + "\n" + strHelpMsg + "\nOK";
      // Serial.println(strTitleMsg);
      // Serial.println(strHelpMsg);
    }
    else if (cmd == "led")
    {
      if (g_MainParser.getTokenCount() > 2)
      {
        String _strLed = g_MainParser.getToken(1);
        int _index = g_MainParser.getToken(2).toInt();
        if (_strLed == "on")
        {
          digitalWrite(ledPins[_index], HIGH);
        }
        else if (_strLed == "off")
        {
          digitalWrite(ledPins[_index], LOW);
        }
        else if (_strLed == "toggle")
        {
          digitalWrite(ledPins[_index], !digitalRead(ledPins[_index]));
        }
        else if (_strLed == "pwm")
        {
          if (g_MainParser.getTokenCount() > 3)
          {
            int _value = g_MainParser.getToken(3).toInt();
            analogWrite(ledPins[_index], _value);
          }
          else
          {
            _result = "FAIL";
          }
        }
        else
        {
          _result = "FAIL";
        }
      }
      else
      {
        _result = "FAIL";
      }
    }
    else if (cmd == "button")
    {
      if (g_MainParser.getTokenCount() > 1)
      {

        int _index = g_MainParser.getToken(1).toInt();
        int _value = digitalRead(buttonPins[_index]);
        _result = String(_value) + "\nOK";
      }
      else
      {
        _result = "FAIL";
      }
    }
    else if (cmd == "analog")
    {
      if (g_MainParser.getTokenCount() > 1)
      {

        int _index = g_MainParser.getToken(1).toInt();
        int _value = analogRead(analogPins[_index]);
        _result = String(_value) + "\nOK";
      }
      else
      {
        _result = "FAIL";
      }
    }
    else if (cmd == "config")
    {
      String _strCmd = g_MainParser.getToken(1);
      if (_strCmd == "devid")
      {
        if (g_MainParser.getTokenCount() > 2)
        {
          g_Config.mDeviceNumber = g_MainParser.getToken(2).toInt();
        }
        else
        {
          _result = "FAIL";
        }
      }
      else if (_strCmd == "target")
      {
        if (g_MainParser.getTokenCount() > 3)
        {
          g_Config.mTargetIp = g_MainParser.getToken(2);
          g_Config.mTargetPort = g_MainParser.getToken(3).toInt();
        }
        else
        {
          _result = "FAIL";
        }
      }
      else
      {
        _result = "FAIL";
      }
    }
    else if (cmd == "save")
    {
      g_Config.save();
    }
    else if (cmd == "load")
    {
      g_Config.load();
    }
    else if (cmd == "clear")
    {
      g_Config.clear();
    }
    else if (cmd == "reboot")
    {
      ESP.restart();
    }
    else if (cmd == "print")
    {
      _result = g_Config.dump() + "\nOK";
    }
    else if (cmd == "imu")
    {

      String _strCmd = g_MainParser.getToken(1);
      if (_strCmd == "start")
      {
        // initDmp(g_Config.mOffsets);
        arhs::setup();
      }
      else if (_strCmd == "stop")
      {
        // closeDmp();
      }
      else if (_strCmd == "zero")
      {
        // doZero();
        arhs::calibration();
      }
      else if (_strCmd == "offset")
      {
        _result += String(arhs::getGyroXoffset()) + "\n";
        _result += String(arhs::getGyroYoffset()) + "\n";
        _result += String(arhs::getGyroZoffset()) + "\nOK";
      }
      else if (_strCmd == "saveOffset")
      {
        g_Config.mOffsets[3] = arhs::getGyroXoffset() * 100;
        g_Config.mOffsets[4] = arhs::getGyroYoffset() * 100;
        g_Config.mOffsets[5] = arhs::getGyroZoffset() * 100;

        // int16_t *pOffset = GetActiveOffset();
        // memcpy(g_Config.mOffsets, pOffset, sizeof(int16_t) * 6);

        g_Config.save();
      }
      else if (_strCmd == "verbose")
      {
        bVerbose = !bVerbose;
      }
      else if (_strCmd == "status")
      {
        _result = String("ax : ") + String(imudata[0]) + "\n" +
                  String("ay : ") + String(imudata[1]) + "\n" +
                  String("az : ") + String(imudata[2]) + "\n" +
                  String("yaw : ") + String(imudata[3]) + "\n" +
                  String("pitch : ") + String(imudata[4]) + "\n" +
                  String("roll : ") + String(imudata[5]) + "\nOK";
      }
      else
      {
        _result = "FAIL";
      }
    }
    else
    {
      _result = "FAIL";
    }
    return _result;
  }
  else
  {
    return "NOK";
  }
  
}

Task task_Cmd(
    100, TASK_FOREVER, []()
    {
    if (Serial.available() > 0)
    {
        String _strLine = Serial.readStringUntil('\n');
        String _r = processCommand(_strLine);
        Serial.println(_r);
        
    } });

void sendDataToServer(S_Udp_IMU_RawData_Packet &packet)
{
  IPAddress serverIP;
  serverIP.fromString(udpAddress); // Convert the C string to an IPAddress
  udp.writeTo((uint8_t *)&packet, sizeof(S_Udp_IMU_RawData_Packet), serverIP, udpPort);
}

Task task_Packet(50, TASK_FOREVER, []()
                 {
  packet.checkCode = 20230903;
  packet.cmd = 0x10;
  packet.dev_id = (uint16_t)g_Config.mDeviceNumber;

  // linear acceleration
  packet.aX = imudata[0];
  packet.aY = imudata[1];
  packet.aZ = imudata[2];

  //yaw pitch roll
  packet.yaw = imudata[3];
  packet.pitch = imudata[4];
  packet.roll = imudata[5];

  //quaternion
  packet.qw = imudata[6];
  packet.qx = imudata[7];
  packet.qy = imudata[8];
  packet.qz = imudata[9];

  packet.battery = analogRead(batteryAnalogPin) / 4096.0 * 3.3 * 2;

  sendDataToServer(packet); });

// WiFi 이벤트 핸들러 함수
void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_DISCONNECTED:
    digitalWrite(ledPins[statusLedPin], LOW); // WiFi 연결 해제 시 LED 끄기
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
  case SYSTEM_EVENT_STA_GOT_IP:
    digitalWrite(ledPins[statusLedPin], HIGH); // WiFi 연결 시 LED 켜기
    break;
  default:
    break;
  }
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("#AP mode started. Please enter config.");
  digitalWrite(ledPins[statusLedPin], HIGH);
}

void setup()
{

  // io setup
  for (int i = 0; i < sizeof(ledPins) / sizeof(ledPins[0]); i++)
  {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  for (int i = 0; i < sizeof(buttonPins) / sizeof(buttonPins[0]); i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP); // LOW: button pressed
  }

  for (int i = 0; i < sizeof(analogPins) / sizeof(analogPins[0]); i++)
  {
    pinMode(analogPins[i], INPUT);
  }

  Serial.begin(115200);

  while (!Serial)
    ;

  g_Config.load();

  WiFiManager wifiManager;

  if (digitalRead(buttonPins[setupButtonPin]) == LOW)
  {
    
    g_Config.clear();

    // ap mode enter signal
    digitalWrite(ledPins[motorLedPin], HIGH);
    delay(500);
    digitalWrite(ledPins[motorLedPin], LOW);

    // 사용자 정의 매개변수 (장치 번호를 위한)
    WiFiManagerParameter custom_device_number("device_number", "장치 번호", String(g_Config.mDeviceNumber).c_str(), 10);
    WiFiManagerParameter custom_target_ip("target_ip", "타겟 IP", g_Config.mTargetIp.c_str(), 16);
    WiFiManagerParameter custom_target_port("target_port", "타겟 포트", String(g_Config.mTargetPort).c_str(), 6);

    // WiFiManagerParameter custom_offset0("offset0", "Gyro X offset", String(g_Config.mOffsets[0]).c_str(), 6);
    // WiFiManagerParameter custom_offset1("offset1", "Gyro Y offset", String(g_Config.mOffsets[1]).c_str(), 6);
    // WiFiManagerParameter custom_offset2("offset2", "Gyro Z offset", String(g_Config.mOffsets[2]).c_str(), 6);

    WiFiManagerParameter custom_offset3("offset3", "Accel X offset", String(g_Config.mOffsets[3]).c_str(), 6);
    WiFiManagerParameter custom_offset4("offset4", "Accel Y offset", String(g_Config.mOffsets[4]).c_str(), 6);
    WiFiManagerParameter custom_offset5("offset5", "Accel Z offset", String(g_Config.mOffsets[5]).c_str(), 6);

    // 'Reset Settings'라는 이름의 추가 메뉴 항목을 만들어서 WiFi 설정을 리셋할 수 있게 함
    // WiFiManager에 장치 번호 매개변수 추가
    wifiManager.addParameter(&custom_device_number);
    wifiManager.addParameter(&custom_target_ip);
    wifiManager.addParameter(&custom_target_port);

    // wifiManager.addParameter(&custom_offset0);
    // wifiManager.addParameter(&custom_offset1);
    // wifiManager.addParameter(&custom_offset2);
    wifiManager.addParameter(&custom_offset3);
    wifiManager.addParameter(&custom_offset4);
    wifiManager.addParameter(&custom_offset5);

    // WiFiManager에 AP 모드 콜백 설정
    wifiManager.setAPCallback(configModeCallback);
    Serial.println("Resetting WiFi settings...");
    wifiManager.resetSettings();

    // ESP32 칩셋 ID를 얻어서 문자열로 변환
    String chipId = String((uint32_t)ESP.getEfuseMac(), HEX);
    chipId.toUpperCase(); // 대문자로 변환

    // AP 모드의 이름을 "BSQ-"와 칩셋 ID의 조합으로 설정
    String ssid = "BSQ-" + chipId;
    const char *ssid_ptr = ssid.c_str(); // const char* 타입으로 변환

    // 여기서 "AutoConnectAP"는 ESP32가 AP 모드로 전환했을 때 나타나는 네트워크의 이름입니다.
    // "password"는 AP 모드의 비밀번호입니다 (최소 8자 필요).
    // 이 부분은 필요에 따라 수정하면 됩니다.
    wifiManager.autoConnect(ssid_ptr, "123456789");

    g_Config.mDeviceNumber = String(custom_device_number.getValue()).toInt();
    g_Config.mTargetIp = custom_target_ip.getValue();
    g_Config.mTargetPort = String(custom_target_port.getValue()).toInt();

    // g_Config.mOffsets[0] = String(custom_offset0.getValue()).toInt();
    // g_Config.mOffsets[1] = String(custom_offset1.getValue()).toInt();
    // g_Config.mOffsets[2] = String(custom_offset2.getValue()).toInt();
    g_Config.mOffsets[3] = String(custom_offset3.getValue()).toInt();
    g_Config.mOffsets[4] = String(custom_offset4.getValue()).toInt();
    g_Config.mOffsets[5] = String(custom_offset5.getValue()).toInt();

    // Serial.println(g_Config.dump() + "");
    g_Config.save();

    // 연결된 WiFi 네트워크의 이름과 IP 주소를 표시
    Serial.print("연결된 네트워크 이름: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP 주소: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    wifiManager.autoConnect();
    Serial.println("\nConnected to WiFi network!");
    // digitalWrite(LED1_PIN, HIGH);
  }

  udpAddress = g_Config.mTargetIp;
  udpPort = g_Config.mTargetPort;

  if (udp.listen(udpPort))
  {
    udp.onPacket([](AsyncUDPPacket packet)
                 {
                   // Handle incoming packets here
                 });
  }

  Serial.printf("target address : %s:%d\n", udpAddress.c_str(), udpPort);

  digitalWrite(ledPins[statusLedPin], HIGH);
  delay(500);
  digitalWrite(ledPins[statusLedPin], LOW);

  //imu setup
  // initDmp(g_Config.mOffsets); // start dmp
  arhs::setup();
  arhs::setGyroOffsets(
    g_Config.mOffsets[3] / 100.0, 
    g_Config.mOffsets[4] / 100.0, 
    g_Config.mOffsets[5] / 100.0);

  digitalWrite(ledPins[statusLedPin], HIGH);

  // delay(100);
  Serial.println("imu init done");

  runner.init();
  runner.addTask(task_Cmd);
  runner.addTask(task_Packet);

  task_Cmd.enable();
  task_Packet.enable();

  // delay(100);
  Serial.println(strTitleMsg);
}

void loop()
{
  // if(arhs::updateXZY(imudata)) {
  if(arhs::updateXYZ(imudata)) {
    if (bVerbose)
    {
      Serial.printf("yaw:%.2f pitch:%.2f roll:%.2f \n", imudata[3], imudata[4], imudata[5]);
    }
  }

  // if (getDmpReady())
  // {
  //   // if (getAccYpr(imudata))
  //   // if(getAccel(imudata))
  //   if(getQuaternion( &(imudata[6])) )
  //   {
  //     if (bVerbose)
  //     {
  //       Serial.printf("ax:%.2f ay:%.2f az:%.2f ", imudata[0], imudata[1], imudata[2]);
  //       Serial.printf("yaw:%.2f pitch:%.2f roll:%.2f ", imudata[3], imudata[4], imudata[5]);
  //       Serial.printf("qw:%.2f qx:%.2f qy:%.2f qz:%.2f\n", imudata[6], imudata[7], imudata[8], imudata[9]);
  //     }
  //   }
  // }

  runner.execute();
}
