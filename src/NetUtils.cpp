#include "NetUtils.h"


	void Net::SendEncrypted(SOCKET s, const char* buff, int len)
	{
		char buffer[128];
		int length = len;
		memcpy((void*)&buffer,(void*)buff, len);
		//Align to 8 bytes
		length = length + ((8-((length-2)%8))%8);
		//Calculate checksum
		unsigned long CS = 0;
		for(int p=0; p<(length-2)/4; p++)
		CS ^= *(unsigned long*)&buffer[p*4+2];
		//Add checksum and unknown value
		//PacketOut_AddDword(pb, CS);
		//PacketOut_AddDword(pb, 0x00);
		*(unsigned int*)&buffer[length] = CS;
		*(unsigned int*)&buffer[length+4] = 0;
		length += 8;
		//Update base len
		*(unsigned short*)buffer = length;
		//Encrypt it
		for(int p=0; p<(length-2)/8; p++)
		{
			CryptManager::Instance()->BFEncrypt((unsigned long*)&buffer[p*8+2], (unsigned long*)&buffer[6 + p*8]);
		}
		send(s, buffer, length, 0);
	}

	void Net::SendPlain(SOCKET s, const char* buff, int len)
	{
		send(s, buff, len, 0);
	}

	int Net::Receive(SOCKET s, char* buff, int len, bool clear, int size)
	{
		if (clear)
		{  
			memset((void*)buff, 0x00, size);
		}
		int ret = recv(s, buff, len, 0);
		return ret;
	}

	void Net::SetTimeout(SOCKET s, int time)
	{
		struct timeval timeout;
		timeout.tv_sec = time;
		timeout.tv_usec = 0;
		#ifdef _WIN32
		setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		#else
		setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (void*)&timeout, sizeof(timeout));
		#endif
	}

	void Net::Close(SOCKET s)
	{
		printf("Bye Player: ");
		Net::PrintIP(Net::NumericIP(s));
		printf("\r\n");
		#ifdef _WIN32
		closesocket(s);
		#else
		close((unsigned int)s);
		#endif
	}

	unsigned int Net::NumericIP(SOCKET s)
	{
		sockaddr_in peername;
		int pLen = sizeof(peername);
		getsockname(s, (sockaddr*)&peername, &pLen);
		unsigned int generatedIP = peername.sin_addr.S_un.S_addr;
		return generatedIP;
	}
	void Net::PrintIP(unsigned int IP)
	{
		printf("%d.%d.%d.%d", (IP&0xFF), ((IP>>8)&0xFF), ((IP>>16)&0xFF), ((IP>>24)&0xFF));
	}
