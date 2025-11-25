#include "..\Common.h"

#define SERVERPORT 9000
#define BUFSIZE    1024

#define MAX_PLAYER 3	// 접속 인원 제한

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
player_packet player_robot[MAX_PLAYER];
player_packet block_robot[19];

void player_collision(int id);

HANDLE hKeyEvent, hGameStartEvent;
HANDLE hReadEvent[MAX_PLAYER], hWriteEvent[MAX_PLAYER];

void send_collision_packet();
void send_goal_packet();
void sent_start_packet();

DWORD WINAPI main_thread(LPVOID arg)
{
	SOCKET sock = (SOCKET)arg;
	char buf[BUFSIZE + 1];
	int retval, out = 1, client_id = -1;

	WaitForSingleObject(hGameStartEvent, INFINITE);

	// 게임 시작 패킷 전송
	const char* gameStartMsg = "GAME_START";

	send(sock, gameStartMsg, strlen(gameStartMsg), 0);
	printf("[TCP 서버] GAME_START 패킷 전송 완료\n");

	printf("[Thread] 클라이언트 스레드 시작\n");

	// 클라이언트 ID값 수신
	retval = recv(sock, (char*)&client_id, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
	printf("[Thread] 클라이언트 ID(client_%d) 수신 완료\n", client_id);

	while (out) {
		// recv() 플레이어 정보 받기 - Robot
		retval = recv(sock, (char*)&player_robot[client_id], sizeof(player_robot[client_id]), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		SetEvent(hWriteEvent[client_id]);

		player_collision(client_id);

		// Robot 업데이트 대기
		WaitForMultipleObjects(MAX_PLAYER, hReadEvent, TRUE, INFINITE);
		ResetEvent(hWriteEvent[client_id]);

		// send() 플레이어 정보 보내기 - Robot[3]
		for (int i = 0; i < MAX_PLAYER; i++) {
			retval = send(sock, (char*)&player_robot[i], sizeof(player_robot[i]), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}
		
		ResetEvent(hReadEvent[client_id]);
	}

	closesocket(sock);
	printf("[Thread] 클라이언트 스레드 종료\n");

	return 0;
}

DWORD WINAPI server_key_thread(LPVOID arg)
{

}

int main(int argc, char* argv[])
{
	int retval;

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
	SOCKET client_sock, client_socks[MAX_PLAYER];
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	// 클라이언트 접속 수
	int client_sock_count = 0;

	while (1) {
		// 최대 접속 인원 수 제한
		if (client_sock_count >= MAX_PLAYER) {
			if (WaitForSingleObject(hGameStartEvent, 0)) {
				printf("\n3명 접속 완료\n");
				SetEvent(hGameStartEvent);
				continue;
			}
			else
				continue;
		}
		
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}
		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 클라이언트에 ID값 전송
		send(client_sock, (char*)&client_sock_count, sizeof(int), 0);

		// 쓰레드 생성
		HANDLE hThread = CreateThread(NULL, 0, main_thread, (LPVOID)client_sock, 0, NULL);
		if (hThread != NULL)
			CloseHandle(hThread);
		else
			closesocket(client_sock);
		hReadEvent[client_sock_count] = CreateEvent(NULL, TRUE, FALSE, NULL);
		hWriteEvent[client_sock_count] = CreateEvent(NULL, TRUE, FALSE, NULL);

		// client sock count 증가
		client_sock_count++;
	}
	
	// 이벤트 핸들 닫기
	CloseHandle(hKeyEvent);
	CloseHandle(hGameStartEvent);
	for(int i=0; i< MAX_PLAYER; i++) {
		CloseHandle(hReadEvent[i]);
		CloseHandle(hWriteEvent[i]);
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

void send_collision_packet()
{

}
void send_goal_packet()
{

}
void sent_start_packet()
{

}

void player_collision(int id)
{
	WaitForMultipleObjects(MAX_PLAYER, hWriteEvent, TRUE, INFINITE);
	//충돌 체크

	SetEvent(hReadEvent[id]);
}
