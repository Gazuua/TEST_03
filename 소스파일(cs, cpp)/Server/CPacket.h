#pragma once
#include<iostream>
#include<string>

using namespace std;

#pragma warning(disable:4996)

// 1�� ��Ŷ�� �� ����� ��� ����ü
typedef struct
{
	char* data;		// ����� ���� ���Ǵ� char �迭
	int size;		// �迭 ��ü �������� ũ�⸦ ǥ���ϴ� int�� ����
}PER_IO_PACKET, * LPPER_IO_PACKET;

#define PACKET_STANDARD		0		// ä�� �޼��� ����
#define PACKET_NICKNAME		1		// ���� �г��� ����
#define PACKET_USERLIST		2		// ���� ����Ʈ ��������
#define PACKET_WHISPER		3		// �ӼӸ� ����

// CPacket :: ��Ŷ�� Ŭ����ȭ��Ų Ŭ�����̴�.
class CPacket
{
public:
	CPacket(char* data);
	CPacket(char* data, unsigned char size, unsigned char code);
	~CPacket();

	// Ŭ����ȭ�� ��Ŷ�� ��� �����ϵ��� char �����ͷ� ���ڵ��ϴ� �Լ�
	LPPER_IO_PACKET Encode();

	unsigned char	GetCode();
	unsigned char	GetDataSize();
	char* GetData();

private:
	unsigned char	m_DataSize;
	unsigned char	m_Code;
	char* m_Data;
};