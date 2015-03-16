// TcpNetModule.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <iostream>

#include "../TcpModule.Net/TcpModule.Net.h"
#pragma comment(lib, "../Debug/TcpModule.Net.lib")


unsigned long server_socket = 0;

void __stdcall _connected_callback(unsigned long socket, int status, void* user_data)
{
	if (socket != server_socket)
		std::cout << "�����: socket=" << socket << " status=" << status << std::endl;
	else
		std::cout << "�ͻ���: socket=" << socket << " status=" << status << std::endl;
}

void __stdcall _rec_data_callback(unsigned long socket, char* buffer, int buffer_size, void* user_data)
{
	if (socket != server_socket)
	{
		std::cout << "�����: socket=" << socket << " buffer=" << buffer << " buffer_size=" << buffer_size << std::endl;
		std::cout << "�����Ӧ��: " << tnet_send(socket, "54321", 5) << std::endl;
	}
	else
	{
		std::cout << "�ͻ���: socket=" << socket << " buffer=" << buffer << " buffer_size=" << buffer_size << std::endl;
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "��ʼ��TcpModule.Net.dll: " << tnet_init() << std::endl;


	int cmd = 0;
	long server_handle = 0;
	

	std::cout << "����ָ��" << std::endl;
	while (cmd != 10)
	{ 
		cmd = 0;
		std::cout << "->";
		std::cin >> cmd;

		switch (cmd)
		{
		case 1:
			server_handle = tnet_start(1234, _connected_callback, 0, _rec_data_callback, 0);
			std::cout << "��������: " << server_handle << "=>127.0.0.1:1234" << std::endl;
			break;
		case 2:
			std::cout << "ֹͣ����: " << server_handle << "=>" << tnet_stop(server_handle) << std::endl;
			server_handle = 0;
			break;
		case 3:
			server_socket = tnet_connect("127.0.0.1", 1234, _connected_callback, 0, _rec_data_callback, 0);
			std::cout << "���ӷ���: " << server_socket << std::endl;
			break;
		case 4:
			std::cout << "�Ͽ�����: " << server_socket << "=>" << tnet_close(server_socket) << std::endl;
			server_socket = 0;
			break;
		case 5:
			if (server_socket != 0)
			{
				std::cout << "���Ͳ�������: " << server_socket << "=>"<< tnet_send(server_socket, "12345", 5) << std::endl;
			}
			else
			{
				std::cout << "δ��������" << std::endl;
			}
			break;
		default:
			std::cout << "��Чָ��" << std::endl;
			break;
		}
		
	}



	std::cout << "�ͷ�TcpModule.Net.dll" << std::endl;
	tnet_cleanup();

	std::cout << "����������˳�" << std::endl;
	std::cout << "->";
	std::cin >> cmd;

	return 0;
}

