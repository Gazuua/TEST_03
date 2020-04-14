#include "CPacket.h"

CPacket::CPacket(char* data)
{
	char* ptData = data + 2;

	this->m_DataSize = data[0];
	this->m_Code = data[1];
	this->m_Data = (char*)malloc(this->m_DataSize);
	memcpy(this->m_Data, ptData, this->m_DataSize);
}

CPacket::CPacket(char* data, unsigned char size, unsigned char code)
{
	this->m_DataSize = size;
	this->m_Code = code;
	this->m_Data = (char*)malloc(size);
	memcpy(this->m_Data, data, size);
}

CPacket::~CPacket()
{
	free(this->m_Data);
}

// ���� Ŭ������ ����� ������ ���·� ����ȭ���ִ� �Լ�
LPPER_IO_PACKET CPacket::Encode()
{
	LPPER_IO_PACKET packet;

	// �޸𸮸� �Ҵ��Ѵ�
	packet = (LPPER_IO_PACKET)malloc(sizeof(PER_IO_PACKET));
	packet->size = this->m_DataSize + 2;
	packet->data = (char*)malloc(packet->size);

	// ����� ���� 1����Ʈ = ������ ������ ũ��(��� ���� 2����Ʈ ����)
	packet->data[0] = this->m_DataSize;
	
	// ����� �ι�° ����Ʈ = �ڵ�(�������� ���� �� ���� ����)
	packet->data[1] = this->m_Code;

	// ����� ����° ����Ʈ���ʹ� ������ ����
	memcpy(packet->data + 2, this->m_Data, this->m_DataSize);

	return packet;
}

unsigned char CPacket::GetCode()
{
	return this->m_Code;
}

unsigned char CPacket::GetDataSize()
{
	return this->m_DataSize;
}

char* CPacket::GetData()
{
	return this->m_Data;
}