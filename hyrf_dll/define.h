/*
*  History				:
*2016.05.01  ����������ָ������ݷ��͡�ָ��ʱ��Ƭ���硢�����뿨Ƭ����ʱ�书�ܣ�ָ���뿨Ƭ�޽�����ֻ������������������ݵ���UsbRead_Test����
*2016.05.10  �޸�UsbRead_14443�����н���ʱ��ѭ�����Խ���֧��"e080"�Ķ��������ش���һ����������
*2016.05.12  �޸�selectָ�����ʱ��Ҫ��UID��Ϊ����ָ�봫�뺯�����д����Դﵽѡ���Ĺ���
*2016.05.12  ����hy_pro_command�������󣬸ô�����û��ͨ������hy_pro_reset����ʹ��Ƭ����cpuģʽ����ֱ�ӷ���e080��ʹ�ö�����û�н���Ƭ���������ȱʡ״̬��
*2016.05.12  ����hy_pro_command�����У����ڶ�����δ��Ӧʱ�Ĵ���
*2016.05.19  �� - 4Э���еķ�������ã���hy_reset������ʵ�֡�
*2016.05.24  ����COS�������ָ��(����\ִ��ʱ�䣩�з������ݻ��������ù�С���¶�ջ����Խ�����⡣
*2016.06.29  ���COS���ԽӴ�ʽ��Ƭ����Э��ָ�
*2016.07.15  Ϊ����HYReader��λ������Ĺ���������hy_select_HYReader�������˺�������Ҫ��UID��Ϊʵ�δ��뺯���С�
*2016.07.19  hy_pro_commandЭ�麯����, ��NAD������ʱȡ����ȫ����������ѭ�����ơ�
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

