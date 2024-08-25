using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System.Threading;
using System;

using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;

// TextMeshPro 네임스페이스 추가
using TMPro;
using Unity.VisualScripting;
using UnityEngine.UI;



public class commandSample : MonoBehaviour
{
    private UDPCmdReceiver udpCmdReceiver;
    private CancellationTokenSource cts;

    [SerializeField] private Button button;
    [SerializeField] private TMP_InputField remoteIpInputField;
    [SerializeField] private TMP_InputField remotePortInputField;
    [SerializeField] private TMP_Text textBattery;

    // Start is called before the first frame update
    void Start()
    {
        udpCmdReceiver = new UDPCmdReceiver(7204); // Use the same port as in your Arduino code
        cts = new CancellationTokenSource();
        ReceivePacketsAsync(cts.Token);

        button.onClick.AddListener(() => {
            Debug.Log("Button Clicked");

            string remoteIp = remoteIpInputField.text;
            int remotePort = int.Parse(remotePortInputField.text);

            IPEndPoint remoteEndPoint = new IPEndPoint(IPAddress.Parse(remoteIp), remotePort);

            udpCmdReceiver.SendPacket("battery", remoteEndPoint);

            // udpCmdReceiver.SendPacket("battery");
        });
        
    }

    async void ReceivePacketsAsync(CancellationToken cancellationToken)
    {
        while (!cancellationToken.IsCancellationRequested)
        {
            try
            {
                // Receive a packet text command ex> 
                var (data, endPoint) = await udpCmdReceiver.ReceivePacketAsync();
                Debug.Log(data);

                //ex> #BC_BOLT-F81CB2B0

                // _ or \n 으로 구분하여 처리
                string[] tokens = data.Split(new char[] { '_', '\n' }, StringSplitOptions.RemoveEmptyEntries);

                if(tokens.Length < 2) {
                    Debug.Log("Invalid packet: " + data);
                    continue;
                }
                else {

                    String typeCmd = tokens[0];

                    if(typeCmd == "#BC") {
                        string device_id = tokens[1];

                        if(device_id.StartsWith("BSQ")) {
                            remoteIpInputField.text = endPoint.Address.ToString();
                            remotePortInputField.text = endPoint.Port.ToString();
                        }


                        Debug.Log("device_id: " + device_id + "from " + endPoint.Address.ToString() + " : " + endPoint.Port);
                    }
                    else if(typeCmd == "#RES") {
                        string resultType = tokens[1];

                        if(resultType == "bat") {
                            string result = tokens[2];
                            Debug.Log("Battery: " + result);
                            // textBattery.text = result;
                            //percentages
                            float battery = float.Parse(result);
                            battery = battery / 4.2f * 100.0f; // we use 4.2 volt cell
                            textBattery.text = battery.ToString("F2") + "%";


                        }

                    }

                }
                
                // Do something with the packet
            }
            catch (OperationCanceledException)
            {
                // Handle the exception when the operation is cancelled
                Debug.LogWarning("UDP Receive operation was cancelled.");
            }
            catch (ObjectDisposedException)
            {
                // Handle the exception when the UdpClient is disposed
                Debug.LogWarning("UDP Client has been disposed.");
            }
            catch (SocketException e)
            {
                // Handle socket exceptions here
                Debug.LogError($"Socket Exception: {e.Message}");
            }
            catch (Exception e)
            {
                // Handle other exceptions here
                Debug.LogError($"An error occurred: {e.Message}");
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
        udpCmdReceiver.Dispose();
    }

    
}
