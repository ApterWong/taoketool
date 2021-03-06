#include "node.h"
#include <iostream>

#define DEBUG

#include "myutil.h"

#include "base64.h"

using namespace myutil;

namespace demo {

	using v8::Exception;
	using v8::FunctionCallbackInfo;
	using v8::Isolate;
	using v8::Local;
	using v8::Number;
	using v8::Object;
	using v8::String;
	using v8::Value;
	using v8::Array;

	// 这是 "add" 方法的实现
	// 输入参数使用 const FunctionCallbackInfo<Value>& args 结构传入
	void testAdd(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();

		// 检查传入的参数的个数
		if (args.Length() < 2) {
			// 抛出一个错误并传回到 JavaScript
			string errmsg("参数的数量错误");
			string base64errmsg;
			Base64::Decode(errmsg, &base64errmsg);
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, base64errmsg.c_str())));
			return;
		}

		// 检查参数的类型
		if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "参数错误")));
			return;
		}

		// 执行操作
		double value = args[0]->NumberValue() + args[1]->NumberValue();
		Local<Number> num = Number::New(isolate, value);

		// 设置返回值
		args.GetReturnValue().Set(num);
	}

	/********************************************************************/
	//所有的句柄， 每次loadWindow句柄时清零
	HWND kk_hwnd[256];
	HWND kk_hwndCht[256];

	// 窗口标题
	CHAR kk_wndText[256][512];
	CHAR kk_wndTextCht[256][512];

	//句柄到窗口标题的映射
	mapHwnd2title_t kk_hwnd2title;
	mapHwnd2title_t kk_hwnd2titleCht;

	int kk_count = 0;
	int kk_countCht = 0;

	void init_data()
	{
		kk_count = 0;
		kk_countCht = 0;
	}
	// 数据定义部分end
	/********************************************************************************/

	//初始化数据
	void InitData(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();

		string curtime = getCurrTime() + " started.\n";
		cout << curtime.c_str(), curtime.length();
	}

	//所有QQ窗口句柄
	//args: None
	//return: {hWnd:title}的json串
	void loadAllWindow(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();

		init_data();

		kk_count = getAllWindow(kk_hwnd, kk_wndText, kk_hwnd2title, "TXGuiFoundation");

		char *buf = new char[8192]{0};
		int pos = 0;
		int len = 0;
		
		len = sprintf(&buf[pos], "{\"QQ\":{");
		pos += len;
		
		for (int i = 0; i < kk_count; i++) {
			HWND wd = kk_hwnd[i];
			string title = kk_hwnd2title.at(wd);
			if (wd == 0) {
				cout << "[ERROR]: 窗口句柄已经改变 hwnd=0, title=" << kk_wndText[i] << "\n";
			}

			if (title == string("QQ")) {
				//cout << "[INFO:] 标题QQ的窗口已过滤\n";
				continue;
			}
			
			//cout << "[INFO]:窗口句柄:" << wd << ",title:" << kk_wndText[i] << "title2:" << title << "\n";
			
			if (i+1 == kk_count) {
				len = sprintf(&buf[pos], "%d:\"%s\"", (int)wd, kk_wndText[i]);
				pos += len;
			}
			else {
				len = sprintf(&buf[pos], "%d:\"%s\",", (int)wd, kk_wndText[i]);
				pos += len;
			}
		}
		buf[pos++] = '}';

		// 微信窗口获取
		len = sprintf(&buf[pos], ",\"Wechat\":{");
		pos += len;
		kk_countCht = myutil::getAllWindow(kk_hwndCht, kk_wndTextCht, kk_hwnd2titleCht, "ChatWnd");
		
		for (int i = 0; i < kk_countCht; i++) {
			HWND wd = kk_hwndCht[i];
			string title = kk_hwnd2titleCht[wd];
			if (wd == 0) {
				cout << "[ERROR]: 窗口句柄已经改变 hwnd=0, title=" << kk_wndTextCht[i] << "\n";
			}
			if (i + 1 == kk_countCht) {
				len = sprintf(&buf[pos], "%d:\"%s\"}", (int)wd, kk_wndTextCht[i]);
				pos += len;
			}
			else {
				len = sprintf(&buf[pos], "%d:\"%s\",", (int)wd, kk_wndTextCht[i]);
				pos += len;
			}
		}
		//buf[pos++] = '}';  // end Wechat
		buf[pos++] = '}'; // end all


#ifdef DEBUG
		cout << "resultJson:" << buf << "\n";
		v8::Local<v8::String> jsTitle = v8::String::NewFromUtf8(isolate, buf);
#else
		string strRes(buf);
		string b64StrRes;
		Base64::Decode(strRes, &b64StrRes);
		v8::Local<v8::String> jsTitle = v8::String::NewFromUtf8(isolate, b64StrRes.c_str());
#endif
		delete[] buf;

		args.GetReturnValue().Set(jsTitle);
	}

	// 向窗口发送消息
	// 本实现是根据句柄[参数需要传入]的窗口发送消息【粘贴、发送】
	void sendMsgToWindow(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();

		// 检查传入的参数的个数
		if (args.Length() < 2) {
			// 抛出一个错误并传回到 JavaScript
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "参数的数量错误")));
			return;
		}

		// 检查参数的类型
		if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "参数错误")));
			return;
		}

		int64_t val = args[0]->IntegerValue();
		int64_t enterKey = args[1]->IntegerValue();
		if (val <= 0) {
			cout << "[ERROR:] " << __FILE__ << ":" << __LINE__ << " 参数错误, hwnd:" << val << "\n";
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "参数错误")));
			return;
		}

		if (enterKey != 2 && enterKey != 1) {
			cout << "[ERROR:] " << __FILE__ << ":" << __LINE__ << " 参数错误, enterkey:" << enterKey << "\n";
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "参数错误")));
			return;
		}

		HWND hwnd = (HWND)val;
		string titleName = "";
		int flag = 0; // QQ窗口：【默认0】， 微信窗口【1】

		if (kk_hwnd2title.find(hwnd) != kk_hwnd2title.end()) {
			titleName = kk_hwnd2title[hwnd];
			/*
			cout << "[ERROR:] " << __FILE__ << ":" << __LINE__ << " 窗口句柄已经改变了\n";
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "窗口句柄已经改变")));
			return;
			*/
		}
		else if (kk_hwnd2titleCht.find(hwnd) != kk_hwnd2titleCht.end()) {
			titleName = kk_hwnd2titleCht[hwnd];
			flag = 1;
		}

		if (titleName == "")  {
			cout << "[ERROR:] " << __FILE__ << ":" << __LINE__ << " 窗口句柄已经改变了 " << 
				"flag=" << flag << "\n";
			cout << "QQHwnd:" << kk_hwnd2title[hwnd] << "\n";
			cout << "WechatHwnd:" << kk_hwnd2titleCht[hwnd] << "\n";
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "窗口句柄已经改变")));
			return;
		}

		CHAR title[512] = { 0, };
		int ret = myutil::getWindowTitle(hwnd, title);
		if (ret != 0) {
			cout << "[ERROR:] " << __FILE__ << ":" << __LINE__ << " 窗口句柄获取失败,窗口句柄可能已经改变了\n";
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "窗口句柄获取失败")));
			return;
		}

		if ((flag == 0 && string(title) != kk_hwnd2title[hwnd]) && (
			flag == 1 && string(title) != kk_hwnd2titleCht[hwnd]) ) {
			cout << "[ERROR:] " << __FILE__ << ":" << __LINE__ << " 窗口句柄可能已经改变了\n";
			cout << "title:" << title << ", hwnd2title:" << kk_hwnd2title[hwnd] << "\n";
			isolate->ThrowException(Exception::TypeError(
				String::NewFromUtf8(isolate, "窗口句柄可能已经改变了")));
			return;
		}

		char *buf = new char[8192]{'{'};
		int pos = 1;
		int len = 0;

		BOOL result = myutil::SendKey(hwnd, title, (int)enterKey, flag);
		if (result)
			sprintf(&buf[pos], "status:%d, msg:\"%s\"}", result, "发送成功");
		else
			sprintf(&buf[pos], "status:%d, msg:\"%s\"}", result, "失败");

#ifdef DEBUG
		args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, buf));
#else
		string strRes(buf);
		string b64StrRes;
		Base64::Decode(strRes, &b64StrRes);
		args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, b64StrRes.c_str()));
#endif
		delete[] buf;
	}

	void Init(Local<Object> exports) {
		NODE_SET_METHOD(exports, "InitData", InitData);
		NODE_SET_METHOD(exports, "loadAllWindow", loadAllWindow);
		NODE_SET_METHOD(exports, "sendMsgToWindow", sendMsgToWindow);
	}

	NODE_MODULE(kkqWinTool, Init);

}  // namespace demo
