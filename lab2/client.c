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
char buf[BUFFER_LENGTH];
int socketfd;
socklen_t addr_len;
struct sockaddr_in server_addr;

//创建接收线程
void* recvThread(void *arg)
{
	while(!flag)
	{ 
		n = recvfrom(socketfd,&buf,sizeof(buf),0,(struct sockaddr*)&server_addr,&addr_len);
		if (n == -1)
		{
			perror("接收失败\n");
			exit(-1);
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


int main(int argc,char *argv[])
{
	pthread_t recv_thr_id;
	addr_len = sizeof(server_addr);
	
	//判断命令行输入参数
	if(argc < 3)
	{
		printf("uasge:%s ip port\n",argv[0]);
		exit(-1);
	}
	
	//创建套接字
	if((socketfd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		perror("socket failed");
		exit(-1);
	}
	
	//初始化操作
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	
	//发送数据
	if(sendto(socketfd,&buf,sizeof(buf),0,(struct sockaddr*)&server_addr,addr_len) < 0)
	{
		perror("sendto");
		exit(-1);
	}	
	
	//创建线程
	if(pthread_create(&recv_thr_id, NULL, recvThread, NULL) != 0)
	{
		printf("Create Thread ERROR!!\n");
		exit(1);
	}

	//循环监听
	while(strcmp(buf,"exit"))
	{
		memset(buf, 0, BUFFER_LENGTH);     //清零
		printf("Client send:");
		gets(buf);                         //获取buf值
		//fgets(buf,BUFFER_LENGTH,stdin);
		if(sendto(socketfd,&buf,sizeof(buf),0,(struct sockaddr*)&server_addr,addr_len) < 0)
		{
			perror("sendto failed");
			exit(-1);
		}
			
		
	}
	
	if(close(socketfd) < 0)
	{
		perror("close socket failed");
		exit(-1);
	}
	puts("TCP客户端已关闭\n");
	
	return 0;
}







