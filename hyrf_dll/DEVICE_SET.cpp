// DEVICE_SET.cpp : 实现文件
//

#include "stdafx.h"
#include "hyrf_dll.h"
#include "DEVICE_SET.h"


// CDEVICE_SET 对话框

IMPLEMENT_DYNAMIC(CDEVICE_SET, CDialog)

CDEVICE_SET::CDEVICE_SET(CWnd* pParent /*=NULL*/)
	: CDialog(CDEVICE_SET::IDD, pParent)
{

}

CDEVICE_SET::~CDEVICE_SET()
{
}

void CDEVICE_SET::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDEVICE_SET, CDialog)
	//ON_MESSAGE(WM_DEVICECHANGE,OnDeviceChange)
END_MESSAGE_MAP()


// CDEVICE_SET 消息处理程序

BOOL CDEVICE_SET::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

/*LRESULT CDEVICE_SET::OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case DBT_DEVICEARRIVAL:
		{
			if(hy_Reader_Open() == 110)
			{
				::AfxMessageBox(_T("设备已连接"));
			}
			break;
		}
	case DBT_DEVICEREMOVECOMPLETE:
		{
			if(hy_Reader_Open() < 0)
			{
				::AfxMessageBox(_T("设备已断开"));
			}
			break;
		}
		
	}
	return LRESULT();
}*/