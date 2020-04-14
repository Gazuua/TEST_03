#include "CIOCPServer.h"

// 싱글톤 인스턴스를 프로그램에서 호출하는 함수
CIOCPServer* CIOCPServer::GetInstance()
{
	// 최초 인스턴스에 메모리 할당
	if (!m_pInstance) {
		m_pInstance = new CIOCPServer();
	}
	return m_pInstance;
}

// 메모리에서 해제
void CIOCPServer::Release()
{
	WSACleanup();
	DeleteCriticalSection(&GetInstance()->m_CS);
	delete m_pInstance;
	m_pInstance = nullptr;
}

// IOCP 서버 초기화 함수
bool CIOCPServer::Init(const int PORT)
{
	SYSTEM_INFO			SystemInfo;		// CPU 코어 개수 파악
	char				hostname[256];	// 자신의 hostname 파악
	hostent* host;						// 자신의 ip 주소 파악
	unsigned char		ipAddr[4];		// 자신의 ip 주소 저장

	// Windows Socket API(WSA)를 초기화
	if (WSAStartup(MAKEWORD(2, 2), &m_WsaData) != 0)
		return false;

	// CRITICAL_SECTION 핸들 할당
	InitializeCriticalSection(&GetInstance()->m_CS);

	// I/O Completion Port 핸들 할당
	m_hCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// System에서 사용 가능한 CPU 코어 갯수의 2배만큼 worker thread 시작
	// 인수에 IOCP 핸들을 할당함으로서 운영체제에서 완료신호를 읽을 수 있게 한다
	// (코어갯수 * 2 = 마이크로소프트에서 권장하는 수치)
	GetSystemInfo(&SystemInfo);
	for (unsigned int i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++)
		_beginthreadex(NULL, 0, workerProcedure, (LPVOID)m_hCP, 0, NULL);

	// OVERLAPPED IO(중첩된 입출력)가 가능하도록 WSASocket 함수로 서버 소켓을 할당
	m_hServerSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&m_ServerAddress, 0, sizeof(m_ServerAddress));
	m_ServerAddress.sin_family = AF_INET;
	m_ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	m_ServerAddress.sin_port = htons(55248);

	// bind 및 listen
	::bind(m_hServerSocket, (SOCKADDR*)&m_ServerAddress, sizeof(m_ServerAddress));
	listen(m_hServerSocket, SOMAXCONN);

	// listen 준비가 끝나고 accept를 위해 accept thread를 하나 시작
	_beginthreadex(NULL, 0, acceptProcedure, NULL, 0, NULL);

	// 모든 서버 동작 준비가 끝나면 서버의 IP주소와 포트번호를 콘솔창에 출력하여 알림
	gethostname(hostname, 256);
	host = gethostbyname(hostname);
	for (int i = 0; i < 4; i++)
		ipAddr[i] = *(host->h_addr_list[0] + i);

	puts("============= IOCP 서버 초기 설정 완료!=============");
	printf("서버 주소 -> %d.%d.%d.%d:%d\n", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3], PORT);

	return true;
}

// 클라이언트의 접속에 따른 동작을 담당하는 accept thread의 proc 함수
unsigned int __stdcall CIOCPServer::acceptProcedure(void* nullpt)
{
	SOCKET				hClientSocket;		// accept된 클라이언트 소켓을 담을 핸들
	SOCKADDR_IN			clientAddress;		// 클라이언트 주소 구조체
	LPPER_HANDLE_DATA	handleInfo;			// 클라이언트가 접속해있는 동안 관리하기 위한 구조체
	LPPER_IO_DATA		ioInfo;				// 클라이언트로부터 입력받기 위해 할당되는 구조체

	int					addrLength = sizeof(clientAddress);
	DWORD				flags = 0;

	while (1)
	{
		// 클라이언트를 accept 받는다
		hClientSocket = accept((SOCKET)GetInstance()->m_hServerSocket, (SOCKADDR*)&clientAddress, &addrLength);

		// 클라이언트가 접속 중일 때 사용될 HANDLE_DATA 구조체를 할당한다.
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClientSocket = hClientSocket;
		memcpy(&(handleInfo->clientAddress), &clientAddress, addrLength);
		handleInfo->packetQueue = new CPacketQueue();

		// 클라이언트의 접속을 콘솔창에 알린다.
		printf("Client Accepted :: %s/%d\n", inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port);

		// 해당 소켓을 Completion Port에 추가하여 감시한다.
		CreateIoCompletionPort((HANDLE)hClientSocket, GetInstance()->m_hCP, (DWORD)handleInfo, 0);

		// 최초 WSARecv를 위해 셋팅한다.
		GetInstance()->initWSARecv(&ioInfo);
		WSARecv(handleInfo->hClientSocket, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
	}
	return 0;
}

// 실제 서버의 Recv/Send 동작을 실행하는 proc 함수
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
		// GQCS(소켓에서 어떠한 작업이 완료되면 반환하는 함수)의 리턴값을 받는다.
		gqcsRet = GetQueuedCompletionStatus(hCP, &transferredBytes, (LPDWORD)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
		socket = handleInfo->hClientSocket;

		// 리턴값이 1인 경우 (정상적인 I/O 결과를 통지받은 경우)
		if (gqcsRet)
		{
			// 읽기 완료 시
			if (ioInfo->readWriteFlag == READ)
			{
				// EOF(연결 종료 신호) 전달 시 클라이언트와의 접속을 끊는다
				if (transferredBytes == 0)
				{
					CIOCPServer::GetInstance()->closeClient(&handleInfo, &ioInfo);
					continue;
				}

				// 이 부분은 자체 프로토콜에 기반하여 PacketQueue를 이용하는 통신 방법이다.
				// 패킷이 잘리거나 붙어서 올 때도 에러 없이 패킷을 처리하여 준다.
				// OnRecv()의 반환값은 클래스화되어 바로 사용할 수 있는 패킷의 개수이다.
				int recv = handleInfo->packetQueue->OnRecv(ioInfo->buffer, transferredBytes);

				// OnRecv()에서 -1을 반환하면 뭔가 잘못되었으니 클라이언트의 연결을 해제하면 된다.
				if (recv == -1) {
					CIOCPServer::GetInstance()->closeClient(&handleInfo, &ioInfo);
					continue;
				}

				// 이번 WSARecv에 쓰인 ioInfo는 더 이상 필요하지 않으니 해제한다.
				free(ioInfo);

				// 사용 가능한 패킷 개수만큼 분석 및 작업 요청을 즉시 진행한다.
				// 외부 모듈에서 작업시 외부 모듈의 스레드를 이용하며 결과는 나중에 받는다.
				// 즉시 처리 가능한 내부 작업은 곧바로 처리해 준다.
				for (int i = 0; i < recv; i++)
				{
					// 도착한 패킷을 전부 분리한다.
					// 여기 선언한 패킷은 지역 변수라서 for문 탈출 시 자동으로 소멸된다.
					CPacket packet = *(handleInfo->packetQueue->getPacket());
					unsigned char size = packet.GetDataSize();
					unsigned char code = packet.GetCode();
					char* data = packet.GetData();

					switch (packet.GetCode())
					{
					// 기본 메세지를 유저 모두에게 통지하는 패킷
					case PACKET_STANDARD:
					{
						// 1. 유저의 이름과 보낸 메세지를 선언한다.
						string name(GetInstance()->m_UserList[socket]);
						string msg(data, size);

						// 2. 모든 유저에게 뿌린다.
						GetInstance()->NoticeMessage(name, msg);
					}
					break;

					// 유저의 닉네임 설정 요청에 따라 전송하는 패킷
					case PACKET_NICKNAME:
					{
						// 1. 유저가 정한 닉네임을 선언한다.
						string name(data, size);

						// 2. 해당하는 닉네임이 이미 있는지 리스트를 순회하며 체크한다.
						bool bDup = false;

						EnterCriticalSection(&GetInstance()->m_CS);
						map<SOCKET, string>::iterator iter;
						for (iter = GetInstance()->m_UserList.begin(); iter != GetInstance()->m_UserList.end(); iter++)
						{
							// 같은 것이 있으면 bDup를 true로 바꾼다.
							if (iter->second._Equal(name))
								bDup = true;
						}
						LeaveCriticalSection(&GetInstance()->m_CS);

						// 3. 결과에 따라 유저에게 돌려줄 응답값을 셋팅한다.
						char result[7];
						unsigned char resultSize = 0;

						// 중복이 있으면 실패
						if (bDup) {
							resultSize = 4;
							memcpy(result, "FAIL", resultSize);
						} // 없으면 맵에 닉네임을 추가 후 성공을 알린다
						else {
							EnterCriticalSection(&GetInstance()->m_CS);
							GetInstance()->m_UserList[socket] = name;
							LeaveCriticalSection(&GetInstance()->m_CS);

							resultSize = 7;
							memcpy(result, "SUCCESS", resultSize);
						}

						// 4. 결과를 해당 유저에게 전송한다.
						GetInstance()->sendRequest(socket, result, resultSize, PACKET_NICKNAME);
						
						// 5. 성공일 경우 후처리를 해준다
						if (!bDup) {
							GetInstance()->RefreshUserList();
							GetInstance()->NoticeMessage(name, "방에 입장하였습니다. 반갑습니다.");
						}
					}
					break;

					// 유저의 입/퇴장으로 인하여 유저 리스트 갱신이 필요할 경우 전송하는 패킷
					case PACKET_USERLIST:
					{
						GetInstance()->RefreshUserList();
					}
					break;

					// 유저의 귓속말 요청을 인지한 이후 전송하는 패킷
					// 서버에 올 때 - "귓속말 대상자/메세지"
					// 대상자에게 갈 때 - "귓속말 보낸 사람/메세지"
					case PACKET_WHISPER:
					{
						// 1. 유저의 이름과 보낸 메세지를 선언한다.
						string fromname(GetInstance()->m_UserList[socket]);
						string strdata(data, size);
						string toname(strdata.substr(0, strdata.find('/')));
						string msg(strdata.substr(strdata.find('/') + 1, strdata.length() - strdata.find('/')));

						// 2. 귓속말 대상자와 자신에게 조용히 귓속말을 보낸다.
						GetInstance()->WhisperMessage(fromname, toname, msg);
					}
					break;

					default:
						break;
					}
				}

				// 응답과 요청이 모두 끝났다면 다음 패킷 수신을 위해 WSARecv를 실행한다.
				LPPER_IO_DATA newIoInfo;
				GetInstance()->initWSARecv(&newIoInfo);
				WSARecv(socket, &(newIoInfo->wsaBuf), 1, NULL, &flags, &(newIoInfo->overlapped), NULL);
			}
			// 쓰기 완료 시 클라이언트로부터 응답이 오는 경우는 없으므로 바로 메모리를 해제한다
			else free(ioInfo);
		}
		// 리턴값이 0인 경우 (뭔가 처리를 해야 하는 결과를 통지받은 경우)
		else
		{
			int err = WSAGetLastError();
			switch (err)
			{
			case 64: // 클라이언트(소켓) 연결이 종료되었음을 의미하는 코드
				puts("INFO :: 클라이언트 프로세스의 종료를 감지하여, 해당 클라이언트와의 연결을 종료합니다.");
				CIOCPServer::GetInstance()->closeClient(&handleInfo, &ioInfo);
				break;
			default: // 뭔지는 모르겠으나 에러가 있는 경우 일단 출력한다.
				printf("============ ERROR CODE :: %d\n", err);
				break;
			}
		}
	}
	return 0;
}

// 기본 메세지 형태를 구성하는 함수이다
// [시:분:초] 이름 :: 메세지 형식이다
string CIOCPServer::configureMessage(string name, string msg)
{
	// 1. 유저가 보낸 메세지를 형식에 맞게 완성한다.
	time_t now = time(0);
	tm* ltm = localtime(&now);
	char ret[256];
	sprintf(ret, "[%d:%d:%d] %s :: %s", ltm->tm_hour, ltm->tm_min, ltm->tm_sec, name.c_str(), msg.c_str());

	return string(ret);
}

// 유저 리스트를 모든 유저에게 통지하여 업데이트해준다
void CIOCPServer::RefreshUserList()
{
	// 1. 유저 리스트를 불러와서 문자열을 구성한다.
	// "유저1/유저2/유저3/..." 형식으로 메세지를 구성한다
	string liststr = "";

	EnterCriticalSection(&GetInstance()->m_CS);
	map<SOCKET, string>::iterator iter;
	for (iter = GetInstance()->m_UserList.begin(); iter != GetInstance()->m_UserList.end(); iter++)
		liststr += iter->second + "/";
	LeaveCriticalSection(&GetInstance()->m_CS);

	// 2. 마지막 구분자는 지운다
	liststr.erase(liststr.end() - 1);

	// 3. 데이터를 구성한다.
	char result[256];
	memcpy(result, liststr.c_str(), liststr.length());

	// 3. 모든 유저에게 전송한다.
	// 특이사항 - size 제한으로 인해 유저가 너무 많으면 서버가 터진다.
	// 리스트를 여러번 나눠서 보내거나, 패킷 헤더를 short이나 int로 확장하면 되는데... 일단 냅둔다.
	EnterCriticalSection(&GetInstance()->m_CS);
	for (iter = GetInstance()->m_UserList.begin(); iter != GetInstance()->m_UserList.end(); iter++)
		GetInstance()->sendRequest(iter->first, result, liststr.length(), PACKET_USERLIST);
	LeaveCriticalSection(&GetInstance()->m_CS);
}

// 모든 유저에게 기본적인 형태의 메세지를 통지한다
// [시:분:초] 이름 :: 메세지 형식이다.
void CIOCPServer::NoticeMessage(string name, string msg)
{
	// 1. 유저가 보낸 메세지를 형식에 맞게 완성한다.
	string temp(GetInstance()->configureMessage(name, msg).c_str());
	char message[256];
	unsigned char size = temp.length();
	memcpy(message, temp.c_str(), size);

	// 2. 형식에 맞게 완성한 메세지를 모든 유저에게 전송한다.
	EnterCriticalSection(&GetInstance()->m_CS);
	map<SOCKET, string>::iterator iter;
	for (iter = GetInstance()->m_UserList.begin(); iter != GetInstance()->m_UserList.end(); iter++)
		GetInstance()->sendRequest(iter->first, message, size, PACKET_STANDARD);
	LeaveCriticalSection(&GetInstance()->m_CS);
}

void CIOCPServer::WhisperMessage(string fromname, string toname, string msg)
{
	// 1. 유저가 보낸 메세지를 형식에 맞게 완성한다.
	string temp(GetInstance()->configureMessage(fromname + "->" + toname + "(귓속말)", msg).c_str());
	char message[256];
	unsigned char size = temp.length();
	memcpy(message, temp.c_str(), size);
	
	// 2. 형식에 맞게 완성한 메세지를 "귓속말 대상자와 자신"에게만 전송한다.
	EnterCriticalSection(&GetInstance()->m_CS);
	map<SOCKET, string>::iterator iter;
	for (iter = GetInstance()->m_UserList.begin(); iter != GetInstance()->m_UserList.end(); iter++)
	{
		// 귓속말 대상자와 자신을 찾으면 전송한다
		if (iter->second._Equal(fromname) || iter->second._Equal(toname))
			GetInstance()->sendRequest(iter->first, message, size, PACKET_WHISPER);
	}
	LeaveCriticalSection(&GetInstance()->m_CS);
}

void CIOCPServer::sendRequest(SOCKET socket, char* data, unsigned char size, unsigned char code)
{
	// 패킷 송신 순서
	// 모든 클래스, 구조체 생성 시 데이터는 깊은 복사가 된다.

	// 1. CPacket 생성
	// 보낼 패킷에 대한 타입과 데이터를 저장한다.
	// 지역 변수라서 함수 종료 시 자동 소멸된다.
	CPacket sendPacket(data, size, code);

	// 2. LPPER_IO_PACKET 생성 
	// CPacket의 Encode() 함수로 생성된, 통신 가능한 완전한 패킷이다.
	// char* 패킷 한 덩어리 / 패킷 사이즈가 저장된 구조체이다.
	// 동적 할당되었으며, 구조체이기 때문에 소멸하기 전 반드시 free()를 해줘야 한다.
	LPPER_IO_PACKET sendData = sendPacket.Encode();

	// 3. LPPER_IO_DATA 생성
	// 단방향(여기서는 송신)으로 가는 단위 입출력 패킷이다.
	LPPER_IO_DATA sendIoInfo;

	// 4. WSASend 설정
	// ioInfo와 sendData를 WSASend에 이용하기 위해 초기화해 준다.
	GetInstance()->initWSASend(&sendIoInfo, sendData->data, sendData->size);

	// 5. WSASend
	// 모든 설정이 끝나면 드디어 응답 패킷을 해당하는 클라이언트로 보낸다.
	WSASend(socket, &(sendIoInfo->wsaBuf), 1, NULL, 0, &(sendIoInfo->overlapped), NULL);

	// 6. LPPER_IO_PACKET 해제
	// 2번에서 동적 할당된 패킷을 반드시 해제한다.
	free(sendData->data);
	free(sendData);

	// 모든 과정이 끝나면 함수가 종료된다.
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
	// 1. 소켓 통신 종료 전, 유저가 방에서 나감을 우선 서버에 통보한다
	GetInstance()->NoticeMessage(GetInstance()->m_UserList[(*handleInfo)->hClientSocket], "방에서 퇴장합니다. 안녕히 계세요.");

	// 2. 해당 유저를 리스트에서 지운다
	EnterCriticalSection(&GetInstance()->m_CS);
	GetInstance()->m_UserList.erase((*handleInfo)->hClientSocket);
	LeaveCriticalSection(&GetInstance()->m_CS);

	// 3. 남아있는 사람들의 유저 리스트를 리프레쉬한다.
	// 남아있는 사람이 없을 경우가 있으므로 해당 경우도 처리한다.
	if (GetInstance()->m_UserList.size() > 0)
		GetInstance()->RefreshUserList();

	// 4. 서버단에 남아있는 소켓과 그외 찌꺼기들을 마저 정리하고 끝낸다.
	closesocket((*handleInfo)->hClientSocket);
	printf("Client Disconnected :: %s/%d\n", inet_ntoa((*handleInfo)->clientAddress.sin_addr),
		(*handleInfo)->clientAddress.sin_port);
	delete (*handleInfo)->packetQueue;
	free(*handleInfo);
	free(*ioInfo);
}