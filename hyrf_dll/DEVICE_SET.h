#pragma once


// CDEVICE_SET 对话框

class CDEVICE_SET : public CDialog
{
	DECLARE_DYNAMIC(CDEVICE_SET)

public:
	CDEVICE_SET(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDEVICE_SET();

// 对话框数据
	enum { IDD = IDD_DIALOG_DEVSET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	//LRESULT OnDeviceChange(WPARAM wParam, LPARAM lParam);
};
