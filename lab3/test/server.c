    #include <stdio.h>  
    #include <string.h>  
    #include <sys/select.h>  
    #include <sys/types.h>   
    #include <sys/socket.h>  
    #include <sys/time.h>  
    #include <unistd.h>  
    #include <errno.h>  
    #include <netinet/in.h>  //for INADDR_ANY  
      
    #define MAX_CLIENT_NUM 10   /* 客户端最大连接数 */  
    #define MAXLENGHT 1024      /* 最大缓存长度 */  
    #define SERVER_PORT 3001    /* 服务器端的端口号 */  
    #define LISTEN_MAX_NUM 5    /* 监听队列的最大值 */  
      
    int main(void)  
    {  
        int clientSocket = -1, listenSocket = -1;  
        int client[MAX_CLIENT_NUM];  
        char buf[MAXLENGHT] = {0};  
        fd_set allset, rset;  
        socklen_t clilen;  
        struct sockaddr_in cliaddr,  seraddr;  
        int i = -1, maxi = -1, maxfd = -1, ret = -1, reuse = -1, iReady = -1;  
        struct timeval tv;  
          
        /* 创建TCP监听socket */  
        listenSocket = socket(AF_INET, SOCK_STREAM, 0);  
        if (-1 == listenSocket)  
        {  
            printf("create socket error: %s\n",strerror(errno));  
            return -1;  
        }  
        else  
        {  
            printf("listenSocket = %d\n", listenSocket);  
        }  
          
        bzero(&seraddr, sizeof(seraddr));  
        seraddr.sin_family = AF_INET;  
        seraddr.sin_addr.s_addr = htonl(INADDR_ANY);  
        seraddr.sin_port = htons(SERVER_PORT);  
          
        /* 设置端口重用 */  
        reuse = 1;  
        ret = setsockopt(listenSocket,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));   
        if (0 > ret)  
        {  
            printf("setsockopt error: %s\n",strerror(errno));  
            return -1;  
        }  
          
        /* 绑定IP和端口 */  
        ret = bind(listenSocket, (struct sockaddr *)&seraddr, sizeof(seraddr));  
        if (0 > ret)  
        {  
            printf("bind error: %s\n",strerror(errno));  
            return -1;  
        }  
          
        /* 设置监听 */  
        ret = listen(listenSocket, LISTEN_MAX_NUM);  
        if (0 > ret)  
        {  
            printf("listen error: %s\n",strerror(errno));  
            return -1;  
        }  
          
        /* 初始化 */  
        maxfd = listenSocket;  
        memset(buf, 0 ,sizeof(buf));  
        for (i = 0; i < MAX_CLIENT_NUM; i++)  
            client[i] = -1;  
        FD_ZERO(&allset);  
        FD_SET(listenSocket, &allset);  
          
        /* set two seconds to timeout */  
        //tv.tv_sec = 2;  
        //tv.tv_usec = 0;  
          
        for (;;)  
        {  
            //iReady = select(maxfd + 1, &allset, NULL, NULL, &tv);  
            //将allset进行赋值的好处是，当监听到客户端的连接，if (FD_ISSET(fd, &rset))不会马上执行，因为新的客户端还没设置给rset  
            rset = allset;  
            iReady = select(maxfd + 1, &rset, NULL, NULL, NULL);  
            if (0 > iReady)  
            {  
                printf("select error: %s, errno: %d\n",strerror(errno), errno);  
                return -1;  
            }  
            else if (0 == iReady) //限定时间内没有数据到来  
            {  
                printf("no data within two seconds.\n");  
                continue;  
            }  
            else  
            {  
                printf("iReady=%d\n", iReady);  
            }  
              
            /* 客户端发起连接 */  
            if (FD_ISSET(listenSocket, &rset))  
            {  
                /* 接受来自客户端的连接 */  
                clilen = sizeof(cliaddr);  
                clientSocket = accept(listenSocket, (struct sockaddr *)&cliaddr, &clilen);  
                printf("clientSocket : %d\n", clientSocket);  
                if (-1 == clientSocket)  
                {  
                    printf("accept error: %s\n",strerror(errno));  
                    continue;  
                }  
                  
                /* 保存已连接客户端的socket */  
                for (i = 0; i < MAX_CLIENT_NUM; i++)  
                {  
                    if (client[i] < 0)  
                    {  
                        client[i] = clientSocket;  
                        break;  
                    }  
                }  
                  
                FD_SET(clientSocket, &allset);  
                if (clientSocket > maxfd)  
                    maxfd = clientSocket;  
                if (i > maxi)    // maxi表示当前连接的客户端数量  
                    maxi = i;  
            }  
              
              
            /* 处理与客户端的通信 */  
            for (i = 0; i <= maxi; i++)  
            {  
                int fd = -1, n = -1;  
                if ((fd = client[i]) < 0)  
                    continue;  
      
                /* FD_ISSET为true,只是表明fd在rset集合中有设置，不能说明fd有数据到来*/  
                if (FD_ISSET(fd, &rset))  
                {  
                    for (;;)  
                    {  
                        //接受来自客户端的数据，当前是阻塞的  
                        memset(buf, 0, sizeof(buf));  
                        n = recv(fd, buf, sizeof(buf), 0);  
                        if ( n < 0)  
                        {  
                            printf("recv error: %s\n",strerror(errno));  
                            if(errno == EAGAIN)  
                                continue;  
                            else  
                                break;;   
                        }  
                        else if (n == 0) //客户端关闭了socket  
                        {  
                            printf("client close: %d\n", fd);  
                            close(fd);  
                            FD_CLR(fd, &allset);  
                            client[i] = -1;  
                            break;  
                        }  
                          
                        if (n != sizeof(buf)) //说明没有数据可读  
                        {  
                            printf("recv:%s\n", buf);  
                            //将接受的数据都发送给连接的每个客户端  
                            int j = 0, size = 0, fd2 = -1;  
                            for (j = 0; j <= maxi; j++)  
                            {  
                                if ((fd2 = client[j]) < 0)  
                                    continue;  
                                      
                                char tmp[MAXLENGHT] = {0};  
                                snprintf(tmp, sizeof(tmp), "%d say: %s", i, buf);     
                                size = send(fd2, tmp, strlen(tmp), 0);  
                                //printf("send: %s, size: %d, n: %d, fd2:%d\n", buf, size, n, fd2);  
                                if (size < 0)  
                                {  
                                    printf("send error: %s\n",strerror(errno));  
                                    continue;  
                                }         
                            }         
                              
                            break;  
                        }  
                        else  
                        {  
                            continue; //可能有数据，还要尝试再读取     
                        }  
                    }  
                }                     
            }  
        }     
    }  
