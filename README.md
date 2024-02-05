# MOAI for mp6050 with motionpps2.0 library (DMP)

<img src="Moai.jpg"
width="400" height="400">
>


## Introduction

esp32 보드를 활용하여 imu 센서를 다루기위한 펌웨어입니다. udp 통신을 통해 imu 센서의 데이터를 전송합니다.  
보드에대한 설정과 제어는 Serial 을 통하여 커멘트를 입력하여 처리할수 있습니다.  

## pin layout

esp32-c3 보드의 핀 배치는 다음과 같습니다.  
```c
const int ledPins[] = {D10, D9};
const int analogPins[] = {D0, D1};
const int buttonPins[] = {D8, D2};
```
digital ouput 은 D10, D9 핀을 사용합니다.  
analog input 은 D0, D1 핀을 사용합니다.  
button input 은 D8, D2 핀을 사용합니다.  
6050 용 interrupt 핀은 D3 핀을 사용합니다.  


## Command

명령어 형식은 다음과 같습니다.  
```css
<command> <option> <value>
```

**help**

설명: 사용 가능한 모든 명령어와 기능을 나열합니다.
사용 예: help

**led**

설명: 지정된 LED를 제어합니다.  
다음과 같은 보조 명령어를 사용할 수 있습니다.  
on - LED를 켭니다.  
off - LED를 끕니다.  
toggle - LED의 상태를 토글합니다.  
pwm - LED의 밝기를 PWM으로 조절합니다 (0-255 범위).  

사용 예:  
led on 0 - 첫 번째 LED를 켭니다.
led off 1 - 두 번째 LED를 끕니다.
led toggle 0 - 첫 번째 LED의 상태를 토글합니다.
led pwm 1 128 - 두 번째 LED의 밝기를 PWM으로 조절합니다 (0-255 범위).

**button**

설명: 지정된 버튼의 상태를 읽습니다.  
사용 예: button 0 - 첫 번째 버튼의 상태를 읽습니다.

**analog**

설명: 지정된 아날로그 핀에서 값을 읽습니다.  
사용 예: analog 0 - 첫 번째 아날로그 핀의 값을 읽습니다.

**config**

설명: 구성 설정을 보여주거나 수정합니다.  
사용 예: config mNumber 123 - 구성 설정 mNumber를 123으로 설정합니다.
config target 192.168.4.48 9250

**save**

설명: 현재 구성을 저장합니다.  


**load**

설명: 저장된 구성을 불러옵니다.

**clear**

설명: 현재 구성을 초기화합니다.
사용 예: clear

**reboot**

설명: ESP32를 재부팅합니다.
사용 예: reboot

**print**

설명: 현재 구성을 출력합니다.
사용 예: print

**imu**

설명: IMU 관련 명령어를 처리합니다.  
다음과 같은 보조 명령어를 사용할 수 있습니다.  
start - IMU를 시작합니다.  
stop - IMU를 중지합니다.  
zero - IMU 오프셋을 조정합니다.  
offset - 현재 IMU 오프셋 값을 보여줍니다.  
saveOffset - 현재 IMU 오프셋 값을 저장합니다.  
verbose - 자세한 IMU 데이터 출력을 토글합니다.  
status - 현재 IMU 데이터를 보여줍니다.  

사용 예:
imu start - IMU를 시작합니다.  
imu stop - IMU를 중지합니다.  
imu zero - IMU 오프셋을 조정합니다.  
imu offset - 현재 IMU 오프셋 값을 보여줍니다.  
imu saveOffset - 현재 IMU 오프셋 값을 저장합니다.  
imu verbose - 자세한 IMU 데이터 출력을 토글합니다.  
imu status - 현재 IMU 데이터를 보여줍니다.  

**wifi**

wifi 관련 명령어를 처리합니다.  

scan - 주변 wifi를 검색합니다.  

connect - ssid , password 를 입력하여 wifi에 연결합니다.  
ex> wifi connect ssid password  

disconnect - station 모드에서 연결을 해제합니다.  
status - 현재 wifi 상태를 보여줍니다.  
ip - 현재 ip 주소를 보여줍니다.  
mac - 현재 mac 주소를 보여줍니다.  
rssi - 현재 rssi 값을 보여줍니다.  
dns - 현재 dns 주소를 보여줍니다.  

start_broadcast - udp broadcast 를 시작합니다. 7204 포트로 데이터를 전송합니다.    
stop_broadcast - udp broadcast 를 중지합니다.  

send - udp broadcast 를 통해 데이터를 전송합니다.  
ex> wifi send server_ip server_port data








