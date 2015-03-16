#pragma once

enum
{
	STATUS_CONNECTED = 0,
	STATUS_DISCONNECT = 1
};

//����֪ͨ
typedef void (__stdcall *connected_callback)(unsigned long socket, int status, void* user_data);

//���յ�����֪ͨ
typedef void (__stdcall *rec_data_callback)(unsigned long socket, char* buffer, int buffer_size, void* user_data);

//��ʼ���뷴��ʼ��
extern "C" long __stdcall tnet_init();
extern "C" void __stdcall tnet_cleanup();

//����tcp����ˣ���������
//����:
//listen_port:  �����˿�
//conn_cb:      ���ӻص�����ָ��
//conn_cb_data: ���ӻص��������һ���������û�����
//rec_cb:       ���ݽ��ջص�
//rec_cb_data:  ���ݽ��ջص��������һ���������û�����
//����ֵ: >0�������� ����ֵΪ������
extern "C" long __stdcall tnet_start(unsigned short listen_port, connected_callback conn_cb, void* conn_cb_data, rec_data_callback rec_cb, void* rec_cb_data );

//ֹͣ����
//server_handle: tnet_start����ֵ
extern "C" long __stdcall tnet_stop(long server_handle);

//����Զ�̷���
//����:
//server_ip:    �����ַ
//server_port:  ����˿�
//conn_cb:      ���ӻص�����ָ��
//conn_cb_data: ���ӻص��������һ���������û�����
//rec_cb:       ���ݽ��ջص�
//rec_cb_data:  ���ݽ��ջص��������һ���������û�����
//����ֵ: >0 socket����� ����ֵΪ������
extern "C" unsigned long __stdcall tnet_connect(const char* server_ip, unsigned short server_port, connected_callback conn_cb, void* conn_cb_data, rec_data_callback rec_cb, void* rec_cb_data);

//��������
//����ֵ: >0�������ݳ���, =0����ʧ��
extern "C" long __stdcall tnet_send(unsigned long socket, char* buffer, int buffer_size);

//�Ͽ�����
extern "C" long __stdcall tnet_close(unsigned long socket);