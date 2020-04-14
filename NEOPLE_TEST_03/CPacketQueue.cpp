#include "CPacketQueue.h"

CPacketQueue::CPacketQueue()
{
	this->m_PacketList.clear();
	this->init();
}

CPacketQueue::~CPacketQueue()
{

}

// WSARecv�� 1ȸ ȣ��� ������ ����Ǵ� public �Լ�
int CPacketQueue::OnRecv(char* data, int recvBytes)
{
	// ������� ����� ������ �ʾҴٸ�
	// ��Ʈ��ũ�� �Ҿ����ϰų�, ����� �� ���������� �ƴ� Ȯ���� �����Ƿ�
	// �����ϰ� ������ ���ܽ�Ű���� �Ѵ�.
	if (recvBytes < 2)
		return -1;

	// �켱 ���� ��Ŷ�� ���� �Ҵ��� �޸𸮿� �����ϸ�
	LPPER_IO_PACKET packet = (LPPER_IO_PACKET)malloc(sizeof(PER_IO_PACKET));

	packet->data = (char*)malloc(recvBytes);
	memset(packet->data, 0, recvBytes);
	memcpy(packet->data, data, recvBytes);
	packet->size = recvBytes;

	// �װ��� ����Ʈ�� �߰��Ѵ�.
	this->m_PacketList.push_back(packet);

	// ��Ŷ�� ����Ʈ�� ���� �� ��ٷ� �м��� �����Ѵ�.
	// private���� ����� �� �м� �Լ��� �ʿ�� ���ο��� ��������� �ݺ� ����ǹǷ� 1���� ȣ���ϸ� �ȴ�.
	this->analysis();

	// �м��� ������ ���� ���ο� ���� ��ȯ���� �޸��Ѵ�.
	if (m_bFatalError)
	{
		this->init();
		return -1;
	}

	// ���� ���� �� ���� ��� �۾��� ������ ��Ŷ�� ���ڸ� ��ȯ�Ѵ�.
	this->init();
	return this->m_AvailablePacketList.size();
}

CPacket* CPacketQueue::getPacket()
{
	CPacket* ret = this->m_AvailablePacketList.front();
	this->m_AvailablePacketList.pop();
	return ret;
}

// 1ȸ �м��� �ʿ��� ��� �����͸� �ʱ�ȭ��Ű�� �Լ��̴�.
void CPacketQueue::init()
{
	m_Payload = 0;
	this->m_bFatalError = false;
}

// ���� ����Ʈ ���� �ִ� ��� ��Ŷ�� �м��Ͽ� ���� ��Ŷ���� ���� �� �����ϴ� �Լ��̴�.
void CPacketQueue::analysis()
{
	LPPER_IO_PACKET packet = this->m_PacketList.front();
	char* temp = NULL;


	// ó�� ����(���� ����) ��Ŷ�� ������ ���
	if (this->m_PacketList.size() == 1)
	{
		// �̰� �񶧸��� ����ε� ��Ŷ�� �ָ��ϰ� �پ�ͼ� ����� ©���� ����̴�.
		// �� ��� ���� ��Ŷ�� ���� ������ ��ٸ���.
		if (packet->size < 2) return;

		// ������ ������ ũ�� üũ(1����Ʈ)
		// ������ ������ ũ�⸦ �ǹ��ϴ� ù ��° ����Ʈ�� 0�� ��� �߸��� ��Ŷ�̹Ƿ� �����Ų��.
		if (packet->data[0] == 0)
		{
			m_bFatalError = true;
			return;
		}
		// ����� �� ��Ŷ�̸� �Է��Ѵ�.
		m_Payload = packet->data[0];

		// ��Ŷ ũ�� ���� üũ (Payload + 2����Ʈ)
		// payload�� ����� ��ģ ���� Ŭ ��� ��Ŷ�� �� ���Դٴ� ���̹Ƿ� �׳� �ѱ��
		// ���� ���� �� ����� ��Ȯ�ϰ� ���Դٴ� ���̸�,
		// ���� ���� ���� ��Ŷ�� �ϳ��� �پ�Դٴ� ���ε�
		// ������ ���߿� ��ó���� �� ���̹Ƿ� ���� �� ������ ���� �б�� �ѱ��.
		// �˻� ��� ������ ��Ŷ�� ������ ��� Ŭ����ȭ�Ͽ� worker���� ���� �� �� �ֵ��� �����Ѵ�
		if (m_Payload + 2 <= packet->size && m_Payload != 0)
		{
			// 1ȸ �м� �� 1���� ������ ��Ŷ�� �и��ȴ�.
			// �ϴ� ���� Ȯ�ε� ������ ��Ŷ �ϳ��� Ŭ����ȭ�Ͽ� �ϼ����� ����.
			CPacket* cPacket = new CPacket(packet->data);
			if (cPacket != NULL)
				this->m_AvailablePacketList.push(cPacket);

			// ��Ŷ�� Ŭ����ȭ�ߴٸ� ���� ����Ʈ ���� �ٸ� �����Ͱ� ���� �����ִ��� Ȯ���Ѵ�.
			if (m_Payload + 2 < packet->size)
			{
				// �����ִٸ� �� ���� ��ŭ �޸𸮸� ���Ҵ��Ͽ� ������ �ΰ�
				// ���� �޸𸮴� �������Ѵ�.
				char* remain = (char*)malloc(packet->size - (m_Payload + 2));
				memcpy(remain, packet->data + m_Payload + 2, packet->size - (m_Payload + 2));
				free(packet->data);
				packet->data = remain;
				packet->size = packet->size - (m_Payload + 2);
			}
			// �� ����� ��Ŷ�� ��Ȯ�ϰ� �������� �� �м��� ���� ���� �����Ƿ� pop �� free ���ش�.
			else if (m_Payload + 2 == packet->size)
			{
				m_PacketList.pop_front();
				free(packet->data);
				free(packet);
			}

			// 1�� ��Ŷ �м� �� �и��� �Ϸ�Ǿ��ٸ� ���� �м��� ���� �����͸� �ʱ�ȭ�Ѵ�.
			this->init();

			// �̰����� ��Ŷ�� �����ִٸ� �м��� �簳�Ѵ�.
			if (!this->m_PacketList.empty())
				this->analysis();
		}

		// ���� �� �������� �б�Ǿ��ٸ�, ����� ã������ �� ���ڿ��� ã�� ���� ���̹Ƿ�
		// ���� WSARecv���� ��ٷ��� �Ѵ�.
		// ����� Timeout ����� ���ϱ� �ϴµ� �ϴ� ������.
	}
	// �̹� ����Ʈ �ȿ� �ٸ� ��Ŷ�� �� ����־��� ���
	else if (this->m_PacketList.size() > 1)
	{
		// �켱 �ȿ� ��Ŷ �������� �˻��ؾ� �Ѵ�.
		list<LPPER_IO_PACKET>::iterator iter;
		int sum = 0;
		for (iter = m_PacketList.begin(); iter != m_PacketList.end(); iter++)
			sum += (*iter)->size;

		// �켱 ��Ŷ ������ ������ ���ؼ��� �����͸� �̾���̴� ���� ����.
		char* mergeData = (char*)malloc(sum);
		char* temp = mergeData;
		for (iter = m_PacketList.begin(); iter != m_PacketList.end(); iter++)
		{
			memcpy(mergeData, (*iter)->data, (*iter)->size);
			mergeData += (*iter)->size;
		}
		mergeData = temp;

		// �̾���� ���ο� ��Ŷ ����� �޸� �Ҵ��Ͽ� �ش�.
		LPPER_IO_PACKET mergePacket = (LPPER_IO_PACKET)malloc(sizeof(PER_IO_PACKET));
		mergePacket->data = mergeData;
		mergePacket->size = sum;

		// �� �̾�ٿ����� ���� ����Ʈ�� �ִ� �и��� ��Ŷ �������� �� ����������.
		while (!m_PacketList.empty())
		{
			free(m_PacketList.front()->data);
			free(m_PacketList.front());
			m_PacketList.pop_front();
		}

		// 2����Ʈ�� �ȵȴٸ� ������ ��Ŷ�� ���� ����ְ� ���� ��Ŷ�� ��ٷ��� �Ѵ�.
		if (sum < 2)
		{
			this->m_PacketList.push_back(mergePacket);
			return;
		}
		// 2����Ʈ�� ������ �ּ��� ��� �˻�� �� �� �ִٴ� ���̹Ƿ� �˻��� �ش�.
		else
		{
			// ������ ������ ũ�� üũ(1����Ʈ)
			// ������ ������ ũ�⸦ �ǹ��ϴ� ù ��° ����Ʈ�� 0�� ��� �߸��� ��Ŷ�̹Ƿ� �����Ų��.
			if (packet->data[0] == 0)
			{
				m_bFatalError = true;
				return;
			}
			// ����� �� ��Ŷ�̸� �Է��Ѵ�.
			m_Payload = packet->data[0];
		}

		// �˻� ��� ������ ��Ŷ�� ������ ��� Ŭ����ȭ�Ͽ� worker���� ���� �� �� �ֵ��� �����Ѵ�
		if (m_Payload + 2 <= packet->size && m_Payload != 0)
		{
			// 1ȸ �м� �� 1���� ������ ��Ŷ�� �и��ȴ�.
			// �ϴ� ���� Ȯ�ε� ������ ��Ŷ �ϳ��� Ŭ����ȭ�Ͽ� �ϼ����� ����.
			CPacket* cPacket = new CPacket(mergeData);
			if (cPacket != NULL)
				this->m_AvailablePacketList.push(cPacket);

			// ��Ŷ�� Ŭ����ȭ�ߴٸ� ������ ������ �̿ܿ� ���� ���� �ִ°� Ȯ���Ѵ�.
			if (m_Payload + 2 < sum)
			{
				// Ŭ����ȭ�� ��Ŷ �̿ܿ��� �����ִٸ� �ٽ� ��Ŷ����Ʈ�� ����־� �ش�.
				char* remain = (char*)malloc(mergePacket->size - (m_Payload + 2));
				memcpy(remain, mergePacket->data + m_Payload + 2, mergePacket->size - (m_Payload + 2));
				free(mergePacket->data);
				mergePacket->data = remain;
				mergePacket->size = mergePacket->size - (m_Payload + 2);
				m_PacketList.push_back(mergePacket);
			}

			// �Ʊ� ������ ��Ŷ �������� ����Ʈ���� �� �����������Ƿ� �ļ� �۾��� �ʿ����
			// 1�� ��Ŷ �м� �� �и��� �Ϸ�Ǿ��ٸ� ���� �м��� ���� �����͸� �ʱ�ȭ�Ѵ�.
			this->init();

			// �̰����� ��Ŷ�� �����ִٸ� �м��� �簳�Ѵ�.
			if (!this->m_PacketList.empty())
				this->analysis();
		}
	}
}