#pragma once


// CDEVICE_SET �Ի���

class CDEVICE_SET : public CDialog
{
	DECLARE_DYNAMIC(CDEVICE_SET)

public:
	CDEVICE_SET(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDEVICE_SET();

// �Ի�������
	enum { IDD = IDD_DIALOG_DEVSET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	//LRESULT OnDeviceChange(WPARAM wParam, LPARAM lParam);
};
