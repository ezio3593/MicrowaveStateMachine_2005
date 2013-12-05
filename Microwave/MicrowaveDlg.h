// MicrowaveDlg.h : header file
//
#include "MicrowaveStateMachine.h"
#pragma once


// CMicrowaveDlg dialog
class CMicrowaveDlg : public CDialog
{
	int maxTimerValue;
	int minTimerValue;

	CRichEditCtrl* log;
	CSpinButtonCtrl* spin;
	std::wstring endline;
	MicrowaveStateMachine* microwave;

	CriticalSection cs;
	class CallbackTimer: public CallbackFunc
	{
		CRichEditCtrl* log;
		MicrowaveStateMachine* sm;
		CSpinButtonCtrl* spin;
		std::wstring endline;
		CriticalSection &cs;

		public:
			CallbackTimer(CRichEditCtrl* _log, MicrowaveStateMachine* _sm, CSpinButtonCtrl* _spin, CriticalSection& _cs, std::wstring _endline): 
				log(_log), sm(_sm), spin(_spin), cs(_cs), endline(_endline) {}
			void operator()();
			~CallbackTimer(){}
	};

	CallbackTimer* callbackTimer;
	
	std::wstring getMsg(std::wstring eventName, MicrowaveStateMachine* ms, bool isPossible);
	std::wstring to_wstring(int i);
// Construction
public:
	CMicrowaveDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MICROWAVE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCancel();
public:
	afx_msg void OnBnClickedButtonOpenDoor();
public:
	afx_msg void OnBnClickedButtonCloseDoor();
	~CMicrowaveDlg() {delete microwave; delete callbackTimer;}
public:
	afx_msg void OnDeltaposSpinTimer(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnBnClickedButtonStart();
};
