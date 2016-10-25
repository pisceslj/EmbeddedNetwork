#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define BUFFER_LENGTH 1024
#define SERV_PORT 3001
#define LISTENQ  10  

int main(int argc, char const *argv[])
{
	int connfd,newfd;
	char sendbuf[BUFFER_LENGTH];
	char recvbuf[BUFFER_LENGTH];
	fd_set allset,rset;
	int n = 0,i = 0;
	int flag = 0;
	int retval,maxfd;
	pid_t pid;
	struct sockaddr_in server_addr, client_addr;  //定义服务器端和客户端地址变量
	int listenfd; //定义监听fd
	socklen_t client_addr_len = sizeof(struct sockaddr_in);
	
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
	if(bind(listenfd,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1)
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
	
	FD_ZERO(&allset);//把可读文件描述符集合清空
	FD_SET(0,&allset);
	maxfd = 0;
	FD_SET(listenfd,&allset);//把当前连接的文件描述符加入到集合中
	maxfd = listenfd;//设置当前最大描述符为监听套接字
	//响应客户端
	printf("*****************聊天开始***************\n");	
	while(1)
	{	
		rset = allset;
		/*等待聊天*/
		retval = select(maxfd+1,&rset,NULL,NULL,NULL);
		if(retval == -1)
		{
			printf("select 出错\n");
			break;
		}
		else
		{
			/*用户输入信息，开始处理信息并发送*/
			if(FD_ISSET(0,&rset))
			{		
				gets(sendbuf);
				printf("enter");
				if(!strcmp(sendbuf,"exit"))
				{
					//flag = 1;
					printf("close ok2\n");
					if(close(connfd) == -1)
					{
						perror("close failed");
						exit(-1);
					}
					//_exit(0);
					break;
				}
			}
			
			if(FD_ISSET(listenfd,&rset))
			{
				while(1)
				{
					if((connfd = accept(listenfd,(struct sockaddr*)&client_addr,&client_addr_len)) == -1)
					{
						perror("accept");
						exit(-1);
					}
					printf("\n加入聊天的客户端是：%s: %d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
					maxfd = connfd > maxfd ? connfd:maxfd;//设置当前最大描述符为
					FD_SET(connfd,&allset);//将新建套接字加入集合
					
					//创建进程
					if((pid=fork()) > 0)//父进程
					{
						if(close(connfd) == -1)
						{
							perror("close failed");
							exit(-1);
						}
						/*用户输入信息，开始处理信息并发送*/
						if(FD_ISSET(0,&rset))
						{		
							gets(sendbuf);
							printf("enter");
							if(!strcmp(sendbuf,"exit"))
							{
								//flag = 1;
								printf("close ok2\n");
								if(close(connfd) == -1)
								{
									perror("close failed");
									exit(-1);
								}
								_exit(0);
								//break;
							}
						}
						continue;
					}
					else if(pid == 0) //子进程
					{	
						if(FD_ISSET(connfd,&allset))
						{
							/*关闭服务器的套接字*/ 
	       					 	if(close(listenfd) == -1)
							{	
								perror("close failed");
								exit(-1);
							} 
	       					 	while(1)
							{
								memset(recvbuf,0,BUFFER_LENGTH);
								n = recv(connfd,recvbuf,BUFFER_LENGTH,0);
								if(n < 0)
								{
									perror("recv failed");
									exit(-1);
								}
								recvbuf[n] = '\0';
								if(!strcmp(recvbuf,"exit"))
								{
									if(close(connfd) == -1)
									{
										perror("failed to close");
										exit(-1);
									}
									FD_CLR(connfd,&allset);//清除集合中对应的connfd
									printf("close ok\n");
									//flag = 1;
									_exit(0);
								}
								else
								{
									printf("\n正参与聊天的客户端是：%s: %d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
									printf("server recv:%s\n",recvbuf);
									n = send(connfd,recvbuf,sizeof(recvbuf),0);
									if(n < 0)
									{
										perror("send failed");
										exit(-1);
									}				
								}
							}
						}
					}
					else
					{
						perror("fork failed");
						exit(-1);
					}
				}
			}	
		}
	}//while结束处 
	if(close(listenfd) == -1)
	{	
		perror("close failed");
		exit(-1);
	} 
	printf("server 退出!\n");
          
	return 0;    
}

