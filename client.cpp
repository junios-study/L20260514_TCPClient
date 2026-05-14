#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <conio.h>
#include <iostream>

#include "Packet.h"

#pragma comment(lib, "ws2_32")


#define SERVER_IP		"127.0.0.1"


//4byte
void MakePacketHeader(PacketHeader& OutPacketHeader, int DataSize, PacketType Type)
{
	OutPacketHeader.Size = htonl(DataSize);
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

void ProcessPositionPacket(SOCKET SenderSocket, int DataSize)
{
	PositionData Data;

	int RecvBytes = recv(SenderSocket, (char*)&Data, DataSize, MSG_WAITALL);
	Data.X = ntohl(Data.X);
	Data.Y = ntohl(Data.Y);

	printf("Player Position(%d, %d)\n", Data.X, Data.Y);
}


int main()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	SOCKET ServerSocket = socket(PF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN ServerAdress;
	memset(&ServerAdress, 0, sizeof(ServerAdress));
	ServerAdress.sin_family = PF_INET;
	ServerAdress.sin_addr.s_addr = inet_addr(SERVER_IP);
	ServerAdress.sin_port = htons(31000);

	int SendBufferSize = 0;
	int RecvBufferSize = 0;
	int BufferSizeLength = sizeof(SendBufferSize);

	RecvBufferSize = 8 * 1024; // 1000 -> 1k, 2^10  1024
	BufferSizeLength = sizeof(RecvBufferSize);

	//getsockopt(ServerSocket, SOL_SOCKET, SO_SNDBUF, (char*)&SendBufferSize, &BufferSizeLength);
	//printf("Send Buffer Size: %d\n", SendBufferSize);

	
	//setsockopt(ServerSocket, SOL_SOCKET, SO_SNDBUF, (char*)&SendBufferSize, BufferSizeLength);

	getsockopt(ServerSocket, SOL_SOCKET, SO_RCVBUF, (char*)&RecvBufferSize, &BufferSizeLength);
	printf("Receive Buffer Size: %d\n", RecvBufferSize);

	setsockopt(ServerSocket, SOL_SOCKET, SO_RCVBUF, (char*)&RecvBufferSize, sizeof(RecvBufferSize));

	getsockopt(ServerSocket, SOL_SOCKET, SO_RCVBUF, (char*)&RecvBufferSize, &BufferSizeLength);
	printf("Receive Buffer Size: %d\n", RecvBufferSize);

	//3way handshake
	connect(ServerSocket, (SOCKADDR*)&ServerAdress, sizeof(ServerAdress));

	//authentication(인증), 로그인, 몬가 준다.(token, key), 인가(authorization)

	while (true)
	{
		PacketHeader Header;

		MoveData Data;
		Data.Dir = _getch();

		if (Data.Dir == 'F' || Data.Dir == 'f')
		{
			//파일 요청
			//Header
			//4byte
			MakePacketHeader(Header, sizeof(MoveData), PacketType::C2S_File);

			//[][][][] []... []
			char Buffer[1024] = { 0, };
			//[][][][] [][]
			memcpy(Buffer, &Header, sizeof(Header));
			SendAll(ServerSocket, Buffer, sizeof(Header) + sizeof(Data));

			//파일 받기 헤더
			int RecvBytes = recv(ServerSocket, (char*)&Header, sizeof(Header), MSG_WAITALL);
			Header.Size = ntohl(Header.Size);
			Header.Code = ntohs(Header.Code);

			//[][][][][][].. []
			switch ((PacketType)(Header.Code))
			{
			case PacketType::S2C_File:

				//파일 받아서 작성
				FILE* OutputFile = fopen("카네이션_2.png", "wb");

				int FileSize = Header.Size;
				int TotalFileWriteSize = 0;
				char Buffer[512] = { 0, };
				size_t WriteSize = 0;
				int Count = 0;

				do
				{
					printf("%d\n", ++Count);
					int RecvBytes = 0;
					//8192 작은 파일 이면 버퍼만큼 받는게 아니라 
					if (FileSize - TotalFileWriteSize < sizeof(Buffer))
					{
						//받을 사이즈가 버퍼보다 작을경우
						RecvBytes = recv(ServerSocket, Buffer, FileSize - TotalFileWriteSize, MSG_WAITALL);
						if (RecvBytes <= 0)
						{
							break;
						}
					}
					else
					{
						RecvBytes = recv(ServerSocket, Buffer, sizeof(Buffer), MSG_WAITALL);
						if (RecvBytes <= 0)
						{
							break;
						}
					}

					WriteSize = fwrite(Buffer, sizeof(char), RecvBytes, OutputFile);
					TotalFileWriteSize += (int)WriteSize;
				} while (TotalFileWriteSize < FileSize);

				fclose(OutputFile);
				break;
			}

		}
		else
		{
			//이동 요청
			MakePacketHeader(Header, sizeof(MoveData), PacketType::Move);


			//[][][][] []... []
			char Buffer[1024] = { 0, };
			//[][] [][]
			memcpy(Buffer, &Header, sizeof(Header));
			memcpy(&Buffer[0] + sizeof(Header), &Data, sizeof(Data));

			SendAll(ServerSocket, Buffer, sizeof(Header) + sizeof(Data));


			//Header
			//4byte
			int RecvBytes = recv(ServerSocket, (char*)&Header, sizeof(Header), MSG_WAITALL);
			Header.Size = ntohl(Header.Size);
			Header.Code = ntohs(Header.Code);

			switch ((PacketType)(Header.Code))
			{
			case PacketType::Position:
				ProcessPositionPacket(ServerSocket, Header.Size);
				break;
			}
		}

	}

	closesocket(ServerSocket);


	WSACleanup();
}




