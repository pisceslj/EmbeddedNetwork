#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define BUFFER_LENGTH 1024

int n;
int flag = 0;
int socketfd;
socklen_t addr_len;
char sendbuf[BUFFER_LENGTH];
char recvbuf[BUFFER_LENGTH];
struct sockaddr_in server_addr,client_addr;

//创建接收线程
void* recvThread(void *arg)
{
	while(!flag)
	{ 
		n = recvfrom(socketfd,&recvbuf,sizeof(recvbuf),0,(struct sockaddr*)&client_addr,&addr_len);
		printf("\nClient IP is %s and port is %d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		if (n == -1)
		{
			perror("接收失败\n");
			exit(-1);
		}
		else
		{
			recvbuf[n] = '\0';
			printf("The Messages are: %s\n", recvbuf);
			if(!strcmp(recvbuf,"exit"))
			{ 
				flag = 1;
			}
		}	
	} 
	//关闭套接字  
	if (close(socketfd) == -1)
	{
		perror("关闭套接字失败");
		exit(1);
	}
	puts("UDP服务端已关闭\n");
	exit(0);
} 

int main(int argc,char *argv[])
{
	pthread_t recv_thr_id;
	addr_len = sizeof(client_addr);
	
	//判断命令行输入参数
	if(argc < 3)
	{
		printf("uasge: %s ip port\n",argv[0]);
		exit(-1);
	}
	
	//初始化
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	
	//创建套接字
	if((socketfd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		perror("socket failed");
		exit(-1);
	}
	
	//绑定端口号
	if(bind(socketfd,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
	{
		perror("bind failed");
		exit(-1);
	}
	
	//创建子线程
	if(pthread_create(&recv_thr_id, NULL, recvThread, NULL) != 0)
	{
		printf("Create Thread ERROR!!\n");
		exit(-1);
	}
	
	//循环监听
	while(strcmp(recvbuf,"exit") && !flag)
	{
		memset(sendbuf,0,BUFFER_LENGTH);
		sendbuf[n] = '\0';
		
		gets(sendbuf);
		//fgets(sendbuf,sizeof(sendbuf),stdin);
		n = sendto(socketfd,&sendbuf,sizeof(sendbuf),0,(struct sockaddr*)&client_addr,sizeof(client_addr));
		if(n == -1)
		{
			perror("sendto");
			exit(-1);
		}
		
	}
	
	return 0;
}

