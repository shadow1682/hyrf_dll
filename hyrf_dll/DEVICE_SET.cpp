// DEVICE_SET.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "hyrf_dll.h"
#include "DEVICE_SET.h"


// CDEVICE_SET �Ի���

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


// CDEVICE_SET ��Ϣ�������

BOOL CDEVICE_SET::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

/*LRESULT CDEVICE_SET::OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
	case DBT_DEVICEARRIVAL:
		{
			if(hy_Reader_Open() == 110)
			{
				::AfxMessageBox(_T("�豸������"));
			}
			break;
		}
	case DBT_DEVICEREMOVECOMPLETE:
		{
			if(hy_Reader_Open() < 0)
			{
				::AfxMessageBox(_T("�豸�ѶϿ�"));
			}
			break;
		}
		
	}
	return LRESULT();
}*/