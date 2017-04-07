/*
*  History				:
*2016.05.01  添加特殊测试指令，裸数据发送、指定时间片掉电、返回与卡片交互时间功能，指令与卡片无交互，只与读卡器交互，读数据调用UsbRead_Test函数
*2016.05.10  修改UsbRead_14443函数中接收时间循环，以接收支持"e080"的读卡器返回大于一个包的数据
*2016.05.12  修改select指令，调用时需要将UID作为数组指针传入函数进行处理，以达到选卡的功能
*2016.05.12  修正hy_pro_command函数错误，该错误导致没有通过调用hy_pro_reset函数使卡片进入cpu模式而是直接发送e080，使得读卡器没有将卡片分组号设置缺省状态。
*2016.05.12  修正hy_pro_command函数中，关于读卡器未响应时的处理。
*2016.05.19  将 - 4协议中的分组号重置，在hy_reset函数中实现。
*2016.05.24  修正COS特殊测试指令(掉电\执行时间）中返回数据缓存区设置过小导致堆栈数组越界问题。
*2016.06.29  添加COS测试接触式卡片测试协议指令。
*2016.07.15  为满足HYReader上位机软件的工作，建立hy_select_HYReader函数，此函数不需要将UID作为实参传入函数中。
*2016.07.19  hy_pro_command协议函数中, 将NAD功能暂时取消，全面修正程序循环机制。
*/

#ifndef __STDAFX_H__
	#include "StdAfx.h"
#endif

#include <wtypes.h>
#include <initguid.h>
#define MAX_LOADSTRING 256

extern "C" {

// This file is in the Windows DDK available from Microsoft.
#include "hidsdi.h"

#include <setupapi.h>
#include <dbt.h>

}

// This file is in the Windows DDK available from Microsoft.


DWORD								ActualBytesRead; 
DWORD								BytesRead;
HIDP_CAPS							Capabilities;
DWORD								cbBytesRead;
PSP_DEVICE_INTERFACE_DETAIL_DATA	detailData;
HANDLE								DeviceHandle;
DWORD								dwError;
char								FeatureReport[1024];
HANDLE								hEventObject;
HANDLE								hDevInfo;
GUID								HidGuid;
OVERLAPPED							HIDOverlapped;
char								InputReport[1024];
ULONG								Length;
LPOVERLAPPED						lpOverLap;
bool								MyDeviceDetected = FALSE; 
CString								MyDevicePathName;
DWORD								NumberOfBytesRead;
char								OutputReport[1024];
HANDLE								ReadHandle;
DWORD								ReportType;
ULONG								Required;
CString								ValueToDisplay;
HANDLE								WriteHandle;
DWORD								counts;
int									report;
bool								MyComDevice = FALSE; 

BOOL m_bDeviceNoficationRegistered;
BOOL Card_state;



//These are the vendor and product IDs to look for.
//Uses Lakeview Research's Vendor ID.
int VendorID = 0x04B4;
int ProductID = 0xA112;

