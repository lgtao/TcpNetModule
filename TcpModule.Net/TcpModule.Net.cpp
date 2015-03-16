// TcpNetModule.Server.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"


#include "TcpModule.Net.h"
#include "TcpServer.h"
#include <string>


#include "Dbghelp.h"
#pragma comment(lib,"dbghelp.lib")


LONG WINAPI _SELF_UnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{

	char szFileName[MAX_PATH];
	::GetModuleFileNameA(NULL, szFileName, _MAX_PATH);
	std::string str = szFileName;
	str = str.substr(0, str.length() - 4) + ".dmp";

	HANDLE hFile = ::CreateFileA(str.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION einfo;
		einfo.ThreadId = ::GetCurrentThreadId();
		einfo.ExceptionPointers = ExceptionInfo;
		einfo.ClientPointers = FALSE;

		::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, &einfo, NULL, NULL);
		::CloseHandle(hFile);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

extern "C" long __stdcall tnet_init()
{
	long result = 0;
	::SetUnhandledExceptionFilter(_SELF_UnhandledExceptionFilter);
	WSADATA wsd = { 0 };

	if (::WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		result = -1;
#ifdef _DEBUG
		CLog::GetObj().tmWrite("[tnms] WSAStartup 失败");
#endif
	}

	TcpClientManage::GetInstance()->init();

	return result;
}

extern "C" void __stdcall tnet_cleanup()
{
	TcpClientManage::GetInstance()->cleanup();

	::WSACleanup();
}


extern "C" long __stdcall tnet_start(unsigned short listen_port,
	connected_callback conn_cb, void* conn_cb_data, 
	rec_data_callback rec_cb, void* rec_cb_data )
{
	TcpServer* tcps = new TcpServer(listen_port);
	if (tcps != NULL)
	{
		tcps->reg_callback(conn_cb, conn_cb_data, rec_cb, rec_cb_data);
		if (tcps->start())
		{
			return (long)tcps;
		}

		delete tcps;
		return -1;
	}
	return -2;
}

extern "C" long __stdcall tnet_stop(long server_handle)
{
	TcpServer* tcps = (TcpServer*)server_handle;
	if (tcps != NULL)
	{
		return tcps->stop() ? 0 : -1;
	}
	return -2;
}

extern "C" unsigned long __stdcall tnet_connect(const char* server_ip, unsigned short server_port,
	connected_callback conn_cb, void* conn_cb_data, rec_data_callback rec_cb, void* rec_cb_data)
{
	SOCKET sSocket;
	struct sockaddr_in serveraddr;
	FD_SET mask;
	u_long value = 1;
	TIMEVAL timeout;

	sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sSocket == INVALID_SOCKET)
	{
		sSocket = 0;
	}
	else
	{
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(server_port);
		serveraddr.sin_addr.s_addr = inet_addr(server_ip);

		ioctlsocket(sSocket, FIONBIO, &value);//设置为非阻塞
		connect(sSocket, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		FD_ZERO(&mask);
		FD_SET(sSocket, &mask);

		value = select(NULL, NULL, &mask, NULL, &timeout);
		if (value && value != SOCKET_ERROR)
		{
			//连接成功
			value = 0;
			ioctlsocket(sSocket, FIONBIO, &value);//设置为阻塞
			TcpClient* client = new TcpClient(sSocket, conn_cb, conn_cb_data, rec_cb, rec_cb_data);
			if ((client!=NULL) && client->start())
			{
				TcpClientManage::GetInstance()->add_client(sSocket, client);
			}
			else
			{
				if (client != NULL)
				{
					delete client;
				}
				else 
				{
					shutdown(sSocket, SD_BOTH);
					closesocket(sSocket);
					sSocket = 0;
				}
			}
		}
		else
		{
			shutdown(sSocket, SD_BOTH);
			closesocket(sSocket);
			sSocket = 0;
		}
	}

	return sSocket;
}

extern "C" long __stdcall tnet_send(unsigned long socket, char* buffer, int buffer_size)
{
	return _SocketSendFunc(socket, buffer, buffer_size);
}

//断开连接
extern "C" long __stdcall tnet_close(unsigned long socket)
{
	::shutdown(socket, SD_BOTH);
	closesocket(socket);
	return ::WSAGetLastError();
}