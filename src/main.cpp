#include <Arduino.h> // Arduino 헤더 파일 포함
#include <TaskScheduler.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include <tonkey.hpp>

#include "config.hpp"
#include "imu.hpp"

#include "SparkFun_BNO08x_Arduino_Library.h" // CTRL+Click here to get the library: http://librarymanager/All#SparkFun_BNO08x

Scheduler g_runner;
tonkey g_MainParser;
CCongifData configData;

void (*updateData)() = nullptr;

String processCommand(String _strLine)
{
  String _strResult = "";
  g_MainParser.parse(_strLine);

  if (g_MainParser.getTokenCount() > 0)
  {
    String cmd = g_MainParser.getToken(0);

    if (cmd == "help")
    {
      _strResult = "help: show help\n";
      _strResult += "reboot: reboot device\n";
    }
    else if (cmd == "reboot")
    {
      ESP.restart();
    }
    else if (cmd == "qt")
    {

      BNO080_IMU::quaternion::setReports();

      delay(300);

      updateData = BNO080_IMU::quaternion::update;

      // g_runner.addTask(BNO080_IMU::quaternion::task);

      _strResult = "Quaternion set\n";
    }
    else if (cmd == "la")
    {
      BNO080_IMU::linear_acceleration::setReports();

      delay(300);

      updateData = BNO080_IMU::linear_acceleration::update;

      _strResult = "Linear acceleration set\n";
    }
    else if(cmd == "trace")
    {
      BNO080_IMU::trace::setReports();

      delay(300);

      updateData = BNO080_IMU::trace::update;

      _strResult = "Trace set\n";
    }
    else if(cmd == "euler")
    {
      BNO080_IMU::euler::setReports();

      delay(300);

      updateData = BNO080_IMU::euler::update;

      _strResult = "Euler set\n";
    }
    else
    {
      _strResult = "Invalid command\n";
    }
  }
  else
  {
    _strResult = "Invalid command\n";
  }

  return "#RES_" + _strResult + "\nOK\n";
}

Task task_Cmd(
    100, TASK_FOREVER, []()
    {
    if (Serial.available() > 0)
    {
        String _strLine = Serial.readStringUntil('\n');
        _strLine.trim();
        Serial.println(_strLine);
        String _strResult = processCommand(_strLine);
        Serial.print(_strResult);
    } });

void setup()
{

  Serial.begin(115200);
  // delay(100);
  // Serial.println("BNO080 IMU Test");

  delay(300);

  while (1)
  {
    if (!BNO080_IMU::begin())
    {
      Serial.println("BNO080 not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
      // while(1);
    }
    else
    {
      Serial.println("BNO080 detected.");
      break;
    }
    Serial.println("Retrying...");
    delay(300);
  }

  // BNO080_IMU::quaternion::setReports();
  // Serial.println("Reading events");
  delay(100);

  configData.load();

  g_runner.init();
  g_runner.addTask(task_Cmd);
  task_Cmd.enable();
}

void loop()
{
  // BNO080_IMU::quaternion::update();

  if (updateData != nullptr)
  {
    updateData();
  }

  g_runner.execute();
}