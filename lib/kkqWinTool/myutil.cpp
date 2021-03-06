#define _CRT_SECURE_NO_WARNINGS
#include "myutil.h"

using namespace std;

string myutil::ws2s(const wstring& ws)
{
	size_t convertedChars = 0;
	string curLocale = setlocale(LC_ALL, NULL); //curLocale="C"
	setlocale(LC_ALL, "chs");
	const wchar_t* wcs = ws.c_str();
	size_t dByteNum = sizeof(wchar_t)*ws.size() + 1;
	//    cout<<"ws.size():"<<ws.size()<<endl;            //5

	char* dest = new char[dByteNum];
	wcstombs_s(&convertedChars, dest, dByteNum, wcs, _TRUNCATE);
	//    cout<<"convertedChars:"<<convertedChars<<endl; //8
	string result = dest;
	delete[] dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

// 获取窗口句柄
int myutil::getAllWindow(HWND hwnds[], CHAR titles[][512], mapHwnd2title_t &hwnd2title, string filter)
{
	if (filter.empty() || filter.length() == 0) {
		return 0;
	}

	int count = 0;
	CHAR classname[512];

	HWND hd = ::GetDesktopWindow();
	HWND hwnd = ::GetWindow(hd, GW_CHILD);
	while (hwnd != NULL)
	{
		memset(classname, 0, 512);
		::GetClassName(hwnd, classname, 512);

		string res = string(classname);

		//"TXGuiFoundation" ： QQ Window，"ChatWnd":微信窗口
		if (res == filter)
		{
			CHAR wndText[512] = { 0 };
			::GetWindowText(hwnd, wndText, 512);
			string str = string(wndText);

			if (!str.empty()) {
				if (str.find(string("TX")) != string::npos &&
					str.find(string("Window")) != string::npos)
				{
					//cout << "[INFO]: TXWindow:title:" << str << "\n";
				}
				else
				{
					cout << "[INFO]: windowTitle:" << str << "\n";

					hwnd2title.insert(pair<HWND, string>({ hwnd, str }));
					hwnds[count] = hwnd;
					memcpy(titles[count], wndText, sizeof(wndText));
					count++;
				}
			}
			else {
				//cout << "[INFO]: title is \"" << res << "\"\n";
			}
		}
		else
		{
			//cout << "[WARNING]: title != filter, title:" << res << ", filter:"
			//	<< filter << "\n";
			//printf("className:%s\n", classname);
		}

		hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT);
	}

	return count;
}

int myutil::getWindowTitle(HWND hwnd, CHAR title[512])
{
	int ret = 0;

	ret = ::GetWindowText(hwnd, title, 512);
	if (ret <= 0) {
		return -1;
	}

	return 0;
}

void myutil::PostCtrlAndKey(HWND hWnd, BYTE vkey)
{

	keybd_event(VK_CONTROL, 0, KEYEVENTF_EXTENDEDKEY, 0);
	::SendMessage(hWnd, WM_KEYDOWN, vkey, 0);
	::Sleep(10);
	::SendMessage(hWnd, WM_KEYUP, vkey, 0);
	::Sleep(10);
	// Simulate a key release
	keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY, 0);//*/
}

//void myutil::copyToClipboard()
//{
//	QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
//	QString originalText = clipboard->text();         //获取剪贴板上文本信息
//	clipboard->setText("newText");                  //设置剪贴板内容</span>
//}

//void CopyToClipboard(HWND hWndOwner, LPCTSTR strText)
//{
//    if( !OpenClipboard(hWndOwner))
//    {
//        return;
//    }
//    EmptyClipboard();
//    size_t cbStr = (strlen(strText) + 1) * sizeof(TCHAR);
//    HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, cbStr);
//    memcpy_s(GlobalLock(hData), cbStr, strText, cbStr);
//    GlobalUnlock(hData);
//    // For the appropriate data formats...
//    UINT uiFormat = (sizeof(TCHAR) == sizeof(WCHAR)) ? CF_UNICODETEXT : CF_TEXT;
//    if (::SetClipboardData(uiFormat, hData) == NULL)
//    {
//        DWORD dwErro = GetLastError();
//    }
//    CloseClipboard();
//    GlobalFree(hData);
//}

// 发送消息到指定句柄的窗口，strTitle：窗口标题
// enterKey: 1: enter， 2: ctrl + enter
// flag：0 QQ窗口， 1 微信窗口
BOOL myutil::SendKey(HWND hParentWnd, LPCTSTR strTitle, int enterKey, int flag)
{
	HWND hWnd = ::FindWindow(NULL, strTitle);
	if (hWnd)
	{
		// showWindow
		//if (::IsIconic(hWnd))
		//{
			//::ShowWindow(hWnd, SW_RESTORE);
			//Sleep(50);
			//::SetForegroundWindow(hParentWnd);
			//Sleep(50);
			//::SetFocus(hParentWnd);
			//Sleep(50);
			//            ::SetTimer(hParentWnd, 1000, 1000, NULL);
			//::SendMessage(hWnd, WM_PASTE, 0, 0);
			//Sleep(50);
			//cout << "[INFO:] SendKey() IsIconic case ok." << std::endl;
			//return TRUE;
		//}
		::ShowWindow(hParentWnd, SW_RESTORE);
		Sleep(50);
		::SetForegroundWindow(hParentWnd);
		Sleep(50);
		::SetFocus(hParentWnd);
		Sleep(50);

		// setFocus
		if (flag == 1) {
			// 微信需要设置下窗口的焦点
			// 微信需要Ctrl+V粘贴
			setWechatInputAreaFocus(hWnd);
			Sleep(50);
			PostCtrlAndKey(hParentWnd, 'V');
			Sleep(50);
		}
		else 
		{
			// QQ窗口
			::SendMessage(hParentWnd, WM_PASTE, 0, 0);
			Sleep(50);
		}

		// sendMsg
		if (enterKey == 1) {
			::SendMessage(hWnd, WM_KEYDOWN, VK_RETURN, 0);
			Sleep(10);
			::SendMessage(hWnd, WM_KEYUP, VK_RETURN, 0);
		}
		else if (enterKey == 2)
			PostCtrlAndKey(hWnd, VK_RETURN);
	}
	else
	{
		cout << "[WARNING:]" << __FILE__ << ":" << __LINE__ << " 句柄已经失效或被关闭...\n";
		return FALSE;
	}

	//cout << "[INFO:]" << __FILE__ << ":" << __LINE__ << "function SendKey end.\n";
	return TRUE;
}

void myutil::PostAltAndKey(HWND hWnd, BYTE vkey)
{
	::PostMessage(hWnd, WM_SYSKEYDOWN, vkey, 1 << 29);
}

void myutil::setWechatInputAreaFocus(HWND hwnd)
{
	::RECT rect = RECT();
	::GetWindowRect(hwnd, &rect);

	::MoveWindow(hwnd, 0, 0, rect.right - rect.left, rect.bottom - rect.top, true);
	Sleep(20);
	::GetWindowRect(hwnd, &rect);

	struct POS{
		int x, y;
		POS() {
			x = y = 0;
		}
	}lft, rt;

	lft.x = rect.left + 20;
	lft.y = rect.bottom - 40;

	SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(lft.x, lft.y));
	Sleep(20);
	SendMessage(hwnd, WM_LBUTTONUP, NULL, MAKELPARAM(lft.x, lft.y));
	Sleep(20);
}

/*****************************************************************************/
//  other util functions

string myutil::getCurrTime()
{
	time_t timep;
	time(&timep);
	char tmp[256]{0};
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
	return string(tmp);
}
