#pragma once

enum
{
	STATUS_CONNECTED = 0,
	STATUS_DISCONNECT = 1
};

//连接通知
typedef void (__stdcall *connected_callback)(unsigned long socket, int status, void* user_data);

//接收到数据通知
typedef void (__stdcall *rec_data_callback)(unsigned long socket, char* buffer, int buffer_size, void* user_data);

//初始化与反初始化
extern "C" long __stdcall tnet_init();
extern "C" void __stdcall tnet_cleanup();

//创建tcp服务端，启动服务
//参数:
//listen_port:  监听端口
//conn_cb:      连接回调函数指针
//conn_cb_data: 连接回调函数最后一个参数，用户参数
//rec_cb:       数据接收回调
//rec_cb_data:  数据接收回调函数最后一个参数，用户参数
//返回值: >0服务句柄， 其它值为错误码
extern "C" long __stdcall tnet_start(unsigned short listen_port, connected_callback conn_cb, void* conn_cb_data, rec_data_callback rec_cb, void* rec_cb_data );

//停止服务
//server_handle: tnet_start返回值
extern "C" long __stdcall tnet_stop(long server_handle);

//连接远程服务
//参数:
//server_ip:    服务地址
//server_port:  服务端口
//conn_cb:      连接回调函数指针
//conn_cb_data: 连接回调函数最后一个参数，用户参数
//rec_cb:       数据接收回调
//rec_cb_data:  数据接收回调函数最后一个参数，用户参数
//返回值: >0 socket句柄， 其它值为错误码
extern "C" unsigned long __stdcall tnet_connect(const char* server_ip, unsigned short server_port, connected_callback conn_cb, void* conn_cb_data, rec_data_callback rec_cb, void* rec_cb_data);

//发送数据
//返回值: >0发送数据长度, =0发送失败
extern "C" long __stdcall tnet_send(unsigned long socket, char* buffer, int buffer_size);

//断开连接
extern "C" long __stdcall tnet_close(unsigned long socket);