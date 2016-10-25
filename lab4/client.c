#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define BUFFER_LENGTH 1024

int n = 0;
char buf[BUFFER_LENGTH];
int connfd;
int flag = 0;

void* recvThread(void *arg)
{
	while(!flag)
	{ 
		n = recv(connfd, buf, BUFFER_LENGTH, 0);
		if (n == -1)
		{
			perror("接收失败\n");
			exit(1);
		}
		else
		{
			buf[n] = '\0';
			printf("Client recv: %s\n", buf);
			if(!strcmp(buf,"exit"))
			{
				flag = 1;
			}
		}	
	}
	pthread_exit(0);
}

int main(int argc, char const *argv[])
{
	struct sockaddr_in server_addr;
	pthread_t recv_thr_id;
	//验证输入参数
	if (argc != 3)
	{
		fputs("usage: ./udpserver serverIP serverPort\n", stderr);
		exit(1);
	}

	//创建套接字
	connfd = socket(AF_INET, SOCK_STREAM, 0);
	if(connfd == -1)
	{
		perror("套接字创建失败\n");
		exit(1);  
	}

	//初始化参数
	bzero(&server_addr, sizeof(server_addr));     //清零
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));  //接收命令行输入的第二个参数
	
	//网络地址转换
	if (inet_aton(argv[1], &server_addr.sin_addr) == 0)
	{
		perror("网络地址转换失败\n");
		exit(1);
	}

	//连接套接字
	if (connect(connfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("连接失败\n");
		exit(1);
	}

	if(pthread_create(&recv_thr_id, NULL, recvThread, NULL) != 0)
	{
		printf("Create Thread ERROR!!\n");
		exit(1);
	}

	while(strcmp(buf,"exit"))
	{
		memset(buf, 0, BUFFER_LENGTH);     //清零
		gets(buf);                         //获取buf值
		n = send(connfd, buf, strlen(buf), 0);
		if (n == -1)
		{
			perror("响应失败\n");
			exit(1);
		}
	}
	
	if (close(connfd) == -1)
	{
		perror("关闭套接字失败");
		exit(1);
	}
	puts("TCP客户端已关闭\n");
    
	return 0;
}
