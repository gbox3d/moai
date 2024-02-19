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

public class imuReciever_Quaternion : MonoBehaviour
{
    private UDPReceiver udpReceiver;
    private CancellationTokenSource cts;
    private float lastUpdateTime;

    [SerializeField] private GameObject imuObject_Ypr;
    [SerializeField] private GameObject imuObject_Corrected; // 이 오브잭트에 맞추어 방향을 보정한다.

    //textui textmeshpro for acc x y z
    [SerializeField] private TextMeshProUGUI textFireCount;
    [SerializeField] private TextMeshProUGUI textNetWorkFps;
    
    [SerializeField] private Button btnReset;

    private Quaternion mDeltaRotation; // 회전을 보정하기 위한 값, 센서에서 들어온값에 곱해준다.
    private Quaternion mImuRotation;

    private int mPrevFireCount = 0;

    // Start is called before the first frame update
    void Start()
    {
        udpReceiver = new UDPReceiver(9250); // Use the same port as in your Arduino code
        cts = new CancellationTokenSource();
        ReceivePacketsAsync(cts.Token);

        mDeltaRotation = Quaternion.identity;

        btnReset.onClick.AddListener(()=> {

            //틀어진 값을 보정하기 위해 현재 imu의 rotation과 target rotation의 차이를 구한다.
            Quaternion target = imuObject_Corrected.transform.rotation;
            // target - current
            mDeltaRotation = target * Quaternion.Inverse(mImuRotation);
            mDeltaRotation.Normalize();
        });

    }

    async void ReceivePacketsAsync(CancellationToken cancellationToken)
    {
        Quaternion backup_imuObject_Ypr = imuObject_Ypr.transform.rotation;

        while (!cancellationToken.IsCancellationRequested)
        {
            try
            {
                // Calculate the time delta
                float deltaTime = Time.time - lastUpdateTime;
                lastUpdateTime = Time.time;

                // Receive a packet
                S_Udp_IMU_RawData_Packet packet = await udpReceiver.ReceivePacketAsync();
                
                // y,z 축이 서로 바뀌어 있고 x,y축의 방향이 역으로 되어있다.
                float qW = packet.qW;
                float qX = -packet.qX;
                float qY = -packet.qZ; // imu qY = unity qZ
                float qZ = packet.qY; // imu qZ = unity qY

                // Debug.Log("qW : " + qW + " qX : " + qX + " qY : " + qY + " qZ : " + qZ);

                mImuRotation = new Quaternion(qW, qX, qY, qZ); //imu sensor rotation
                
                imuObject_Ypr.transform.rotation =  mDeltaRotation * mImuRotation; //값을 보정한다.

                textFireCount.text = packet.fire_count.ToString();

                //탄수가 변하면 발사로 간주한다.
                if (mPrevFireCount != packet.fire_count)
                {
                    mPrevFireCount = packet.fire_count;
                    Debug.Log("fire count : " + packet.fire_count);
                }

                textNetWorkFps.text = (1.0f / deltaTime).ToString("F2");

                // Debug.Log("deltaTime : " + deltaTime + " fire count : " + packet.fire_count + " qW : " + qW + " qX : " + qX + " qY : " + qY + " qZ : " + qZ);
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
        imuObject_Ypr.transform.rotation = backup_imuObject_Ypr;
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
