#include "..\Common.h"

#define SERVERPORT 9000
#define BUFSIZE    1024

#define MAX_PLAYER 3	// 접속 인원 제한

typedef struct Bounding_Box {
	float x1, z1, x2, z2;
}BB;

typedef struct player_packet {
	float x, y, z, size, road[2][2], speed, y_radian;
	BB bb;
	bool move;
};

void send_collision_packet();
void send_goal_packet();
void sent_start_packet();

DWORD WINAPI main_thread(LPVOID arg)
{
	SOCKET sock = (SOCKET)arg;
	char buf[BUFSIZE + 1];
	int retval;

	printf("[Thread] 클라이언트 스레드 시작\n");

	while (1)
	{
		// recv()
		retval = recv(sock, buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0) {
			printf("[Thread] 클라이언트 종료 요청\n");
			break;
		}

		buf[retval] = '\0';
		printf("[Thread] 수신 데이터: %s\n", buf);

		// echo send() - 받은 걸 그대로 클라에 돌려줌
		retval = send(sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	closesocket(sock);
	printf("[Thread] 클라이언트 스레드 종료\n");

	return 0;
}

//DWORD WINAPI server_key_thread(LPVOID arg)
//{
//
//}

player_packet player_collision();

HANDLE hKeyEvent;

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

	// 데이터 통신에 사용할 변수
	SOCKET client_sock, client_socks[MAX_PLAYER];
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	// 클라이언트 접속 수
	int client_sock_count = 0;

	while (client_sock_count < MAX_PLAYER) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 소켓 배열에 저장
		client_socks[client_sock_count] = client_sock;
		client_sock_count++;

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
		
		// 쓰레드 생성
		HANDLE hThread = CreateThread(NULL, 0, main_thread, (LPVOID)client_sock, 0, NULL);
		if (hThread != NULL)
			CloseHandle(hThread);
		else
			closesocket(client_sock);
	}

	printf("\n3명 접속 완료\n");

	// 게임 시작 패킷 전송
	const char* gameStartMsg = "GAME_START";

	for (int i = 0; i < MAX_PLAYER; i++) {
		send(client_socks[i], gameStartMsg, strlen(gameStartMsg), 0);
	}

	printf("[TCP 서버] GAME_START 패킷 전송 완료\n");

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

player_packet player_collision()
{

}
