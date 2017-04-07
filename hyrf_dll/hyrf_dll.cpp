
/****************************************************************************************************************************************************
*								(C) Copyright 1992-2016 HUA YI Microelectronics Techology Co.Ltd
*														All Rights Reserved
*
*  System Application Team
*
*  File name			:hyrf_dll.cpp
*  Author				:gHan
*  Version				:V1.0.0
*  Date					:2015.04.22
*  Description			:用于HY_Reader双界面读卡器接口函数动态链接库的操作函数
*  Interface			:USB_HID
*  Stand by				:ISO/IEC14443 IS0/IEC15693 ISO/IEC7816
****************************************************************************************************************************************************/


// hyrf_dll.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "hyrf_dll.h"
#include "define.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// Chyrf_dllApp

BEGIN_MESSAGE_MAP(Chyrf_dllApp, CWinApp)
END_MESSAGE_MAP()


// Chyrf_dllApp 构造

Chyrf_dllApp::Chyrf_dllApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中

}


// 唯一的一个 Chyrf_dllApp 对象

Chyrf_dllApp theApp;


// Chyrf_dllApp 初始化

BOOL Chyrf_dllApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}




unsigned char buffertem[1024] = { 0 };
int  gcount = 0, number_Send = 0, Iread = 0;
unsigned char resp_ATS[1024] = { 0 };
unsigned char SeeData_Send[1024] = { 0 };
unsigned char SeeData_Receive[1024] = { 0 };

BOOL Cos_cmdtime_state = true;
unsigned char Pro_cmd_buffer[3] = { 0 };
/***************************************************************************

    函数名称：CloseHandles()
    函数功能：在关闭设备时，关闭设备句柄，以免造成内存泄漏
    参数说明：无

****************************************************************************/
void CloseHandles()
{
	{
	//Close open handles.
	
	if (DeviceHandle != INVALID_HANDLE_VALUE)
		{
		CloseHandle(DeviceHandle);
		}

	if (ReadHandle != INVALID_HANDLE_VALUE)
		{
		CloseHandle(ReadHandle);
		}

	if (WriteHandle != INVALID_HANDLE_VALUE)
		{
		CloseHandle(WriteHandle);
		}
	}
}

/***************************************************************************

    函数名称：datadeal
    函数功能：供usb接收数据线程函数调用以处理返回的数据
    参数说明：char* pchar 
	          int count

****************************************************************************/
void datadeal(char* pchar,int count)                //数据处理
{
	if(count < 0) count = 0;
	memcpy(&buffertem[gcount],pchar+1,count);
	gcount+=count;
}

/***************************************************************************

    函数名称：listenDate
    函数功能：usb接收数据线程函数
    参数说明：LPVOID lpParamete  //线程参数

****************************************************************************/
DWORD WINAPI listenDate(LPVOID lpParamete)                //接受返回信息
{
	HANDLE hEvent = NULL;	
	while(1)
	{
		DWORD	Result= 0;
		NumberOfBytesRead = 0;
		InputReport[0]=0;
		char send_dat[1024]={0};
		hEvent = NULL;
		hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		HIDOverlapped.Internal     = 0;
		HIDOverlapped.InternalHigh = 0;
		HIDOverlapped.Offset       = Capabilities.InputReportByteLength;
		HIDOverlapped.Offset       = 0;
		HIDOverlapped.OffsetHigh   = 0;
		HIDOverlapped.hEvent       = hEvent;

		if (ReadHandle != INVALID_HANDLE_VALUE)
			{
			Result = ReadFile 
			(ReadHandle, 
			InputReport, 
			Capabilities.InputReportByteLength, 
			&NumberOfBytesRead,
			(LPOVERLAPPED) &HIDOverlapped); 
			}
		else
		{
			continue;
		}
		DWORD dwError = GetLastError();
	
		Result = WaitForSingleObject (hEvent,1000);
		
		switch (Result)
		{
		case WAIT_OBJECT_0:
			{		
				DWORD dresult = GetOverlappedResult(ReadHandle, &HIDOverlapped,&NumberOfBytesRead, FALSE);
				if(dresult)
				{
					memcpy(send_dat,InputReport,NumberOfBytesRead);
					datadeal(send_dat,NumberOfBytesRead-1);
					
					Result = CancelIo(ReadHandle);
					ResetEvent(hEvent);
					memset(InputReport,0,1024);
				}
				break;
			}
		case WAIT_TIMEOUT:
			{
				datadeal(NULL,0);
				ValueToDisplay.Format("%s", "ReadFile timeout");
				Result = CancelIo(ReadHandle);
				ResetEvent(hEvent);
				break;
			}
		default:
			{
				ValueToDisplay.Format("%s", "Undefined error");
				Result = CancelIo(ReadHandle);
				ResetEvent(hEvent);
				break;
			}
		}
		CloseHandle(hEvent);//创建hEvent线程句柄后需要关闭！
	}
	return 0;
}

/***************************************************************************

    函数名称：UsbWrite
    函数功能：向设备发送指定的数据
    参数说明：unsigned char* pchar  //需要发送的数据
              int number            //发送数据的字节数

****************************************************************************/

int WINAPI UsbWrite(BYTE* pchar,int number)        //发送的指令
{
	DWORD	BytesWritten = 0;
	INT		Index =0;
	ULONG	Result;
	unsigned char send_buffer[1024] = {0}; 
	int i;
	CString	strBytesWritten = "";
    
    send_buffer[0]=0x00;
	switch (number)
	{
	    case 1://reset
			{
				if(pchar[0] == 0xAA && pchar [1] == 0xBB && pchar[2] == 0xCC) send_buffer[1] = pchar[3];//发送自定义数据
				else
				{
					send_buffer[1]=pchar[0];
					send_buffer[2]=0x00;
					send_buffer[3]=number;
					send_buffer[4]=pchar[0];
				}
				break;
			}
		case 2:
			{
				if(pchar[0]==0x30||pchar[0]==0xb0||pchar[0]==0x60||pchar[0]==0x61)//ISO1443_READ/Transfer/Authen
				{
					send_buffer[1]=pchar[0];
					send_buffer[2]=0x00;
					send_buffer[3]=number;
					send_buffer[4]=pchar[0];
					send_buffer[5]=pchar[1];
				}
				else if(pchar[0] == 0xAA && pchar [1] == 0xBB && pchar[2] == 0xCC)//发送自定义数据
				{
					send_buffer[1] = pchar[3];
					send_buffer[2] = pchar[4];
				}
				else
				{
		           send_buffer[1]=pchar[0];
				   send_buffer[2]=pchar[1];
	               send_buffer[3]=number;
		           send_buffer[4]=pchar[0];
				   send_buffer[5]=pchar[1];
				}
			  break;
			}
		default :
			{
				if(pchar[0]==0x48)//ISO1443_DirectCmd
				{
					send_buffer[1]=0x48;
					send_buffer[2]=0x00;
					send_buffer[3]=number;
					for(i=0;i<number;i++)
						send_buffer[i+4]=pchar[i];
				}
				else if(pchar[0]==0xa2||pchar[0]==0x11||pchar[0]==0x12||pchar[0]==0xc1||pchar[0]==0xc0||pchar[0]==0xc2)//ISO1443_Write4B/SetKeyA,B/Decrease/Increase/Restore
				{
					send_buffer[1]=pchar[0];
					send_buffer[2]=0x00;
                    send_buffer[3]=number;
					for(i=0;i<number;i++)
		              send_buffer[i+4]=pchar[i];
				}
				else if(pchar[0]==0xa0)//ISO1443_Write16B
				{
					send_buffer[1]=pchar[0];
					send_buffer[2]=0x00;
                    send_buffer[3]=number;
					for(i=0;i<19;i++)
		              send_buffer[i+4]=pchar[i];
				}
				else if((pchar[0]==0x14&&pchar[1]==0x44)||(pchar[0]==0x15&&pchar[1]==0x69))//协议模式选择
				{
					send_buffer[1]=pchar[0];
                    send_buffer[2]=pchar[1];
	                send_buffer[3]=pchar[2];
				}
				else if(pchar[0]==0x16)//接触式IC卡命令函数
				{
					for(i=0;i<number;i++)
						send_buffer[i+1]=pchar[i];
				}
				else if (pchar[0] == 0xCE && pchar[1] == 0xDB)//cos特殊指令测试模式
				{
					for (int isend_cos = 0; isend_cos < number; isend_cos++)
					{
						send_buffer[isend_cos + 1] = pchar[isend_cos];
					}
				}
				else if(pchar[0] == 0xAA && pchar [1] == 0xBB && pchar[2] == 0xCC)//发送自定义数据
				{
					for(int m = 0;m<number;m++)
					{
						send_buffer[m+1] = pchar[m+3];
					}
				}
				else
				{
					if(pchar[0]==0x93&&pchar[1]==0x70)//ISO14443_Select
					{
						send_buffer[1]=pchar[0];
						send_buffer[2]=pchar[1];
	                    send_buffer[3]=number;
	                    for(i=0;i<number;i++)
		                   send_buffer[i+4]=pchar[i];
					}
					else
					{
					    send_buffer[1]=pchar[0];
                        send_buffer[2]=0x00;
	                    send_buffer[3]=number;
	                    for(i=0;i<number;i++)
		                   send_buffer[i+4]=pchar[i];
					}
				}
				break;
			}
    }
	memset(SeeData_Send,0,1024);

	for(i=0;i<number+3;i++)//显示发送数据
		SeeData_Send[i]=send_buffer[i+1];
	number_Send=number;

	if (MyDeviceDetected==FALSE) return IFD_UnConnected;//未连接读卡器

	unsigned char Send_buf[65] = {0};

	if(number>=62) //超长链接发送
	{
		memcpy(Send_buf,&send_buffer[0],65);
		report = 0 ;
		if (report)
		{		
			if (WriteHandle != INVALID_HANDLE_VALUE)
			{
				Result = HidD_SetOutputReport
					(WriteHandle,
					OutputReport,
					Capabilities.OutputReportByteLength);
			}
			if (!Result)
			{
				CloseHandles();
				MyDeviceDetected = FALSE;
			}
			
		}
		else
		{		
			if (WriteHandle != INVALID_HANDLE_VALUE)
			{
				Result = WriteFile 
					(WriteHandle, 
					Send_buf, 
					Capabilities.OutputReportByteLength,
					&BytesWritten, 
					NULL);
			}	
			if (!Result)
			{
				CancelIo(WriteHandle);
				int ierror = GetLastError();	
			}
		}
		memset(Send_buf,0,65);

		int num_N = (number-61)/64;

		if(num_N == 0)
		{
			memcpy(&Send_buf[1],&send_buffer[65],64);

			report = 0 ;
			if (report)
			{		
				if (WriteHandle != INVALID_HANDLE_VALUE)
				{
					Result = HidD_SetOutputReport
						(WriteHandle,
						OutputReport,
						Capabilities.OutputReportByteLength);
				}
				if (!Result)
				{
					CloseHandles();
					MyDeviceDetected = FALSE;
				}
				
			}
			else
			{		
				if (WriteHandle != INVALID_HANDLE_VALUE)
				{
					Result = WriteFile 
						(WriteHandle, 
						Send_buf, 
						Capabilities.OutputReportByteLength,
						&BytesWritten, 
						NULL);
				}	
				if (!Result)
				{
					CancelIo(WriteHandle);
					int ierror = GetLastError();	
				}
			}
			memset(Send_buf,0,65);
		}
		else
		{
			if(((number-61)%64) != 0) num_N=num_N+1;
			
			for(int num_1=0;num_1<num_N;num_1++)
			{
				memcpy(&Send_buf[1],&send_buffer[65+64*num_1],64);
				
				if (report)
				{		
					if (WriteHandle != INVALID_HANDLE_VALUE)
					{
						Result = HidD_SetOutputReport
							(WriteHandle,
							OutputReport,
							Capabilities.OutputReportByteLength);
					}
					if (!Result)
					{
						CloseHandles();
						MyDeviceDetected = FALSE;
					}
					
				}
				else
				{		
					if (WriteHandle != INVALID_HANDLE_VALUE)
					{
						Result = WriteFile 
							(WriteHandle, 
							Send_buf, 
							Capabilities.OutputReportByteLength,
							&BytesWritten, 
							NULL);
					}	
					if (!Result)
					{
						CancelIo(WriteHandle);
						int ierror = GetLastError();	
					}
				}
				memset(Send_buf,0,65);
			}
		}
	}
	else//正常数据单包发送
	{
		report = 0 ;
		if (report)
		{		
			if (WriteHandle != INVALID_HANDLE_VALUE)
			{
				Result = HidD_SetOutputReport
					(WriteHandle,
					OutputReport,
					Capabilities.OutputReportByteLength);
			}
			if (!Result)
			{
				CloseHandles();
				MyDeviceDetected = FALSE;
			}
			
		}
		else
		{		
			if (WriteHandle != INVALID_HANDLE_VALUE)
			{
				Result = WriteFile 
					(WriteHandle, 
					send_buffer, 
					Capabilities.OutputReportByteLength,
					&BytesWritten, 
					NULL);
			}	
			if (!Result)
			{
				CancelIo(WriteHandle);
				int ierror = GetLastError();	
			}
		}
		memset(send_buffer,0,65);
	}
	return IFD_ICC_OK;
}

/***************************************************************************

    函数名称：UsbRead_15693
    函数功能：接收设备返回数据(15693非接卡片)
    参数说明：unsigned char* pchar    //数据缓存区
		      unsigned long resp_time //设置等待延时

****************************************************************************/

int __stdcall UsbRead_15693(unsigned char* pchar,unsigned long resp_time)//接收15693的指令
{
	unsigned char sum=0,i,dat_15693;
	unsigned int j=0;
	
	if(resp_time <= 80)
	{
		while(1)
		{
			::Sleep(1);
			j++;
			if(buffertem[0]!=0) break;
			if(j==500) return IFD_ICC_NoResponse;
		}
	}
	else
	{
		::Sleep(resp_time);
	}

	if (gcount==0)
	{
		return IFD_ICC_NoResponse;//卡片无应答
	}

	CString str_gcount = _T("");
	str_gcount.Format("%d",gcount);
	if(gcount >= sizeof(pchar))
	{
		MessageBox(NULL,"接收缓存区小于返回长度("+str_gcount+")","hyrf_dll",MB_ICONEXCLAMATION);
	}

	memcpy_s(pchar,sizeof(pchar),buffertem,gcount);
    memcpy_s(SeeData_Receive,1024,pchar,gcount);
	memset(buffertem,0,gcount);		//清空数组 等下次接收使用

	if(pchar[2]==0x03&&pchar[3]==0xff)
	{	   
	    gcount = 0;
		Iread=3;
        return IFD_ICC_NoResponse;//卡片无应答
	}
	else if(pchar[2]==0x02&&pchar[3]==0x01)
	{	  
	    gcount = 0;
		Iread=2;
        return IFD_ICC_ERROR;//执行错误
	}
	else if(pchar[0]==0x00&&pchar[1]==0x00&&pchar[2]==0x00&&pchar[3]==0x00)
	{
	    gcount = 0;
		Iread=1;
        return IFD_ICC_ERROR;//执行错误（返回ERROR FLAG）
	}
	else
	{
		dat_15693=pchar[2];
		for(i=0;i<dat_15693;i++)
		{
			pchar[i]=pchar[i+3];
		}
		pchar[i]=0x00;
		pchar[i+1]=0x00;
		pchar[i+2]=0x00;
		pchar[i+3]=0x00;
	}
	gcount = 0;
	Iread=dat_15693;
	return dat_15693;
}

/***************************************************************************

    函数名称：UsbRead_14443(14443非接卡片)
    函数功能：接收设备返回的数据
    参数说明：unsigned char* pchar    //数据缓存区
              unsigned long resp_time //设置等待延时

****************************************************************************/
	
int __stdcall UsbRead_14443(unsigned char* pchar,unsigned long delay_ms)//接收14443/M1的指令
{
	int i = 0,dat = 0,j = 0;
	unsigned char read_buffer[1024] = {0};

	if(delay_ms <50)
	{
		while(1)
		{
			::Sleep(1);
			j++;
			if (buffertem[0] != 0 && ((buffertem[2]+2)/gcount) == 0 ) break;//2016.5.6 modify time for response_data by hanguang
			if(j==1000) break;
		}
	}
	else ::Sleep(delay_ms);// modify by ghan for delay 2016.8.30

	if (gcount == 0)return IFD_ICC_NoResponse;//卡片无应答

	memcpy_s(read_buffer,1024, buffertem, gcount);
    memcpy_s(SeeData_Receive,1024,read_buffer,gcount);
	memset(buffertem,0,1024);		//清空数组 等下次接收使用
  
	if(read_buffer[2]==0x01)
	{	   
		dat=read_buffer[2];
		for (i = 0; i < dat; i++)
		{
			pchar[i] = read_buffer[i + 3];
		}
		pchar[i]=0x00;
		pchar[i+1]=0x00;
		pchar[i+2]=0x00;
		pchar[i+3]=0x00;
	}
	else if(read_buffer[2]>0x01)
	{
		if(read_buffer[0]==0x48&&read_buffer[1]==0x00)//ISO14443_DirectCmd
		{
			if(read_buffer[2] == 0x03 && read_buffer[3]==0xff)
			{
				gcount = 0;
				return IFD_ICC_ERROR;//执行错误
			}
			else
			{
				dat = read_buffer[2] - 1;
				for (i = 0; i<dat; i++)
					pchar[i] = read_buffer[i + 4];
			}
		}
		else if(read_buffer[0]==0xa2||read_buffer[0]==0xa0)//ISO1443_Write4/16
		{
			if(read_buffer[4]==0xfe)
			{
				gcount = 0;
				return IFD_ICC_ERROR;//执行错误
			}
			dat=read_buffer[2];
			for(i=0;i<dat;i++) pchar[i]=read_buffer[i+3];
			pchar[i]=0x00;
			pchar[i+1]=0x00;
			pchar[i+2]=0x00;
			pchar[i+3]=0x00;
		}
		else if(read_buffer[0]==0xC0||read_buffer[0]==0xC1||read_buffer[0]==0xC2)//ISO1443_Decrease/Increase/Restore
		{
			if(read_buffer[3]==0x0a&&read_buffer[4]==0xff)
			{
				gcount = 0;
				return 100;
			}
			else if(read_buffer[3]==0x0a&&read_buffer[4]!=0xff)
			{
				gcount = 0;
				return 101;
			}
			else
			{
				gcount = 0;
				return 102;
			}
		}
		else
		{
			dat=read_buffer[2];
			for(i=0;i<dat;i++)
				pchar[i]=read_buffer[i+3];
			pchar[i]=0x00;
			pchar[i+1]=0x00;
			pchar[i+2]=0x00;
			pchar[i+3]=0x00;
		}
	}
	else
	{
		gcount = 0;
		return IFD_ICC_CheckSumError;//信息校验和出错
	}
	gcount = 0;
	Iread=dat;
	return dat;
}

/***************************************************************************

    函数名称：UsbRead_Test
    函数功能：接收设备返回数据(测试专用)
    参数说明：unsigned char* pchar    //数据缓存区
		      unsigned long resp_time //设置等待延时

****************************************************************************/
int WINAPI UsbRead_Test(BYTE* pchar,ULONG resp_time)//测试专用
{
	unsigned char dat = 0;
	unsigned int loop_num=0;

	if(resp_time < 60)
	{
		while(1)
		{
			::Sleep(1);
			loop_num++;
			if(buffertem[0]!=0) break;
			if(loop_num==500)break;
		}
	}
	else ::Sleep(resp_time);//延迟resp_time ms等所有的数据都存储到buffertem数组中
 
	if (gcount==0) return IFD_ICC_NoResponse;//卡片无应答

	memcpy(pchar,buffertem,gcount);
	memset(buffertem,0,1024);		//清空数组 等下次接收使用

	if ((pchar[0] == 0xCE || pchar[0] == 0xAE) && pchar[1] == 0xDB)//读卡器进行COS测试模式
	{
		int cos_cmd_i = 0;

		switch (pchar[2])
		{
		case 0x00://指令执行时间显示
		{
			dat = pchar[3];

			if (pchar[dat + 2] == 0x90 && pchar[dat + 3] == 0x00)
			{
				for (cos_cmd_i = 0; cos_cmd_i < dat; cos_cmd_i++)
					pchar[cos_cmd_i] = pchar[cos_cmd_i + 4];
				pchar[cos_cmd_i] = 0x00;
				pchar[cos_cmd_i + 1] = 0x00;
				pchar[cos_cmd_i + 2] = 0x00;
				pchar[cos_cmd_i + 3] = 0x00;
			}
			else
			{
				gcount = 0;
				return IFD_Dev_CmdError;//设备执行错误
			}
			break;
		}
		case 0x01://掉电测试
		{
			dat = pchar[3];

			if (pchar[4] == 0x90 && pchar[5] == 0x00)
			{
				for (cos_cmd_i = 0; cos_cmd_i < 2; cos_cmd_i++)
					pchar[cos_cmd_i] = pchar[cos_cmd_i + 4];
				pchar[cos_cmd_i] = 0x00;
				pchar[cos_cmd_i + 1] = 0x00;
				pchar[cos_cmd_i + 2] = 0x00;
				pchar[cos_cmd_i + 3] = 0x00;
			}
			else
			{
				gcount = 0;
				return IFD_Dev_CmdError;//设备执行错误
			}
			break;
		}
		case 0x02://RF协议测试指令
		{
			dat = pchar[3];

			if (pchar[4] == 0x90 && pchar[5] == 0x00)
			{
				for (cos_cmd_i = 0; cos_cmd_i < 2; cos_cmd_i++)
					pchar[cos_cmd_i] = pchar[cos_cmd_i + 4];
				pchar[cos_cmd_i] = 0x00;
				pchar[cos_cmd_i + 1] = 0x00;
				pchar[cos_cmd_i + 2] = 0x00;
				pchar[cos_cmd_i + 3] = 0x00;
			}
			else
			{
				gcount = 0;
				return IFD_Dev_CmdError;//设备执行错误
			}
			break;
		}
		}

	}
	else
	{
		int i = 0;
		dat = pchar[2];
		for (i = 0; i < dat; i++)
			pchar[i] = pchar[i + 3];
		pchar[i] = 0x00;
		pchar[i + 1] = 0x00;
		pchar[i + 2] = 0x00;
		pchar[i + 3] = 0x00;
	}

	gcount = 0;
	return dat;
}

/***************************************************************************

    函数名称：UsbRead_uni
    函数功能：接收设备返回数据(特殊数据即无协议数据接收)
    参数说明：unsigned char* pchar    //数据缓存区

****************************************************************************/
int __stdcall UsbRead_uni(unsigned char* pchar)           //接收通用函数的指令
{
	int i = 0;
	unsigned int j=0;

	while(1)
	{
		::Sleep(1);
		j++;
		if(buffertem[0]!=0) break;
		if(j==200)break;
	}

	if (gcount==0) return IFD_ICC_NoResponse;//卡片无应答

	memcpy(pchar,buffertem,gcount);
	memset(buffertem,0,1024);		//清空数组 等下次接收使用


	for(i=0;i<2;i++)
		pchar[i]=pchar[i];
	pchar[i]=0x00;
	pchar[i+1]=0x00;
	pchar[i+2]=0x00;
	pchar[i+3]=0x00;

	gcount = 0;
	return 2;
}

/***************************************************************************

    函数名称：UsbRead_tch
    函数功能：接收设备返回数据(支持7816接触式协议)
    参数说明：unsigned char* pchar    //数据缓存区

****************************************************************************/
int __stdcall UsbRead_tch(unsigned char* pchar)//接收接触式IC卡命令函数的指令
{
	unsigned char pchar_recv[1024] = {0};
	int dat = 0;
	unsigned int j = 0;
	while (1)
	{
		::Sleep(1);
		j++;
		if (buffertem[0] != 0 && ((buffertem[2] + 2) / gcount) == 0) break;
		if (j == 1000) break;
	}
 
	if (gcount==0) return IFD_ICC_NoResponse;//卡片无应答
	memcpy(pchar_recv,buffertem,gcount);

	dat=pchar_recv[1];
	for(int i=0;i<dat;i++)
	{
		pchar[i]=pchar_recv[i];
	}

	memset(buffertem,0,gcount);//清空数组 等下次接收使用		
	memset(pchar_recv,0,1024);
	gcount = 0;
	return dat;
}

/***************************************************************************

    函数名称：hy_Reader_Open
    函数功能：该函数通知终端操作系统打开与读卡器所对应的终端设备端口，以便两者建立通信的逻辑关系。
    参数说明：HWND hwnd //传入窗口句柄
	返回值：  若正常，返回值为不小于0的设备句柄；反之返回值为状态码，其含义见返回值说明。
	注：      对16位Windows环境下运行的动态链接库、DOS环境下运行的静态函数库返回的设备句柄，
	          其含义均不同于32位Windows环境下动态链接库返回的设备句柄，仅为区分设备之用。

****************************************************************************/
long WINAPI hy_Reader_Open(HWND hwnd)              //打开设备
{

	//Use a series of API calls to find a HID with a specified Vendor IF and Product ID.
	HIDD_ATTRIBUTES						Attributes;
	DWORD								DeviceUsage;
	SP_DEVICE_INTERFACE_DATA			devInfoData;
	bool								LastDevice = FALSE;
	int									MemberIndex = 0;
	LONG								Result;	
	CString								UsageDescription;

	Length = 0;
	detailData = NULL;
	DeviceHandle=NULL;

	HidD_GetHidGuid(&HidGuid);	

	hDevInfo=SetupDiGetClassDevs 
		(&HidGuid, 
		NULL, 
		NULL, 
		DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);

	devInfoData.cbSize = sizeof(devInfoData);

	//Step through the available devices looking for the one we want. 
	//Quit on detecting the desired device or checking all available devices without success.

	MemberIndex = 0;
	LastDevice = FALSE;


	do
	{

		Result=SetupDiEnumDeviceInterfaces 
			(hDevInfo, 
			0, 
			&HidGuid, 
			MemberIndex, 
			&devInfoData);

		if (Result != 0)
		{
			//A device has been detected, so get more information about it.

			//Get the Length value.
			//The call will return with a "buffer too small" error which can be ignored.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				NULL, 
				0, 
				&Length, 
				NULL);

			//Allocate memory for the hDevInfo structure, using the returned Length.

			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);

			//Set cbSize in the detailData structure.

			detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			//Call the function again, this time passing it the returned buffer size.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				detailData, 
				Length, 
				&Required, 
				NULL);

			// Open a handle to the device.
			// To enable retrieving information about a system mouse or keyboard,
			// don't request Read or Write access for this handle.

			DeviceHandle=CreateFile 
				(detailData->DevicePath, 
				0, 
				FILE_SHARE_READ|FILE_SHARE_WRITE, 
				(LPSECURITY_ATTRIBUTES)NULL,
				OPEN_EXISTING, 
				0, 
				NULL);

			//Set the Size to the number of bytes in the structure.

			Attributes.Size = sizeof(Attributes);

			Result = HidD_GetAttributes 
				(DeviceHandle, 
				&Attributes);

			//Is it the desired device?

			MyDeviceDetected = FALSE;

			if(m_bDeviceNoficationRegistered == FALSE)
			{

				//Register to receive device notifications.

				DEV_BROADCAST_DEVICEINTERFACE DevBroadcastDeviceInterface;

				ZeroMemory(&DevBroadcastDeviceInterface,sizeof(DevBroadcastDeviceInterface));
				DevBroadcastDeviceInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
				DevBroadcastDeviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
				DevBroadcastDeviceInterface.dbcc_classguid = HidGuid;


				HDEVNOTIFY DeviceNotificationHandle = RegisterDeviceNotification(hwnd, &DevBroadcastDeviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

				m_bDeviceNoficationRegistered = TRUE;

			}

			if (Attributes.VendorID == VendorID)
			{
				if (Attributes.ProductID == ProductID)
				{
					//Both the Vendor ID and Product ID match.

					MyDeviceDetected = TRUE;
					MyDevicePathName = detailData->DevicePath;

					//Get the device's capablities.

					PHIDP_PREPARSED_DATA	PreparsedData;

					HidD_GetPreparsedData 
						(DeviceHandle, 
						&PreparsedData);


					HidP_GetCaps 
						(PreparsedData, 
						&Capabilities);



					HidD_FreePreparsedData(PreparsedData);

					DeviceUsage = (Capabilities.UsagePage * 256) + Capabilities.Usage;

					if (DeviceUsage == 0x102)
					{
						UsageDescription = "mouse";
					}

					if (DeviceUsage == 0x106)
					{
						UsageDescription = "keyboard";
					}

					// Get a handle for writing Output reports.

					WriteHandle=CreateFile 
						(detailData->DevicePath, 
						GENERIC_WRITE, 
						FILE_SHARE_READ|FILE_SHARE_WRITE, 
						(LPSECURITY_ATTRIBUTES)NULL,
						OPEN_EXISTING, 
						0, 
						NULL);

					// Prepare to read reports using Overlapped I/O.

					ReadHandle=CreateFile 
						(detailData->DevicePath, 
						GENERIC_READ, 
						FILE_SHARE_READ|FILE_SHARE_WRITE,
						(LPSECURITY_ATTRIBUTES)NULL, 
						OPEN_EXISTING, 
						FILE_FLAG_OVERLAPPED, 
						NULL);

				} //if (Attributes.ProductID == ProductID)

				else
					//The Product ID doesn't match.

					CloseHandle(DeviceHandle);

			} //if (Attributes.VendorID == VendorID)

			else
				//The Vendor ID doesn't match.

				CloseHandle(DeviceHandle);

			//Free the memory used by the detailData structure (no longer needed).

			free(detailData);

		}  //if (Result != 0)

		else
			//SetupDiEnumDeviceInterfaces returned 0, so there are no more devices to check.

			LastDevice=TRUE;

		MemberIndex++;

	} //do
	while ((LastDevice == FALSE) && (MyDeviceDetected == FALSE));

	SetupDiDestroyDeviceInfoList(hDevInfo);

	HANDLE  hThread  = CreateThread(NULL,NULL,listenDate,NULL,0,0);

	//CloseHandle(hThread);

	if (MyDeviceDetected == FALSE)
	{
		return IFD_USBError;	//USB连接错误
	}
	else return 110;

}
	
/***************************************************************************

    函数名称：hy_Reader_Close
    函数功能：关闭设备函数
    参数说明：long ReaderHandle //设备句柄

****************************************************************************/
long __stdcall hy_Reader_Close (long ReaderHandle)//关闭设备(USB/RS232)
{
	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110 )
		{
			CloseHandles();
            MyDeviceDetected = FALSE;
		    return IFD_ICC_OK;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接
}

/***************************************************************************

    函数名称：hy_fuc_14443()
    函数功能：使读写器进入ISO14443模式，必须在hy_Reader_Open之后调用
    参数说明：无

****************************************************************************/
long __stdcall hy_fuc_14443()
{
	int iwrite = 0;
	unsigned char buffer[1024] = {0x14,0x44,0x30};

	if(MyDeviceDetected == TRUE)
	{
		iwrite = UsbWrite(buffer,3);

		memset(buffer,0,32);	//清空数组内容

		if(iwrite == 0) return IFD_ICC_OK;
		else return IFD_SelModeError ;//选择模式错误
	}
	else return IFD_UnConnected;//未建立连接 
}
               
/***************************************************************************

    函数名称：hy_fuc_15693()
    函数功能：使读写器进入ISO14443模式，必须在hy_Reader_Open之后调用
    参数说明：无

****************************************************************************/
long __stdcall hy_fuc_15693()
{
	int iwrite = 0;
	unsigned char buffer[1024] = {0x15,0x69,0x30};
		      
	if(MyDeviceDetected == TRUE)
	{
		iwrite = UsbWrite(buffer,3);

		memset(buffer,0,32);	//清空数组内容

		if(iwrite == 0) return IFD_ICC_OK;
		else return IFD_SelModeError ;//选择模式错误
	}
	else return IFD_UnConnected;//未建立连接 
}

/***************************************************************************

函数名称：hy_fuc_7816()
函数功能：使读写器进入ISO7816模式，必须在hy_Reader_Open之后调用
参数说明：无

****************************************************************************/
long __stdcall hy_fuc_7816()
{
	int iwrite = 0;
	unsigned char buffer[1024] = { 0x16,0x04,0x0a,0x00,0x0e,0x0d };

	if (MyDeviceDetected == TRUE)
	{
		iwrite = UsbWrite(buffer, 6);

		memset(buffer, 0, 32);	//清空数组内容

		if (iwrite == 0) return IFD_ICC_OK;
		else return IFD_SelModeError;//选择模式错误
	}
	else return IFD_UnConnected;//未建立连接 
}



/**********设备操作函数**********/



/***************************************************************************

    函数名称：hy_beep
    函数功能：控制读写器蜂鸣时间
    参数说明：long ReaderHandle      //读写器返回句柄
	          unsigned int _Btime    //蜂鸣时间，单位ms

****************************************************************************/
BOOL __stdcall hy_beep(long ReaderHandle,unsigned int _Btime)
{
	unsigned char buffer[7] = {0};

	if(MyDeviceDetected == TRUE)
	{
		buffer[0] = 0xAA;
		buffer[1] = 0xBB;
		buffer[2] = 0xCC;
		buffer[3] = 0x0b;
		buffer[4] = 0x0e;
		buffer[5] = 0x0e;
		buffer[6] = _Btime;

		UsbWrite(buffer,4);

		return TRUE;

	}
	else return IFD_UnConnected;//未建立连接 
}

/***************************************************************************

    函数名称：hy_getver
    函数功能：获取读写器当前单片机软件版本
    参数说明：long ReaderHandle          //读写器返回句柄
	          unsigned char* Response    //读写器软件版本，例：V1.1即返回0x01,0x01(HEX)

****************************************************************************/
long __stdcall hy_getver(long ReaderHandle,unsigned char* Response)
{
	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			int iread = 0;
			unsigned char buffer[7] = {0};

			buffer[0] = 0xAA;
			buffer[1] = 0xBB;
			buffer[2] = 0xCC;
			buffer[3] = 0x0f;
			buffer[4] = 0x0f;
			buffer[5] = 0x0f;
			buffer[6] = 0x01;

			UsbWrite(buffer,4);

			iread=UsbRead_uni(Response);

			if(iread == 2) return iread;
			else return IFD_GetVerError;

		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

/****************************************************************************

    函数名称：hy_7816_cos_debug_cmd_time
    函数作用：用于公司内部芯片COS测试-测试执行时间
    参数说明：long ReaderHandle
              unsigned char* resp_cmd_time_buffer

****************************************************************************/

long WINAPI hy_7816_cos_debug_cmd_time(long ReaderHandle, BYTE* Recv_cmd_time_buffer)
{
	if (MyDeviceDetected == TRUE)
	{
		if (ReaderHandle == 110)
		{
			unsigned char cos_debug_buffer[4] = { 0xAE,0XDB,0x00,0x02 };
			unsigned char resp_buffer[64] = { 0 };
			long iread = 0;

			UsbWrite(cos_debug_buffer, 4);

			iread = UsbRead_Test(resp_buffer, 0);

			if (iread > 0)
			{
				memcpy(Recv_cmd_time_buffer, resp_buffer, iread);
			}
			return iread;//返回状态值
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

/****************************************************************************

    函数名称: hy_7816_cos_debug_poweroff
    函数作用：用于公司内部芯片COS测试-在规定时间内完成掉电功能
    参数说明：long ReaderHandle
              unsigned char* poweroff_ntime_buffer
              unsigned char* resp_cos_debug_buffer

****************************************************************************/
long WINAPI hy_7816_cos_debug_poweroff(long ReaderHandle, BYTE* Ntime_poweroff_buffer, BYTE* Recv_cos_debug_buffer)
{
	if (MyDeviceDetected == TRUE)
	{
		if (ReaderHandle == 110)
		{
			unsigned char send_cos_debug_buffer[1024] = { 0xAE,0XDB,0x01,0x02 };
			unsigned char resp_buffer[64] = { 0 };
			int iread = 0;

			send_cos_debug_buffer[4] = Ntime_poweroff_buffer[0];
			send_cos_debug_buffer[5] = Ntime_poweroff_buffer[1];

			UsbWrite(send_cos_debug_buffer, 6);

			iread = UsbRead_Test(resp_buffer, 0);

			if (iread > 0)
			{
				memcpy(Recv_cos_debug_buffer, resp_buffer, iread);
			}
			return iread;//返回状态值
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

/***************************************************************************

    函数名称：hy_7816_cos_debug_rftest_protocol
    函数作用：用于公司内部芯片COS测试-RF协议测试指令
    参数说明：long ReaderHandle
              unsigned char* rftest_enable_buffer
              unsigned char* resp_rftest_protocol_buffer

****************************************************************************/
/*long __stdcall hy_7816_cos_debug_rftest_protocol(long ReaderHandle, unsigned char* Enable_rftest_protocol_buffer, unsigned char* Recv_rftest_protocol_buffer)
{
	if (MyDeviceDetected == TRUE)
	{
		if (ReaderHandle == 110)
		{
			unsigned char send_cos_debug_buffer[4] = { 0xAE,0XDB,0x02 };
			unsigned char resp_buffer[32] = { 0 };
			int iread = 0;

			send_cos_debug_buffer[3] = Enable_rftest_protocol_buffer[0];

			memcpy(Recv_rftest_protocol_buffer, send_cos_debug_buffer, 4);

			return iread;//返回状态值
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}*/

/***************************************************************************

函数名称：hy_cos_debug_cmd_time
函数作用：用于公司内部芯片COS测试-测试执行时间
参数说明：long ReaderHandle
          unsigned char* resp_cmd_time_buffer 

****************************************************************************/

long WINAPI hy_cos_debug_cmd_time(long ReaderHandle, BYTE* Recv_cmd_time_buffer)
{
	if (MyDeviceDetected == TRUE)
	{
		if (ReaderHandle == 110)
		{
			
			/*if (Cos_cmdtime_state == true)
			{*/
				unsigned char cos_debug_buffer[4] = { 0xCE,0XDB,0x00,0x02 };
				unsigned char resp_buffer[64] = { 0 };
				long iread = 0;

				UsbWrite(cos_debug_buffer, 4);

				iread = UsbRead_Test(resp_buffer, 0);

				if (iread > 0)
				{
					memcpy(Recv_cmd_time_buffer, resp_buffer, iread);
				}
				return iread;//返回状态值
			/*}
			else
			{
				memcpy(Recv_cmd_time_buffer, Pro_cmd_buffer,3);
				return 3;
			}*/
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}
/***************************************************************************

函数名称：long __stdcall hy_cos_debug_poweroff
函数作用：用于公司内部芯片COS测试-在规定时间内完成掉电功能
参数说明：long ReaderHandle
          unsigned char* poweroff_ntime_buffer
		  unsigned char* resp_cos_debug_buffer

****************************************************************************/
long WINAPI hy_cos_debug_poweroff(long ReaderHandle, BYTE* Ntime_poweroff_buffer ,BYTE* Recv_cos_debug_buffer)
{
	if (MyDeviceDetected == TRUE)
	{
		if (ReaderHandle == 110)
		{
			unsigned char send_cos_debug_buffer[1024] = { 0xCE,0XDB,0x01,0x02};
			unsigned char resp_buffer[64] = { 0 }; 
			int iread = 0;

			send_cos_debug_buffer[4] = Ntime_poweroff_buffer[0];
			send_cos_debug_buffer[5] = Ntime_poweroff_buffer[1];

			UsbWrite(send_cos_debug_buffer, 6);

			iread = UsbRead_Test(resp_buffer, 0);

			if (iread > 0)
			{
				memcpy(Recv_cos_debug_buffer, resp_buffer, iread);
			}
			return iread;//返回状态值
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}
/********************************************************

函数名称：hy_cos_debug_rftest_protocol
函数作用：用于公司内部芯片COS测试-RF协议测试指令
参数说明：long ReaderHandle
          unsigned char* rftest_enable_buffer
		  unsigned char* resp_rftest_protocol_buffer

****************************************************************************/
/*long __stdcall hy_cos_debug_rftest_protocol(long ReaderHandle, unsigned char* Enable_rftest_protocol_buffer,unsigned char* Recv_rftest_protocol_buffer)
{
	if (MyDeviceDetected == TRUE)
	{
		if (ReaderHandle == 110)
		{
			unsigned char send_cos_debug_buffer[4] = { 0xCE,0XDB,0x02 };
			unsigned char resp_buffer[32] = { 0 };
			int iread = 0;

			send_cos_debug_buffer[3] = Enable_rftest_protocol_buffer[0];

			memcpy(Recv_rftest_protocol_buffer, send_cos_debug_buffer, 4);

			return iread;//返回状态值
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}*/

//--------------------------------------------------------------ISO14443指令部分-------------------------------------------------------


long WINAPI hy_request(long ReaderHandle,unsigned char* recv_buf,unsigned char* recv_lenth, unsigned long delay_ms)
{
    int iread=0;
	unsigned char buffer[1] = {0x26},Response[1024] = {0};

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
	       UsbWrite(buffer,1);
	
	       iread=UsbRead_14443(Response, delay_ms);

		   if(iread == 2)
		   {
			   memcpy(recv_buf,Response,iread);
			   *recv_lenth = iread;
			   return IFD_ICC_OK;
		   }
		   else return IFD_ICC_InitError;//卡片初始化失败
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_RequestAll(long ReaderHandle,unsigned char* recv_buf,unsigned char* recv_lenth,unsigned long delay_ms)
{
	int iread=0;
	unsigned char buffer[1] = {0x52},Response[1024] = {0};

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			UsbWrite(buffer,1);

			iread=UsbRead_14443(Response, delay_ms);

			if(iread == 2) 
			{
				memcpy(recv_buf,Response,iread);
				*recv_lenth = iread;
				return IFD_ICC_OK;
			}
			else return IFD_ICC_InitError;//卡片初始化失败
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

/***************************************************************************

    函数名称：hy_reset
    函数功能：读卡器对卡片进行重新上电
    参数说明：long ReaderHandle//设备句柄
	          unsigned long resp_time 等待返回时间设置

****************************************************************************/
long __stdcall hy_reset(long ReaderHandle,unsigned long delay_ms)
{
	int iread=0;
	unsigned char Response[1024] = {0},reset_buf[1] = {0x10};

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			UsbWrite(reset_buf,1);

			iread = UsbRead_14443(Response, delay_ms);
			Card_state = TRUE;//重置卡片分组号

			if (iread == 1)
			{
				::Sleep(1);
				return IFD_ICC_OK;//执行操作成功
			}
			else return IFD_ResetError;//读卡器上电失败
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_halt(long ReaderHandle,unsigned char* recv_buf,unsigned char* recv_lenth,unsigned long delay_ms)
{
	int iread=0;
	unsigned char buffer[1] = {0x50},Response[1024] = {0};

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			UsbWrite(buffer,1);

			iread = UsbRead_14443(Response, delay_ms);

			if(iread == 2)
			{
				memcpy(recv_buf,Response,iread);
				*recv_lenth = iread;
				return IFD_ICC_OK;
			}
			else return IFD_ICC_ERROR;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

unsigned char HYReader_uid[5] = { 0 };

long __stdcall hy_anticoll(long ReaderHandle,unsigned char* recv_buf,unsigned char* recv_lenth,unsigned long delay_ms)
{
	int iread=0;
	unsigned char buffer[2] = {0x93,0x20},Response[1024] = {0};

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{	
			UsbWrite(buffer,2);

			iread=UsbRead_14443(Response, delay_ms);

			if (iread == 5)
			{
				memcpy(HYReader_uid, Response, iread);
                memcpy(recv_buf,Response,iread);
				*recv_lenth = iread;
				return IFD_ICC_OK;
			}
			else 
			{ 
				return IFD_ICC_AnticoError;//获取卡片UID错误
			}
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_select_HYReader(long ReaderHandle, unsigned char* recv_buf, unsigned char* recv_lenth, unsigned long delay_ms)
{
	if (MyDeviceDetected == TRUE)
	{
		if (ReaderHandle == 110)
		{
			int iread = 0, i;
			unsigned char buffer[64] = { 0 }, Response[1024] = { 0 };

			buffer[0] = 0x93;
			buffer[1] = 0x70;
			for (i = 0; i < 5; i++)
				buffer[i + 2] = HYReader_uid[i];

			UsbWrite(buffer, 7);
			iread = UsbRead_14443(Response, delay_ms);
			if (iread == 1)
			{
				*recv_lenth = iread;
				memcpy(recv_buf, Response, iread);
				return IFD_ICC_OK;
			}
			else
			{
				return IFD_ICC_ERROR;	//执行错误
			}
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

/***************************************************************************

    函数名称：hy_select
    函数功能：选卡指令
    参数说明：long ReaderHandle//设备句柄
	          unsigned long resp_time 等待返回时间设置
			  unsigned char* _uid//将卡片UID传入函数中
			  unsigned char* recv_buf//卡片返回数据

	函数备注：2016.5.19 UID读卡器会自主计算CRC字节再发送给卡片，因此UID只需前四字节正确，第五字节错误也可正确执行选卡
****************************************************************************/
long __stdcall hy_select(long ReaderHandle,unsigned char* recv_buf,unsigned char* _uid,unsigned long delay_ms)
{
	int iread=0,i;
	unsigned char buffer[64]={0},Response[1024] = {0};

	buffer[0] = 0x93;
	buffer[1] = 0x70;
	for(i=0;i<5;i++)
		buffer[i+2]= _uid[i];

	if(MyDeviceDetected == FALSE) return IFD_UnConnected;//未建立连接 
	if(ReaderHandle != 110) return IFD_DeviceError;//连接错误的读卡器

	UsbWrite(buffer,7);
	iread=UsbRead_14443(Response, delay_ms);
	if (iread == 1)
	{
		memcpy(recv_buf, Response, iread);
		return IFD_ICC_OK;
	}
	else
	{
		return IFD_ICC_ERROR;	//执行错误
	}
}

long __stdcall hy_card(long ReaderHandle,unsigned char* UID)
{
	int iread=0;
	unsigned char recv_buf_REQA[2] = { 0 }, recv_buf_SEL[1] = { 0 }, recv_buf_UID[5] = { 0 }, recv_lenth = 0;

	if (MyDeviceDetected == FALSE) return IFD_UnConnected;//未建立连接 
	if (ReaderHandle != 110) return IFD_DeviceError;//连接错误的读卡器

	if (hy_reset(ReaderHandle, 0) == 0)
	{
		if (hy_request(ReaderHandle, recv_buf_REQA, &recv_lenth, 0) == 0)
		{
			if (hy_anticoll(ReaderHandle, recv_buf_UID, &recv_lenth, 0) == 0)
			{
				if (hy_select(ReaderHandle, recv_buf_SEL, recv_buf_UID, 0) == 0)
				{
					memcpy(UID, recv_buf_UID, 5);
					return IFD_ICC_OK;//执行操作成功
				}
				else return IFD_ICC_InitError;//卡片初始化错误
			}
			else return IFD_ICC_InitError;//卡片初始化错误
		}
		else return IFD_ICC_InitError;//卡片初始化错误
	}
	else return IFD_ICC_InitError;//卡片初始化错误

}

long __stdcall hy_DirectCmd(long ReaderHandle,unsigned char Lenth_of_DirectCmd_buffer,unsigned char* Response,unsigned char parity_CRC,unsigned char send_bits,unsigned char* Data_buffer,unsigned char receive_bits,unsigned char receive_bytes,unsigned long delay_ms)
{
	int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x48;
	buffer[1]=parity_CRC;
	buffer[2]=send_bits;
	buffer[3]=receive_bits;
	buffer[4]=receive_bytes;
	buffer[5]=Lenth_of_DirectCmd_buffer;
	for(i=0;i<Lenth_of_DirectCmd_buffer;i++)
		buffer[i+6]=Data_buffer[i];

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110)  
		{
			if(Lenth_of_DirectCmd_buffer+15)
			
			UsbWrite(buffer,Lenth_of_DirectCmd_buffer+6);

			memset(buffer,0,1024);	//读取数组内容

			iread=UsbRead_14443(Response, delay_ms);

			return iread;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_Read(long ReaderHandle,unsigned char* Response,unsigned char _Adr,unsigned long delay_ms)
{
	int iread=0;
	unsigned char buffer[1024]={0};

	buffer[0]=0x30;
	buffer[1]=_Adr;

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			
			UsbWrite(buffer,2);

			memset(buffer,0,32);	//读取数组内容  

			iread=UsbRead_14443(Response, delay_ms);

			if(iread==2) return IFD_ICC_ERROR;		//执行错误
			else return iread;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_Write4B(long ReaderHandle,unsigned char* Response,unsigned char* Write4B,unsigned char _Adr,unsigned long delay_ms)
{
	int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0xa2;
	buffer[1]=_Adr;
	for(i=0;i<4;i++)
		buffer[i+2]=Write4B[i];

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			
			UsbWrite(buffer,6);

			memset(buffer,0,32);	//读取数组内容  

			iread=UsbRead_14443(Response, delay_ms);

			return iread;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_SetKey(long ReaderHandle,unsigned char* Response,unsigned char Setkey,unsigned char* Keydata,unsigned char _Adr,unsigned long delay_ms)
{
	int iread=0,i;
	unsigned char buffer[1024]={0};

	if(Setkey == 0x0A)
		buffer[0]=0x11;
	else
		buffer[0]=0x12;
	buffer[1]=_Adr;
	for(i=0;i<6;i++)
		buffer[i+2]=Keydata[i];

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			
			UsbWrite(buffer,8);

			memset(buffer,0,32);	//读取数组内容  

			iread=UsbRead_14443(Response, delay_ms);

			return iread;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_Authen(long ReaderHandle,unsigned char* Response,unsigned char _Adr,int choice_key,unsigned long delay_ms)
{
	int iread=0;
	unsigned char buffer[1024]={0};

	if(choice_key == 0)
		buffer[0]=0x60;
	else
		buffer[0]=0x61;
	buffer[1]=_Adr;

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110)   
		{
			
			UsbWrite(buffer,2);

			memset(buffer,0,32);	//读取数组内容  

			iread=UsbRead_14443(Response, delay_ms);

			switch(iread)
			{
			case 2:return IFD_ICC_ERROR;		//执行错误
			case 3:return IFD_ICC_OK;       //执行成功		       
			default:return iread;
			}
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}


long __stdcall hy_Increase(long ReaderHandle,unsigned char* Response,unsigned char* Increase,unsigned char _Adr,unsigned long delay_ms)
{
	int iread=0;
	unsigned char buffer[1024]={0};

	buffer[0]=0xc1;
	buffer[1]=_Adr;
	buffer[2]=Increase[3];
	buffer[3]=Increase[2];
	buffer[4]=Increase[1];
	buffer[5]=Increase[0];

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110)   
		{
			
			UsbWrite(buffer,6);

			memset(buffer,0,32);	//读取数组内容  

			iread=UsbRead_14443(Response, delay_ms);

			return iread;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_Decrease(long ReaderHandle,unsigned char* Response,unsigned char* Decrease,unsigned char _Adr,unsigned long delay_ms)
{
	int iread=0;
	unsigned char buffer[1024]={0};

	buffer[0]=0xc0;
	buffer[1]=_Adr;
	buffer[2]=Decrease[3];
	buffer[3]=Decrease[2];
	buffer[4]=Decrease[1];
	buffer[5]=Decrease[0];

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110)  
		{
			
			UsbWrite(buffer,6);

			memset(buffer,0,32);	//读取数组内容  

			iread=UsbRead_14443(Response, delay_ms);

			return iread;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_Transfer(long ReaderHandle,unsigned char* Response,unsigned char _Adr,unsigned long delay_ms)
{
    int iread=0;
	unsigned char buffer[1024]={0};

	buffer[0]=0xb0;
	buffer[1]=_Adr;

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110)  
		{
		
	       UsbWrite(buffer,2);

	       memset(buffer,0,1024);	//读取数组内容  
	
	       iread=UsbRead_14443(Response, delay_ms);

		   switch(iread)
		   {
		       case 1:return IFD_ICC_ERROR;		//执行错误
		       case 2:return IFD_ICC_OK;       //执行成功		       
		       default:return iread;
		   }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_Restore(long ReaderHandle,unsigned char* Response,unsigned char* Restore,unsigned char _Adr,unsigned long delay_ms)
{
	int iread=0;
	unsigned char buffer[1024]={0};

	buffer[0]=0xc2;
	buffer[1]=_Adr;
	buffer[2]=Restore[3];
	buffer[3]=Restore[2];
	buffer[4]=Restore[1];
	buffer[5]=Restore[0];

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110)  
		{
			
			UsbWrite(buffer,6);

			memset(buffer,0,32);	//读取数组内容  

			iread=UsbRead_14443(Response, delay_ms);

			return iread;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_Write16B(long ReaderHandle,unsigned char* Response,unsigned char* Write16B,unsigned char _Adr,unsigned long delay_ms)
{
	int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0xa0;
	buffer[1]=_Adr;
	for(i=0;i<16;i++)
		buffer[i+2]=Write16B[i];

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			
			UsbWrite(buffer,12);

			memset(buffer,0,32);	//读取数组内容  

			iread=UsbRead_14443(Response, delay_ms);

			return iread;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}
long __stdcall hy_Write4To16(long ReaderHandle,unsigned char* Response,unsigned char* Write4To16,unsigned char _Adr,unsigned char* Write4To16_1,unsigned long delay_ms)
{
	int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0xa0;
	buffer[1]=_Adr;
	for(i=0;i<4;i++)
	{
		buffer[i+2]=Write4To16[i];
		buffer[i+10]=Write4To16[i];
		buffer[i+6]=Write4To16_1[i];
	}
	buffer[14]=_Adr;
	buffer[15]=~_Adr;
	buffer[16]=_Adr;
	buffer[17]=~_Adr;

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			
			UsbWrite(buffer,12);

			memset(buffer,0,32);	//读取数组内容  

			iread=UsbRead_14443(Response, delay_ms);

			return iread;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 

}

//-----------------------------------------------------14443-4部分----------------------------------------------

int R_Block(unsigned char* CID,unsigned char* recv_buf)
{
	/******************************************
	CID为配置字节
	CID[0]----->0x01(有CID),0x02(无CID)
	CID[1]----->CID字节
	CID[2]----->0x01(有NAD),0x02(无NAD)
	CID[3]----->NAD字节
	CID[4]----->0x01(ACK),0x02(NAK)
	CID[5]----->分组号（0x00,0x01）
	CID[6]----->0x01(无连接),0x02(有连接)
	CID[7]----->0x01(DESELECT),0x02(WTX)
	CID[8]----->PCB字节
	*******************************************/
	int iread = 0,i_iread = 0,k = 0;
	unsigned char send_buffer[1024] = {0};

	if(CID[0] == 0x01)//有CID
	{
		CID[4] = (CID[4] == 0x01) ? 0xEF:0x10;//ACK\NAK
		CID[5] = (CID[5] == 0x00) ? 0xFE:0x01;//分组号

		send_buffer[0] = 0x48;
		send_buffer[1] = 0xf0;
		send_buffer[2] = 0x10;//发送数据的比特数(Hex)
		send_buffer[3] = 0x00;
		send_buffer[4] = 0x00;
		send_buffer[5] = 0x02;
		if(CID[4] == 0xEF)
			send_buffer[6] = 0xaa&CID[4];
		else
			send_buffer[6] = 0xaa|CID[4];
		if(CID[5] == 0xFE)
			send_buffer[6] = send_buffer[6]&CID[5];
		else
			send_buffer[6] = send_buffer[6]|CID[5];
		send_buffer[7] = CID[1];//CID字节

		UsbWrite(send_buffer,8);
		memset(send_buffer,0,1024);
		iread=UsbRead_14443(recv_buf,0);

		if(iread < 0)
		{
			if(iread == -6)
			{
				for(k = 0;k<3;k++)
				{
					recv_buf[k] = recv_buf[k+3];
				}
				recv_buf[k] = 0x00;
				recv_buf[k+1] = 0x00;
				recv_buf[k+2] = 0x00;
				recv_buf[k+3] = 0x00;

				return iread;//返回错误值
			}
			else return iread;//返回错误值
		}

		CID[8] = recv_buf[0];
		for(i_iread = 0;i_iread<iread-2;i_iread++)
		{
			recv_buf[i_iread] = recv_buf[i_iread+2];
		}
		recv_buf[i_iread] = 0x00;
		recv_buf[i_iread+1] = 0x00;
		recv_buf[i_iread+2] = 0x00;
		recv_buf[i_iread+3] = 0x00;

		return i_iread;
		
	}
	else//无cid
	{
		CID[4] = (CID[4] == 0x01) ? 0xEF:0x10;//ACK\NAK
		CID[5] = (CID[5] == 0x00) ? 0xFE:0x01;//分组号

		send_buffer[0] = 0x48;
		send_buffer[1] = 0xf0;
		send_buffer[2] = 0x08;//发送数据的比特数(Hex)
		send_buffer[3] = 0x00;
		send_buffer[4] = 0x00;
		send_buffer[5] = 0x01;
		if(CID[4] == 0xEF)
			send_buffer[6] = 0xaa&CID[4];
		else
			send_buffer[6] = 0xaa|CID[4];
		if(CID[5] == 0xFE)
			send_buffer[6] = send_buffer[6]&CID[5];
		else
			send_buffer[6] = send_buffer[6]|CID[5];

		UsbWrite(send_buffer,7);
		memset(send_buffer,0,1024);
		iread=UsbRead_14443(recv_buf,0);

		if(iread < 0)
		{
			if(iread == -6)
			{
				for(k = 0;k<3;k++)
				{
					recv_buf[k] = recv_buf[k+3];
				}
				recv_buf[k] = 0x00;
				recv_buf[k+1] = 0x00;
				recv_buf[k+2] = 0x00;
				recv_buf[k+3] = 0x00;

				return iread;//返回错误值
			}
			else return iread;//返回错误值
		}

		CID[8] = recv_buf[0];
		for(i_iread = 0;i_iread<iread-1;i_iread++)
		{
			recv_buf[i_iread] = recv_buf[i_iread+1];
		}
		recv_buf[i_iread] = 0x00;
		recv_buf[i_iread+1] = 0x00;
		recv_buf[i_iread+2] = 0x00;
		recv_buf[i_iread+3] = 0x00;

		return i_iread;
	}
}
int I_Block(unsigned char* CID,unsigned char send_len,unsigned char* send_data,unsigned char* recv_buf)
{
	/******************************************
	CID为配置字节
	CID[0]----->0x01(有CID),0x02(无CID)
	CID[1]----->CID字节
	CID[2]----->0x01(有NAD),0x02(无NAD)
	CID[3]----->NAD字节
	CID[4]----->0x01(ACK),0x02(NAK)
	CID[5]----->分组号（0x00,0x01）
	CID[6]----->0x01(无连接),0x02(有连接)
	CID[7]----->0x01(DESELECT),0x02(WTX)
	CID[8]----->PCB字节
	*******************************************/
	int iread = 0,i_iread = 0,k = 0;
	unsigned char send_buffer[1024] = {0};

	if(CID[0] == 0x01)//有CID
	{
		if(CID[2] == 0x01)//有NAD
		{
			CID[6] = (CID[6] == 0x01) ? 0xEF:0x10;//无链接/有链接
			CID[5] = (CID[5] == 0x00) ? 0xFE:0x01;//分组号

			send_buffer[0] = 0x48;
			send_buffer[1] = 0xf0;
			send_buffer[2] = (send_len+3)*8;//发送数据的比特数(Hex)
			send_buffer[3] = 0x00;
			send_buffer[4] = 0x00;
			send_buffer[5] = send_len+3;
			if(CID[6] == 0xEF)
				send_buffer[6] = 0x0E&CID[6];
			else
				send_buffer[6] = 0x0E|CID[6];
			if(CID[5] == 0xFE)
				send_buffer[6] = send_buffer[6]&CID[5];
			else
				send_buffer[6] = send_buffer[6]|CID[5];
			send_buffer[7] = CID[1];//CID字节
			send_buffer[8] = CID[3];//NAD字节
			for(int i = 0;i<send_len;i++)
			{
				send_buffer[i+9] = send_data[i];
			}

			UsbWrite(send_buffer,send_len+9);
			memset(send_buffer,0,1024);
			iread=UsbRead_14443(recv_buf,0);

			if(iread < 0)
			{
				if(iread == -6)
				{
					for(k = 0;k<3;k++)
					{
						recv_buf[k] = recv_buf[k+3];
					}
					recv_buf[k] = 0x00;
					recv_buf[k+1] = 0x00;
					recv_buf[k+2] = 0x00;
					recv_buf[k+3] = 0x00;

					return iread;//返回错误值
				}
				else return iread;//返回错误值
			}

			CID[8] = recv_buf[0];//卡片返回的PCB字节
			for(i_iread = 0;i_iread<iread-3;i_iread++)
			{
				recv_buf[i_iread] = recv_buf[i_iread+3];
			}
			recv_buf[i_iread] = 0x00;
			recv_buf[i_iread+1] = 0x00;
			recv_buf[i_iread+2] = 0x00;
			recv_buf[i_iread+3] = 0x00;

		    return i_iread;
		}
		else
		{
			CID[6] = (CID[6] == 0x01) ? 0xEF:0x10;//无链接/有链接
			CID[5] = (CID[5] == 0x00) ? 0xFE:0x01;//分组号

			send_buffer[0] = 0x48;
			send_buffer[1] = 0xf0;
			send_buffer[2] = (send_len+2)*8;//发送数据的比特数(Hex)
			send_buffer[3] = 0x00;
			send_buffer[4] = 0x00;
			send_buffer[5] = send_len+2;
			if(CID[6] == 0xEF)
				send_buffer[6] = 0x0A&CID[6];
			else
				send_buffer[6] = 0x0A|CID[6];
			if(CID[5] == 0xFE)
				send_buffer[6] = send_buffer[6]&CID[5];
			else
				send_buffer[6] = send_buffer[6]|CID[5];
			send_buffer[7] = CID[1];//CID字节
			for(int i = 0;i<send_len;i++)
			{
				send_buffer[i+8] = send_data[i];
			}

			UsbWrite(send_buffer,send_len+8);
			memset(send_buffer,0,1024);
			iread=UsbRead_14443(recv_buf,0);

			if(iread < 0)
			{
				if(iread == -6)
				{
					for(k = 0;k<3;k++)
					{
						recv_buf[k] = recv_buf[k+3];
					}
					recv_buf[k] = 0x00;
					recv_buf[k+1] = 0x00;
					recv_buf[k+2] = 0x00;
					recv_buf[k+3] = 0x00;

					return iread;//返回错误值
				}
				else return iread;//返回错误值
			}

			CID[8] = recv_buf[0];
			for(i_iread = 0;i_iread<iread-2;i_iread++)
			{
				recv_buf[i_iread] = recv_buf[i_iread+2];
			}
			recv_buf[i_iread] = 0x00;
			recv_buf[i_iread+1] = 0x00;
			recv_buf[i_iread+2] = 0x00;
			recv_buf[i_iread+3] = 0x00;

		    return i_iread;
		}
		
	}
	else//无cid
	{
		if(CID[2] == 0x01)//有NAD
		{
			CID[6] = (CID[6] == 0x01) ? 0xEF:0x10;//无链接/有链接
			CID[5] = (CID[5] == 0x00) ? 0xFE:0x01;//分组号

			send_buffer[0] = 0x48;
			send_buffer[1] = 0xf0;
			send_buffer[2] = (send_len+2)*8;//发送数据的比特数(Hex)
			send_buffer[3] = 0x00;
			send_buffer[4] = 0x00;
			send_buffer[5] = send_len+2;
			if(CID[6] == 0xEF)
				send_buffer[6] = 0x06&CID[6];
			else
				send_buffer[6] = 0x06|CID[6];
			if(CID[5] == 0xFE)
				send_buffer[6] = send_buffer[6]&CID[5];
			else
				send_buffer[6] = send_buffer[6]|CID[5];
			send_buffer[7] = CID[3];//NAD字节
			for(int i = 0;i<send_len;i++)
			{
				send_buffer[i+8] = send_data[i];
			}

			UsbWrite(send_buffer,send_len+8);
			memset(send_buffer,0,1024);
			iread=UsbRead_14443(recv_buf,0);

			if(iread < 0)
			{
				if(iread == -6)
				{
					for(k = 0;k<3;k++)
					{
						recv_buf[k] = recv_buf[k+3];
					}
					recv_buf[k] = 0x00;
					recv_buf[k+1] = 0x00;
					recv_buf[k+2] = 0x00;
					recv_buf[k+3] = 0x00;

					return iread;//返回错误值
				}
				else return iread;//返回错误值
			}

			CID[8] = recv_buf[0];
			for(i_iread = 0;i_iread<iread-2;i_iread++)
			{
				recv_buf[i_iread] = recv_buf[i_iread+2];
			}
			recv_buf[i_iread] = 0x00;
			recv_buf[i_iread+1] = 0x00;
			recv_buf[i_iread+2] = 0x00;
			recv_buf[i_iread+3] = 0x00;

			return i_iread;
		}
		else
		{
			CID[6] = (CID[6] == 0x01) ? 0xEF:0x10;//无链接/有链接
			CID[5] = (CID[5] == 0x00) ? 0xFE:0x01;//分组号

			send_buffer[0] = 0x48;
			send_buffer[1] = 0xf0;
			send_buffer[2] = (send_len+1)*8;//发送数据的比特数(Hex)
			send_buffer[3] = 0x00;
			send_buffer[4] = 0x00;
			send_buffer[5] = send_len+1;
			if(CID[6] == 0xEF)
				send_buffer[6] = 0x02&CID[6];
			else
				send_buffer[6] = 0x02|CID[6];
			if(CID[5] == 0xFE)
				send_buffer[6] = send_buffer[6]&CID[5];
			else
				send_buffer[6] = send_buffer[6]|CID[5];
			for(int i = 0;i<send_len;i++)
			{
				send_buffer[i+7] = send_data[i];
			}

			UsbWrite(send_buffer,send_len+7);
			memset(send_buffer,0,1024);
			iread=UsbRead_14443(recv_buf,0);

			if(iread < 0)
			{
				if(iread == -6)
				{
					for(k = 0;k<3;k++)
					{
						recv_buf[k] = recv_buf[k+3];
					}
					recv_buf[k] = 0x00;
					recv_buf[k+1] = 0x00;
					recv_buf[k+2] = 0x00;
					recv_buf[k+3] = 0x00;

					return iread;//返回错误值
				}
				else return iread;//返回错误值
			}

			CID[8] = recv_buf[0];
			for(i_iread = 0;i_iread<iread-1;i_iread++)
			{
				recv_buf[i_iread] = recv_buf[i_iread+1];
			}
			recv_buf[i_iread] = 0x00;
			recv_buf[i_iread+1] = 0x00;
			recv_buf[i_iread+2] = 0x00;
			recv_buf[i_iread+3] = 0x00;

			return i_iread;
		}
	}
}
int S_Block(unsigned char* CID,unsigned char INF,unsigned char* recv_buf)
{
	/******************************************
	CID为配置字节
	CID[0]----->0x01(有CID),0x02(无CID)
	CID[1]----->CID字节
	CID[2]----->0x01(有NAD),0x02(无NAD)
	CID[3]----->NAD字节
	CID[4]----->0x01(ACK),0x02(NAK)
	CID[5]----->分组号（0x00,0x01）
	CID[6]----->0x01(无连接),0x02(有连接)
	CID[7]----->0x01(DESELECT),0x02(WTX)
	CID[8]----->PCB字节
	*******************************************/
	int iread = 0,i_iread = 0,k = 0;
	unsigned char send_buffer[1024] = {0};

	if(CID[0] == 0x01)//有CID
	{
		if(CID[7] == 0x01)//DESELECT
		{
			send_buffer[0] = 0x48;
			send_buffer[1] = 0xf0;
			send_buffer[2] = 0x10;//发送数据的比特数(Hex)
			send_buffer[3] = 0x00;
			send_buffer[4] = 0x00;
			send_buffer[5] = 0x02;
			send_buffer[6] = 0xca;
			send_buffer[7] = CID[1];//CID字节

			UsbWrite(send_buffer,8);
			memset(send_buffer,0,1024);
			iread=UsbRead_14443(recv_buf,0);
			return iread;
		}
		else
		{
			send_buffer[0] = 0x48;
			send_buffer[1] = 0xf0;
			send_buffer[2] = 0x18;//发送数据的比特数(Hex)
			send_buffer[3] = 0x00;
			send_buffer[4] = 0x00;
			send_buffer[5] = 0x03;
			send_buffer[6] = 0xfa;
			send_buffer[7] = CID[1];//CID字节
			send_buffer[8] = INF;

			UsbWrite(send_buffer,9);
			memset(send_buffer,0,1024);
			iread=UsbRead_14443(recv_buf,0);

			if(iread < 0)
			{
				if(iread == -6)
				{
					for(k = 0;k<3;k++)
					{
						recv_buf[k] = recv_buf[k+3];
					}
					recv_buf[k] = 0x00;
					recv_buf[k+1] = 0x00;
					recv_buf[k+2] = 0x00;
					recv_buf[k+3] = 0x00;

					return iread;//返回错误值
				}
				else return iread;//返回错误值
			}

			CID[8] = recv_buf[0];
			for(i_iread = 0;i_iread<iread-2;i_iread++)
			{
				recv_buf[i_iread] = recv_buf[i_iread+2];
			}
			recv_buf[i_iread] = 0x00;
			recv_buf[i_iread+1] = 0x00;
			recv_buf[i_iread+2] = 0x00;
			recv_buf[i_iread+3] = 0x00;

			return i_iread;
		}
	}
	else//无cid
	{
		if(CID[7] == 0x01)//DESELECT
		{
			send_buffer[0] = 0x48;
			send_buffer[1] = 0xf0;
			send_buffer[2] = 0x08;//发送数据的比特数(Hex)
			send_buffer[3] = 0x00;
			send_buffer[4] = 0x00;
			send_buffer[5] = 0x01;
			send_buffer[6] = 0xc2;

			UsbWrite(send_buffer,7);
			memset(send_buffer,0,1024);
			iread=UsbRead_14443(recv_buf,0);
			return iread;
		}
		else
		{
			send_buffer[0] = 0x48;
			send_buffer[1] = 0xf0;
			send_buffer[2] = 0x08;//发送数据的比特数(Hex)
			send_buffer[3] = 0x00;
			send_buffer[4] = 0x00;
			send_buffer[5] = 0x02;//ERROR：2016.7.12由0x03修改为0x02
			send_buffer[6] = 0xf2;
			send_buffer[7] = INF;

			UsbWrite(send_buffer,8);
			memset(send_buffer,0,1024);
			iread=UsbRead_14443(recv_buf,0);

			if(iread < 0)
			{
				if(iread == -6)
				{
					for(k = 0;k<3;k++)
					{
						recv_buf[k] = recv_buf[k+3];
					}
					recv_buf[k] = 0x00;
					recv_buf[k+1] = 0x00;
					recv_buf[k+2] = 0x00;
					recv_buf[k+3] = 0x00;

					return iread;//返回错误值
				}
				else return iread;//返回错误值
			}

			CID[8] = recv_buf[0];
			for(i_iread = 0;i_iread<iread-1;i_iread++)
			{
				recv_buf[i_iread] = recv_buf[i_iread+1];
			}
			recv_buf[i_iread] = 0x00;
			recv_buf[i_iread+1] = 0x00;
			recv_buf[i_iread+2] = 0x00;
			recv_buf[i_iread+3] = 0x00;

			return i_iread;
		}
	}
}

/***************************************************************************

    函数名称：hy_pro_reset
    函数功能：选卡指令
    参数说明：long ReaderHandle//设备句柄
	          unsigned char* len 返回数据的长度
			  unsigned char* recv_buf//卡片返回数据

	函数备注：此函数根据读卡器功能，发送了E080,0xE0为RATS的开始字节
	          0x80为RATS的参数字节，定义了FSDI以及CID。此处CID为0x00
****************************************************************************/

long __stdcall hy_pro_reset(long ReaderHandle,unsigned char* len,unsigned char* recv_buf)
{
	if (MyDeviceDetected == FALSE) return IFD_UnConnected;//未建立连接 
	if (ReaderHandle != 110) return IFD_DeviceError;//连接错误的读卡器

	int iread = 0;
	unsigned char recv_buf_REQA[2] = { 0 }, recv_buf_ANTI[5] = { 0 }, recv_buf_SEL[1] = { 0 }, Response[1024] = { 0 }, rats_buf[8] = { 0x48,0xf0,0x10,0x00,0x00,0x02,0xe0,0x80 }, reset_buf[1] = { 0x10 };

	UsbWrite(rats_buf,8);//发送E080
	iread=UsbRead_14443(Response,0);
	*len = iread;

	if(recv_buf[3] == 0xff&&recv_buf[4] == 0xff) return IFD_ICC_InitError;//卡片初始化错误
	else if(iread < 0) return IFD_ICC_InitError;//卡片初始化错误
	else 
	{
		memset(resp_ATS,0,1024);
		memcpy(resp_ATS,Response,iread);
        memcpy(recv_buf,Response,iread);
		return IFD_ICC_OK;
	}
}

unsigned char pcb_blc = 0x00;//块号
unsigned char CID[9] = {0};//开端域的配置

/******************************************
Prologue_Field为配置字节
Prologue_Field[0]----->0x01(有CID),0x02(无CID)
Prologue_Field[1]----->CID字节
Prologue_Field[2]----->0x01(有NAD),0x02(无NAD)
Prologue_Field[3]----->NAD字节
Prologue_Field[4]----->0x01(ACK),0x02(NAK)
Prologue_Field[5]----->分组号（0x00,0x01）
Prologue_Field[6]----->0x01(无连接),0x02(有连接)
Prologue_Field[7]----->0x01(DESELECT),0x02(WTX)
Prologue_Field[8]----->PCB字节
*******************************************/

long __stdcall hy_pro_command(long ReaderHandle,// Add test pro_command_cmdtime for measure WTX time by gHan 2016.7.10
							  unsigned char send_len,
							  unsigned int* recv_len,
							  unsigned char* send_buf,
							  unsigned char* recv_buf)//此函数不包含链接！
{
	int iread=0;
	unsigned char Response[1024] = {0},sBuf_Cmd[1024] = {0},rBuf_Cmd[1024] = {0},INF_S = 0x00;

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			if(Card_state == TRUE)pcb_blc = 0x00;  //重置卡片分组号
			else pcb_blc = 0x01;

			if((resp_ATS[4]&0x02) == 0x02) CID[0] = 0x01;//CID标志位
			else CID[0] = 0x02;
			//if((resp_ATS[4]&0x01) == 0x01) CID[2] = 0x01;//NAD标志位
			//else CID[2] = 0x02;

			if(pcb_blc == 0x00)
			{
				while(1)
				{
					memcpy(sBuf_Cmd,send_buf,send_len);//发送数据放入缓存区

					CID[4] = 0x00;CID[5] = 0x00;CID[6] = 0x01;CID[7] = 0x00;
					iread = I_Block(CID,send_len,sBuf_Cmd,rBuf_Cmd);//发送I(0)0

					memset(sBuf_Cmd,0,1024);//清空缓存区

					if(iread<0)
					{
						if(iread == -6)
						{
							*recv_len = 0x03;
							return iread;//返回错误值
						}
						else return iread;//返回错误值
					}

					if(CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)//返回I(0)0
					{
						Card_state = FALSE;

						memcpy(recv_buf,rBuf_Cmd,iread);
						*recv_len = iread;
						return IFD_ICC_OK;	
					}
					else if(CID[8] == 0xfa || CID[8] == 0xf2)//返回S(WTX)request(带CID)-Waiting time extension-Scenario 2
					{
						while(1)//2016.7.12修正(hg)：WTXM为卡片返回的S_Block中的INF域,无限循环发送
						{
							INF_S = rBuf_Cmd[0]&0x3F;//WTXM

							CID[4] = 0x00;CID[5] = 0x00;CID[6] = 0x01;CID[7] = 0x02;
							memset(rBuf_Cmd,0,iread);

							iread = S_Block(CID,INF_S,rBuf_Cmd);//发送S(WTX)

							if(iread<0)
							{
								if(iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if(CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)
							{//返回I(0)0
								Card_state = FALSE;

								memcpy(recv_buf,rBuf_Cmd,iread);
								*recv_len = iread;
								return IFD_ICC_OK;
							}
							else if (CID[8] == 0xfa || CID[8] == 0xf2)
							{//返回WTX
								continue;
							}
							else
							{//卡片无响应
								while(1)
								{
									CID[4] = 0x02;CID[5] = 0x00;CID[6] = 0x00;CID[7] = 0x00;
									memset(rBuf_Cmd,0,iread);

									iread = R_Block(CID,rBuf_Cmd);//发送R(NAK)0

									if(iread<0)
									{
										if(iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}
									if(CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)//返回I(0)0--Scenario 8
									{
										Card_state = FALSE;

										memcpy(recv_buf,rBuf_Cmd,iread);
										*recv_len = iread;
										return IFD_ICC_OK;
									}
									else if(CID[8] == 0xfa || CID[8] == 0xf2)break;//--Scenario 10
									else continue;//--Scenario 9
								}
							}
						}
					}
					else if(CID[8] == 0x1a || CID[8] == 0x12 || CID[8] == 0x1e || CID[8] == 0x16)//返回I(1)0-PICC uses chaining-Scenario 5
					{
						int iread_k = 0;
						unsigned char recv_piccbuf[1024] = {0};

						for(int iread_1 = 0;iread_1<iread;iread_1++)//将返回值保存
						{
							recv_piccbuf[iread_k] = rBuf_Cmd[iread_1];
							iread_k++;
						}
						while(1)
						{//PICC使用链接，读卡器无限轮询R(ACK)1
							CID[4] = 0x01;CID[5] = 0x01;CID[6] = 0x00;CID[7] = 0x00;
							memset(recv_piccbuf,0,iread);

							iread = R_Block(CID,recv_piccbuf);//发送R(ACK)1

							if(iread<0)
							{//此时PCD出现意外错误
								if(iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if(CID[8] == 0x0b || CID[8] == 0x03 || CID[8] == 0x0f || CID[8] == 0x07)//返回I(0)1
							{
								for(int iread_1 = 0;iread_1<iread;iread_1++)//将返回值保存(翻转)
								{
									rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
									iread_k++;
								}
								Card_state = TRUE;

								memcpy(recv_buf,rBuf_Cmd,iread_k);
								*recv_len = iread_k;
								return IFD_ICC_OK;
							}
							else if(CID[8] == 0x1b || CID[8] == 0x13 || CID[8] == 0x1f || CID[8] == 0x17)//返回I(1)1
							{
								for(int iread_1 = 0;iread_1<iread;iread_1++)//将返回值保存(翻转)
								{
									rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
									iread_k++;
								}
								while(1)
								{//返回I(1)1,PCD向PICC无限轮询R(ACK)0
									CID[4] = 0x01;CID[5] = 0x00;CID[6] = 0x00;CID[7] = 0x00;
									memset(recv_piccbuf,0,iread);

									iread = R_Block(CID,recv_piccbuf);//发送R(ACK)0

									if(iread<0)
									{
										if(iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}
									if(CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)
									{//正确的返回I(0)0
										for(int iread_1 = 0;iread_1<iread;iread_1++)//将返回值保存(翻转)
										{
											rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
											iread_k++;
										}
										Card_state = FALSE;
										//将PCD的块号置1
										memcpy(recv_buf,rBuf_Cmd,iread_k);
										*recv_len = iread_k;
										return IFD_ICC_OK;
									}
									else continue;//返回除I(0)0以外的其他任何错误开端域，都将认为错误。
								}
							}
							else continue;//返回除I(1)1以外的其他任何错误开端域，都将认为错误,重新发送R(ACK)1。
						}
					}
					else //无返回-time out
					{
						while(1)
						{//PICC无返回时，PCD向PICC无限轮询R(NAK)0
							CID[4] = 0x02;CID[5] = 0x00;CID[6] = 0x00;CID[7] = 0x00;
							memset(rBuf_Cmd,0,iread);

							iread = R_Block(CID,rBuf_Cmd);//发送R(NAK)0

							if(iread<0)
							{//此时PCD出现意外错误
								if(iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if(CID[8] == 0xab || CID[8] == 0xa3) break;//返回R(ACK)1，跳出循环，重新发送I(0)0--Scenario 6
							else if(CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)//返回I(0)0--Scenario 8
							{
								Card_state = FALSE;

								memcpy(recv_buf,rBuf_Cmd,iread);
								*recv_len = iread;
								return IFD_ICC_OK;
							}
							else if(CID[8] == 0xfa || CID[8] == 0xf2)//返回S（WTX）--Scenario 10
							{
								while(1)
								{
									INF_S = rBuf_Cmd[0]&0x3F;//WTXM

									CID[4] = 0x00;CID[5] = 0x00;CID[6] = 0x01;CID[7] = 0x02;
									memset(rBuf_Cmd,0,iread);

									iread = S_Block(CID,INF_S,rBuf_Cmd);//发送S(WTX)

									if(iread<0)
									{
										if(iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}
									if(CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)//返回I(0)0
									{
										Card_state = FALSE;

										memcpy(recv_buf,rBuf_Cmd,iread);
										*recv_len = iread;
										return IFD_ICC_OK;
									}
									else break;//S（WTX）无响应，跳出WTX循环，重新发送R(NAK)0
								}
							}
							else continue;//R(NAK)0无返回，继续发送R（NAK）0
						}
					}
				}
			}
			else//发送I(0)1
			{
				while(1)
				{
					memcpy(sBuf_Cmd,send_buf,send_len);//发送数据放入缓存区
					CID[4] = 0x00;CID[5] = 0x01;CID[6] = 0x01;CID[7] = 0x00;
					iread = I_Block(CID,send_len,sBuf_Cmd,rBuf_Cmd);//发送I(0)1

					if(iread<0)
					{
						if(iread == -6)
						{
							*recv_len = 0x03;
							return iread;//返回错误值
						}
						else return iread;//返回错误值
					}
					if(CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1
					{
						Card_state = TRUE;

						memcpy(recv_buf,rBuf_Cmd,iread);
						*recv_len = iread;
						return IFD_ICC_OK;
					}
					else if(CID[8] == 0xfa || CID[8] == 0xf2)//返回S(WTX)request-Waiting time extension-Scenario 2
					{
						while(1)
						{
							INF_S = rBuf_Cmd[0]&0x3F;//WTXM

							CID[4] = 0x00;CID[5] = 0x00;CID[6] = 0x01;CID[7] = 0x02;
							memset(rBuf_Cmd,0,iread);

							iread = S_Block(CID,INF_S,rBuf_Cmd);//发送S(WTX)

							if(iread<0)
							{
								if(iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if(CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1
							{
								Card_state = TRUE;

								memcpy(recv_buf,rBuf_Cmd,iread);
								*recv_len = iread;
								return IFD_ICC_OK;
							}
							else if (CID[8] == 0xfa || CID[8] == 0xf2)
							{//返回S(WTX)
								continue;
							}
							else
							{
								while(1)
								{
									CID[4] = 0x02;CID[5] = 0x01;CID[6] = 0x00;CID[7] = 0x00;
									memset(rBuf_Cmd,0,iread);

									iread = R_Block(CID,rBuf_Cmd);//发送R(NAK)1

									if(iread<0)
									{
										if(iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}
									if(CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1--Scenario 8
									{
										Card_state = TRUE;

										memcpy(recv_buf,rBuf_Cmd,iread);
										*recv_len = iread;
										return IFD_ICC_OK;
									}
									else if(CID[8] == 0xfa || CID[8] == 0xf2)break;//--Scenario 10
									else continue;//--Scenario 9
								}
							}
						}
					}
					else if(CID[8] == 0x1b || CID[8] == 0x13 || CID[8] == 0x1f || CID[8] == 0x17)//返回I(1)1-PICC uses chaining-Scenario 5
					{
						int iread_k = 0;
						unsigned char recv_piccbuf[1024] = {0};

						for(int iread_1 = 0;iread_1<iread;iread_1++)//将返回值保存
						{
							recv_piccbuf[iread_k] = rBuf_Cmd[iread_1];
							iread_k++;
						}
						while(1)
						{
							CID[4] = 0x01;CID[5] = 0x00;CID[6] = 0x00;CID[7] = 0x00;
							memset(recv_piccbuf,0,iread);	

							iread = R_Block(CID,recv_piccbuf);//发送R(ACK)0

							if(iread<0)
							{
								if(iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if(CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)//返回I(0)0
							{
								for(int iread_1 = 0;iread_1<iread;iread_1++)//将返回值保存(翻转)
								{
									rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
									iread_k++;
								}
								Card_state = FALSE;

								memcpy(recv_buf,rBuf_Cmd,iread);
								*recv_len = iread_k;
								return IFD_ICC_OK;
							}
							else if(CID[8] == 0x1a || CID[8] == 0x12 || CID[8] == 0x1e || CID[8] == 0x16)//返回I(1)0,需要接收后续信息
							{
								for(int iread_1 = 0;iread_1<iread;iread_1++)//将返回值保存(翻转)
								{
									rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
									iread_k++;
								}
								while(1)
								{
									CID[4] = 0x01;CID[5] = 0x01;CID[6] = 0x00;CID[7] = 0x00;
									memset(recv_piccbuf,0,iread);

									iread = R_Block(CID,recv_piccbuf);//发送R(ACK)1

									if(iread<0)
									{
										if(iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}
									if(CID[8] == 0x1b || CID[8] == 0x13 || CID[8] == 0x1f || CID[8] == 0x17)//返回I(1)1
									{
										for(int iread_1 = 0;iread_1<iread;iread_1++)//将返回值保存(翻转)
										{
											rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
											iread_k++;
										}
										break;
									}
									else if(CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1
									{
										for(int iread_1 = 0;iread_1<iread;iread_1++)//将返回值保存(翻转)
										{
											rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
											iread_k++;
										}
										Card_state = TRUE;

										memcpy(recv_buf,rBuf_Cmd,iread_k);
										*recv_len = iread_k;
										return IFD_ICC_OK;
									}
									else continue;//无返回
								}
							}
							else continue;
						}
						return IFD_ICC_ERROR;
					}
					else //无返回-time out
					{
						while(1)
						{
							CID[4] = 0x02;CID[5] = 0x01;CID[6] = 0x00;CID[7] = 0x00;
							memset(rBuf_Cmd,0,iread);

							iread = R_Block(CID,rBuf_Cmd);//发送R(NAK)1

							if(iread<0)
							{
								if(iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if(CID[8] == 0xaa || CID[8] == 0xa2) break;//返回R(ACK)0--Scenario 6
							else if(CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1--Scenario 8
							{
								Card_state = FALSE;

								memcpy(recv_buf,rBuf_Cmd,iread);
								*recv_len = iread;
								return IFD_ICC_OK;
							}
							else if(CID[8] == 0xfa || CID[8] == 0xf2)//--Scenario 10
							{
								while(1)
								{
									INF_S = rBuf_Cmd[0]&0x3F;//WTXM

									CID[4] = 0x00;CID[5] = 0x00;CID[6] = 0x01;CID[7] = 0x02;
									memset(rBuf_Cmd,0,iread);

									iread = S_Block(CID,INF_S,rBuf_Cmd);//发送S(WTX)

									if(iread<0)
									{
										if(iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}

									if(CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1
									{
										Card_state = TRUE;

										memcpy(recv_buf,rBuf_Cmd,iread);
										*recv_len = iread;
										return IFD_ICC_OK;
									}
									else
									{
										while(1)
										{
											CID[4] = 0x02;CID[5] = 0x01;CID[6] = 0x00;CID[7] = 0x00;
											memset(rBuf_Cmd,0,iread);

											iread = R_Block(CID,rBuf_Cmd);//发送R(NAK)1

											if(iread<0)
											{
												if(iread == -6)
												{
													*recv_len = 0x03;
													return iread;//返回错误值
												}
												else return iread;//返回错误值
											}
											if(CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1--Scenario 8
											{
												Card_state = TRUE;

												memcpy(recv_buf,rBuf_Cmd,iread);
												*recv_len = iread;
												return IFD_ICC_OK;
											}
											else if(CID[8] == 0xfa || CID[8] == 0xf2)break;//--Scenario 10
											else continue;//--Scenario 9
										}
									}
								}
							}
							else continue;
						}
					}
				}
			}	
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_pro_command_CmdTime(long ReaderHandle,
	unsigned char send_len,
	unsigned int* recv_len,
	unsigned char* send_buf,
	unsigned char* recv_buf)//此函数不包含链接！
{
	int iread = 0;
	unsigned char Response[1024] = { 0 }, sBuf_Cmd[1024] = { 0 }, rBuf_Cmd[1024] = { 0 }, INF_S = 0x00;

	if (MyDeviceDetected == TRUE)
	{
		if (ReaderHandle == 110)
		{
			if (Card_state == TRUE)pcb_blc = 0x00;  //重置卡片分组号
			else pcb_blc = 0x01;

			if ((resp_ATS[4] & 0x02) == 0x02) CID[0] = 0x01;//CID标志位
			else CID[0] = 0x02;
			//if((resp_ATS[4]&0x01) == 0x01) CID[2] = 0x01;//NAD标志位
			//else CID[2] = 0x02;

			if (pcb_blc == 0x00)
			{
				while (1)
				{
					memcpy(sBuf_Cmd, send_buf, send_len);//发送数据放入缓存区

					CID[4] = 0x00; CID[5] = 0x00; CID[6] = 0x01; CID[7] = 0x00;
					iread = I_Block(CID, send_len, sBuf_Cmd, rBuf_Cmd);//发送I(0)0

					memset(sBuf_Cmd, 0, 1024);//清空缓存区

					if (iread<0)
					{
						if (iread == -6)
						{
							*recv_len = 0x03;
							return iread;//返回错误值
						}
						else return iread;//返回错误值
					}

					if (CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)//返回I(0)0
					{
						Card_state = FALSE;

						memcpy(recv_buf, rBuf_Cmd, iread);
						*recv_len = iread;
						return IFD_ICC_OK;
					}
					else if (CID[8] == 0xfa || CID[8] == 0xf2)//返回S(WTX)request(带CID)-Waiting time extension-Scenario 2
					{
						while (1)//2016.7.12修正(hg)：WTXM为卡片返回的S_Block中的INF域,无限循环发送
						{
							INF_S = rBuf_Cmd[0] & 0x3F;//WTXM

							CID[4] = 0x00; CID[5] = 0x00; CID[6] = 0x01; CID[7] = 0x02;
							memset(rBuf_Cmd, 0, iread);

							iread = S_Block(CID, INF_S, rBuf_Cmd);//发送S(WTX)

							if (iread<0)
							{
								if (iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if (CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)
							{//返回I(0)0
								Card_state = FALSE;

								memcpy(recv_buf, rBuf_Cmd, iread);
								*recv_len = iread;
								return IFD_ICC_OK;
							}
							else if (CID[8] == 0xfa || CID[8] == 0xf2)
							{//返回WTX
								continue;
							}
							else
							{//卡片无响应
								while (1)
								{
									CID[4] = 0x02; CID[5] = 0x00; CID[6] = 0x00; CID[7] = 0x00;
									memset(rBuf_Cmd, 0, iread);

									iread = R_Block(CID, rBuf_Cmd);//发送R(NAK)0

									if (iread<0)
									{
										if (iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}
									if (CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)//返回I(0)0--Scenario 8
									{
										Card_state = FALSE;

										memcpy(recv_buf, rBuf_Cmd, iread);
										*recv_len = iread;
										return IFD_ICC_OK;
									}
									else if (CID[8] == 0xfa || CID[8] == 0xf2)break;//--Scenario 10
									else continue;//--Scenario 9
								}
							}
						}
					}
					else if (CID[8] == 0x1a || CID[8] == 0x12 || CID[8] == 0x1e || CID[8] == 0x16)//返回I(1)0-PICC uses chaining-Scenario 5
					{
						int iread_k = 0;
						unsigned char recv_piccbuf[1024] = { 0 };

						for (int iread_1 = 0; iread_1<iread; iread_1++)//将返回值保存
						{
							recv_piccbuf[iread_k] = rBuf_Cmd[iread_1];
							iread_k++;
						}
						while (1)
						{//PICC使用链接，读卡器无限轮询R(ACK)1
							CID[4] = 0x01; CID[5] = 0x01; CID[6] = 0x00; CID[7] = 0x00;
							memset(recv_piccbuf, 0, iread);

							iread = R_Block(CID, recv_piccbuf);//发送R(ACK)1

							if (iread<0)
							{//此时PCD出现意外错误
								if (iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if (CID[8] == 0x0b || CID[8] == 0x03 || CID[8] == 0x0f || CID[8] == 0x07)//返回I(0)1
							{
								for (int iread_1 = 0; iread_1<iread; iread_1++)//将返回值保存(翻转)
								{
									rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
									iread_k++;
								}
								Card_state = TRUE;

								memcpy(recv_buf, rBuf_Cmd, iread_k);
								*recv_len = iread_k;
								return IFD_ICC_OK;
							}
							else if (CID[8] == 0x1b || CID[8] == 0x13 || CID[8] == 0x1f || CID[8] == 0x17)//返回I(1)1
							{
								for (int iread_1 = 0; iread_1<iread; iread_1++)//将返回值保存(翻转)
								{
									rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
									iread_k++;
								}
								while (1)
								{//返回I(1)1,PCD向PICC无限轮询R(ACK)0
									CID[4] = 0x01; CID[5] = 0x00; CID[6] = 0x00; CID[7] = 0x00;
									memset(recv_piccbuf, 0, iread);

									iread = R_Block(CID, recv_piccbuf);//发送R(ACK)0

									if (iread<0)
									{
										if (iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}
									if (CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)
									{//正确的返回I(0)0
										for (int iread_1 = 0; iread_1<iread; iread_1++)//将返回值保存(翻转)
										{
											rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
											iread_k++;
										}
										Card_state = FALSE;
										//将PCD的块号置1
										memcpy(recv_buf, rBuf_Cmd, iread_k);
										*recv_len = iread_k;
										return IFD_ICC_OK;
									}
									else continue;//返回除I(0)0以外的其他任何错误开端域，都将认为错误。
								}
							}
							else continue;//返回除I(1)1以外的其他任何错误开端域，都将认为错误,重新发送R(ACK)1。
						}
					}
					else //无返回-time out
					{
						while (1)
						{//PICC无返回时，PCD向PICC无限轮询R(NAK)0
							CID[4] = 0x02; CID[5] = 0x00; CID[6] = 0x00; CID[7] = 0x00;
							memset(rBuf_Cmd, 0, iread);

							iread = R_Block(CID, rBuf_Cmd);//发送R(NAK)0

							if (iread<0)
							{//此时PCD出现意外错误
								if (iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if (CID[8] == 0xab || CID[8] == 0xa3) break;//返回R(ACK)1，跳出循环，重新发送I(0)0--Scenario 6
							else if (CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)//返回I(0)0--Scenario 8
							{
								Card_state = FALSE;

								memcpy(recv_buf, rBuf_Cmd, iread);
								*recv_len = iread;
								return IFD_ICC_OK;
							}
							else if (CID[8] == 0xfa || CID[8] == 0xf2)//返回S（WTX）--Scenario 10
							{
								while (1)
								{
									INF_S = rBuf_Cmd[0] & 0x3F;//WTXM

									CID[4] = 0x00; CID[5] = 0x00; CID[6] = 0x01; CID[7] = 0x02;
									memset(rBuf_Cmd, 0, iread);

									iread = S_Block(CID, INF_S, rBuf_Cmd);//发送S(WTX)

									if (iread<0)
									{
										if (iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}
									if (CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)//返回I(0)0
									{
										Card_state = FALSE;

										memcpy(recv_buf, rBuf_Cmd, iread);
										*recv_len = iread;
										return IFD_ICC_OK;
									}
									else break;//S（WTX）无响应，跳出WTX循环，重新发送R(NAK)0
								}
							}
							else continue;//R(NAK)0无返回，继续发送R（NAK）0
						}
					}
				}
			}
			else//发送I(0)1
			{
				while (1)
				{
					memcpy(sBuf_Cmd, send_buf, send_len);//发送数据放入缓存区
					CID[4] = 0x00; CID[5] = 0x01; CID[6] = 0x01; CID[7] = 0x00;
					iread = I_Block(CID, send_len, sBuf_Cmd, rBuf_Cmd);//发送I(0)1

					if (iread<0)
					{
						if (iread == -6)
						{
							*recv_len = 0x03;
							return iread;//返回错误值
						}
						else return iread;//返回错误值
					}
					if (CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1
					{
						Card_state = TRUE;

						memcpy(recv_buf, rBuf_Cmd, iread);
						*recv_len = iread;
						return IFD_ICC_OK;
					}
					else if (CID[8] == 0xfa || CID[8] == 0xf2)//返回S(WTX)request-Waiting time extension-Scenario 2
					{
						while (1)
						{
							INF_S = rBuf_Cmd[0] & 0x3F;//WTXM

							CID[4] = 0x00; CID[5] = 0x00; CID[6] = 0x01; CID[7] = 0x02;
							memset(rBuf_Cmd, 0, iread);

							iread = S_Block(CID, INF_S, rBuf_Cmd);//发送S(WTX)

							if (iread<0)
							{
								if (iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if (CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1
							{
								Card_state = TRUE;

								memcpy(recv_buf, rBuf_Cmd, iread);
								*recv_len = iread;
								return IFD_ICC_OK;
							}
							else if (CID[8] == 0xfa || CID[8] == 0xf2)
							{//返回S(WTX)
								continue;
							}
							else
							{
								while (1)
								{
									CID[4] = 0x02; CID[5] = 0x01; CID[6] = 0x00; CID[7] = 0x00;
									memset(rBuf_Cmd, 0, iread);

									iread = R_Block(CID, rBuf_Cmd);//发送R(NAK)1

									if (iread<0)
									{
										if (iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}
									if (CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1--Scenario 8
									{
										Card_state = TRUE;

										memcpy(recv_buf, rBuf_Cmd, iread);
										*recv_len = iread;
										return IFD_ICC_OK;
									}
									else if (CID[8] == 0xfa || CID[8] == 0xf2)break;//--Scenario 10
									else continue;//--Scenario 9
								}
							}
						}
					}
					else if (CID[8] == 0x1b || CID[8] == 0x13 || CID[8] == 0x1f || CID[8] == 0x17)//返回I(1)1-PICC uses chaining-Scenario 5
					{
						int iread_k = 0;
						unsigned char recv_piccbuf[1024] = { 0 };

						for (int iread_1 = 0; iread_1<iread; iread_1++)//将返回值保存
						{
							recv_piccbuf[iread_k] = rBuf_Cmd[iread_1];
							iread_k++;
						}
						while (1)
						{
							CID[4] = 0x01; CID[5] = 0x00; CID[6] = 0x00; CID[7] = 0x00;
							memset(recv_piccbuf, 0, iread);

							iread = R_Block(CID, recv_piccbuf);//发送R(ACK)0

							if (iread<0)
							{
								if (iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if (CID[8] == 0x0a || CID[8] == 0x02 || CID[8] == 0x0e || CID[8] == 0x06)//返回I(0)0
							{
								for (int iread_1 = 0; iread_1<iread; iread_1++)//将返回值保存(翻转)
								{
									rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
									iread_k++;
								}
								Card_state = FALSE;

								memcpy(recv_buf, rBuf_Cmd, iread);
								*recv_len = iread_k;
								return IFD_ICC_OK;
							}
							else if (CID[8] == 0x1a || CID[8] == 0x12 || CID[8] == 0x1e || CID[8] == 0x16)//返回I(1)0,需要接收后续信息
							{
								for (int iread_1 = 0; iread_1<iread; iread_1++)//将返回值保存(翻转)
								{
									rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
									iread_k++;
								}
								while (1)
								{
									CID[4] = 0x01; CID[5] = 0x01; CID[6] = 0x00; CID[7] = 0x00;
									memset(recv_piccbuf, 0, iread);

									iread = R_Block(CID, recv_piccbuf);//发送R(ACK)1

									if (iread<0)
									{
										if (iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}
									if (CID[8] == 0x1b || CID[8] == 0x13 || CID[8] == 0x1f || CID[8] == 0x17)//返回I(1)1
									{
										for (int iread_1 = 0; iread_1<iread; iread_1++)//将返回值保存(翻转)
										{
											rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
											iread_k++;
										}
										break;
									}
									else if (CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1
									{
										for (int iread_1 = 0; iread_1<iread; iread_1++)//将返回值保存(翻转)
										{
											rBuf_Cmd[iread_k] = recv_piccbuf[iread_1];
											iread_k++;
										}
										Card_state = TRUE;

										memcpy(recv_buf, rBuf_Cmd, iread_k);
										*recv_len = iread_k;
										return IFD_ICC_OK;
									}
									else continue;//无返回
								}
							}
							else continue;
						}
						return IFD_ICC_ERROR;
					}
					else //无返回-time out
					{
						while (1)
						{
							CID[4] = 0x02; CID[5] = 0x01; CID[6] = 0x00; CID[7] = 0x00;
							memset(rBuf_Cmd, 0, iread);

							iread = R_Block(CID, rBuf_Cmd);//发送R(NAK)1

							if (iread<0)
							{
								if (iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if (CID[8] == 0xaa || CID[8] == 0xa2) break;//返回R(ACK)0--Scenario 6
							else if (CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1--Scenario 8
							{
								Card_state = FALSE;

								memcpy(recv_buf, rBuf_Cmd, iread);
								*recv_len = iread;
								return IFD_ICC_OK;
							}
							else if (CID[8] == 0xfa || CID[8] == 0xf2)//--Scenario 10
							{
								while (1)
								{
									INF_S = rBuf_Cmd[0] & 0x3F;//WTXM

									CID[4] = 0x00; CID[5] = 0x00; CID[6] = 0x01; CID[7] = 0x02;
									memset(rBuf_Cmd, 0, iread);

									iread = S_Block(CID, INF_S, rBuf_Cmd);//发送S(WTX)

									if (iread<0)
									{
										if (iread == -6)
										{
											*recv_len = 0x03;
											return iread;//返回错误值
										}
										else return iread;//返回错误值
									}

									if (CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1
									{
										Card_state = TRUE;

										memcpy(recv_buf, rBuf_Cmd, iread);
										*recv_len = iread;
										return IFD_ICC_OK;
									}
									else
									{
										while (1)
										{
											CID[4] = 0x02; CID[5] = 0x01; CID[6] = 0x00; CID[7] = 0x00;
											memset(rBuf_Cmd, 0, iread);

											iread = R_Block(CID, rBuf_Cmd);//发送R(NAK)1

											if (iread<0)
											{
												if (iread == -6)
												{
													*recv_len = 0x03;
													return iread;//返回错误值
												}
												else return iread;//返回错误值
											}
											if (CID[8] == 0x0b || CID[8] == 0x0f || CID[8] == 0x07 || CID[8] == 0x03)//返回I(0)1--Scenario 8
											{
												Card_state = TRUE;

												memcpy(recv_buf, rBuf_Cmd, iread);
												*recv_len = iread;
												return IFD_ICC_OK;
											}
											else if (CID[8] == 0xfa || CID[8] == 0xf2)break;//--Scenario 10
											else continue;//--Scenario 9
										}
									}
								}
							}
							else continue;
						}
					}
				}
			}
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_pro_halt(long ReaderHandle,unsigned char* recv_buf)//发送DESELECT
{
	int iread=0;
	unsigned char Response[1024] = {0},send_buffer[128] = {0},INF = 0x00;

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			if((resp_ATS[4]&0x02) == 0x02) CID[0] = 0x01;//CID标志位
			else CID[0] = 0x02;

			memset(resp_ATS,0,1024);

			while(1)
			{
				CID[4] = 0x00;CID[5] = 0x00;CID[6] = 0x00;CID[7] = 0x01;
				iread = S_Block(CID,INF,recv_buf);//发送S(DESELECT)

				if(iread <0)return iread;//返回错误值
				if(recv_buf[0] == 0xca || CID[8] == 0xc2) return IFD_ICC_OK;//返回S(DESELECT)
				else continue;
			}
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_pro_commandlink(long ReaderHandle,
							      unsigned char send_len,
							      unsigned char* send_buf,
								  unsigned int* recv_len,
							      unsigned char* recv_buf,
							      unsigned char FG)//此函数包含链接！
{
	int iread=0;
	int k = 0, FSD = 0;
	unsigned char Response[1024] = {0},send_buffer[1024] = {0},send_buf_link[1024] = {0};

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110) 
		{
			if((resp_ATS[4]&0x02) == 0x02) CID[0] = 0x01;//CID标志位
			else CID[0] = 0x02;
			//if((resp_ATS[4]&0x01) == 0x01) CID[2] = 0x01;//NAD标志位
			//else CID[2] = 0x02;

			if(Card_state == TRUE) pcb_blc = 0x00;
			else pcb_blc = 0x01;

			if(FG<0x00)
			{
				AfxMessageBox("请输入正确分割长度（大于或等于0）!");
				return IFD_ICC_ERROR;
			}
			if(FG >= send_len && send_len<=FSD)//判断不分割数据
			{
				memcpy(send_buf_link,send_buf,send_len);
				iread = hy_pro_command(ReaderHandle,send_len,recv_len,send_buf_link,recv_buf);
				if(iread<0)
				{
					if(iread == -6)
					{
						*recv_len = 0x03;
						return iread;//返回错误值
					}
					else return iread;//返回错误值
				}

				memset(send_buf_link,0,1024);
				*recv_len = iread;
				return IFD_ICC_OK;
			}
			for(int i_i = 0;i_i<(send_len/FG);i_i++)
			{
				if(k >= (send_len-(send_len%FG)))break;
				for(int i_i1 = 0;i_i1<FG;i_i1++)
				{
					send_buffer[i_i1] = send_buf[k];
					k++;
				}
				if(pcb_blc == 0x00)
				{
					CID[4] = 0x00;CID[5] = 0x00;CID[6] = 0x02;CID[7] = 0x00;
					iread = I_Block(CID,FG,send_buffer,recv_buf);//发送I(1)0
					if(iread<0)
					{
						if(iread == -6)
						{
							*recv_len = 0x03;
							return iread;//返回错误值
						}
						else return iread;//返回错误值
					}
					memset(send_buffer,0,FG);
				}
				else
				{
					CID[4] = 0x00;CID[5] = 0x01;CID[6] = 0x02;CID[7] = 0x00;
					iread = I_Block(CID,FG,send_buffer,recv_buf);//发送I(1)1
					if(iread<0)
					{
						if(iread == -6)
						{
							*recv_len = 0x03;
							return iread;//返回错误值
						}
						else return iread;//返回错误值
					}
					memset(send_buffer,0,FG);
				}

				if(CID[8] == 0xaa || CID[8] == 0xa2)//返回R(ACK)0
				{
					Card_state = FALSE;
					pcb_blc = 0x01;
					continue;

				}
				else if(CID[8] == 0xab || CID[8] == 0xa3)//返回R(ACK)1
				{
					Card_state = TRUE;
					pcb_blc = 0x00;
					continue;
				}
				else//没有返回
				{
					if(pcb_blc == 0x00)
					{
						while(1)
						{
							CID[4] = 0x02;CID[5] = 0x00;CID[6] = 0x00;CID[7] = 0x00;
							iread = R_Block(CID,recv_buf);//发送R(NAK)0

							if(iread<0)
							{
								if(iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if(CID[8] == 0xaa || CID[8] == 0xa2)//返回R(ACK)0
							{
								Card_state = FALSE;
								pcb_blc = 0x01;
								break;
							}
							else continue;
						}
					}
					else
					{
						while(1)
						{
							CID[4] = 0x02;CID[5] = 0x01;CID[6] = 0x00;CID[7] = 0x00;
							iread = R_Block(CID,recv_buf);//发送R(NAK)1

							if(iread<0)
							{
								if(iread == -6)
								{
									*recv_len = 0x03;
									return iread;//返回错误值
								}
								else return iread;//返回错误值
							}

							if(CID[8] == 0xab || CID[8] == 0xa3)//返回R（ACK）1
							{
								Card_state = TRUE;
								pcb_blc = 0x00;
								break;
							}
							else continue;
						}
					}
				}
			}
			FG = send_len%FG;
			for(int i_i2 = 0;i_i2<FG;i_i2++)
			{
				send_buffer[i_i2] = send_buf[k];
				k++;
			}
			iread = hy_pro_command(ReaderHandle,FG,recv_len,send_buffer,recv_buf);

			if(iread<0)
			{
				if(iread == -6)
				{
					*recv_len = 0x03;
					return iread;//返回错误值
				}
				else return iread;//返回错误值
			}
			*recv_len = iread;
			return IFD_ICC_OK;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_pro_commanddirect(long ReaderHandle,
									unsigned char send_len,
									unsigned char* send_buf,
									unsigned char* recv_len,
									unsigned char* recv_buf,
									unsigned long delay_ms)//此函数不封装，需用户自行定义  Modify unsigned char delay_ms-->unsigned long delay_ms by gHan 2016.8.23
{
	int iread=0,i;
	unsigned char send_buffer[1024]={0};

	send_buffer[0]=0x48;
	send_buffer[1]=0xf0;
	send_buffer[2]=send_len*8;
	send_buffer[3]=0x00;
	send_buffer[4]=0x00;
	send_buffer[5]=send_len;
	for(i=0;i<send_len;i++)
		send_buffer[i+6]=send_buf[i];

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110)  
		{
			UsbWrite(send_buffer,send_len+6);

			memset(send_buffer,0,1024);	//读取数组内容

			iread=UsbRead_14443(recv_buf, delay_ms);

			if(iread<0)
			{
				if(iread == -6)
				{
					*recv_len = 0x03;
					return iread;//返回错误值
				}
				else return iread;//返回错误值
			}

			*recv_len = iread;

			//if(recv_buf[3] == 0xff&&recv_buf[4] == 0xff) return IFD_ICC_ERROR;//执行操作失败
			 return IFD_ICC_OK;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_pro_commanddirect_judgecrc(
	long ReaderHandle,
	unsigned char parity_CRC,
	unsigned char send_len,
	unsigned char* send_buf,
	unsigned char* recv_len,
	unsigned char* recv_buf,
	unsigned long delay_ms)//此函数不封装，需用户自行定义,用于CRC造错 by gHan 2016.11.28
{
	int iread = 0, i;
	unsigned char send_buffer[1024] = { 0 };

	send_buffer[0] = 0x48;
	send_buffer[1] = parity_CRC;
	send_buffer[2] = send_len * 8;
	send_buffer[3] = 0x00;
	send_buffer[4] = 0x00;
	send_buffer[5] = send_len;
	for (i = 0; i<send_len; i++)
		send_buffer[i + 6] = send_buf[i];

	if (MyDeviceDetected == TRUE)
	{
		if (ReaderHandle == 110)
		{
			UsbWrite(send_buffer, send_len + 6);

			memset(send_buffer, 0, 1024);	//读取数组内容

			iread = UsbRead_14443(recv_buf, delay_ms);

			if (iread<0)
			{
				if (iread == -6)
				{
					*recv_len = 0x03;
					return iread;//返回错误值
				}
				else return iread;//返回错误值
			}

			*recv_len = iread;

			//if(recv_buf[3] == 0xff&&recv_buf[4] == 0xff) return IFD_ICC_ERROR;//执行操作失败
			return IFD_ICC_OK;
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

//-------------------------------------------------------------------------------------------------------------------------

BOOL __stdcall hy_clearBuffertem()//清空缓存区
{
	memset(buffertem,0,1024);
	gcount=0;
	return TRUE;
}

//---------------------------------------------------------显示数据---------------------------------------------------------

BOOL __stdcall hy_SeeData(unsigned char* SeeData_send,unsigned char* SeeData_receive,unsigned char* Seenumber)
{
	int number_send=0, number_iread=0;
	number_iread=Iread;
	number_send=number_Send;
	Seenumber[0]=number_send+3;
    Seenumber[1]=number_iread+3;
	for(int i=0;i<(number_Send+3);i++)
	{
		SeeData_send[i]=SeeData_Send[i];
	}
	for(int i=0;i<(number_iread+4);i++)
	{
        SeeData_receive[i]=SeeData_Receive[i];
	}
	memset(SeeData_Send,0,1024);
    memset(SeeData_Receive,0,1024);
	return TRUE;
}

//-----------------------------------------------------------ISO15693部分------------------------------------------------------

long __stdcall hy_15693_Reset(long ReaderHandle,
							  unsigned char* recv_buffer,
							  unsigned long resp_time)
{
	int iread=0;
	unsigned char buffer[1] = {0x10};

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110)  
		{     
	          UsbWrite(buffer,1);
			  
			  memset(buffer,0,sizeof(buffer));	
	
	          iread=UsbRead_15693(recv_buffer,resp_time);

			  switch(iread)
			  {
			  case 0:return IFD_ICC_OK;//复位成功
			  case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
			  default:return iread;
			  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_EOF(long ReaderHandle,
							unsigned char* recv_buffer,
							unsigned long resp_time)
{
	int iread=0;
	unsigned char buffer[1] = {0x11};

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
	          UsbWrite(buffer,1);
			  
			  memset(buffer,0,sizeof(buffer));
			  
			  iread=UsbRead_15693(recv_buffer,resp_time);
		  
			  return iread;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_DirectCmd(long ReaderHandle,												
								  long len,
								  unsigned char* recv_buffer,
								  unsigned char* CRC,
								  unsigned char* Send_buffer,
								  unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x48;
	buffer[1]=CRC[0];
	for(i=0;i<len;i++)
	   buffer[i+2]=Send_buffer[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,len+2);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		  case 0:return IFD_ICC_OK;//复位成功
		  case -6:return IFD_ICC_ERROR;       //执行错误
		  case -4:return IFD_ICC_NoResponse;//卡片无应答
		  default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Inventory(long ReaderHandle,										 
								  long len,
								  unsigned char* recv_buffer,
								  unsigned char* CRC,
								  unsigned char* flag,
						          unsigned char* Inventory_data,
								  unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};
	buffer[0]=0x01;                              //命令代码
    buffer[1]=CRC[0];  
	buffer[2]=flag[0];                     //标志位
	for(i=0;i<len;i++)   //数据
	   buffer[i+3]=Inventory_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
	      UsbWrite(buffer,len+3);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);
          
		  return iread;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Stay_Quiet(long ReaderHandle,
								   unsigned char* recv_buffer,
								   unsigned char* Stay_Quiet_CRC,
								   unsigned char* Stay_Quiet_flag,
								   unsigned char* UID,
								   unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x02;                              //命令代码
	buffer[1]=Stay_Quiet_CRC[0];
	buffer[2]=Stay_Quiet_flag[0];  
	for(i=0;i<8;i++)   //数据
	   buffer[i+3]=UID[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
	      UsbWrite(buffer,11);//读取数组内容

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//执行成功
		      case -6:return IFD_ICC_ERROR;//执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Read_Single(long ReaderHandle,
									long Lenth_of_Read_Single_buffer,
									unsigned char* recv_buffer,
									unsigned char* Read_Single_CRC,
									unsigned char* Read_Single_flag,
									unsigned char* Read_Single_data,
									unsigned char* Read_Single_block_number,
									unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x20;                              //命令代码
	buffer[1]=Read_Single_CRC[0];
	buffer[2]=Read_Single_flag[0];
	buffer[3]=Read_Single_block_number[0];
	for(i=0;i<Lenth_of_Read_Single_buffer;i++)   //数据
	   buffer[i+4]=Read_Single_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Read_Single_buffer+4);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  return iread;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Write_Single(long ReaderHandle,
									 long Lenth_of_Write_Single_buffer,
									 unsigned char* recv_buffer,
									 unsigned char* Write_Single_CRC,
									 unsigned char* Write_Single_flag,
									 unsigned char* Write_Single_data,
									 unsigned char* Write_Single_block_number,
									 unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x21;                              //命令代码
	buffer[1]=Write_Single_CRC[0];
	buffer[2]=Write_Single_flag[0];
	buffer[3]=Write_Single_block_number[0];
	for(i=0;i<Lenth_of_Write_Single_buffer;i++)   //数据
	   buffer[i+4]=Write_Single_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Write_Single_buffer+4);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case  0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Lock_Block(long ReaderHandle,											 
								   long Lenth_of_Lock_Block_buffer,
								   unsigned char* recv_buffer,
								   unsigned char* Lock_Block_CRC,
								   unsigned char* Lock_Block_flag,
								   unsigned char* Lock_Block_data,
								   unsigned char* Lock_Block_block_number,
								   unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x22;                              //命令代码
	buffer[1]=Lock_Block_CRC[0];
	buffer[2]=Lock_Block_flag[0];
	buffer[3]=Lock_Block_block_number[0];
	for(i=0;i<Lenth_of_Lock_Block_buffer;i++)   //数据
	   buffer[i+4]=Lock_Block_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Lock_Block_buffer+4);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Read_Mulblock(long ReaderHandle,
									  long Lenth_of_Read_Mulblock_buffer,
									  unsigned char* recv_buffer,
									  unsigned char* Read_Mulblock_CRC,
									  unsigned char* Read_Mulblock_flag,
									  unsigned char* Read_Mulblock_data,
									  unsigned char* Read_Mulblock_block_quantity,
									  unsigned char* Read_Mulblock_block_number,
									  unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x23;                              //命令代码
	buffer[1]=Read_Mulblock_CRC[0];
	buffer[2]=Read_Mulblock_flag[0];
	buffer[3]=Read_Mulblock_block_number[0];
	buffer[4]=Read_Mulblock_block_quantity[0];
	for(i=0;i<Lenth_of_Read_Mulblock_buffer;i++)   //数据
	   buffer[i+5]=Read_Mulblock_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Read_Mulblock_buffer+5);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  return iread;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Select_15693(long ReaderHandle,
									 long Lenth_of_Select_15693_buffer,
									 unsigned char* recv_buffer,
									 unsigned char* Select_15693_CRC,
									 unsigned char* Select_15693_flag,
									 unsigned char* Select_15693_data,
									 unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x25;                              //命令代码
    buffer[1]=Select_15693_CRC[0];  
	buffer[2]=Select_15693_flag[0];                     //标志位
	for(i=0;i<Lenth_of_Select_15693_buffer;i++)   //数据
	   buffer[i+3]=Select_15693_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Select_15693_buffer+3);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Reset_to_Ready(long ReaderHandle,
									   long Lenth_of_Reset_to_Ready_buffer,
									   unsigned char* recv_buffer,
									   unsigned char* Reset_to_Ready_CRC,
									   unsigned char* Reset_to_Ready_flag,
									   unsigned char* Reset_to_Ready_data,
									   unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x26;                              //命令代码
    buffer[1]=Reset_to_Ready_CRC[0];  
	buffer[2]=Reset_to_Ready_flag[0];                     //标志位
	for(i=0;i<Lenth_of_Reset_to_Ready_buffer;i++)   //数据
	   buffer[i+3]=Reset_to_Ready_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110)   
		{
		
	      UsbWrite(buffer,Lenth_of_Reset_to_Ready_buffer+3);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Write_AFI(long ReaderHandle,
								  long Lenth_of_Write_AFI_buffer,
								  unsigned char* recv_buffer,
								  unsigned char* Write_AFI_CRC,
								  unsigned char* Write_AFI_flag,
								  unsigned char* Write_AFI_data,
								  unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x27;                              //命令代码
    buffer[1]=Write_AFI_CRC[0];  
	buffer[2]=Write_AFI_flag[0];                     //标志位
	for(i=0;i<Lenth_of_Write_AFI_buffer;i++)   //数据
	   buffer[i+3]=Write_AFI_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Write_AFI_buffer+3);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Lock_AFI(long ReaderHandle,
								 long Lenth_of_Lock_AFI_buffer,
								 unsigned char* recv_buffer,
								 unsigned char* Lock_AFI_CRC,
								 unsigned char* Lock_AFI_flag,
								 unsigned char* Lock_AFI_data,
								 unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x28;                              //命令代码
    buffer[1]=Lock_AFI_CRC[0];  
	buffer[2]=Lock_AFI_flag[0];                     //标志位
	for(i=0;i<Lenth_of_Lock_AFI_buffer;i++)   //数据
	   buffer[i+3]=Lock_AFI_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110)  
		{
		
	      UsbWrite(buffer,Lenth_of_Lock_AFI_buffer+3);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Write_DSFID(long ReaderHandle,
								    long Lenth_of_Write_DSFID_buffer,
									unsigned char* recv_buffer,
									unsigned char* Write_DSFID_CRC,
									unsigned char* Write_DSFID_flag,
									unsigned char* Write_DSFID_data,
									unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x29;                              //命令代码
    buffer[1]=Write_DSFID_CRC[0];  
	buffer[2]=Write_DSFID_flag[0];                     //标志位
	for(i=0;i<Lenth_of_Write_DSFID_buffer;i++)   //数据
	   buffer[i+3]=Write_DSFID_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Write_DSFID_buffer+3);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Lock_DSFID(long ReaderHandle,
								   long Lenth_of_Lock_DSFID_buffer,
								   unsigned char* recv_buffer,
								   unsigned char* Lock_DSFID_CRC,
								   unsigned char* Lock_DSFID_flag,
								   unsigned char* Lock_DSFID_data,
								   unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x2a;                              //命令代码
    buffer[1]=Lock_DSFID_CRC[0];  
	buffer[2]=Lock_DSFID_flag[0];                     //标志位
	for(i=0;i<Lenth_of_Lock_DSFID_buffer;i++)   //数据
	   buffer[i+3]=Lock_DSFID_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Lock_DSFID_buffer+3);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Get_SystemInfo(long ReaderHandle,
									   long Lenth_of_Get_SystemInfo_buffer,
									   unsigned char* recv_buffer,
									   unsigned char* Get_SystemInfo_CRC,
									   unsigned char* Get_SystemInfo_flag,
									   unsigned char* Get_SystemInfo_data,
									   unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x2b;                              //命令代码
    buffer[1]=Get_SystemInfo_CRC[0];  
	buffer[2]=Get_SystemInfo_flag[0];                     //标志位
	for(i=0;i<Lenth_of_Get_SystemInfo_buffer;i++)   //数据
	   buffer[i+3]=Get_SystemInfo_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Get_SystemInfo_buffer+3);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  return iread;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Get_Mulblock_Security(long ReaderHandle,
											  long Lenth_of_Get_Mulblock_Security_buffer,
											  unsigned char* recv_buffer,
											  unsigned char* Get_Mulblock_Security_CRC,
											  unsigned char* Get_Mulblock_Security_flag,
											  unsigned char* Get_Mulblock_Security_data,
											  unsigned char* Get_Mulblock_Security_block_quantity,
											  unsigned char* Get_Mulblock_Security_block_number,
											  unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0x2c;                              //命令代码
	buffer[1]=Get_Mulblock_Security_CRC[0];
	buffer[2]=Get_Mulblock_Security_flag[0];
	buffer[3]=Get_Mulblock_Security_block_number[0];
	buffer[4]=Get_Mulblock_Security_block_quantity[0];
	for(i=0;i<Lenth_of_Get_Mulblock_Security_buffer;i++)   //数据
	   buffer[i+5]=Get_Mulblock_Security_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Get_Mulblock_Security_buffer+5);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  return iread;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Write_2_Blocks(long ReaderHandle,
									   long Lenth_of_Write_2_Blocks_buffer,
									   unsigned char* recv_buffer,
									   unsigned char* Write_2_Blocks_CRC,
									   unsigned char* Write_2_Blocks_flag,
									   unsigned char* Write_2_Blocks_data,
									   unsigned char* Write_2_Blocks_block_number,
									   unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0xa6;                              //命令代码
	buffer[1]=Write_2_Blocks_CRC[0];
	buffer[2]=Write_2_Blocks_flag[0];
	buffer[3]=Write_2_Blocks_block_number[0];
	buffer[4]=0x23;
	for(i=0;i<Lenth_of_Write_2_Blocks_buffer;i++)   //数据
	   buffer[i+5]=Write_2_Blocks_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Write_2_Blocks_buffer+5);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Lock_2_Blocks(long ReaderHandle,
									  long Lenth_of_Lock_2_Blocks_buffer,
									  unsigned char* recv_buffer,
									  unsigned char* Lock_2_Blocks_CRC,
									  unsigned char* Lock_2_Blocks_flag,
									  unsigned char* Lock_2_Blocks_data,
									  unsigned char* Lock_2_Blocks_block_number,
									  unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0xa7;                              //命令代码
	buffer[1]=Lock_2_Blocks_CRC[0];
	buffer[2]=Lock_2_Blocks_flag[0];
	buffer[3]=Lock_2_Blocks_block_number[0];
	buffer[4]=0x23;
	for(i=0;i<Lenth_of_Lock_2_Blocks_buffer;i++)   //数据
	   buffer[i+5]=Lock_2_Blocks_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Lock_2_Blocks_buffer+5);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Inventory_Read(long ReaderHandle,
									   long Lenth_of_Inventory_Read_buffer,
									   unsigned char* recv_buffer,
									   unsigned char* Inventory_Read_CRC,
									   unsigned char* Inventory_Read_flag,
									   unsigned char* Inventory_Read_data,
									   unsigned char* Inventory_Read_block_quantity,
									   unsigned char* Inventory_Read_block_number,
									   unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0xa0;                              //命令代码
    buffer[1]=Inventory_Read_CRC[0];  
	buffer[2]=Inventory_Read_flag[0];               //标志位
	buffer[3]=Inventory_Read_block_number[0];
	buffer[4]=Inventory_Read_block_quantity[0];
	buffer[5]=0x23;
	for(i=0;i<Lenth_of_Inventory_Read_buffer;i++)   //数据
	   buffer[i+6]=Inventory_Read_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Inventory_Read_buffer+6);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);
          
		  return iread;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Fast_Inventory_Read(long ReaderHandle,
											long Lenth_of_Fast_Inventory_Read_buffer,
											unsigned char* recv_buffer,
											unsigned char* Fast_Inventory_Read_CRC,
											unsigned char* Fast_Inventory_Read_flag,
											unsigned char* Fast_Inventory_Read_data,
											unsigned char* Fast_Inventory_Read_block_quantity,
											unsigned char* Fast_Inventory_Read_block_number,
											unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	buffer[0]=0xa1;                              //命令代码
    buffer[1]=Fast_Inventory_Read_CRC[0];  
	buffer[2]=Fast_Inventory_Read_flag[0];               //标志位
	buffer[3]=Fast_Inventory_Read_block_number[0];
	buffer[4]=Fast_Inventory_Read_block_quantity[0];
	buffer[5]=0x23;
	for(i=0;i<Lenth_of_Fast_Inventory_Read_buffer;i++)   //数据
	   buffer[i+6]=Fast_Inventory_Read_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_Fast_Inventory_Read_buffer+6);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);
          
		  return iread;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_EAS(long ReaderHandle,
							long Lenth_of_EAS_buffer,
							unsigned char* recv_buffer,
							unsigned char* EAS_CRC,
							unsigned char* EAS_flag,
							unsigned char* EAS_data,
							int choice,
							unsigned long resp_time)
{
    int iread=0,i;
	unsigned char buffer[1024]={0};

	if(choice==2) buffer[0]=0xa2;                              //命令代码
	else if(choice==3) buffer[0]=0xa3;
	else if(choice==4) buffer[0]=0xa4;
	else buffer[0]=0xa5;
    buffer[1]=EAS_CRC[0];  
	buffer[2]=EAS_flag[0];                //标志位
	buffer[3]=0x23;
	for(i=0;i<Lenth_of_EAS_buffer;i++)   //数据
	   buffer[i+4]=EAS_data[i];

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,Lenth_of_EAS_buffer+4);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Set_to_Test_State(long ReaderHandle,
										  unsigned char* recv_buffer,
										  unsigned char* Set_to_Test_State_CRC,
										  unsigned char* Set_to_Test_State_flag,
										  unsigned long resp_time)
{
    int iread=0;
	unsigned char buffer[1024]={0};

	buffer[0]=0xe0;                              //命令代码
    buffer[1]=Set_to_Test_State_CRC[0];  
	buffer[2]=Set_to_Test_State_flag[0];                //标志位
	buffer[3]=0x23;
    buffer[4]=0x8a;

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110)   
		{
		
	      UsbWrite(buffer,5);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Factory_Lock(long ReaderHandle,
									 unsigned char* recv_buffer,
									 unsigned char* Factory_Lock_CRC,
									 unsigned char* Factory_Lock_flag,
									 unsigned char* Factory_Lock_block_number,
									 unsigned long resp_time)
{
    int iread=0;
	unsigned char buffer[1024]={0};

	buffer[0]=0xe2;                              //命令代码
    buffer[1]=Factory_Lock_CRC[0];  
	buffer[2]=Factory_Lock_flag[0];               //标志位
	buffer[3]=Factory_Lock_block_number[0];
	buffer[4]=0x23;

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,5);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);
          
		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_15693_Write_all(long ReaderHandle,
								  unsigned char* recv_buffer,
							      unsigned char* Write_all_CRC,
								  unsigned char* Write_all_flag,
								  int choice_00_ff,
								  unsigned long resp_time)
{
    int iread=0;
	unsigned char buffer[1024]={0};

	buffer[0]=0xe1;                              //命令代码
    buffer[1]=Write_all_CRC[0];  
	buffer[2]=Write_all_flag[0];                //标志位
	buffer[3]=0x23;
	if(choice_00_ff==0) buffer[4]=0x00;
	else buffer[4]=0xff;

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110) 
		{
		
	      UsbWrite(buffer,5);

	      memset(buffer,0,sizeof(buffer));
		  
	      iread=UsbRead_15693(recv_buffer,resp_time);

		  switch(iread)
		  {
		      case 0:return IFD_ICC_OK;//复位成功
		      case -6:return IFD_ICC_ERROR;       //执行错误
			  case -4:return IFD_ICC_NoResponse;//卡片无应答
		      default:return iread;
		  }
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

//---------------------------------------------------------CPU(SAM)卡操作函数-------------------------------------------------------


long __stdcall hy_SAMreset(long ReaderHandle,unsigned char* recv_len,unsigned char* recv_buf)
{
	int iread = 0,i = 0;
	unsigned char buffer[5] = {0x16,0x03,0x00,0x03,0x0D},recv_buffer[1024] = {0};

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110)  
		{
		
	       UsbWrite(buffer,5);

	       memset(buffer,0,5);	//读取数组内容
	
	       iread=UsbRead_tch(recv_buffer);

		   if(iread < 0)return iread;

		   for(i = 0;i<iread-2;i++)//前两字节删除
		   {
			   recv_buf[i] = recv_buffer[i+2];
		   }

		   memset(recv_buffer,0,1024);

		   *recv_len = i;

		   if(recv_buffer[0] == 0x16 && recv_buffer[1] == 0x04 && recv_buffer[2] == 0x00 && recv_buffer[3] == 0x00 && recv_buffer[4] == 0x04 && recv_buffer[5] == 0x0D)return IFD_ResetError;//读卡器上电复位失败	
		   else return IFD_ICC_OK;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_SAMapdudirect(long ReaderHandle,unsigned char send_len,unsigned char* send_buf,unsigned char* recv_len,unsigned char* recv_buf)
{
    int iread=0,i = 0,h = 0,n = 0;
	unsigned char buffer[1024]={0},buffer_data[1024] = {0},recv_buffer[1024] = {0};

	buffer[0] = 0x16;
	buffer[1] = send_len+2;
	for(i = 0;i<send_len;i++)
	{
		buffer[i+2] = send_buf[i];
	}
	for(h = 1;h<send_len+1;h++)
	{
		buffer_data[0] = buffer[1];
		buffer_data[h] = buffer_data[h-1]+buffer[h+1]; 
	}
	buffer[i+2] = buffer_data[h-1];
	buffer[i+3] = 0x0D;

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110)  
		{
		
	       UsbWrite(buffer,send_len+4);
	
	       iread=UsbRead_tch(recv_buffer);

		   for(n = 0;n<iread-2;n++)
		   {
			   recv_buf[n] = recv_buffer[n+2];
		   }

		   *recv_len = n;

		   memset(buffer,0,1024);
		   memset(buffer_data,0,1024);
		   memset(recv_buffer,0,1024);

		   if(recv_buffer[0] == 0x16 && recv_buffer[1] == 0x04 && recv_buffer[2] == 0x00 && recv_buffer[3] == 0x00 && recv_buffer[4] == 0x04 && recv_buffer[5] == 0x0D)return IFD_ICC_ERROR;		//执行错误		       
		   else return IFD_ICC_OK;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_SAMsetSAM(long ReaderHandle,unsigned char SAMID)
{
	int iread = 0;
	unsigned char buffer[1024] = {0},recv_buffer[1024] = {0};

    buffer[0] = 0x16;
	buffer[1] = 0x04;
	buffer[2] = SAMID;
	buffer[3] = 0x00;
	buffer[4] = buffer[1]+buffer[2]+buffer[3];
	buffer[5] = 0x0D;

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110)  
		{
		
	       UsbWrite(buffer,6);

	       memset(buffer,0,1024);	//读取数组内容
	
	       iread=UsbRead_tch(recv_buffer);

		   if(recv_buffer[2] != 0x90)return IFD_ICC_ERROR;		//执行错误		       
		   else return IFD_ICC_OK;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_SAMsetETU(long ReaderHandle,unsigned char* Response,unsigned char _etu)
{
	int iread = 0;
	unsigned char buffer[6] = {0};

    buffer[0] = 0x16;
	buffer[1] = 0x04;
	buffer[2] = 0x01;
	buffer[3] = _etu;
	buffer[4] = buffer[1]+buffer[2]+buffer[3];
	buffer[5] = 0x0D;

	if(MyDeviceDetected == TRUE)
	{
		if(ReaderHandle==110)  
		{
			if(buffer[3]<0x03||buffer[3]>0x102) return IFD_ICC_ECUError;//ETU超出范围（3~258）

			UsbWrite(buffer,6);

			memset(buffer,0,1024);	//读取数组内容

			iread=UsbRead_tch(Response);

			if(Response[2] == 0x90) return iread;
			else return IFD_ICC_ERROR;		//执行错误
		}
		else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}

long __stdcall hy_SAMapdu(long ReaderHandle,unsigned char send_len,unsigned char* send_buf,unsigned char* recv_len,unsigned char* recv_buf)
{
    int iread=0,i = 0,h = 0,n = 0;
	unsigned char buffer[1024]={0},buffer_data[1024] = {0},recv_buffer[1024] = {0};

	buffer[0] = 0x16;
	buffer[1] = send_len+2;
	for(i = 0;i<send_len;i++)
	{
		buffer[i+2] = send_buf[i];
	}
	for(h = 1;h<send_len+1;h++)
	{
		buffer_data[0] = buffer[1];
		buffer_data[h] = buffer_data[h-1]+buffer[h+1]; 
	}
	buffer[i+2] = buffer_data[h-1];
	buffer[i+3] = 0x0D;

	if(MyDeviceDetected == TRUE)
	{
	   if(ReaderHandle==110)  
		{
		
	       UsbWrite(buffer,send_len+4);

	       memset(buffer,0,1024);	//读取数组内容
	
	       iread=UsbRead_tch(recv_buffer);

		   for(n = 0;n<iread-2;n++)
		   {
			   recv_buf[n] = recv_buffer[n+2];
		   }

		   *recv_len = n;

		   if(recv_buffer[iread-2] != 0x90)return IFD_ICC_ERROR;		//执行错误		       
		   else return IFD_ICC_OK;
	   }
       else return IFD_DeviceError;//连接错误的读卡器
	}
	else return IFD_UnConnected;//未建立连接 
}