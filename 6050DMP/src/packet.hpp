#ifndef PACKET_HPP
#define PACKET_HPP


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


#endif // PACKET_HPP