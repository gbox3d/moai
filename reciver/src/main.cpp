#include <Arduino.h>
#include <TaskScheduler.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncUDP.h>

#include "tonkey.hpp"

#include "packet.hpp"

AsyncUDP udp;

Scheduler g_runner;
tonkey g_MainParser;
String ssid = "moai_";
String password = "3117-2001";

bool isConnect = false;
u_int32_t lastTime = 0;

IPAddress remoteIp;
uint16_t remotePort = 7204;

float battery;
uint16_t fire_count;
u16_t mode_switch;
int gun_status;
float quat[4]; // wxyz

Task taskBlink(1000, TASK_FOREVER, []()
               {
                 static bool _state = false;
                 _state = !_state;
                 digitalWrite(LED_BUILTIN, _state); });

Task checkConnections(1000, TASK_FOREVER, []()
                      {
                        if (lastTime > 0)
                        {
                          if (millis() - lastTime > 2000 && isConnect == true)
                          {
                            // digitalWrite(LED_BUILTIN, LOW);
                            taskBlink.setInterval(1000);
                            isConnect = false;
                          }
                          else if (millis() - lastTime < 2000 && isConnect == false)
                          {
                            // digitalWrite(LED_BUILTIN, HIGH);
                            taskBlink.setInterval(100);
                            isConnect = true;
                          }
                          else
                          {
                            // digitalWrite(LED_BUILTIN, HIGH);
                          }
                        } });

Task task_Cmd(
    100, TASK_FOREVER, []()
    {
      if (Serial.available() > 0)
      {
        String _strLine = Serial.readStringUntil('\n');
        _strLine.trim();
        Serial.println(_strLine);
        g_MainParser.parse(_strLine);

        String _result = "OK";

        if (g_MainParser.getTokenCount() > 0)
        {
          String cmd = g_MainParser.getToken(0);

          if (cmd == "about")
          {
            _result = "it is [moai receiver],version 1.0.0,";
            _result += "ssid : " + ssid + ",";
            _result += "passwd : " + password + ",";
            _result += "OK";
          }
          else if (cmd == "blinkfast")
          {
            taskBlink.setInterval(100);
          }
          else if (cmd == "blinkslow")
          {
            taskBlink.setInterval(1000);
          }
          else if(cmd =="activate") { 

            String msg = "battery\nwakeup\n"; 
            if(g_MainParser.getTokenCount() > 1 ) {
              msg = "battery\nwakeup " + g_MainParser.getToken(1) + "\n";
            }
            //배터리정보 업데이트 후 30초간 전송 활성 상태후 슬립사태로 전환

            udp.writeTo((const uint8_t *)msg.c_str(), msg.length(),
              remoteIp, remotePort);

          }
          else if(cmd == "gun") {

            if(g_MainParser.getTokenCount() > 1) {
              String msg = "gun ";

              if(g_MainParser.getToken(1) == "enable") {
                msg += "enable";
                
              }
              else if(g_MainParser.getToken(1) == "disable") {
                msg += "disable";
              }
              
              msg += "\n";

              udp.writeTo((const uint8_t *)msg.c_str(), msg.length(),
                  remoteIp, remotePort);
            }
          }
          else if(cmd =="apinfo") {
            struct station_info *station_list = wifi_softap_get_station_info();

            //1개만 가져오기 
            if(station_list != NULL) {
              remoteIp = IPAddress((&station_list->ip)->addr);
              _result += remoteIp.toString() + "\n";
              wifi_softap_free_station_info(); // 목록의 메모리를 해제
            }
            else {
              _result = "nosta,OK\n";
            }
            // while (station_list != NULL) {
            //     remoteIp = IPAddress((&station_list->ip)->addr);
            //     // Serial.print("IP Address: ");
            //     // Serial.println(ip);
            //     station_list = STAILQ_NEXT(station_list, next);
            //     _result += remoteIp.toString() + "\n";
            // }
            // wifi_softap_free_station_info(); // 목록의 메모리를 해제
          }
          // else if(cmd == "wakeup") {

          // }
          else if (cmd == "reboot")
          {
            ESP.restart();
          }
          else
          {
            _result = "FAIL";
          }
          
          Serial.println('%' + _result);
          Serial.println(); // new line
        }
      } });

// void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
//   Serial.print("Station connected: ");
//   char macStr[18];
//   sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", evt.mac[0], evt.mac[1], evt.mac[2], evt.mac[3], evt.mac[4], evt.mac[5]);
//   Serial.println(macStr);
// //remote ip 얻기
//   remoteIp = evt.aid;
//   remotePort = 9250;
// }

void setup()
{

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  Serial.println("\n\n#moai reciver start\n\n");

  g_runner.init();
  g_runner.addTask(task_Cmd);
  g_runner.addTask(taskBlink);
  g_runner.addTask(checkConnections);

  task_Cmd.enable();
  taskBlink.enable();
  checkConnections.enable();

  // esp8266 ap mode
  // ap 이름은 "moai_chipid" 비밀번호는 "12345678"

  // ESP8266 칩 아이디를 문자열로 변환하여 AP 이름 생성
  String chipId = String(ESP.getChipId(), HEX);
  ssid += chipId;

  // AP 모드 설정
  // const char *password = "3117-2001"; // AP의 비밀번호
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), password.c_str());

  // WiFi.onSoftAPModeStationConnected(&onStationConnected);

  // Serial.println("AP 모드로 설정됨");
  Serial.print("#AP SSID: ");
  Serial.println(ssid);

  // UDP 서버 시작
  const int udpPort = 9250; // UDP 포트 번호

  // lastTime = millis(); // 현재 시간 저장

  // UDP 서버 시작
  if (udp.listen(udpPort))
  {
    Serial.print("#UDP start on port : ");
    Serial.println(udpPort);
    // 수신 이벤트 핸들러 등록
    udp.onPacket([](AsyncUDPPacket packet)
                 {
                   // 패킷 크기 확인
                   if (packet.length() == sizeof(S_Udp_IMU_RawData_Packet))
                   {
                     remoteIp = packet.remoteIP();
                     remotePort = packet.remotePort();

                     S_Udp_IMU_RawData_Packet imuData;

                     //  imuData = *((S_Udp_IMU_RawData_Packet *)packet.data());

                     memcpy(&imuData, packet.data(), sizeof(imuData));

                     if (imuData.checkCode == 20230903)
                     {
                       fire_count = imuData.fire_count;
                       battery = imuData.battery;

                       mode_switch = imuData.parm[2];
                       gun_status = imuData.parm[1];

                       quat[0] = imuData.qw;
                       quat[1] = imuData.qx;
                       quat[2] = imuData.qy;
                       quat[3] = imuData.qz;

                       lastTime = millis();

                       Serial.printf("\n$ %d %d %d %f %f %f %f %f\n", fire_count , mode_switch,gun_status ,battery ,quat[0], quat[1], quat[2], quat[3]);
                     }

                     // 데이터 출력
                     //  Serial.print("Device ID: ");
                     //  Serial.println(imuData.dev_id);
                     //  Serial.print("Acceleration X: ");
                     //  Serial.println(imuData.aX);
                     //  Serial.print("Gyro Y: ");
                     //  Serial.println(imuData.gY);
                     //  Serial.print("Magnetometer Z: ");
                     //  Serial.println(imuData.mZ);
                     //  Serial.print("Battery Level: ");
                     //  Serial.println(imuData.battery);
                     //  Serial.print("Yaw: ");
                     //  Serial.println(imuData.yaw);
                     //  // 추가적으로 필요한 데이터 출력을 이곳에 추가
                   }
                   else
                   {
                     Serial.print("Received packet size: ");
                     Serial.println(packet.length());
                     Serial.println((char *)packet.data());
                     //  Serial.println("Received packet size does not match IMU data structure size.");
                   }

                   // Serial.print("수신된 UDP 패킷 크기: ");
                   // Serial.println(packet.length());
                   // Serial.print("수신된 메시지: ");
                   // Serial.println((char *)packet.data());
                 });
  }
  else
  {
    Serial.println("#UDP start failed");
  }
}

void loop()
{
  // put your main code here, to run repeatedly:

  g_runner.execute();
}