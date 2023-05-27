
#include <string>
#include <iostream>
#include <string_view>
#include <fstream>
#include <sstream>

#include <Windows.h>
#include <userenv.h>
#pragma comment(lib,"Userenv.lib")

//#include <atlstr.h>



std::size_t replace_all(std::wstring& inout, std::wstring_view what, std::wstring_view with)
{
	std::size_t count{};
	for (std::string::size_type pos{};
		inout.npos != (pos = inout.find(what.data(), pos, what.length()));
		pos += with.length(), ++count)
		inout.replace(pos, what.length(), with.data(), with.length());
	return count;
}


WCHAR targetapplicationpayth[1000] = L"C:\\Program Files (x86)\\python38-32\\pythonw.exe";
WCHAR args[1000] = L"\"C:\\Program Files (x86)\\python38-32\\pythonw.exe\"  C:\\Users\\admin\\Documents\\agent.pyw";
WCHAR applicationpaythstr[1000] = { 0 };
WCHAR inifilepath[1000] = { 0 };
WCHAR logfile[1000] = { 0 };

//using log= std::wcout()
int main()
{
	// get ini file 

	GetModuleFileNameW(NULL, applicationpaythstr, MAX_PATH);

	std::wstring  applicationpayth = applicationpaythstr;
	replace_all(applicationpayth, L".exe", L"");

	wsprintf(inifilepath, L"%s.ini", applicationpayth.c_str());
	wsprintf(logfile, L"%s__.log", applicationpayth.c_str());

	//auto log = std::wofstream(logfile, std::ios::app);
	std::wostringstream log;

	log << "log init  : " << applicationpayth << std::endl;
	log << "current module : " << applicationpayth << std::endl;
	log << "ini file : " << inifilepath << std::endl;

	//__try
	//{
	///*	volatile int* pInt = 0x00000000;
	//	*pInt = 20;*/


	//	GetPrivateProfileStringW(
	//		L"runassvc",
	//		L"application",
	//		L"C:\\Program Files (x86)\\python38-32\\python.exe",
	//		targetapplicationpayth,
	//		MAX_PATH,
	//		inifilepath
	//	);
	//	log << "GetPrivateProfileStringW : " << GetLastError() << std::endl;

	//	log << "read ini file : application - " << targetapplicationpayth << std::endl;


	//	GetPrivateProfileStringW(
	//		L"runassvc",
	//		L"args",
	//		L"C:\\Users\\admin\\Documents\\agent.pyw",
	//		args,
	//		MAX_PATH,
	//		inifilepath
	//	);
	//	log << "GetPrivateProfileStringW : " << GetLastError() << std::endl;
	//	log << "read ini file : args - " << args << std::endl;
	//}
	//__except (EXCEPTION_EXECUTE_HANDLER)
	//{
	//	log << "[***********]  Executing SEH __except block" << std::endl;
	//	log << "read ini file :  - " << targetapplicationpayth << std::endl;
	//	log << "read ini file :  - " << args << std::endl;
	//	RtlMoveMemory(targetapplicationpayth, L"C:\\Program Files (x86)\\python38-32\\python.exe", sizeof(L"C:\\Program Files (x86)\\python38-32\\python.exe") + 2);
	//	RtlMoveMemory(args, L"C:\\Users\\admin\\Documents\\agent.pyw", sizeof(L"C:\\Users\\admin\\Documents\\agent.pyw")+2);

	//	log << "static value  : targetapplicationpayth  - " << targetapplicationpayth << std::endl;
	//	log << "static value : args - " << args << std::endl;
	//}


	log << "static value  : targetapplicationpayth  |" << targetapplicationpayth << std::endl;
	log << "static value : args  |" << args << std::endl;

	HANDLE hToken = NULL;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
	{
		log << L"OpenProcessToken error  : " << GetLastError() << std::endl;

		return false;
	}
	log << L"OpenProcessToken : " << GetLastError() << std::endl;


	HANDLE hTokenDup = NULL;
	bool bRet = DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityIdentification, TokenPrimary, &hTokenDup);
	if (!bRet || hTokenDup == NULL)
	{
		log << L"DuplicateTokenEx error  : " << GetLastError() << std::endl;

		CloseHandle(hToken);
		return false;
	}

	log << L"DuplicateTokenEx : " << GetLastError() << std::endl;

	DWORD dwSessionId = WTSGetActiveConsoleSessionId();
	log << L"WTSGetActiveConsoleSessionId : " << GetLastError() << std::endl;

	//把服务hToken的SessionId替换成当前活动的Session(即替换到可与用户交互的winsta0下)
	if (!SetTokenInformation(hTokenDup, TokenSessionId, &dwSessionId, sizeof(DWORD)))
	{
		DWORD nErr = GetLastError();
		log << L"SetTokenInformation  error : " << nErr << std::endl;

		CloseHandle(hTokenDup);
		CloseHandle(hToken);
		return false;
	}

	log << L"SetTokenInformation : " << GetLastError() << std::endl;

	STARTUPINFOW si;
	ZeroMemory(&si, sizeof(STARTUPINFO));

	//创建进程环境块
	LPVOID pEnv = NULL;
	bRet = CreateEnvironmentBlock(&pEnv, hTokenDup, FALSE);

	log << L"CreateEnvironmentBlock : " << GetLastError() << std::endl;

	if (!bRet)
	{
		CloseHandle(hTokenDup);
		CloseHandle(hToken);
		return false;
	}

	if (pEnv == NULL)
	{
		CloseHandle(hTokenDup);
		CloseHandle(hToken);
		return false;
	}

	//在活动的Session下创建进程
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
	DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT;



	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = (LPWSTR)L"WinSta0\\Default";
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW /*|STARTF_USESTDHANDLES*/;


	//if (!CreateProcessAsUserW(hTokenDup, (LPWSTR)targetapplicationpayth.AllocSysString(), (LPWSTR)args.AllocSysString(), NULL, NULL, FALSE, dwCreationFlag, pEnv, NULL, &si, &processInfo))
	if (!CreateProcessAsUserW(hTokenDup, targetapplicationpayth, args, NULL, NULL, FALSE, dwCreationFlag, pEnv, NULL, &si, &processInfo))
	{
		DWORD nRet = GetLastError();
		log << L"CreateProcessAsUserW error  : " << nRet << std::endl;
		CloseHandle(hTokenDup);
		CloseHandle(hToken);
		return false;
	}

	log << L"CreateProcessAsUserW : " << GetLastError() << std::endl;

	DestroyEnvironmentBlock(pEnv);

	log << L"DestroyEnvironmentBlock : " << GetLastError() << std::endl;

	CloseHandle(hTokenDup);
	CloseHandle(hToken);

	log << L"runassvc succ  " << GetLastError() << std::endl;
	return true;

}


