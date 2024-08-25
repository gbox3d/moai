using System.Collections;
using System.Collections.Generic;
using UnityEngine;


using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;

using System.Threading;
using System;

// TextMeshPro 네임스페이스 추가
using TMPro;

using UnityEngine.UI;

public class packetMonitor : MonoBehaviour
{
    private UDPReceiver udpReceiver;
    private CancellationTokenSource cts;
    private float lastUpdateTime;

    [SerializeField] private TextMeshProUGUI textNetWorkFps;
    [SerializeField] private TextMeshProUGUI textQuaternionValues;

    [SerializeField] private TextMeshProUGUI textAccelValues;
    [SerializeField] private TextMeshProUGUI textGyroValues;
    [SerializeField] private TextMeshProUGUI textMagValues;
    [SerializeField] private TextMeshProUGUI textBattery;
    [SerializeField] private TextMeshProUGUI textFireCount;
    [SerializeField] private TextMeshProUGUI textDevId;
    [SerializeField] private TextMeshProUGUI textYawPitchRoll;


    // Start is called before the first frame update
    void Start()
    {
        udpReceiver = new UDPReceiver(9250); // Use the same port as in your Arduino code
        cts = new CancellationTokenSource();
        ReceivePacketsAsync(cts.Token);
    }

    async void ReceivePacketsAsync(CancellationToken cancellationToken)
    {
        while (!cancellationToken.IsCancellationRequested)
        {
            try
            {
                // Calculate the time delta
                float deltaTime = Time.time - lastUpdateTime;
                lastUpdateTime = Time.time;

                // Receive a packet
                S_Udp_IMU_RawData_Packet packet = await udpReceiver.ReceivePacketAsync();
                
                textNetWorkFps.text = (1.0f / deltaTime).ToString("F2");

                //소숫점 3자리까지만 표시
                textQuaternionValues.text = string.Format("qW: {0:F3}, qX: {1:F3}, qY: {2:F3}, qZ: {3:F3}", packet.qW, packet.qX, packet.qY, packet.qZ);
                textAccelValues.text = string.Format("AccelX: {0:F3}, AccelY: {1:F3}, AccelZ: {2:F3}", packet.aX, packet.aY, packet.aZ);
                textGyroValues.text = string.Format("GyroX: {0:F3}, GyroY: {1:F3}, GyroZ: {2:F3}", packet.gX, packet.gY, packet.gZ);
                textMagValues.text = string.Format("MagX: {0:F3}, MagY: {1:F3}, MagZ: {2:F3}", packet.mX, packet.mY, packet.mZ);
                textBattery.text = string.Format("{0:F3}", packet.battery);
                textFireCount.text = string.Format("{0}", packet.fire_count);
                textDevId.text = string.Format("{0}", packet.dev_id);
                textYawPitchRoll.text = string.Format("Yaw: {0:F3}, Pitch: {1:F3}, Roll: {2:F3}", packet.yaw, packet.pitch, packet.roll);

                
                
            }
            catch (OperationCanceledException)
            {
                // Handle cancellation
                break;
            }
            catch (Exception ex)
            {
                // Handle other exceptions
                Debug.LogError(ex);
            }
        }
        
    }

    // Update is called once per frame
    void Update()
    {

    }

    void OnDestroy()
    {
        cts.Cancel();
    }

}
