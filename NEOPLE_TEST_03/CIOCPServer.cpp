#include "CIOCPServer.h"

// �̱��� �ν��Ͻ��� ���α׷����� ȣ���ϴ� �Լ�
CIOCPServer* CIOCPServer::GetInstance()
{
	// ���� �ν��Ͻ��� �޸� �Ҵ�
	if (!m_pInstance) {
		m_pInstance = new CIOCPServer();
	}
	return m_pInstance;
}

// �޸𸮿��� ����
void CIOCPServer::Release()
{
	WSACleanup();
	DeleteCriticalSection(&GetInstance()->m_CS);
	delete m_pInstance;
	m_pInstance = nullptr;
}

// IOCP ���� �ʱ�ȭ �Լ�
bool CIOCPServer::Init(const int PORT)
{
	SYSTEM_INFO			SystemInfo;		// CPU �ھ� ���� �ľ�
	char				hostname[256];	// �ڽ��� hostname �ľ�
	hostent* host;						// �ڽ��� ip �ּ� �ľ�
	unsigned char		ipAddr[4];		// �ڽ��� ip �ּ� ����

	// Windows Socket API(WSA)�� �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &m_WsaData) != 0)
		return false;

	// CRITICAL_SECTION �ڵ� �Ҵ�
	InitializeCriticalSection(&GetInstance()->m_CS);

	// I/O Completion Port �ڵ� �Ҵ�
	m_hCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// System���� ��� ������ CPU �ھ� ������ 2�踸ŭ worker thread ����
	// �μ��� IOCP �ڵ��� �Ҵ������μ� �ü������ �Ϸ��ȣ�� ���� �� �ְ� �Ѵ�
	// (�ھ�� * 2 = ����ũ�μ���Ʈ���� �����ϴ� ��ġ)
	GetSystemInfo(&SystemInfo);
	for (unsigned int i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++)
		_beginthreadex(NULL, 0, workerProcedure, (LPVOID)m_hCP, 0, NULL);

	// OVERLAPPED IO(��ø�� �����)�� �����ϵ��� WSASocket �Լ��� ���� ������ �Ҵ�
	m_hServerSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&m_ServerAddress, 0, sizeof(m_ServerAddress));
	m_ServerAddress.sin_family = AF_INET;
	m_ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	m_ServerAddress.sin_port = htons(55248);

	// bind �� listen
	::bind(m_hServerSocket, (SOCKADDR*)&m_ServerAddress, sizeof(m_ServerAddress));
	listen(m_hServerSocket, SOMAXCONN);

	// listen �غ� ������ accept�� ���� accept thread�� �ϳ� ����
	_beginthreadex(NULL, 0, acceptProcedure, NULL, 0, NULL);

	// ��� ���� ���� �غ� ������ ������ IP�ּҿ� ��Ʈ��ȣ�� �ܼ�â�� ����Ͽ� �˸�
	gethostname(hostname, 256);
	host = gethostbyname(hostname);
	for (int i = 0; i < 4; i++)
		ipAddr[i] = *(host->h_addr_list[0] + i);

	puts("============= IOCP ���� �ʱ� ���� �Ϸ�!=============");
	printf("���� �ּ� -> %d.%d.%d.%d:%d\n", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3], PORT);

	return true;
}

// Ŭ���̾�Ʈ�� ���ӿ� ���� ������ ����ϴ� accept thread�� proc �Լ�
unsigned int __stdcall CIOCPServer::acceptProcedure(void* nullpt)
{
	SOCKET				hClientSocket;		// accept�� Ŭ���̾�Ʈ ������ ���� �ڵ�
	SOCKADDR_IN			clientAddress;		// Ŭ���̾�Ʈ �ּ� ����ü
	LPPER_HANDLE_DATA	handleInfo;			// Ŭ���̾�Ʈ�� �������ִ� ���� �����ϱ� ���� ����ü
	LPPER_IO_DATA		ioInfo;				// Ŭ���̾�Ʈ�κ��� �Է¹ޱ� ���� �Ҵ�Ǵ� ����ü

	int					addrLength = sizeof(clientAddress);
	DWORD				flags = 0;

	while (1)
	{
		// Ŭ���̾�Ʈ�� accept �޴´�
		hClientSocket = accept((SOCKET)GetInstance()->m_hServerSocket, (SOCKADDR*)&clientAddress, &addrLength);

		// Ŭ���̾�Ʈ�� ���� ���� �� ���� HANDLE_DATA ����ü�� �Ҵ��Ѵ�.
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClientSocket = hClientSocket;
		memcpy(&(handleInfo->clientAddress), &clientAddress, addrLength);
		handleInfo->packetQueue = new CPacketQueue();

		// Ŭ���̾�Ʈ�� ������ �ܼ�â�� �˸���.
		printf("Client Accepted :: %s/%d\n", inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port);

		// �ش� ������ Completion Port�� �߰��Ͽ� �����Ѵ�.
		CreateIoCompletionPort((HANDLE)hClientSocket, GetInstance()->m_hCP, (DWORD)handleInfo, 0);

		// ���� WSARecv�� ���� �����Ѵ�.
		GetInstance()->initWSARecv(&ioInfo);
		WSARecv(handleInfo->hClientSocket, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
	}
	return 0;
}

// ���� ������ Recv/Send ������ �����ϴ� proc �Լ�
unsigned int __stdcall CIOCPServer::workerProcedure(void* hCompletionPort)
{
	HANDLE				hCP = (HANDLE)hCompletionPort;
	SOCKET				socket;
	DWORD				transferredBytes;
	BOOL				gqcsRet;

	LPPER_HANDLE_DATA	handleInfo;
	LPPER_IO_DATA		ioInfo;
	DWORD				flags = 0;

	while (1)
	{
		// GQCS(���Ͽ��� ��� �۾��� �Ϸ�Ǹ� ��ȯ�ϴ� �Լ�)�� ���ϰ��� �޴´�.
		gqcsRet = GetQueuedCompletionStatus(hCP, &transferredBytes, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		socket = handleInfo->hClientSocket;

		// ���ϰ��� 1�� ��� (�������� I/O ����� �������� ���)
		if (gqcsRet)
		{
			// �б� �Ϸ� ��
			if (ioInfo->readWriteFlag == READ)
			{
				// EOF(���� ���� ��ȣ) ���� �� Ŭ���̾�Ʈ���� ������ ���´�
				if (transferredBytes == 0)
				{
					CIOCPServer::GetInstance()->closeClient(&handleInfo, &ioInfo);
					continue;
				}

				// �� �κ��� ��ü �������ݿ� ����Ͽ� PacketQueue�� �̿��ϴ� ��� ����̴�.
				// ��Ŷ�� �߸��ų� �پ �� ���� ���� ���� ��Ŷ�� ó���Ͽ� �ش�.
				// OnRecv()�� ��ȯ���� Ŭ����ȭ�Ǿ� �ٷ� ����� �� �ִ� ��Ŷ�� �����̴�.
				int recv = handleInfo->packetQueue->OnRecv(ioInfo->buffer, transferredBytes);

				// OnRecv()���� -1�� ��ȯ�ϸ� ���� �߸��Ǿ����� Ŭ���̾�Ʈ�� ������ �����ϸ� �ȴ�.
				if (recv == -1) {
					CIOCPServer::GetInstance()->closeClient(&handleInfo, &ioInfo);
					continue;
				}

				// �̹� WSARecv�� ���� ioInfo�� �� �̻� �ʿ����� ������ �����Ѵ�.
				free(ioInfo);

				// ��� ������ ��Ŷ ������ŭ �м� �� �۾� ��û�� ��� �����Ѵ�.
				// �ܺ� ��⿡�� �۾��� �ܺ� ����� �����带 �̿��ϸ� ����� ���߿� �޴´�.
				// ��� ó�� ������ ���� �۾��� ��ٷ� ó���� �ش�.
				for (int i = 0; i < recv; i++)
				{
					// ������ ��Ŷ�� ���� �и��Ѵ�.
					// ���� ������ ��Ŷ�� ���� ������ for�� Ż�� �� �ڵ����� �Ҹ�ȴ�.
					CPacket packet = *(handleInfo->packetQueue->getPacket());
					unsigned char size = packet.GetDataSize();
					unsigned char code = packet.GetCode();
					char* data = packet.GetData();

					switch (packet.GetCode())
					{
					// �⺻ �޼����� ���� ��ο��� �����ϴ� ��Ŷ
					case PACKET_STANDARD:
					{
						// 1. ������ �̸��� ���� �޼����� �����Ѵ�.
						string name(GetInstance()->m_UserList[socket]);
						string msg(data, size);

						// 2. ��� �������� �Ѹ���.
						GetInstance()->NoticeMessage(name, msg);
					}
					break;

					// ������ �г��� ���� ��û�� ���� �����ϴ� ��Ŷ
					case PACKET_NICKNAME:
					{
						// 1. ������ ���� �г����� �����Ѵ�.
						string name(data, size);

						// 2. �ش��ϴ� �г����� �̹� �ִ��� ����Ʈ�� ��ȸ�ϸ� üũ�Ѵ�.
						bool bDup = false;

						EnterCriticalSection(&GetInstance()->m_CS);
						map<SOCKET, string>::iterator iter;
						for (iter = GetInstance()->m_UserList.begin(); iter != GetInstance()->m_UserList.end(); iter++)
						{
							// ���� ���� ������ bDup�� true�� �ٲ۴�.
							if (iter->second._Equal(name))
								bDup = true;
						}
						LeaveCriticalSection(&GetInstance()->m_CS);

						// 3. ����� ���� �������� ������ ���䰪�� �����Ѵ�.
						char result[7];
						unsigned char resultSize = 0;

						// �ߺ��� ������ ����
						if (bDup) {
							resultSize = 4;
							memcpy(result, "FAIL", resultSize);
						} // ������ �ʿ� �г����� �߰� �� ������ �˸���
						else {
							EnterCriticalSection(&GetInstance()->m_CS);
							GetInstance()->m_UserList[socket] = name;
							LeaveCriticalSection(&GetInstance()->m_CS);

							resultSize = 7;
							memcpy(result, "SUCCESS", resultSize);
						}

						// 4. ����� �ش� �������� �����Ѵ�.
						GetInstance()->sendRequest(socket, result, resultSize, PACKET_NICKNAME);
						
						// 5. ������ ��� ��ó���� ���ش�
						if (!bDup) {
							GetInstance()->RefreshUserList();
							GetInstance()->NoticeMessage(name, "�濡 �����Ͽ����ϴ�. �ݰ����ϴ�.");
						}
					}
					break;

					// ������ ��/�������� ���Ͽ� ���� ����Ʈ ������ �ʿ��� ��� �����ϴ� ��Ŷ
					case PACKET_USERLIST:
					{
						GetInstance()->RefreshUserList();
					}
					break;

					// ������ �ӼӸ� ��û�� ������ ���� �����ϴ� ��Ŷ
					// ������ �� �� - "�ӼӸ� �����/�޼���"
					// ����ڿ��� �� �� - "�ӼӸ� ���� ���/�޼���"
					case PACKET_WHISPER:
					{
						// 1. ������ �̸��� ���� �޼����� �����Ѵ�.
						string fromname(GetInstance()->m_UserList[socket]);
						string strdata(data, size);
						string toname(strdata.substr(0, strdata.find('/')));
						string msg(strdata.substr(strdata.find('/') + 1, strdata.length() - strdata.find('/')));

						// 2. �ӼӸ� ����ڿ� �ڽſ��� ������ �ӼӸ��� ������.
						GetInstance()->WhisperMessage(fromname, toname, msg);
					}
					break;

					default:
						break;
					}
				}

				// ����� ��û�� ��� �����ٸ� ���� ��Ŷ ������ ���� WSARecv�� �����Ѵ�.
				LPPER_IO_DATA newIoInfo;
				GetInstance()->initWSARecv(&newIoInfo);
				WSARecv(socket, &(newIoInfo->wsaBuf), 1, NULL, &flags, &(newIoInfo->overlapped), NULL);
			}
			// ���� �Ϸ� �� Ŭ���̾�Ʈ�κ��� ������ ���� ���� �����Ƿ� �ٷ� �޸𸮸� �����Ѵ�
			else free(ioInfo);
		}
		// ���ϰ��� 0�� ��� (���� ó���� �ؾ� �ϴ� ����� �������� ���)
		else
		{
			int err = WSAGetLastError();
			switch (err)
			{
			case 64: // Ŭ���̾�Ʈ(����) ������ ����Ǿ����� �ǹ��ϴ� �ڵ�
				puts("INFO :: Ŭ���̾�Ʈ ���μ����� ���Ḧ �����Ͽ�, �ش� Ŭ���̾�Ʈ���� ������ �����մϴ�.");
				CIOCPServer::GetInstance()->closeClient(&handleInfo, &ioInfo);
				break;
			default: // ������ �𸣰����� ������ �ִ� ��� �ϴ� ����Ѵ�.
				printf("============ ERROR CODE :: %d\n", err);
				break;
			}
		}
	}
	return 0;
}

// �⺻ �޼��� ���¸� �����ϴ� �Լ��̴�
// [��:��:��] �̸� :: �޼��� �����̴�
string CIOCPServer::configureMessage(string name, string msg)
{
	// 1. ������ ���� �޼����� ���Ŀ� �°� �ϼ��Ѵ�.
	time_t now = time(0);
	tm* ltm = localtime(&now);
	char ret[256];
	sprintf(ret, "[%d:%d:%d] %s :: %s", ltm->tm_hour, ltm->tm_min, ltm->tm_sec, name.c_str(), msg.c_str());

	return string(ret);
}

// ���� ����Ʈ�� ��� �������� �����Ͽ� ������Ʈ���ش�
void CIOCPServer::RefreshUserList()
{
	// 1. ���� ����Ʈ�� �ҷ��ͼ� ���ڿ��� �����Ѵ�.
	// "����1/����2/����3/..." �������� �޼����� �����Ѵ�
	string liststr = "";

	EnterCriticalSection(&GetInstance()->m_CS);
	map<SOCKET, string>::iterator iter;
	for (iter = GetInstance()->m_UserList.begin(); iter != GetInstance()->m_UserList.end(); iter++)
		liststr += iter->second + "/";
	LeaveCriticalSection(&GetInstance()->m_CS);

	// 2. ������ �����ڴ� �����
	liststr.erase(liststr.end() - 1);

	// 3. �����͸� �����Ѵ�.
	char result[256];
	memcpy(result, liststr.c_str(), liststr.length());

	// 3. ��� �������� �����Ѵ�.
	// Ư�̻��� - size �������� ���� ������ �ʹ� ������ ������ ������.
	// ����Ʈ�� ������ ������ �����ų�, ��Ŷ ����� short�̳� int�� Ȯ���ϸ� �Ǵµ�... �ϴ� ���д�.
	EnterCriticalSection(&GetInstance()->m_CS);
	for (iter = GetInstance()->m_UserList.begin(); iter != GetInstance()->m_UserList.end(); iter++)
		GetInstance()->sendRequest(iter->first, result, liststr.length(), PACKET_USERLIST);
	LeaveCriticalSection(&GetInstance()->m_CS);
}

// ��� �������� �⺻���� ������ �޼����� �����Ѵ�
// [��:��:��] �̸� :: �޼��� �����̴�.
void CIOCPServer::NoticeMessage(string name, string msg)
{
	// 1. ������ ���� �޼����� ���Ŀ� �°� �ϼ��Ѵ�.
	string temp(GetInstance()->configureMessage(name, msg).c_str());
	char message[256];
	unsigned char size = temp.length();
	memcpy(message, temp.c_str(), size);

	// 2. ���Ŀ� �°� �ϼ��� �޼����� ��� �������� �����Ѵ�.
	EnterCriticalSection(&GetInstance()->m_CS);
	map<SOCKET, string>::iterator iter;
	for (iter = GetInstance()->m_UserList.begin(); iter != GetInstance()->m_UserList.end(); iter++)
		GetInstance()->sendRequest(iter->first, message, size, PACKET_STANDARD);
	LeaveCriticalSection(&GetInstance()->m_CS);
}

void CIOCPServer::WhisperMessage(string fromname, string toname, string msg)
{
	// 1. ������ ���� �޼����� ���Ŀ� �°� �ϼ��Ѵ�.
	string temp(GetInstance()->configureMessage(fromname + "->" + toname + "(�ӼӸ�)", msg).c_str());
	char message[256];
	unsigned char size = temp.length();
	memcpy(message, temp.c_str(), size);
	
	// 2. ���Ŀ� �°� �ϼ��� �޼����� "�ӼӸ� ����ڿ� �ڽ�"���Ը� �����Ѵ�.
	EnterCriticalSection(&GetInstance()->m_CS);
	map<SOCKET, string>::iterator iter;
	for (iter = GetInstance()->m_UserList.begin(); iter != GetInstance()->m_UserList.end(); iter++)
	{
		// �ӼӸ� ����ڿ� �ڽ��� ã���� �����Ѵ�
		if (iter->second._Equal(fromname) || iter->second._Equal(toname))
			GetInstance()->sendRequest(iter->first, message, size, PACKET_WHISPER);
	}
	LeaveCriticalSection(&GetInstance()->m_CS);
}

void CIOCPServer::sendRequest(SOCKET socket, char* data, unsigned char size, unsigned char code)
{
	// ��Ŷ �۽� ����
	// ��� Ŭ����, ����ü ���� �� �����ʹ� ���� ���簡 �ȴ�.

	// 1. CPacket ����
	// ���� ��Ŷ�� ���� Ÿ�԰� �����͸� �����Ѵ�.
	// ���� ������ �Լ� ���� �� �ڵ� �Ҹ�ȴ�.
	CPacket sendPacket(data, size, code);

	// 2. LPPER_IO_PACKET ���� 
	// CPacket�� Encode() �Լ��� ������, ��� ������ ������ ��Ŷ�̴�.
	// char* ��Ŷ �� ��� / ��Ŷ ����� ����� ����ü�̴�.
	// ���� �Ҵ�Ǿ�����, ����ü�̱� ������ �Ҹ��ϱ� �� �ݵ�� free()�� ����� �Ѵ�.
	LPPER_IO_PACKET sendData = sendPacket.Encode();

	// 3. LPPER_IO_DATA ����
	// �ܹ���(���⼭�� �۽�)���� ���� ���� ����� ��Ŷ�̴�.
	LPPER_IO_DATA sendIoInfo;

	// 4. WSASend ����
	// ioInfo�� sendData�� WSASend�� �̿��ϱ� ���� �ʱ�ȭ�� �ش�.
	GetInstance()->initWSASend(&sendIoInfo, sendData->data, sendData->size);

	// 5. WSASend
	// ��� ������ ������ ���� ���� ��Ŷ�� �ش��ϴ� Ŭ���̾�Ʈ�� ������.
	WSASend(socket, &(sendIoInfo->wsaBuf), 1, NULL, 0, &(sendIoInfo->overlapped), NULL);

	// 6. LPPER_IO_PACKET ����
	// 2������ ���� �Ҵ�� ��Ŷ�� �ݵ�� �����Ѵ�.
	free(sendData->data);
	free(sendData);

	// ��� ������ ������ �Լ��� ����ȴ�.
}

void CIOCPServer::initWSARecv(LPPER_IO_DATA* ioInfo)
{
	*ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&((*ioInfo)->overlapped), 0, sizeof(OVERLAPPED));
	memset(&(*ioInfo)->buffer, 0, sizeof((*ioInfo)->buffer));
	(*ioInfo)->wsaBuf.len = MAX_BUFFER_SIZE;
	(*ioInfo)->wsaBuf.buf = (*ioInfo)->buffer;
	(*ioInfo)->readWriteFlag = READ;
}

void CIOCPServer::initWSASend(LPPER_IO_DATA* ioInfo, const char* message, int msgLength)
{
	*ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
	memset(&((*ioInfo)->overlapped), 0, sizeof(OVERLAPPED));
	memset(&(*ioInfo)->buffer, 0, sizeof((*ioInfo)->buffer));
	memcpy((*ioInfo)->buffer, message, msgLength);
	(*ioInfo)->wsaBuf.len = msgLength;
	(*ioInfo)->wsaBuf.buf = (*ioInfo)->buffer;
	(*ioInfo)->readWriteFlag = WRITE;
}

void CIOCPServer::closeClient(LPPER_HANDLE_DATA* handleInfo, LPPER_IO_DATA* ioInfo)
{
	// 1. ���� ��� ���� ��, ������ �濡�� ������ �켱 ������ �뺸�Ѵ�
	GetInstance()->NoticeMessage(GetInstance()->m_UserList[(*handleInfo)->hClientSocket], "�濡�� �����մϴ�. �ȳ��� �輼��.");

	// 2. �ش� ������ ����Ʈ���� �����
	EnterCriticalSection(&GetInstance()->m_CS);
	GetInstance()->m_UserList.erase((*handleInfo)->hClientSocket);
	LeaveCriticalSection(&GetInstance()->m_CS);

	// 3. �����ִ� ������� ���� ����Ʈ�� ���������Ѵ�.
	// �����ִ� ����� ���� ��찡 �����Ƿ� �ش� ��쵵 ó���Ѵ�.
	if (GetInstance()->m_UserList.size() > 0)
		GetInstance()->RefreshUserList();

	// 4. �����ܿ� �����ִ� ���ϰ� �׿� ������ ���� �����ϰ� ������.
	closesocket((*handleInfo)->hClientSocket);
	printf("Client Disconnected :: %s/%d\n", inet_ntoa((*handleInfo)->clientAddress.sin_addr),
		(*handleInfo)->clientAddress.sin_port);
	delete (*handleInfo)->packetQueue;
	free(*handleInfo);
	free(*ioInfo);
}