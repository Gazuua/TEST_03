#pragma once

#include<iostream>
#include<list>
#include<map>
#include<string>
#include<stdlib.h>
#include<process.h>
#include<WinSock2.h>
#include<Windows.h>
#include<ctime>

#include"CPacket.h"
#include"CPacketQueue.h"

#pragma warning(disable:4996)

using namespace std;

#define READ 0
#define WRITE 1
#define MAX_BUFFER_SIZE 10000

// GQCS���� Ȱ���� Ŭ���̾�Ʈ ����(1Ŭ���̾�Ʈ�� 1�Ҵ�)
typedef struct
{
	SOCKET			hClientSocket;		// Ŭ���̾�Ʈ ����
	SOCKADDR_IN		clientAddress;		// Ŭ���̾�Ʈ ��Ʈ��ũ�� �ּ�
	CPacketQueue* packetQueue;			// ����ȭ�� ��Ŷ�� �ӽ� ������ ���� ť
	// Ŭ���̾�Ʈ Ŭ���� �����ؾ� ��
}PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

// GQCS���� Ȱ���� �޼��� ����(1����´� 1�Ҵ�)
typedef struct
{
	OVERLAPPED		overlapped;					// OVERLAPPED IO �̿뿡 ���̴� ����ü
	WSABUF			wsaBuf;						// OVERLAPPED IO �̿뿡 ���̴� ���� ������ ����ü
	char			buffer[MAX_BUFFER_SIZE];	// OVERLAPPED IO�� ���Ǵ� ���̷�Ʈ ����
	int				readWriteFlag;				// ���� IO �۾��� �������� ǥ���ϴ� ����
}PER_IO_DATA, * LPPER_IO_DATA;

// CIOCPServer Ŭ����
// ��Ư�� �ټ��� Ŭ���̾�Ʈ�� ��Ʈ��ũ ��� ����� �����ϴ� Ŭ����
class CIOCPServer
{
public:
	// Singleton Pattern ������ ���� ��� �Լ�
	static CIOCPServer* GetInstance();

	// ��Ÿ public ��� �Լ�
	bool Init(const int PORT);	// ���� �ʱ�ȭ �Լ�
	void Release();				// �̱��� ��ü�� �޸𸮿��� ������ �� ȣ��Ǵ� �Լ�

private:
	// ������
	CIOCPServer() {};
	~CIOCPServer() {};

	// Singleton Pattern ������ ���� ���
	static CIOCPServer* m_pInstance;

	// ��Ÿ private ���
	WSADATA			m_WsaData;			// Windows Socket API Startup�� ���̴� ����ü
	HANDLE			m_hCP;				// Windows OS���� �����ϴ� IO Multiplexer(IO Completion Port)
	SOCKET			m_hServerSocket;	// Ŭ���̾�Ʈ�� Accept�ϱ� ���� ���̴� ���� ����
	SOCKADDR_IN		m_ServerAddress;	// ���� ������ ���ε��ϱ� ���� ���̴� �ּ� ����ü

	map<SOCKET, string>		m_UserList;		// ���� ������ ���� ���� ���� ����Ʈ
	CRITICAL_SECTION		m_CS;			// ���� Ŭ���� ���� ���� ����� �ǵ帱 �� ��ױ� ���� CS

	// ��Ÿ private ��� �Լ�
	static unsigned int __stdcall acceptProcedure(void* nullpt);			// acceptor
	static unsigned int __stdcall workerProcedure(void* hCompletionPort);	// worker

	// �������� �б⸦ ��ġ�� ���⸦ �� �� ������ �����ϴ� �Լ�
	void sendRequest(SOCKET socket, char* data, unsigned char size, unsigned char code);

	// ��Ÿ ������ �ɵ������� Ŭ���̾�Ʈ�� �޼����� ������ �� �� ���� �Լ���
	string configureMessage(string name, string msg);
	void RefreshUserList();
	void NoticeMessage(string name, string msg);
	void WhisperMessage(string fromname, string toname, string msg);

	void initWSARecv(LPPER_IO_DATA* ioInfo);
	void initWSASend(LPPER_IO_DATA* ioInfo, const char* message, int msgLength);
	void closeClient(LPPER_HANDLE_DATA* handleInfo, LPPER_IO_DATA* ioInfo);
};