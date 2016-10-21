    #include <stdio.h>  
    #include <string.h>  
    #include <sys/select.h>  
    #include <sys/types.h>   
    #include <sys/socket.h>  
    #include <sys/time.h>  
    #include <unistd.h>  
    #include <arpa/inet.h>  
    #include <errno.h>  
    #include <netinet/in.h>  //for INADDR_ANY  
    #include <pthread.h>  
      
    #define SERVER_PORT 3001    /* 服务器端的端口号 */  
    #define MAX_LENGTH 1024     /* 缓存最大长度 */  
      
    /* 表示是否需要发送数据给服务器端  
     * 1表示接收到用户的输入信息，需要发送给服务器  
     * 0表示接收服务端的消息 */  
    static int g_iIsSend = 0;  
      
    /* 发送缓冲区 */  
    char sendBuf[MAX_LENGTH] = {0};  
      
    /* 表示是否正在等待用户的输入 
     * 1正在等待用户的输入 
     * 0用户已输入完成 
     */  
    static int g_iWait = 1;  
      
    /* 线程处理函数 */  
    void *recevie_user_input(void *arg);  
      
    int main(int argc,char const *argv[])  
    {  
        int sockfd = -1, ret = -1, i = -1;  
        struct sockaddr_in servaddr;  
        char buf[MAX_LENGTH] = {0};  
        int n = -1;  
        pthread_t tid;  
        pthread_attr_t attr;  
          
        /* 创建socket s*/  
        sockfd = socket(AF_INET, SOCK_STREAM, 0);  
        if (-1 == sockfd)  
        {  
            printf("create socket error: %s\n",strerror(errno));  
            return -1;  
        }     
          
        bzero(&servaddr, sizeof(servaddr));  
        servaddr.sin_family = AF_INET;  
        servaddr.sin_port = htons(atoi(argv[2]));  
        inet_pton(AF_INET, argv[1], &servaddr.sin_addr);  
        ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));  
        if (-1 == ret)  
        {  
            printf("connect socket error: %s\n",strerror(errno));  
            return -1;  
        }     
          
        pthread_attr_init(&attr);  
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED); //设置分离属性  
        if (pthread_create(&tid, &attr, recevie_user_input, NULL) != 0)  
        {  
            printf("create thread error.\n");  
            return -1;  
        }  
          
        while(1)  
        {     
            /* 如果1 == g_iIsSend，表示用户输入的信息已经存到发送缓存sendBuf,需要发送给服务器 */  
            if (1 == g_iIsSend)  
            {  
                n = send(sockfd, sendBuf, strlen(sendBuf), 0);  
                if (n < 0)  
                {  
                    printf("send error: %s\n",strerror(errno));  
                    break;  
                }  
                g_iIsSend = 0;  
                continue;  
            }  
      
            /* 不断循环检服务器是否发送过来数据，如果有，接受数据，并显示到屏幕上，如果没有立即返回，并跳到while(1)处 */  
            memset(buf, 0, sizeof(buf));  
            //设置为非阻塞MSG_DONTWAIT，如果发生阻塞则返回EAGAIN  
            n = recv(sockfd, buf, sizeof(buf), MSG_DONTWAIT);  
            if ( n < 0)  
            {  
                if(errno == EAGAIN)  
                    continue;  
                else  
                    break;;   
            }  
            else if (n == 0) //对端关闭  
            {  
                close(sockfd);  
                sockfd  = -1;  
            }  
      
            /* 1 == g_iWait表示此时客户端没有做任何操作，但是服务器有数据可能，应该是其他客户端发来的消息 */  
            if (1 == g_iWait)  
            {  
                printf("\n");  
                printf("\033[1A"); //先回到上一行    
                printf("\033[K");  //清除该行“"please input:”     
            }  
            /* 显示完服务器发来的数据，重新显示删除的行“"please input:”*/  
            printf("%s", buf);  
            if (1 == g_iWait)  
            {  
                printf("please input:");  
                fflush(stdout);  
            }  
        }  
          
        if (sockfd < 0)  
        {  
            close(sockfd);  
            sockfd = -1;  
        }  
        return 0;  
    }  
      
    /* 处理用户输入的信息  
     * CTRL + Backspace : 可以一个一个字符删除 
     * CTRL + U: 重新从开始输入  
     */  
    void *recevie_user_input(void *arg)  
    {  
        int i = -1;  
        while(1)  
        {  
            /* 设置标准输出不带缓冲，这样不用加"\n"也会即时输出 */  
            //setbuf(stdout, NULL);  
            if (0 == g_iIsSend)  
            {  
                /* 从标准输入读取字符串 */  
                printf("please input:");  
                //把输出缓冲区里的东西打印到标准输出设备,否则，不会立即输出  
                fflush(stdout);  
                g_iWait = 1;  
                memset(sendBuf, 0, sizeof(sendBuf));  
                read(STDIN_FILENO, sendBuf, sizeof(sendBuf));  
                g_iWait = 0; //表示用户发送消息结束  
                /* 
                 * \e[ 或 \033[ 是 CSI，用来操作屏幕的。 
                 * \e[K 表示从光标当前位置起删除到 EOL （行尾） 
                 * \e[NX 表示将光标往X方向移动N，X = A(上) / B(下) / C(左) / D(右)，\e[1A 就是把光标向上移动1行 
                 */  
                printf("\033[1A"); //先回到上一行    
                printf("\033[K");  //清除该行    
                g_iIsSend = 1;  
            }  
        }  
    }  
