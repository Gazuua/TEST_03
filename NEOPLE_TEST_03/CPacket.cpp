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

// 현재 클래스를 통신이 가능한 형태로 직렬화해주는 함수
LPPER_IO_PACKET CPacket::Encode()
{
	LPPER_IO_PACKET packet;

	// 메모리를 할당한다
	packet = (LPPER_IO_PACKET)malloc(sizeof(PER_IO_PACKET));
	packet->size = this->m_DataSize + 2;
	packet->data = (char*)malloc(packet->size);

	// 헤더의 최초 1바이트 = 데이터 영역의 크기(헤더 앞쪽 2바이트 제외)
	packet->data[0] = this->m_DataSize;
	
	// 헤더의 두번째 바이트 = 코드(데이터의 종류 및 동작 정의)
	packet->data[1] = this->m_Code;

	// 헤더의 세번째 바이트부터는 데이터 영역
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