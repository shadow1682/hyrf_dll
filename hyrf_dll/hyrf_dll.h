/****************************************************************************************************************************************************
*												(C)Copyright 1992-2016 HUA YI Microelectronics Techology Co.Ltd
*																		All Rights Reserved
*
*  System Application Team
*
*  File name			:hyrf_dll.h
*  Author				:ghan
*  Version				:V1.0.0
*  Date					:2015.04.22
*  Description			:用于HY_Reader双界面读卡器接口函数动态链接库的操作函数
*  Interface			:USB_HID
*  Stand by				:ISO/IEC14443 IS0/IEC15693 ISO/IEC7816
****************************************************************************************************************************************************/

// hyrf_dll.h : hyrf_dll DLL 的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#ifndef __RESOURCE_H__
	#include "resource.h"		// 主符号
#endif

#ifndef __DBT_H__
    #include <dbt.h>
#endif
// Chyrf_dllApp
// 有关此类实现的信息，请参阅 hyrf_dll.cpp
//

#ifndef hyrf_EXPORTS
#define hyrf_API __declspec(dllexport)
#else
#define hyrf_API __declspec(dllimport)
#endif

class Chyrf_dllApp : public CWinApp
{
public:
	Chyrf_dllApp();


// 重写
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

#ifdef __cplusplus 
extern "C" {
#endif


#define IFD_ICC_OK	             0	//执行成功
#define IFD_ICC_ERROR           -6  //执行错误

	//连接读卡器错误
#define IFD_PortError           -16 //串口连接错误
#define IFD_USBError            -17 //USB连接错误
#define IFD_DeviceError         -18 //连接错误的读卡器
#define IFD_UnConnected         -19 //未连接读卡器
#define IFD_PortOccupy          -20 //RS232端口被占用
#define IFD_CloseDevError       -21 //关闭端口出错（USB/232）
#define IFD_PortParaError       -22 //设置端口参数错误

	//读卡器操作相关错误
#define IFD_GetVerError         -23 //获取版本号错误
#define IFD_SelModeError        -24 //选择模式错误
#define IFD_ResetError          -25 //读卡器上电复位失败
#define IFD_TimeOver            -26 //超时错误
#define IFD_Dev_NoResponse      -27 //设备无应答
#define IFD_Dev_CmdError        -28 //设备执行错误

	//卡片操作相关错误
#define IFD_ICC_NoCard          -33 //无卡
#define IFD_ICC_NoResponse      -34 //卡片无应答
#define IFD_ICC_TypeError       -35 //卡片类型错误
#define IFD_ICC_NoPower         -36 //有卡未上电
#define IFD_ICC_AnticoError     -37 //读取卡片UID错误（抗冲突错误）
#define IFD_ICC_InitError       -38 //卡片初始化错误
#define IFD_ICC_ParaError       -39 //调用函数参数错误
#define IFD_ICC_CheckSumError   -40	//信息校验和出错
#define IFD_ICC_ECUError        -41	//ETU超出范围（3~258）

//以下函数用于系统应用组进行自定义数据交互的测试。
int __stdcall UsbWrite(/*HID写函数，注意：此函数仅用于测试*/
	BYTE*         pchar,//待发送的数据
	int           number);//待发送数据的字节数


int __stdcall UsbRead_Test(/*HID读函数，注意：此函数仅用于测试*/
	BYTE*         pchar,//已接收的数据
	ULONG         resp_time);//设定延时等待时间(单位ms)


//以下函数用于上海JAVA_COS的平台测试，包含获取应用协议指令交互时间、自定义对卡片掉电、裸数据测试模式。
long __stdcall hy_cos_debug_cmd_time(/*获取卡片与读写器进行交互的时间，此函数对象为14443 TypeA非接卡，单条指令误差10us*/
	long          ReaderHandle,//读写器句柄
	BYTE*         Recv_cmd_time_buffer);//返回的交互时间


long __stdcall hy_cos_debug_poweroff(/*自定义时间内操作读写器进行关闭场强操作，此函数对象为14443 TypeA非接卡，时间片最小设定为7个，每时间片单位为50us
                                    时间片取值范围：7-65535，即0x00 0x07---0xFF 0xFF*/
	long          ReaderHandle,//读写器句柄
	BYTE*         Ntime_poweroff_buffer,//设定关闭场强的时间片个数
	BYTE*         Recv_cos_debug_buffer);//返回的数据


long __stdcall hy_7816_cos_debug_cmd_time(/*获取卡片与读写器进行交互的时间，此函数对象为7816 接触式卡，单条指令误差10us*/
	long         ReaderHandle,//读写器句柄
	BYTE*        Recv_cmd_time_buffer);//返回的交互时间


long __stdcall hy_7816_cos_debug_poweroff(/*自定义时间内操作读写器进行关闭场强操作，此函数对象为7816 接触式卡，时间片最小设定为7个，每时间片单位为50us*/
	long        ReaderHandle,//读写器句柄
	BYTE*       Ntime_poweroff_buffer,//设定关闭场强的时间片个数
	BYTE*       Recv_cos_debug_buffer);//返回的数据


long __stdcall hy_pro_command_CmdTime(/*获取卡片与读写器进行交互的时间，此函数对象为7816 接触式卡，单条指令误差10us*/
	long ReaderHandle,//读写器句柄
	unsigned char send_len,
	unsigned int* recv_len,
	unsigned char* send_buf,
	unsigned char* recv_buf,
	unsigned char* CmdTime_buf);//此函数不包含链接！


/*功能函数*/
long __stdcall hy_Reader_Open(HWND hwnd);
long __stdcall hy_Reader_Close(long ReaderHandle);
long __stdcall hy_fuc_14443();//ISO14443
long __stdcall hy_fuc_15693();//ISO15693
long __stdcall hy_fuc_7816();//ISO7816
long __stdcall hy_getver(long ReaderHandle, unsigned char* Response);
BOOL __stdcall hy_clearBuffertem();
BOOL __stdcall hy_SeeData(unsigned char* SeeData_send,unsigned char* SeeData_receive,unsigned char* Seenumber);
BOOL __stdcall hy_beep(long ReaderHandle,unsigned int _Btime);

/*ISO/IEC14443-3*/

long __stdcall hy_card(long ReaderHandle,unsigned char* UID);
long __stdcall hy_request(long ReaderHandle,unsigned char* recv_buf,unsigned char* recv_lenth,unsigned long delay_ms);
long __stdcall hy_RequestAll(long ReaderHandle,unsigned char* recv_buf,unsigned char* recv_lenth,unsigned long delay_ms);
long __stdcall hy_reset(long ReaderHandle,unsigned long delay_ms);
long __stdcall hy_anticoll(long ReaderHandle,unsigned char* recv_buf,unsigned char* recv_lenth,unsigned long delay_ms);
long __stdcall hy_halt(long ReaderHandle,unsigned char* recv_buf,unsigned char* recv_lenth,unsigned long delay_ms);
long __stdcall hy_select_HYReader(long ReaderHandle, unsigned char* recv_buf, unsigned char* recv_lenth, unsigned long delay_ms);
long __stdcall hy_select(long ReaderHandle,unsigned char* recv_buf,unsigned char* _uid,unsigned long delay_ms);
long __stdcall hy_DirectCmd(long ReaderHandle,
							unsigned char Lenth_of_DirectCmd_buffer,
							unsigned char* Response,
							unsigned char parity_CRC,
							unsigned char send_bits,
							unsigned char* Data_buffer,
							unsigned char receive_bits,
							unsigned char receive_bytes,
							unsigned long delay_ms);

/*ISO/IEC14443 Mifare One Command*/
long __stdcall hy_Read(long ReaderHandle,unsigned char* Response,unsigned char _Adr,unsigned long delay_ms);
long __stdcall hy_Write4B(long ReaderHandle,unsigned char* Response,unsigned char* Write4B,unsigned char _Adr,unsigned long delay_ms);
long __stdcall hy_SetKey(long ReaderHandle,unsigned char* Response,unsigned char Setkey,unsigned char* Keydata,unsigned char _Adr,unsigned long delay_ms);
long __stdcall hy_Authen(long ReaderHandle,unsigned char* Response,unsigned char _Adr,int choice_key,unsigned long delay_ms);
long __stdcall hy_Increase(long ReaderHandle,unsigned char* Response,unsigned char* Increase,unsigned char _Adr,unsigned long delay_ms);
long __stdcall hy_Decrease(long ReaderHandle,unsigned char* Response,unsigned char* Decrease,unsigned char _Adr,unsigned long delay_ms);
long __stdcall hy_Restore(long ReaderHandle,unsigned char* Response,unsigned char* Restore,unsigned char _Adr,unsigned long delay_ms);
long __stdcall hy_Transfer(long ReaderHandle,unsigned char* Response,unsigned char _Adr,unsigned long delay_ms);
long __stdcall hy_Write16B(long ReaderHandle,unsigned char* Response,unsigned char* Write16B,unsigned char _Adr,unsigned long delay_ms);
long __stdcall hy_Write4To16(long ReaderHandle,unsigned char* Response,unsigned char* Write4To16,unsigned char _Adr,unsigned char* Write4To16_1,unsigned long delay_ms);

/*ISO/IEC14443-4*/

long __stdcall hy_pro_command(long ReaderHandle,unsigned char send_len,unsigned int* recv_len,unsigned char* send_buf,unsigned char* recv_buf);
long __stdcall hy_pro_commandlink(long ReaderHandle,unsigned char send_len,unsigned char* send_buf,unsigned int* recv_len,unsigned char* recv_buf,unsigned char FG);
long __stdcall hy_pro_halt(long ReaderHandle,unsigned char* recv_buf);
long __stdcall hy_pro_reset(long ReaderHandle,unsigned char* len,unsigned char* recv_buf);
long __stdcall hy_pro_commanddirect(long ReaderHandle,unsigned char send_len,unsigned char* send_buf,unsigned char* recv_len,unsigned char* recv_buf,unsigned long delay_ms);
long __stdcall hy_pro_commanddirect_judgecrc(long ReaderHandle, unsigned char parity_CRC, unsigned char send_len, unsigned char* send_buf, unsigned char* recv_len, unsigned char* recv_buf, unsigned long delay_ms);

/*ISO/IEC7816*/

long __stdcall hy_SAMreset(long ReaderHandle,unsigned char* recv_len,unsigned char* recv_buf);
long __stdcall hy_SAMapdudirect(long ReaderHandle,unsigned char send_len,unsigned char* send_buf,unsigned char* recv_len,unsigned char* recv_buf);
long __stdcall hy_SAMsetSAM(long ReaderHandle,unsigned char SAMID);
long __stdcall hy_SAMsetETU(long ReaderHandle,unsigned char* Response,unsigned char _etu);
long __stdcall hy_SAMapdu(long ReaderHandle,unsigned char send_len,unsigned char* send_buf,unsigned char* recv_len,unsigned char* recv_buf);

/*ISO/IEC15693*/

long __stdcall hy_15693_DirectCmd(
	long ReaderHandle,
	long len,
	unsigned char* recv_buffer,
	unsigned char* CRC,
	unsigned char* Send_buffer,
	unsigned long resp_time);

long __stdcall hy_15693_Inventory(
	long ReaderHandle,
	long len,
	unsigned char* recv_buffer,
	unsigned char* CRC,
	unsigned char* flag,
	unsigned char* Inventory_data,
	unsigned long resp_time);

long __stdcall hy_15693_Stay_Quiet(
	long ReaderHandle,
	unsigned char* recv_buffer,
	unsigned char* Stay_Quiet_CRC,
	unsigned char* Stay_Quiet_flag,
	unsigned char* UID,
	unsigned long resp_time);

long __stdcall hy_15693_Read_Single(
	long ReaderHandle,
	long Lenth_of_Read_Single_buffer,
	unsigned char* recv_buffer,
	unsigned char* Read_Single_CRC,
	unsigned char* Read_Single_flag,
	unsigned char* Read_Single_data,
	unsigned char* Read_Single_block_quantity,
	unsigned long resp_time);

long __stdcall hy_15693_Reset(
	long ReaderHandle,
	unsigned char* recv_buffer,
	unsigned long resp_time);

long __stdcall hy_15693_EOF(
	long ReaderHandle,
	unsigned char* recv_buffer,
	unsigned long resp_time);

long __stdcall hy_15693_Write_Single(
	long ReaderHandle,
	long Lenth_of_Write_Single_buffer,
	unsigned char* recv_buffer,
	unsigned char* Write_Single_CRC,
	unsigned char* Write_Single_flag,
	unsigned char* Write_Single_data,
	unsigned char* Write_Single_block_quantity,
	unsigned long resp_time);

long __stdcall hy_15693_Lock_Block(
	long ReaderHandle,
	long Lenth_of_Lock_Block_buffer,
	unsigned char* recv_buffer,
	unsigned char* Lock_Block_CRC,
	unsigned char* Lock_Block_flag,
	unsigned char* Lock_Block_data,
	unsigned char* Lock_Block_block_quantity,
	unsigned long resp_time);

long __stdcall hy_15693_Read_Mulblock(
	long ReaderHandle,
	long Lenth_of_Read_Mulblock_buffer,
	unsigned char* recv_buffer,
	unsigned char* Read_Mulblock_CRC,
	unsigned char* Read_Mulblock_flag,
	unsigned char* Read_Mulblock_data,
	unsigned char* Read_Mulblock_block_quantity,
	unsigned char* Read_Mulblock_block_number,
	unsigned long resp_time);

long __stdcall hy_15693_Select_15693(
	long ReaderHandle,
	long Lenth_of_Select_15693_buffer,
	unsigned char* recv_buffer,
	unsigned char* Select_15693_CRC,
	unsigned char* Select_15693_flag,
	unsigned char* Select_15693_data,
	unsigned long resp_time);

long __stdcall hy_15693_Reset_to_Ready(
	long ReaderHandle,
	long Lenth_of_Reset_to_Ready_buffer,
	unsigned char* recv_buffer,
	unsigned char* Reset_to_Ready_CRC,
	unsigned char* Reset_to_Ready_flag,
	unsigned char* Reset_to_Ready_data,
	unsigned long resp_time);

long __stdcall hy_15693_Write_AFI(
	long ReaderHandle,
	long Lenth_of_Write_AFI_buffer,
	unsigned char* recv_buffer,
	unsigned char* Write_AFI_CRC,
	unsigned char* Write_AFI_flag,
	unsigned char* Write_AFI_data,
	unsigned long resp_time);

long __stdcall hy_15693_Lock_AFI(
	long ReaderHandle,
	long Lenth_of_Lock_AFI_buffer,
	unsigned char* recv_buffer,
	unsigned char* Lock_AFI_CRC,
	unsigned char* Lock_AFI_flag,
	unsigned char* Lock_AFI_data,
	unsigned long resp_time);

long __stdcall hy_15693_Write_DSFID(
	long ReaderHandle,
	long Lenth_of_Write_DSFID_buffer,
	unsigned char* recv_buffer,
	unsigned char* Write_DSFID_CRC,
	unsigned char* Write_DSFID_flag,
	unsigned char* Write_DSFID_data,
	unsigned long resp_time);

long __stdcall hy_15693_Lock_DSFID(
	long ReaderHandle,
	long Lenth_of_Lock_DSFID_buffer,
	unsigned char* recv_buffer,
	unsigned char* Lock_DSFID_CRC,
	unsigned char* Lock_DSFID_flag,
	unsigned char* Lock_DSFID_data,
	unsigned long resp_time);

long __stdcall hy_15693_Get_SystemInfo(
	long ReaderHandle,
	long Lenth_of_Get_SystemInfo_buffer,
	unsigned char* recv_buffer,
	unsigned char* Get_SystemInfo_CRC,
	unsigned char* Get_SystemInfo_flag,
	unsigned char* Get_SystemInfo_data,
	unsigned long resp_time);

long __stdcall hy_15693_Get_Mulblock_Security(
	long ReaderHandle,
	long Lenth_of_Get_Mulblock_Security_buffer,
	unsigned char* recv_buffer,
	unsigned char* Get_Mulblock_Security_CRC,
	unsigned char* Get_Mulblock_Security_flag,
	unsigned char* Get_Mulblock_Security_data,
	unsigned char* Get_Mulblock_Security_block_quantity,
	unsigned char* Get_Mulblock_Security_block_number,
	unsigned long resp_time);

long __stdcall hy_15693_Write_2_Blocks(
	long ReaderHandle,
	long Lenth_of_Write_2_Blocks_buffer,
	unsigned char* recv_buffer,
	unsigned char* Write_2_Blocks_CRC,
	unsigned char* Write_2_Blocks_flag,
	unsigned char* Write_2_Blocks_data,
	unsigned char* Write_2_Blocks_block_number,
	unsigned long resp_time);

long __stdcall hy_15693_Lock_2_Blocks(
	long ReaderHandle,
	long Lenth_of_Lock_2_Blocks_buffer,
	unsigned char* recv_buffer,
	unsigned char* Lock_2_Blocks_CRC,
	unsigned char* Lock_2_Blocks_flag,
	unsigned char* Lock_2_Blocks_data,
	unsigned char* Lock_2_Blocks_block_number,
	unsigned long resp_time);

long __stdcall hy_15693_Inventory_Read(
	long ReaderHandle,
	long Lenth_of_Inventory_Read_buffer,
	unsigned char* recv_buffer,
	unsigned char* Inventory_Read_CRC,
	unsigned char* Inventory_Read_flag,
	unsigned char* Inventory_Read_data,
	unsigned char* Inventory_Read_block_quantity,
	unsigned char* Inventory_Read_block_number,
	unsigned long resp_time);

long __stdcall hy_15693_Fast_Inventory_Read(
	long ReaderHandle,
	long Lenth_of_Fast_Inventory_Read_buffer,
	unsigned char* recv_buffer,
	unsigned char* Fast_Inventory_Read_CRC,
	unsigned char* Fast_Inventory_Read_flag,
	unsigned char* Fast_Inventory_Read_data,
	unsigned char* Fast_Inventory_Read_block_quantity,
	unsigned char* Fast_Inventory_Read_block_number,
	unsigned long resp_time);

long __stdcall hy_15693_Set_to_Test_State(
	long ReaderHandle,
	unsigned char* recv_buffer,
	unsigned char* Set_to_Test_State_CRC,
	unsigned char* Set_to_Test_State_flag,
	unsigned long resp_time);

long __stdcall hy_15693_Factory_Lock(
	long ReaderHandle,
	unsigned char* recv_buffer,
	unsigned char* Factory_Lock_CRC,
	unsigned char* Factory_Lock_flag,
	unsigned char* Factory_Lock_block_number,
	unsigned long resp_time);

long __stdcall hy_15693_Write_all(
	long ReaderHandle,
	unsigned char* recv_buffer,
	unsigned char* Write_all_CRC,
	unsigned char* Write_all_flag,
	int choice_00_ff,
	unsigned long resp_time);
long __stdcall hy_15693_EAS(
	long ReaderHandle,
	long Lenth_of_EAS_buffer,
	unsigned char* recv_buffer,
	unsigned char* EAS_CRC,
	unsigned char* EAS_flag,
	unsigned char* EAS_data,
	int choice,
	unsigned long resp_time);

#ifdef __cplusplus
}
#endif