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
#define SERV_PORT 3001
#define LISTENQ  10

int connfd;
char buf[BUFFER_LENGTH];  
int n = 0;
int flag = 0;

void* recvThread(void *arg)
{
	while(!flag)
	{ 
		n = recv(connfd, buf, BUFFER_LENGTH, 0);
		if (n == -1){
			perror("接收失败\n");
			exit(1);
		}
		else
		{
			buf[n] = '\0';
			printf("Server recv: %s\n", buf);
			if(!strcmp(buf,"exit"))
			{ 
				flag = 1;
			}
		}	
	} 
	//关闭套接字  
	if (close(connfd) == -1)
	{
		perror("关闭套接字失败");
		exit(1);
	}
	puts("TCP服务端已关闭\n");
	exit(0);
}

int main(int argc, char const *argv[])
{
	struct sockaddr_in server_addr, client_addr;  //定义服务器端和客户端地址变量
	int listenfd; //定义监听fd
	socklen_t client_addr_len = sizeof(struct sockaddr_in);
	pthread_t recv_thr_id;
	
	//创建套接字
	if((listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("套接字创建失败\n");
		exit(1);  
	}

	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERV_PORT);

	//绑定套接字
	if(bind(listenfd,(struct sockaddr_in *)&server_addr,sizeof(server_addr))==-1)
	{
		perror("绑定套接字失败\n");
		exit(1);
	} 

	//监听套接字函数
	if(listen(listenfd,LISTENQ)==-1) 
	{  
		perror("设置监听失败\n");  
		exit(1);  
	}
	
	//接收请求套接字
	if((connfd = accept(listenfd,(struct sockaddr_in *)&client_addr,&client_addr_len)) == -1)
	{
		perror("接收失败\n");
		exit(1);
	}

	if(pthread_create(&recv_thr_id, NULL, recvThread, NULL) != 0)
	{
		printf("Create Thread ERROR!!\n");
		exit(1);
	}

	//响应客户端
	while(strcmp(buf,"exit") && !flag)
	{
		memset(buf,0,BUFFER_LENGTH);            
		buf[n] = '\0'; 
		printf("Server send:");
		gets(buf);
		n = send(connfd, buf, BUFFER_LENGTH, 0);
		if (n == -1)
		{
			perror("响应失败\n");
			exit(1);
		}
	}

	return 0;    
}




