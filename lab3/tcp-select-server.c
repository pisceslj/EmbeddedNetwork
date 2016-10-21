#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/select.h>
#define BUFFER_LENGTH 1024
//length of buf
#define SERV_PORT 3003
#define BACKLOG 5
int main(int argc,char* argv[]){
	struct sockaddr_in server_addr,client_addr;
	int listenfd,connfd[BACKLOG];//�����׽�������Ϊ����ΪBACKLOG������
	int newfd,maxfd;
	socklen_t client_addr_len = sizeof(struct sockaddr);
	char buf[BUFFER_LENGTH];
	int n,nready;
	int i=0,count=0;
	int m;
	struct timeval tv;
	fd_set allset,rset;
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	//create ipv4 listen socket
	if(listenfd == -1){
		perror("created TCP socket");
		exit(0);
	}
	bzero(&server_addr,sizeof(server_addr));
	//init server_addr
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERV_PORT);
	if(bind(listenfd,(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1){
		perror("call to bind");
		exit(1);
	}
	listen(listenfd,BACKLOG);//�����׽��ֽ��м���
	FD_ZERO(&allset);//��������е�����
	FD_SET(listenfd, &allset);//�������׽�����ӵ�����
	FD_SET(0, &allset);//����׼������ӵ�����
	maxfd = listenfd;//���õ�ǰ���������Ϊ�����׽���
	while(1){
		rset = allset;
		tv.tv_sec = 1;//�������ȴ�ʱ��
    	tv.tv_usec =0;
	 	nready = select(maxfd + 1, &rset, NULL, NULL, &tv);
	 	if (nready < 0) {
            perror("select");
            break;
        } 
    else if (nready == 0) {
        //printf("timeout\n");
        continue;
    }
        if(FD_ISSET(0, &rset))
        {//����м������룬��׼����׼������
        	printf("enter exit to log out\n");
        	fgets(buf,BUFFER_LENGTH,stdin);
        	//scanf("%s",buf);
        	if(strcmp(buf,"exit\n") == 0){//��������exit�Ƚ�
        		for (i = 0; i < BACKLOG; i++) {
        			if (connfd[i] != 0) {
           	 			close(connfd[i]);
           	 			connfd[i] = 0;
        			}
    			}
    			close(listenfd);
    			exit(1);
        	}
        }
       	if(FD_ISSET(listenfd, &rset))
       	{//�����׽���׼������
       		newfd = accept(listenfd,(struct sockaddr*)&client_addr,&client_addr_len); //���½��׽��ַ�����ʱ����
       		if (newfd <= 0) {
                perror("accept");
                continue;
          }
          if(count < BACKLOG){//�ж��Ƿ�ﵽ���������
            connfd[count] = newfd;
            printf("new connection client %d\n",client_addr.sin_port);
            maxfd = connfd[count] > maxfd ? connfd[count]:maxfd;//���õ�ǰ���������
            FD_SET(connfd[count], &allset);//���½��׽��ּ��뼯��
            count++;//��������+1
          }
            else{
            	printf("max connections arrive, exit\n");
            	send(newfd,"bye", 4 ,0);
            	close(newfd);
            }
        	//exit(1);          
       	}
       	for(i = 0; i < count; i++)
       	{//����ÿ�������׽���
       		if(FD_ISSET(connfd[i],&rset))
       		{
       			n = recv(connfd[i],buf,BUFFER_LENGTH,0);    		
       			if(n > 0)
       			{
       				buf[n]='\0';
				if(strcmp(buf,"exit") == 0)
				{
					n = send(connfd[i],buf,strlen(buf),0);
					printf("client %d close\n", client_addr.sin_port);
					if(close(connfd[i])==-1)
					{
						perror("fail to close");
						exit(1);
					}
					FD_CLR(connfd[i], &allset);//��������е�connfd
               			//connfd[i] = 0;//��������е�connfd
               			//count--;//���Ӹ���-1
					for(m=i; m<count; m++)
					{
						connfd[m] == connfd[m+1];
					}
					connfd[--count]=0;
               				continue;
				}
				printf("client %d send:%s\n",client_addr.sin_port, buf);
				n = send(connfd[i],buf,strlen(buf),0);
				if(n == -1)
				{
					perror("fail to reply");
					exit(1);
				}
       			}
       			else
       			{
       				printf("client %d close\n", client_addr.sin_port);
       				close(connfd[i]);
       				FD_CLR(connfd[i], &allset);
                		connfd[i] = 0;
                		//count--;
				for(m=i; m<count-1; m++)
				{
					connfd[m] == connfd[m+1];
				}
				connfd[--count]=0;
            		}
       		}
       	}
    }
    //close(listenfd);
    return 0;
}
