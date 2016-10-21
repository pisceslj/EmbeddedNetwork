#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_LENGTH 1024

int main(int argc, char const *argv[])
{
	int n = 0;
	char sendbuf[BUFFER_LENGTH];
	char recvbuf[BUFFER_LENGTH];
	int connfd;
	int flag = 0;
	fd_set allset;
	int retval,maxfd;
	struct sockaddr_in server_addr;
	
	//验证输入参数
	if (argc != 3)
	{
		fputs("usage: ./udpserver serverIP serverPort\n", stderr);
		exit(-1);
	}

	//创建套接字
	connfd = socket(AF_INET, SOCK_STREAM, 0);
	if(connfd == -1)
	{
		perror("套接字创建失败\n");
		exit(-1);  
	}

	//初始化参数
	bzero(&server_addr, sizeof(server_addr));     //清零
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));  //接收命令行输入的第二个参数
	
	//网络地址转换
	if (inet_aton(argv[1], &server_addr.sin_addr) == 0)
	{
		perror("网络地址转换失败\n");
		exit(-1);
	}

	//连接套接字
	if (connect(connfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("连接失败\n");
		exit(-1);
	}

	while(!flag)
	{
		FD_ZERO(&allset);//把可读文件描述符集合清空
		FD_SET(0,&allset);
		maxfd = 0;
		FD_SET(connfd,&allset);//把当前连接的文件描述符加入到集合中
		if(maxfd < connfd)//找出最大
			maxfd = connfd;
			
		retval = select(maxfd+1,&allset,NULL,NULL,NULL);
		if(retval == -1)
		{
			printf("select 出错\n");
			break;
		}
		else if(retval == 0)
		{
			printf("waiting...\n");
			continue;
		}
		else
		{
			if(FD_ISSET(connfd,&allset))
			{
				n = recv(connfd, recvbuf, BUFFER_LENGTH, 0);
				if (n == -1)
				{
					perror("接收失败\n");
					exit(-1);
				}
				else
				{
					recvbuf[n] = '\0';
					printf("Client recv: %s\n", recvbuf);
				}	
			}
			
			if(FD_ISSET(0,&allset))
			{
				memset(sendbuf, 0, BUFFER_LENGTH);     //清零
				gets(sendbuf);                         //获取buf值
				if(!strcmp(sendbuf,"exit"))
				{
					flag = 1;
					n = send(connfd, sendbuf, strlen(sendbuf), 0);
					if (n == -1)
					{
						perror("响应失败\n");
						exit(-1);
					}
				}
				else
				{	
					n = send(connfd, sendbuf, strlen(sendbuf), 0);
					if (n == -1)
					{
						perror("响应失败\n");
						exit(-1);
					}
			
				
				}
			}
			
		}
	}//内循环结束处
	if (close(connfd) == -1)
	{
		perror("关闭套接字失败");
		exit(-1);
	}
	puts("客户端已关闭\n");
	
	return 0;
}
