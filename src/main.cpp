#include <Arduino.h>
#include <TaskScheduler.h>
// #include <WiFiManager.h> // WiFiManager 라이브러리를 포함
#include <WiFi.h>
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

String strTitleMsg = "it is MOAI-C3 (DMP) revision 1";

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
imu start|stop|zero|verbose|status : imu control\n\
wifi scan|setup_sta <ssid> <password>|connect <ssid> <password>| \n\
wifi disconnect|status|ip|mac|rssi|gateway|dns|start_broadcast|stop_broadcast|send <ip> <port> <msg> : wifi control\n\
";

String strBroadCastMsg;
unsigned int localUdpPort = 7204;

CCongifData g_Config;
tonkey g_MainParser;
Scheduler runner;

bool bVerbose = false;
bool bImuInit = false;

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

  // quaternion
  float qw;
  float qx;
  float qy;
  float qz;
};

static S_Udp_IMU_RawData_Packet packet;
static float imudata[10];

Task task_udpBroadCast(
    5000, TASK_FOREVER, []()
    {
      udp.broadcastTo(strBroadCastMsg.c_str(), localUdpPort);
      // Serial.println("udp broadcast");
    });

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
      }
      else if (_strCmd == "stop")
      {
        closeDmp();
      }
      else if (_strCmd == "zero")
      {
        doZero();

        int16_t *pOffset = GetActiveOffset();
        memcpy(g_Config.mOffsets, pOffset, sizeof(int16_t) * 6);
      }
      else if (_strCmd == "verbose")
      {
        bVerbose = !bVerbose;
      }
      else if (_strCmd == "status")
      {
        int16_t *pOffset = GetActiveOffset();
        _result = String("ax : ") + String(imudata[0]) + "\n" +
                  String("ay : ") + String(imudata[1]) + "\n" +
                  String("az : ") + String(imudata[2]) + "\n" +
                  String("yaw : ") + String(imudata[3]) + "\n" +
                  String("pitch : ") + String(imudata[4]) + "\n" +
                  String("roll : ") + String(imudata[5]) + "\n" +
                  String("qw : ") + String(imudata[6]) + "\n" +
                  String("qx : ") + String(imudata[7]) + "\n" +
                  String("qy : ") + String(imudata[8]) + "\n" +
                  String("qz : ") + String(imudata[9]) + "\n" +
                  String("offset0 : ") + String(pOffset[0]) + "\n" +
                  String("offset1 : ") + String(pOffset[1]) + "\n" +
                  String("offset2 : ") + String(pOffset[2]) + "\n" +
                  String("offset3 : ") + String(pOffset[3]) + "\n" +
                  String("offset4 : ") + String(pOffset[4]) + "\n" +
                  String("offset5 : ") + String(pOffset[5]) + "\nOK";
      }
      else
      {
        _result = "FAIL";
      }
    }

    else if (cmd == "wifi")
    {
      cmd = g_MainParser.getToken(1);
      if ((cmd == "scan"))
      {
        int n = WiFi.scanNetworks();
        Serial.println("scan done");
        if (n == 0)
        {
          Serial.println("no networks found");
          _result = "no networks found\nOK";
        }
        else
        {
          _result = String(n) + " networks found\nOK";

          for (int i = 0; i < n; i++)
          {
            _result += String(i + 1) + ": " + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + ") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*") + "\n";
          }
        }
        // Serial.println("");
      }
      else if ((cmd == "setup_sta") && (g_MainParser.getTokenCount() > 2))
      {
        String _strAp = g_MainParser.getToken(2);
        String _strPassword = g_MainParser.getToken(3);

        g_Config.mStrAp = _strAp;
        g_Config.mStrPassword = _strPassword;

        WiFi.mode(WIFI_STA);
        WiFi.begin(_strAp.c_str(), _strPassword.c_str());
      }
      else if ((cmd == "connect") && (g_MainParser.getTokenCount() > 2))
      {
        g_Config.mStrAp = g_MainParser.getToken(2);
        g_Config.mStrPassword = g_MainParser.getToken(3);
      }
      else if ((cmd == "disconnect"))
      {
        WiFi.disconnect();
      }
      else if ((cmd == "status"))
      {
        _result = String("WiFi status: " + WiFi.status()) + "\nOK";
        // Serial.print("WiFi status: ");
        // Serial.println(WiFi.status());
      }
      else if ((cmd == "ip"))
      {
        // Serial.print("IP address: ");
        // Serial.println(WiFi.localIP());
        _result = String("IP address: " + WiFi.localIP().toString()) + "\nOK";
      }
      else if ((cmd == "mac"))
      {
        // Serial.print("MAC address: ");
        // Serial.println(WiFi.macAddress());
        _result = String("MAC address: " + WiFi.macAddress()) + "\nOK";
      }
      else if ((cmd == "rssi"))
      {
        // Serial.print("RSSI: ");
        // Serial.println(WiFi.RSSI());
        _result = String("RSSI: " + String(WiFi.RSSI())) + "\nOK";
      }
      else if ((cmd == "gateway"))
      {
        // Serial.print("Gateway: ");
        // Serial.println(WiFi.gatewayIP());
        _result = String("Gateway: " + WiFi.gatewayIP().toString()) + "\nOK";
      }
      else if ((cmd == "dns"))
      {
        // Serial.print("DNS: ");
        // Serial.println(WiFi.dnsIP());
        _result = String("DNS: " + WiFi.dnsIP().toString()) + "\nOK";
      }
      else if (cmd == "start_broadcast")
      {
        task_udpBroadCast.enable();
      }
      else if (cmd == "stop_broadcast")
      {
        task_udpBroadCast.disable();
      }
      else if (cmd == "send")
      {
        IPAddress serverIP;
        serverIP.fromString(g_MainParser.getToken(1));
        unsigned int port = g_MainParser.getToken(2).toInt();

        String msg = g_MainParser.getToken(3);

        udp.writeTo((const uint8_t *)msg.c_str(), msg.length(), serverIP, port);
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
  case SYSTEM_EVENT_STA_CONNECTED:
    Serial.println("Connected to WiFi");
    digitalWrite(ledPins[statusLedPin], HIGH);

    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("Disconnected from WiFi");
    digitalWrite(ledPins[statusLedPin], LOW);
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());
    break;
  default:
    break;
  }
}

void setup()
{
  String chipId = String((uint32_t)ESP.getEfuseMac(), HEX);
  chipId.toUpperCase(); // 대문자로 변환

  strBroadCastMsg = "#BC__BSQ-" + chipId;

  Serial.begin(115200);

  g_Config.load();

  udpAddress = g_Config.mTargetIp;
  udpPort = g_Config.mTargetPort;

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

  // connect to wifi
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_STA);

  if (g_Config.mStrAp.length() > 0)
  {
    WiFi.begin(g_Config.mStrAp.c_str(), g_Config.mStrPassword.c_str());
  }
  else
  {
    WiFi.begin();
  }

  // receive incoming UDP packets
  if (udp.listen(localUdpPort))
  {
    udp.onPacket([](AsyncUDPPacket _packet)
                 {
        String _strRes = processCommand((const char *)_packet.data());
        //response to the client
        _packet.printf("#RES__%s",_strRes.c_str()); });
  }

  // imu setup
  initDmp(g_Config.mOffsets); // start dmp

  // task setup
  runner.init();

  runner.addTask(task_Cmd);
  runner.addTask(task_Packet);
  runner.addTask(task_udpBroadCast);

  task_Cmd.enable();
  task_Packet.enable();
  task_udpBroadCast.enable();

  // Serial.println(strTitleMsg);
}

void _updateImu()
{
  if (getDmpReady())
  {
    // if (getAccYpr(imudata))
    // if(getAccel(imudata))
    if (getQuaternion(&(imudata[6])))
    {
      if (bVerbose)
      {
        // Serial.printf("ax:%.2f ay:%.2f az:%.2f ", imudata[0], imudata[1], imudata[2]);
        // Serial.printf("yaw:%.2f pitch:%.2f roll:%.2f ", imudata[3], imudata[4], imudata[5]);
        Serial.printf("qw:%.2f qx:%.2f qy:%.2f qz:%.2f\n", imudata[6], imudata[7], imudata[8], imudata[9]);
      }
    }
  }
}

void _updateTrigger() // trigger button process
{
  static int btnTrigerStatus = 0;
  static unsigned long btnTrigerTime = 0;

  switch (btnTrigerStatus)
  {
  case 0:
    if (digitalRead(buttonPins[triggerButtonPin]) == LOW && digitalRead(buttonPins[setupButtonPin]) == HIGH)
    {
      btnTrigerStatus = 1;
      btnTrigerTime = millis();
      packet.fire_count++;
      Serial.println("fire count : " + String(packet.fire_count));
      digitalWrite(ledPins[motorLedPin], HIGH);
    }
    break;
  case 1:
    if (digitalRead(buttonPins[triggerButtonPin]) == HIGH)
    {
      btnTrigerStatus = 2;
    }
    break;
  case 2: // wait cool time
    if (millis() - btnTrigerTime > 300)
    {
      btnTrigerStatus = 0;
      digitalWrite(ledPins[motorLedPin], LOW);
    }
    break;
  default:
    break;
  }
}

void loop()
{
  _updateImu();
  _updateTrigger();

  runner.execute();
}