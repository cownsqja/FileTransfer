#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <pthread.h>
#include <stdio.h>

#define BUFSIZE 256
#define NAMESIZE 10

void* send_message(void* arg);
void* recv_message(void* arg);
void error_handling(char * message);
void cmd_handling(char * message, int sock);
void send_file(int sock);
void yolo();

char name[NAMESIZE] = "[Default]";
char message[BUFSIZE + 1];

int main(int argc, char **argv) {
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread, file_thread;
	void* thread_result;

	if (argc != 4) {
		printf("Usage : %s <ip> <port> <name>\n", argv[0]);
		exit(1);
	}

	sprintf(name, "[%s]", argv[3]);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1) error_handling("socket() error");
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	if (connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error!");

	pthread_create(&snd_thread, NULL, send_message, (void*)sock);
	pthread_create(&rcv_thread, NULL, recv_message, (void*)sock);

	pthread_join(snd_thread, &thread_result);
	pthread_join(rcv_thread, &thread_result);


	close(sock);
	return 0;
}

void* send_message(void * arg) {
	int sock = (int)arg;
	char name_message[NAMESIZE + BUFSIZE];
	while (1) {
		fgets(message, BUFSIZE, stdin);
		if (!strcmp(message, "q\n")) {
			close(sock);
			exit(0);
		}
		sprintf(name_message, "%s %s", name, message);

		write(sock, name_message, strlen(name_message));
	}
}

void* recv_message(void* arg) {
	int sock = (int)arg;
	char name_message[NAMESIZE + BUFSIZE];
	int str_len;

	while (1) {
		str_len = read(sock, name_message, NAMESIZE + BUFSIZE - 1);
		if (str_len == -1)
			return (void*)1;

		name_message[str_len] = 0;
		fputs(name_message, stdout);

		//running cmdline
		if (strstr(name_message, "/") != NULL) {
			cmd_handling(name_message, sock);
		}
	}
}

void error_handling(char * message) {
	fputs(message, stderr);
	fputc('\n', stderr);
}

void cmd_handling(char * message, int sock) {
	char pipchar[BUFSIZE];
	//photo shoot
	if (strstr(message, "/image") != NULL) {
		printf("photo shoot \n");
		sprintf(pipchar, "raspistill -o %s.jpg", name);
		popen(pipchar, "r");
	}

	//���� ����
	if (strstr(message, "/get_image") != NULL) {
		send_file(sock);
	}

	if (strstr(message, "/yolo") != NULL) {
		yolo();

	}
}

void send_file(int sock) {
	FILE *fp;
	char filename[BUFSIZE];
	char message[BUFSIZE];
	int readsum = 0;
	int filesize = 0;

	sprintf(filename, "%s.jpg", name);

	fp = fopen(filename, "rb"); // read mode open
	if (fp == NULL) {
		printf("File open error\n");
		exit(0);
	}

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	rewind(fp);

	// send the file info
	sprintf(message, "%s %s %d\n", "/get_image", filename, filesize);
	write(sock, message, BUFSIZE);

	// send the file fragments
	while (readsum < filesize) {
		fread(message, 1, BUFSIZE, fp);
		write(sock, message, BUFSIZE); // send network
		readsum += BUFSIZE;
	}
	fclose(fp);
	printf("File Send End\n");
}

void yolo() {
	char yolo_cmd[20];
	FILE *fp;


	chdir("darknet");

	fp = popen("./darknet detect", "r");
	if (NULL == fp)
	{
		perror("popen() 실패");
		return -1;
	}

	while (fgets(yolo_cmd, BUFSIZE, fp))
		printf("%s", yolo_cmd);

}