#pragma once

#ifndef __PACKET_H__
#define __PACKET_H__


enum class PacketType
{
    Move = 0,   // 클라이언트 -> 서버: 이동 방향
    Position,   // 서버 -> 클라이언트: 플레이어 위치
    C2S_File,   // 파일 요청
    S2C_File,   // 파일 전송
    Max
};

//S2C_File 만들때 
//Data -> 파일 사이즈를 먼저 보내고 그담에 쭈욱보낸다.

//파일 보낸다 -> 잘라서 보내고 -> 파일 다 보냈음




#pragma pack(push, 1)


// 공통 헤더: Size(데이터 크기) + Code(패킷 종류)
// 4 + 2 -> 6byte
struct PacketHeader
{
    unsigned long Size;  //2 ^ 32
    unsigned short Code;
};

// 이동 방향 데이터 (클라이언트 -> 서버)
struct MoveData
{
    char Dir;   // 'W', 'A', 'S', 'D'
};

// 위치 데이터 (서버 -> 클라이언트)
struct PositionData
{
    int X;
    int Y;
};

#pragma pack(pop)

#endif // __PACKET_H__
