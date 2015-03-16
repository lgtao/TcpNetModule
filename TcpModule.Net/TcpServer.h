#pragma once
#include "TcpModule.Net.h"
#include <map>

int _SocketSendFunc(SOCKET s, const char *buf, int len);

class TcpServer
{
public:
	TcpServer(unsigned short port);
	~TcpServer(void);

	void reg_callback(connected_callback conn_cb, void* conn_cb_data, rec_data_callback rec_cb, void* rec_cb_data)
	{
		m_conn_cb = conn_cb;
		m_rec_cb = rec_cb;
		m_conn_cb_data = conn_cb_data;
		m_rec_cb_data = rec_cb_data;
	}
	
	bool start();
	bool stop();

private:
	
	static void _accept_thread(void* param)
	{
		TcpServer* server = (TcpServer*)param;
		if(server != NULL)
			server->on_accept();
	}

private:
	TcpServer(void){}

	void on_accept();

	unsigned short m_port;
	SOCKET m_listen_socket;

	connected_callback m_conn_cb;
	rec_data_callback m_rec_cb;
	void* m_rec_cb_data;
	void* m_conn_cb_data;
};


class TcpClient
{
public:
	TcpClient(SOCKET& client_socket, connected_callback conn_cb, void* conn_cb_data, rec_data_callback rec_cb, void* rec_cb_data);
	~TcpClient();

	bool is_running() { return m_is_running; }
	bool start();
	void stop();

private:
	TcpClient(){}

	static unsigned int __stdcall _recv_thread(void* param)
	{
		TcpClient* client = (TcpClient*)param;
		if (client != NULL)
			client->on_recvs();

		return 0;
	}
	void on_recvs();

	bool m_is_running;
	SOCKET m_client_socket;
	HANDLE m_rec_thread;

	connected_callback m_conn_cb;
	rec_data_callback m_rec_cb;
	void* m_rec_cb_data;
	void* m_conn_cb_data;
};

typedef std::map<SOCKET, TcpClient*> ClientMap;
typedef std::map<SOCKET, TcpClient*>::iterator ClientMapIter;

class TcpClientManage
{
public:
	~TcpClientManage();
	
	static TcpClientManage* GetInstance() { return &_instance; }

	void init();
	void add_client(SOCKET socket, TcpClient* client);
	void cleanup();

private:
	TcpClientManage();

	static unsigned int __stdcall _manage_thread(void* param)
	{
		TcpClientManage* manager = (TcpClientManage*)param;
		if (manager != NULL)
			manager->on_manage_clients();

		return 0;
	}
	void on_manage_clients();

	bool m_is_running;
	HANDLE m_manage_thread;
	ClientMap m_clients;
	CRITICAL_SECTION m_cs;


	static TcpClientManage _instance;
};
