#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <conio.h>
#include <iostream>

#include "Packet.h"

#pragma comment(lib, "ws2_32")


#define SERVER_IP		"127.0.0.1"


//4byte
void MakePacketHeader(PacketHeader& OutPacketHeader, int DataSize, PacketType Type)
{
	OutPacketHeader.Size = htons(DataSize);
	OutPacketHeader.Code = htons(static_cast<unsigned short>(Type));
}

void SendAll(SOCKET ReceiverSocket, char* Data, int Size)
{
	int TotalSendDataSize = 0;
	int WantSendDataSize = Size;
	int SentBytes = 0;
	int Count = 0;
	do
	{
		SentBytes = send(ReceiverSocket, Data + TotalSendDataSize, WantSendDataSize - TotalSendDataSize, 0);
		TotalSendDataSize += SentBytes;
		printf("Send %dBytes %d Count\n", SentBytes, ++Count);
	} while (TotalSendDataSize < WantSendDataSize);
}


int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN ServerAdress;
	memset(&ServerAdress, 0, sizeof(ServerAdress));
	ServerAdress.sin_family = PF_INET;
	ServerAdress.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerAdress.sin_port = htons(31000);

	int SendBufferSize = 0;
	int RecvBufferSize = 0;
	int BufferSizeLength = sizeof(SendBufferSize);

	getsockopt(ServerSocket, SOL_SOCKET, SO_SNDBUF, (char*)&SendBufferSize, &BufferSizeLength);
	printf("Send Buffer Size: %d\n", SendBufferSize);

	SendBufferSize = 4;
	setsockopt(ServerSocket, SOL_SOCKET, SO_SNDBUF, (char*)&SendBufferSize, BufferSizeLength);

	getsockopt(ServerSocket, SOL_SOCKET, SO_SNDBUF, (char*)&SendBufferSize, &BufferSizeLength);
	printf("Send Buffer Size: %d\n", SendBufferSize);


	//getsockopt(ServerSocket, SOL_SOCKET, SO_RCVBUF, (char*)&RecvBufferSize, &BufferSizeLength);
	//printf("Receive Buffer Size: %d\n", RecvBufferSize);

	//3way handshake
	connect(ServerSocket, (SOCKADDR*)&ServerAdress, sizeof(ServerAdress));

	//authentication(인증), 로그인, 몬가 준다.(token, key), 인가(authorization)

	while (true)
	{
		PacketHeader Header;
		MakePacketHeader(Header, sizeof(MoveData), PacketType::Move);

		MoveData Data;
		Data.Dir = _getch();

		//[][][][] []... []
		char Buffer[1024] = { 0, };
		//[][] [][]
		memcpy(Buffer, &Header, sizeof(Header));
		memcpy(&Buffer[0] + sizeof(Header), &Data, sizeof(Data));

		SendAll(ServerSocket, Buffer, sizeof(Header) + sizeof(Data));

		//OS 버퍼에서 가져올수 있는 만큼 가져올때까지 기다린다.
		int RecvBytes = recv(ServerSocket, Buffer, sizeof(Buffer), 0);
	}

	closesocket(ServerSocket);


	WSACleanup();
}




