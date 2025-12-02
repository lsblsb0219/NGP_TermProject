#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>
#include <Windows.h>
#include <ctime>

#include "..\Common.h"

#define SERVERPORT 9000
#define BUFSIZE    1024

#define MAX_PLAYER 3	// 접속 인원 제한
#define BLOCK_NUM 19	// 장애물 수

typedef struct Bounding_Box {
	float x1, z1, x2, z2;
}BB;

#pragma pack(1)
typedef struct player_packet {
	float x{}, z{}, road[2][2]{},
		speed = 0.0f,
		shake = 1, y_radian = 180.0f, // shake = (발,다리)회전 각도, radian = 몸 y축 회전 각도
		y{};
	BB bb{}; //왼쪽 상단, 오른쪽 하단
	int shake_dir{}, dir{};
	bool move = false; // 움직이고 있는지(대기 후 이동)
}Robot;

#pragma pack()
player_packet player_robot[MAX_PLAYER], block_robot[BLOCK_NUM];

HANDLE hKeyEvent, hGameStartEvent;
HANDLE hWriteEvent[MAX_PLAYER], hReadEvent[MAX_PLAYER];
bool bump[MAX_PLAYER];

void send_goal_packet();
void sent_start_packet();

void InitBuffer();
BB get_bb(Robot robot);
bool collision(BB obj_a, BB obj_b);

// 클라이언트 접속 수
int client_sock_count = 0;

DWORD WINAPI main_thread(LPVOID arg)
{
	SOCKET sock = (SOCKET)arg;
	int retval, client_id = -1;

	// 게임 시작 대기
	WaitForSingleObject(hGameStartEvent, INFINITE);

	// 게임 시작 패킷 전송
	const char* gameStartMsg = "GAME_START";

	retval = send(sock, gameStartMsg, strlen(gameStartMsg), 0);
	if (retval == SOCKET_ERROR)
		err_display("send()");
	else
		printf("[TCP 서버] GAME_START 패킷 전송 완료\n");

	printf("[Thread] 클라이언트 스레드 시작\n");

	// 클라이언트 ID값 수신
	retval = recv(sock, (char*)&client_id, sizeof(int), 0);
	if (retval == SOCKET_ERROR) 
		err_display("send()");

	printf("[Thread] 클라이언트 ID(client_%d) 수신 완료\n", client_id);

	while (1/*게임 중일 때*/) {
		// recv() 플레이어 정보 받기 - Robot
		retval = recv(sock, (char*)&player_robot[client_id], sizeof(player_robot[client_id]), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		SetEvent(hWriteEvent[client_id]);	// 플레이어 정보 확인

		WaitForSingleObject(hReadEvent[client_id], INFINITE);
		// send() 플레이어 정보 보내기 - Robot[3]
		for (int i = 0; i < MAX_PLAYER; i++) {
			retval = send(sock, (char*)&player_robot[i], sizeof(player_robot[i]), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}
		ResetEvent(hReadEvent[client_id]);
		
		// send() 로봇 정보 보내기 - Robot[19]
		for (int i = 0; i < BLOCK_NUM; ++i) {
			retval = send(sock, (char*)&block_robot[i], sizeof(block_robot[i]), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}

		// send() 충돌 여부 보내기 - bump
		retval = send(sock, (char*)&bump[client_id], sizeof(bool), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		bump[client_id] = false;
	}

	closesocket(sock);
	printf("[Thread] 클라이언트 스레드 종료\n");
	client_sock_count -= 1;

	return 0;
}

//DWORD WINAPI server_key_thread()
//{
//
//}

DWORD WINAPI update_thread(LPVOID)
{
	InitBuffer();	// 장애물 초기 설정

	printf("[Thread] 업데이트 스레드 시작\n");

	// 게임 시작 대기
	WaitForSingleObject(hGameStartEvent, INFINITE);

	while (1/*게임 종료되지 않았다면*/) {
		for (int i = 0; i < BLOCK_NUM; ++i) {
			block_robot[i].x += sin(glm::radians(block_robot[i].y_radian)) * block_robot[i].speed;
			block_robot[i].z += cos(glm::radians(block_robot[i].y_radian)) * block_robot[i].speed;
			block_robot[i].bb = get_bb(block_robot[i]);
			block_robot[i].shake += block_robot[i].shake_dir * 20 * block_robot[i].speed;
			if (block_robot[i].shake <= -60.0f || block_robot[i].shake >= 60.0f)
				block_robot[i].shake_dir *= -1;
			if ((block_robot[i].road[0][0] < block_robot[i].x and block_robot[i].x < block_robot[i].road[1][0]) ||
				(block_robot[i].road[0][0] > block_robot[i].x and block_robot[i].x > block_robot[i].road[1][0]) ||
				(block_robot[i].road[0][1] < block_robot[i].z and block_robot[i].z < block_robot[i].road[1][1]) ||
				(block_robot[i].road[0][1] > block_robot[i].z and block_robot[i].z > block_robot[i].road[1][1]));
			else
				block_robot[i].y_radian += 180.0f;
			block_robot[i].bb = get_bb(block_robot[i]);
		}
		WaitForMultipleObjects(MAX_PLAYER, hWriteEvent, TRUE, INFINITE);
		for (int id = 0; id < MAX_PLAYER; ++id) {
			for (int i = 0; i < MAX_PLAYER; ++i) {
				if (i == id) continue;
				if (player_robot[id].move && collision(player_robot[i].bb, player_robot[id].bb)) {	// 플레이어 간 충돌
					player_robot[id].move = false;
					player_robot[id].x -= sin(glm::radians(player_robot[id].y_radian)) * player_robot[id].speed;
					player_robot[id].z -= cos(glm::radians(player_robot[id].y_radian)) * player_robot[id].speed;
					player_robot[id].speed = 0;
					GLfloat radian = atan2(player_robot[id].x - player_robot[i].x, player_robot[id].z - player_robot[i].z);
					player_robot[id].road[0][0] = player_robot[i].x, player_robot[id].road[0][1] = player_robot[i].z;
					player_robot[id].road[1][0] = player_robot[i].x + 2.0f * sin(radian), player_robot[id].road[1][1] = player_robot[i].z + 2.0f * cos(radian);
					bump[id] = true;
				}
			}
			for (int i = 0; i < BLOCK_NUM; ++i) {
				if (player_robot[id].move && collision(block_robot[i].bb, player_robot[id].bb)) {	// 플레이어와 장애물 충돌
					player_robot[id].move = false;
					player_robot[id].x -= sin(glm::radians(player_robot[id].y_radian)) * player_robot[id].speed;
					player_robot[id].z -= cos(glm::radians(player_robot[id].y_radian)) * player_robot[id].speed;
					player_robot[id].speed = 0;
					GLfloat radian = atan2(player_robot[id].x - block_robot[i].x, player_robot[id].z - block_robot[i].z);
					player_robot[id].road[0][0] = block_robot[i].x, player_robot[id].road[0][1] = block_robot[i].z;
					player_robot[id].road[1][0] = block_robot[i].x + 2.0f * sin(radian), player_robot[id].road[1][1] = block_robot[i].z + 2.0f * cos(radian);
					bump[id] = true;
				}
			}
			ResetEvent(hWriteEvent[id]);
			SetEvent(hReadEvent[id]);
		}
	}

	printf("[Thread] 클라이언트 스레드 종료\n");

	return 0;
}

int main(int argc, char* argv[])
{
	int retval;
	DWORD optval = 1; // Nagle 중지

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// CreateEvent()
	hKeyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hGameStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	
	// 클라이언트 접속 수
	client_sock_count = 0;

	while (1) {
		// 최대 접속 인원 수 제한
		if (client_sock_count >= MAX_PLAYER) {
			if (WaitForSingleObject(hGameStartEvent, 0)) {
				printf("\n3명 접속 완료\n");
				SetEvent(hGameStartEvent);

				// 업데이트 쓰레드 생성
				HANDLE hThread = CreateThread(NULL, 0, update_thread, NULL, 0, NULL);
				if (hThread)
					CloseHandle(hThread);

				continue;
			}
			else {
				continue;
			}
		}
		
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// Nagle 중지
		setsockopt(client_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 클라이언트에 ID값 전송
		retval = send(client_sock, (char*)&client_sock_count, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
		}

		// 메인 쓰레드 생성
		HANDLE hThread = CreateThread(NULL, 0, main_thread, (LPVOID)client_sock, 0, NULL);
		if (hThread)
			CloseHandle(hThread);
		hWriteEvent[client_sock_count] = CreateEvent(NULL, TRUE, FALSE, NULL);
		hReadEvent[client_sock_count] = CreateEvent(NULL, TRUE, FALSE, NULL);

		// client sock count 증가
		client_sock_count++;
	}
	
	// 이벤트 핸들 닫기
	CloseHandle(hKeyEvent);
	CloseHandle(hGameStartEvent);
	for (int i = 0; i < MAX_PLAYER; ++i) {
		CloseHandle(hWriteEvent[i]);
		CloseHandle(hReadEvent[i]);
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

void send_goal_packet()
{

}
void sent_start_packet()
{

}

void InitBuffer()
{
	block_robot[0].road[0][0] = -203,	block_robot[0].road[0][1] = 140;
	block_robot[0].road[1][0] = -203,	block_robot[0].road[1][1] = -150;

	block_robot[1].road[0][0] = -199,	block_robot[1].road[0][1] = 140;
	block_robot[1].road[1][0] = -199,	block_robot[1].road[1][1] = -150;

	block_robot[2].road[0][0] = -201,	block_robot[2].road[0][1] = 130;
	block_robot[2].road[1][0] = -201,	block_robot[2].road[1][1] = -150;

	block_robot[3].road[0][0] = -202,	block_robot[3].road[0][1] = 0;
	block_robot[3].road[1][0] = -202,	block_robot[3].road[1][1] = 150;

	block_robot[4].road[0][0] = -200,	block_robot[4].road[0][1] = 0;
	block_robot[4].road[1][0] = -200,	block_robot[4].road[1][1] = 150;

	block_robot[5].road[0][0] = -201,	block_robot[5].road[0][1] = 5;
	block_robot[5].road[1][0] = -201,	block_robot[5].road[1][1] = 150;

	block_robot[6].road[0][0] = -195,	block_robot[6].road[0][1] = -153;
	block_robot[6].road[1][0] = -195,	block_robot[6].road[1][1] = -147;

	block_robot[7].road[0][0] = 200,	block_robot[7].road[0][1] = -150;
	block_robot[7].road[1][0] = 190,	block_robot[7].road[1][1] = -150;

	block_robot[8].road[0][0] = 200,	block_robot[8].road[0][1] = -153;
	block_robot[8].road[1][0] = -200,	block_robot[8].road[1][1] = -148;

	block_robot[9].road[0][0] = 200,	block_robot[9].road[0][1] = -152;
	block_robot[9].road[1][0] = -200,	block_robot[9].road[1][1] = -152;

	block_robot[10].road[0][0] = 198,	block_robot[10].road[0][1] = -150;
	block_robot[10].road[1][0] = -198,	block_robot[10].road[1][1] = -150;

	block_robot[11].road[0][0] = 196,	block_robot[11].road[0][1] = -148;
	block_robot[11].road[1][0] = -196,	block_robot[11].road[1][1] = -148;

	block_robot[12].road[0][0] = 198,	block_robot[12].road[0][1] = -110;
	block_robot[12].road[1][0] = 204,	block_robot[12].road[1][1] = -110;

	block_robot[13].road[0][0] = 204,	block_robot[13].road[0][1] = -40;
	block_robot[13].road[1][0] = 198,	block_robot[13].road[1][1] = -40;

	block_robot[14].road[0][0] = 203,	block_robot[14].road[0][1] = -40;
	block_robot[14].road[1][0] = 203,	block_robot[14].road[1][1] = -110;

	block_robot[15].road[0][0] = 199,	block_robot[15].road[0][1] = -40;
	block_robot[15].road[1][0] = 199,	block_robot[15].road[1][1] = 20;

	block_robot[16].road[0][0] = 200,	block_robot[16].road[0][1] = 50;
	block_robot[16].road[1][0] = 200,	block_robot[16].road[1][1] = 140;

	block_robot[17].road[0][0] = 202,	block_robot[17].road[0][1] = 140;
	block_robot[17].road[1][0] = 202,	block_robot[17].road[1][1] = 50;

	block_robot[18].road[0][0] = 201,	block_robot[18].road[0][1] = 148;
	block_robot[18].road[1][0] = 201,	block_robot[18].road[1][1] = 148;
	for (int i = 0; i < BLOCK_NUM; ++i) {
		block_robot[i].x = block_robot[i].road[0][0];
		block_robot[i].z = block_robot[i].road[0][1];
		block_robot[i].speed = 0.2f, block_robot[i].shake_dir = 1;
		if (block_robot[i].road[0][0] < block_robot[i].road[1][0])
			block_robot[i].y_radian = 90.0f;
		if (block_robot[i].road[0][0] > block_robot[i].road[1][0])
			block_robot[i].y_radian = -90.0f;
		if (block_robot[i].road[0][1] < block_robot[i].road[1][1])
			block_robot[i].y_radian = 0.0f;
		if (block_robot[i].road[0][1] > block_robot[i].road[1][1])
			block_robot[i].y_radian = 180.0f;
	}
}

BB get_bb(Robot robot)
{
	glm::mat4 Transform = glm::mat4(1.0f);//변환 행렬 생성 T
	Transform = glm::translate(Transform, glm::vec3(robot.x, 0.0f, robot.z));
	Transform = glm::rotate(Transform, glm::radians(robot.y_radian), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 spots[4]{ glm::vec3(-0.3f, 0.0f, 0.1f), glm::vec3(0.3f, 0.0f, 0.1f), glm::vec3(-0.3f, 0.0f, -0.1f), glm::vec3(0.3f, 0.0f, -0.1f) };
	for (int i = 0; i < 4; ++i)
		spots[i] = glm::vec3(Transform * glm::vec4(spots[i], 1.0f));

	BB bounding_box{ spots[0].x, spots[0].z, spots[0].x, spots[0].z };
	for (int i = 1; i < 3; ++i) {
		if (bounding_box.x1 > spots[i].x)
			bounding_box.x1 = spots[i].x;
		if (bounding_box.x2 < spots[i].x)
			bounding_box.x2 = spots[i].x;
		if (bounding_box.z1 > spots[i].z)
			bounding_box.z1 = spots[i].z;
		if (bounding_box.z2 < spots[i].z)
			bounding_box.z2 = spots[i].z;
	}

	return bounding_box;
}
bool collision(BB obj_a, BB obj_b) {
	if (obj_a.x1 > obj_b.x2) return false;
	if (obj_a.x2 < obj_b.x1) return false;
	if (obj_a.z1 > obj_b.z2) return false;
	if (obj_a.z2 < obj_b.z1)return false;

	return true;
}
