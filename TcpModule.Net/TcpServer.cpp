#include "StdAfx.h"
#include "TcpServer.h"
//#include "CLog.h"

//接收數據包
int _SocketRecvFunc(SOCKET s, char *buf, int len)
{
	fd_set fdread;
	int iRecvlen, iRead, iRecError;
	int iResult=0;
	LONG lRecvSize = len;
	timeval timeout={3,0};//超时时长3秒 
	
	while(s!=INVALID_SOCKET)
	{
		FD_ZERO(&fdread);
		FD_SET(s,&fdread);

		iRead = select( 0, &fdread, NULL, NULL, &timeout );

		if(iRead==0)//接收超时
			break;

		if(iRead == SOCKET_ERROR)
		{
			iResult = iRead;
			break;
		}

		if(iRead > 0)
		{
			if(FD_ISSET(s, &fdread))
			{
				iRecvlen = recv(s, buf+(len-lRecvSize), lRecvSize, NULL);

				if(iRecvlen==0)
				{
					iResult = iRecvlen;
					break;
				}

				if(iRecvlen==SOCKET_ERROR)
				{
					iRecError = WSAGetLastError();

					if((iRecError==WSAENOTCONN)//(WSABASEERR+57)
						|| (iRecError==WSAESHUTDOWN)//(WSABASEERR+58)
						|| (iRecError==WSAENOTSOCK)		
						|| (iRecError==WSAENETDOWN)		
						|| (iRecError==WSAECONNABORTED)//(WSABASEERR+53)
						|| (iRecError==WSAETIMEDOUT)//(WSABASEERR+60)
						|| (iRecError==WSAECONNRESET))//(WSABASEERR+54)
					{
						iResult = iRecvlen;
						break;
					}
				}

				lRecvSize -= iRecvlen;
				iResult += iRecvlen;

				//接收到完整数据包后再处理（因为每次recv的数据长度不一定等于发送包的长度）
				if (lRecvSize<=0)
					break;
			}
		}
	}

	return iResult;
}

//发送数据包
int _SocketSendFunc(SOCKET s, const char *buf, int len)
{
	fd_set fdwrite;
	int iSendlen;
	int lSendSize=0;
	int iResult=0;
	int iWrite;
	timeval timeout={2,0};//超时时长	

	if (INVALID_SOCKET==s)
		return SOCKET_ERROR;

	do
	{
		FD_ZERO(&fdwrite);
		FD_SET(s,&fdwrite);
	
		iWrite = select(0, NULL, &fdwrite, NULL, &timeout);
		
		if (iWrite==0)
		{
			iResult = iWrite;
			break;
		}
		if (iWrite==SOCKET_ERROR)
		{
			iResult=iWrite;
			break;
		}
		if (iWrite>0)
			if (FD_ISSET(s,&fdwrite))
			{
				iSendlen = send(s, buf+lSendSize, len-lSendSize, NULL);
				if (iSendlen == SOCKET_ERROR)
				{
					iResult = iSendlen;
					break;
				}
				else
					lSendSize += iSendlen;

				iResult = lSendSize;
			}
	}while((lSendSize < len) && (s != INVALID_SOCKET));//判断是否发送完毕

	return iResult;
}

/////////////////////////////////
TcpServer::TcpServer(unsigned short port)
	: m_port(port)
	, m_listen_socket(INVALID_SOCKET)
	, m_conn_cb(NULL)
	, m_rec_cb(NULL)
	, m_rec_cb_data(NULL)
	, m_conn_cb_data(NULL)
{
}

TcpServer::~TcpServer(void)
{
	stop();
}

bool TcpServer::start()
{
	if (m_listen_socket == INVALID_SOCKET)
	{
		m_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (INVALID_SOCKET != m_listen_socket )
		{
			struct sockaddr_in Addr = {0};
			Addr.sin_family = AF_INET;
			Addr.sin_port = htons((u_short)m_port);
			Addr.sin_addr.s_addr = htonl(INADDR_ANY);

			if (SOCKET_ERROR != ::bind(m_listen_socket, (sockaddr*)&Addr, sizeof(Addr)))
			{
				if( SOCKET_ERROR != ::listen(m_listen_socket, 8) )
				{
					::_beginthread(TcpServer::_accept_thread, 0, this);
				}
			}
			else 
			{
				::closesocket(m_listen_socket);
				m_listen_socket = INVALID_SOCKET;
			}
		}
	}

	return (m_listen_socket != INVALID_SOCKET) ? true : false;
}

bool TcpServer::stop()
{
	if (m_listen_socket != INVALID_SOCKET)
	{
		::closesocket(m_listen_socket);
		m_listen_socket = INVALID_SOCKET;
		return true;
	}

	return false;
}

void TcpServer::on_accept()
{
	if (INVALID_SOCKET == m_listen_socket)
	{
#ifdef _DEBUG
		CLog::GetObj().tmWrite("[tnms] INVALID_SOCKET == m_listen_socket");
#endif
		return;
	}

	SOCKET accept_socket = accept( m_listen_socket, NULL, NULL );

	if( INVALID_SOCKET != accept_socket )
	{
		if (INVALID_SOCKET != m_listen_socket)
			::_beginthread(TcpServer::_accept_thread, 0, this);

		TcpClient* client = new TcpClient(accept_socket, m_conn_cb, m_conn_cb_data, m_rec_cb, m_rec_cb_data);
		if (client != NULL)
		{
			if(client->start())
			{
				TcpClientManage::GetInstance()->add_client(accept_socket, client);
			}
			else
			{
				delete client;
			}
		}
		else
		{
			::shutdown(accept_socket, SD_BOTH);
			closesocket(accept_socket);
#ifdef _DEBUG
			CLog::GetObj().tmWrite("[tnms] new TcpClient 失败");
#endif
		}
	}
}
////////////////////////////////////////
TcpClient::TcpClient(SOCKET& client_socket, connected_callback conn_cb, void* conn_cb_data, rec_data_callback rec_cb, void* rec_cb_data)
	: m_is_running(false)
	, m_rec_thread(NULL)
	, m_client_socket(client_socket)
	, m_conn_cb(conn_cb)
	, m_rec_cb(rec_cb)
	, m_conn_cb_data(conn_cb_data)
	, m_rec_cb_data(rec_cb_data)
{
}

TcpClient::~TcpClient()
{
	stop();
}

bool TcpClient::start()
{
	if (m_rec_thread == NULL)
	{
		m_is_running = true;
		m_rec_thread = (HANDLE)::_beginthreadex(NULL, 0, TcpClient::_recv_thread, this, 0, NULL);
		if (m_rec_thread == NULL)
		{
			m_is_running = false;
#ifdef _DEBUG
			CLog::GetObj().tmWrite("[tnms] TcpClient::start() 失败");
#endif
		}
	}

	return m_is_running;
}

void TcpClient::stop()
{
	m_is_running = false;
	if (m_client_socket != INVALID_SOCKET)
	{
		::shutdown(m_client_socket, SD_BOTH);
		closesocket(m_client_socket);
	}

	if (m_rec_thread != NULL)
	{
		::WaitForSingleObject(m_rec_thread, INFINITE);
		::CloseHandle(m_rec_thread);
		m_rec_thread = NULL;
	}

	m_client_socket = INVALID_SOCKET;
}

void TcpClient::on_recvs()
{
	const int MAX_BUFFER_SIZE = 1024;
	char buffer[MAX_BUFFER_SIZE];
	int buffer_size = MAX_BUFFER_SIZE;
	int recv_size = 0;

	//通知链路已建立完毕
	if ((m_client_socket != INVALID_SOCKET) && (m_conn_cb != NULL))
	{
		m_conn_cb(m_client_socket, STATUS_CONNECTED, m_conn_cb_data);
	}

	while (m_is_running)
	{
		memset(buffer, 0, buffer_size);
		recv_size = 0;
		recv_size = _SocketRecvFunc(m_client_socket, buffer, buffer_size);

		if (recv_size == SOCKET_ERROR)
			break;

		if (recv_size > 0)
		{
			if (m_rec_cb != NULL)
			{
				m_rec_cb(m_client_socket, buffer, recv_size, m_rec_cb_data);
			}
		}
	}

	//通知链路断开
	if ((m_client_socket != INVALID_SOCKET) && (m_conn_cb != NULL))
	{
		m_conn_cb(m_client_socket, STATUS_DISCONNECT, m_conn_cb_data);
	}

	m_is_running = false;
}

/////////////////////////////////////
TcpClientManage TcpClientManage::_instance;

TcpClientManage::TcpClientManage()
	: m_is_running(false)
	, m_manage_thread(NULL)
{
	::InitializeCriticalSection(&m_cs);

}

TcpClientManage::~TcpClientManage()
{
	cleanup();
	::DeleteCriticalSection(&m_cs);
}

void TcpClientManage::init()
{
	if (m_manage_thread == NULL)
	{
		m_is_running = true;
		m_manage_thread = (HANDLE)::_beginthreadex(NULL, 0, TcpClientManage::_manage_thread, this, 0, NULL);
	}
}

void TcpClientManage::cleanup()
{
	m_is_running = false;
	if (m_manage_thread != NULL)
	{
		::WaitForSingleObject(m_manage_thread, INFINITE);
		::CloseHandle(m_manage_thread);
		m_manage_thread = NULL;
	}

}

void TcpClientManage::add_client(SOCKET socket, TcpClient* client)
{
	if (m_is_running)
	{
		::EnterCriticalSection(&m_cs);
		m_clients.insert(std::pair<SOCKET, TcpClient*>(socket, client));
		::LeaveCriticalSection(&m_cs);
	}
}

void TcpClientManage::on_manage_clients()
{
	ClientMapIter it = m_clients.begin();
	while (m_is_running)
	{
		Sleep(1000);

		::EnterCriticalSection(&m_cs);

		if (it != m_clients.end())
			it = m_clients.begin();

		while (it != m_clients.end())
		{
			if (!it->second->is_running())
			{
				it->second->stop();
				delete it->second;
				it = m_clients.erase(it);

				break;
			}
		}		

		::LeaveCriticalSection(&m_cs);
	}

	::EnterCriticalSection(&m_cs);
	it = m_clients.begin();
	while (it != m_clients.end())
	{
		it->second->stop();
		delete it->second;
		it = m_clients.erase(it);
	}
	::LeaveCriticalSection(&m_cs);
}
