using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System.Threading.Tasks;

using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using UnityEditor.Experimental.GraphView;
using UnityEditor.PackageManager;

using System.IO;
using System.IO.Compression;


using System.Linq;
using System.Runtime.InteropServices;
[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct Header
{
    public uint MagicNumber;
    public byte Command;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public byte[] Padding;
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct S_Udp_IMU_RawData_Packet
{
    public Header header;

    public float aX;
    public float aY;
    public float aZ;

    public float gX;
    public float gY;
    public float gZ;

    public float mX;
    public float mY;
    public float mZ;

    public float extra;
    public float battery;

    public float pitch;
    public float roll;
    public float yaw;

    public ushort dev_id;
    public ushort fire_count;

    public float qW;
    public float qX;
    public float qY;
    public float qZ;
}

public static class TaskExtensions
{
    public static async Task<T> WithCancellation<T>(this Task<T> task, CancellationToken cancellationToken)
    {
        var tcs = new TaskCompletionSource<bool>();
        using (cancellationToken.Register(
                    s => ((TaskCompletionSource<bool>)s).TrySetResult(true), tcs))
        {
            if (task != await Task.WhenAny(task, tcs.Task))
            {
                throw new OperationCanceledException(cancellationToken);
            }
        }

        return await task;
    }
}


public class UDPReceiver : IDisposable
{
    private UdpClient udpClient;
    private IPEndPoint endPoint;
    private bool isReceiving = false;
    private CancellationTokenSource cancellationTokenSource;

    public UDPReceiver(int port)
    {
        udpClient = new UdpClient(port);
        endPoint = new IPEndPoint(IPAddress.Any, port);
        cancellationTokenSource = new CancellationTokenSource();
    }

    public async Task<S_Udp_IMU_RawData_Packet> ReceivePacketAsync()
    {
        S_Udp_IMU_RawData_Packet packet = default;

        try
        {
            isReceiving = true;
            var result = await udpClient.ReceiveAsync().WithCancellation(cancellationTokenSource.Token);
            byte[] receivedBytes = result.Buffer;

            // Assuming S_Udp_IMU_RawData_Packet is defined and has the same layout as in your Arduino code
            packet = PacketUtilityClass.FromByteArray<S_Udp_IMU_RawData_Packet>(receivedBytes);
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
        finally
        {
            isReceiving = false;
        }

        return packet;
    }

    public void CancelReceiving()
    {
        cancellationTokenSource.Cancel();
    }

    public void Dispose()
    {
        if (!isReceiving)
        {
            udpClient.Close();
        }
        cancellationTokenSource.Dispose();
    }
}

public static class PacketUtilityClass
{

    public static uint muCheckCode = 20230903;


    public static byte[] ToByteArray<T>(T structure) where T : struct
    {
        int size = Marshal.SizeOf(typeof(T));
        byte[] arr = new byte[size];
        IntPtr ptr = Marshal.AllocHGlobal(size);

        Marshal.StructureToPtr(structure, ptr, true);
        Marshal.Copy(ptr, arr, 0, size);
        Marshal.FreeHGlobal(ptr);

        return arr;
    }

    //바이트 배열을 해당형식의 구조체로 변환
    //사용예 > PacketReqColor packet = FromByteArray<PacketReqColor>(byteArray);
    public static T FromByteArray<T>(byte[] byteArray) where T : struct
    {
        GCHandle handle = GCHandle.Alloc(byteArray, GCHandleType.Pinned);
        T packet = (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(T));
        handle.Free();
        return packet;
    }

    public static byte[] DecompressGZip(byte[] compressedData)
    {
        using (MemoryStream compressedStream = new MemoryStream(compressedData))
        using (GZipStream gzipStream = new GZipStream(compressedStream, CompressionMode.Decompress))
        using (MemoryStream decompressedStream = new MemoryStream())
        {
            gzipStream.CopyTo(decompressedStream);
            byte[] decompressedBytes = decompressedStream.ToArray();
            return decompressedBytes;
        }
    }
}

