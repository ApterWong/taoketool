#ifndef MYUTIL
#define MYUTIL

#include <string>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <windows.h>
#include <map>

using mapHwnd2title_t = std::map<HWND, std::string>;
using namespace std;

namespace myutil {

	string ws2s(const wstring& ws);

	int getAllWindow(HWND hwnds[], CHAR titles[][512], mapHwnd2title_t &hwnd2title, string filter);
	int getWindowTitle(HWND hwnd, CHAR title[512]);

	string getCurrTime();

	void PostCtrlAndKey(HWND hWnd, BYTE vkey);

	BOOL SendKey(HWND hParentWnd, LPCTSTR strTitle, int enterKey, int flag);

	void PostAltAndKey(HWND hWnd, BYTE vkey);

	void setWechatInputAreaFocus(HWND hWnd);
}


#endif
