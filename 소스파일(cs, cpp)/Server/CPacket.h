#pragma once
#include<iostream>
#include<string>

using namespace std;

#pragma warning(disable:4996)

// 1개 패킷과 그 사이즈를 담는 구조체
typedef struct
{
	char* data;		// 통신을 위해 사용되는 char 배열
	int size;		// 배열 전체 데이터의 크기를 표현하는 int형 변수
}PER_IO_PACKET, * LPPER_IO_PACKET;

#define PACKET_STANDARD		0		// 채팅 메세지 전송
#define PACKET_NICKNAME		1		// 개인 닉네임 설정
#define PACKET_USERLIST		2		// 유저 리스트 리프레쉬
#define PACKET_WHISPER		3		// 귓속말 전송

// CPacket :: 패킷을 클래스화시킨 클래스이다.
class CPacket
{
public:
	CPacket(char* data);
	CPacket(char* data, unsigned char size, unsigned char code);
	~CPacket();

	// 클래스화된 패킷을 통신 가능하도록 char 포인터로 인코딩하는 함수
	LPPER_IO_PACKET Encode();

	unsigned char	GetCode();
	unsigned char	GetDataSize();
	char* GetData();

private:
	unsigned char	m_DataSize;
	unsigned char	m_Code;
	char* m_Data;
};