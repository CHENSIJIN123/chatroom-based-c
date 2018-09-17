/* server.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "wrap.h"

#define MAX_CLIENT 20
#define MAXLINE 240
#define SERV_PORT 8000

struct client_msg
{
    char name[MAX_CLIENT];          //0-20
    char password[MAX_CLIENT];      //20-40
    char mobile_phone[MAX_CLIENT];   //40-60
    char request[MAX_CLIENT];        //60-80
    char id[MAX_CLIENT];              //80-100 
    char conversation[5*MAX_CLIENT];   //100-200
    char state[MAX_CLIENT];             //200-220
    char is_private[MAX_CLIENT];       //220-240 存放是否为私聊
};

struct client_msg online[MAXLINE];
int count = 0;
char init_number[MAX_CLIENT] = "268217";
long init_number_long;
char buf[MAXLINE];
char temp[MAX_CLIENT];  //暂存数组
int chose_flag =0;

int i, maxi, maxfd, listenfd, connfd, sockfd;
int nready, client[FD_SETSIZE];
ssize_t n;
fd_set rset, allset;        //集合，实际为long类型的数组
char str[INET_ADDRSTRLEN];
socklen_t cliaddr_len;
struct sockaddr_in	cliaddr, servaddr;
void watch_user(int fd);
void group_chat(int fd);
struct client_msg t1;

void translate_buf()   //注册表存放用户的信息。0-20放名字 20-40放密码 40-60放手机号 60-80放请求信息
{
    int i=0,n=0,k=0,l=0,g=0,d=0,m=0;

    for(i = 0;i < MAX_CLIENT;i++)          //将名字存在结构体中
    {
        temp[i] = buf[i];
    }
    strcpy(online[count].name,temp);

    memset(temp,0,MAX_CLIENT);
    for(i = MAX_CLIENT;i < 2*MAX_CLIENT;i++)       //将密码存在结构体中
    {
        temp[n] = buf[i];
        n++;
    }
    strcpy(online[count].password,temp);

    memset(temp,0,MAX_CLIENT);
    for(i = 2*MAX_CLIENT;i < 3*MAX_CLIENT;i++)     //将手机号存在结构体中
    {
        temp[k] = buf[i];
        k++;
    }
    strcpy(online[count].mobile_phone,temp);

    memset(temp,0,MAX_CLIENT);
    for(i = 3*MAX_CLIENT;i < 4*MAX_CLIENT;i++)     //将请求信息存在结构体中
    { 
        temp[l] = buf[i];
        l++;
    }
    strcpy(online[count].request,temp);

    char temp1[5*MAX_CLIENT];
    memset(temp1,0,5*MAX_CLIENT);                     //将聊天内容存在结构体中
    for(i = 5*MAX_CLIENT;i < 10*MAX_CLIENT;i++)
    {
        temp1[g] = buf[i];
        g++;
    }
    strcpy(online[count].conversation,temp1);

    memset(temp,0,MAX_CLIENT);
    for(i = 10*MAX_CLIENT;i < 11*MAX_CLIENT;i++)      //将标志位存在结构体中
    {
        temp[d] = buf[i];
        d++;
    } 
    strcpy(online[count].state,temp);

    memset(temp,0,MAX_CLIENT);
    for(i = 11*MAX_CLIENT;i < 12*MAX_CLIENT;i++)         //存放是否为私聊
    {
        temp[m] = buf[i];
        m++;
    }
    strcpy(online[count].is_private,temp);
}

void register_table(int fd)  //注册
{
    strcpy(online[count].id,init_number);       //将账号存在结构体中

    //将产生的账号传给客户端
    init_number_long = atoi(init_number);
    Write(fd,init_number,sizeof(init_number));

    init_number_long++;
    memset(init_number, 0, sizeof(init_number));
    sprintf(init_number, "%ld", init_number_long); 

    count++;
}

//请求登录
void ask_login(int fd)
{
    int i,l = 0;
    memset(temp,0,MAX_CLIENT);
    for(i = 4*MAX_CLIENT;i < 5*MAX_CLIENT;i++)     //将ID信息存在结构体中
    {
        temp[l] = buf[i];
        l++;
    }
    strcpy(online[count].id,temp);

    for(i = 0;i <= count;i++)
    {
        if(count == 0)
        {
            memset(buf,0,sizeof(buf));
            memset(online[count].state,0,MAX_CLIENT);
            strcpy(online[count].state,"not");
            memcpy(buf,&online[count],sizeof(struct client_msg));
            Write(fd,buf,sizeof(buf));
            memset(&online[count],0,sizeof(online[count]));//将该账号清除
            break;
        }
        else if(strcmp(online[count].id , online[count-1].id) > 0)  //如果账号不存在
        {
            memset(buf,0,sizeof(buf));
            memset(online[count].state,0,MAX_CLIENT);
            strcpy(online[count].state,"not");
            memcpy(buf,&online[count],sizeof(struct client_msg));
            Write(fd,buf,sizeof(struct client_msg));
            memset(&online[count],0,sizeof(online[count]));//将该账号清除
            break;
        }
        else if(strcmp(online[count].id,online[i].id) == 0)
        {
            if(strcmp(online[count].password,online[i].password) == 0) //如果账号存在，密码正确
            {
                memset(buf,0,sizeof(buf));
                memset(online[i].state,0,MAX_CLIENT);
                strcpy(online[i].state,"yes");
                memset(buf,0,MAXLINE);
                memset(online[i].request,0,MAX_CLIENT);
                strcpy(online[i].request,"login");
                memcpy(buf,&online[i],sizeof(struct client_msg));
                Write(fd,buf,sizeof(struct client_msg));
                memset(&online[count],0,sizeof(online[count]));//将该账号清除
                break;
            }
            else    //如果账号存在，密码不正确
            {
                memset(buf,0,sizeof(buf));
                memset(online[i].state,0,MAX_CLIENT);
                strcpy(online[i].state,"ys");
                memcpy(buf,&online[i],sizeof(struct client_msg));
                Write(fd,buf,sizeof(struct client_msg));
                break;
            }
        }
    }

}

void watch_user(int fd)         //查看在线用户
{
    int i,m = 0,n=0,j=0;
    char temp[5*MAX_CLIENT];
    memset(temp,0,5*MAX_CLIENT);
    for(i = 0;i < count;i++)
    {
        if(strcmp(online[i].request,"login") == 0)
        {
            for(m=0;m<MAX_CLIENT;m++)
            {
                temp[n] = online[i].name[m];
                n++;
            }temp[n++] = '\n';
        }
    }
    for(i=0;i<100;i++)
    {printf("%c",temp[i]);}
    printf("\n");
    Write(fd,temp,sizeof(temp));
}

void group_chat(int fd)      //群聊
{
    int i;
    FILE *fp;
    fp=fopen("./1.txt","a");//打开文件以便写入数
    //fseek(fp,0,SEEK_END);
    fprintf(fp,"%s说 %s\n",online[count].name,online[count].conversation);
//    fseek(fp,0,SEEK_END);
    fclose(fp);
    //群聊
    for(i = 0; i <= maxi; i++)
    {
        fd = client[i];
        Write(fd, buf, sizeof(struct client_msg));  //将结构体群发出去
    }
}

void promise_quit(int fd)   //要求退出
{
    Close(sockfd);
    FD_CLR(sockfd, &allset);
    client[i] = -1;
}

void kick_handle(int fd)      //踢人处理
{
    int i,m;
    printf("%s\n",online[count].conversation);
    for(i = 0;i < count;i++)
    {
        if(strcmp(online[i].name,online[count].conversation) == 0)
        {
            strcpy(online[i].request,"offline");

            memset(buf,0,MAXLINE);
            strcpy(buf,online[i].name);
            group_chat(fd);
        }
    }
}

void ban_handle(int fd)
{
    int i,m;
    printf("%s\n",online[count].conversation);
    for(i = 0;i < count;i++)
    {
        if(strcmp(online[i].name,online[count].conversation) == 0)
        {
            strcpy(online[i].request,"offline");

            memset(buf,0,MAXLINE);
            strcpy(buf,online[i].name);
            group_chat(fd);
        }
    }
}

int private_chat(int fd)
{
    strcpy(online[count].is_private,"is");
    memset(buf,0,MAXLINE);
    memcpy(buf,&online[count],sizeof(struct client_msg));
    int m;
    //printf("%s %d\n",online[count].conversation,count);
    if(chose_flag == 0)
    {
        chose_flag = 1;
        if(count == 1)
        {
            memset(buf,0,sizeof(buf));
            strcpy(buf,"!");
            Write(fd,buf,sizeof(buf));
            return 0;
        }
        else
        {
            int i;
            for(i=0;i<count;i++)
            {
                if(strcmp(online[i].name,online[count].conversation) == 0)
                {
                    m = i;
                    memset(buf,0,sizeof(buf));
                    strcpy(buf,"@");
                    Write(fd,buf,sizeof(buf));
                    return 0;
                }
            }
        }
    }
    else
    {
        group_chat(fd);
        //      sockfd = client[m] ;
        //      Write(sockfd,buf,sizeof(struct client_msg));
    }
}

void choose(int fd)
{
    //  translate_buf();
    int n,j=0,k=0;

    if(strcmp(online[count].request,"regist") == 0)   //如果要求注册
    {
        register_table(fd); //注册，并分配一个账hao
    }
    else if(strcmp(online[count].request,"login") == 0)   //如果要求登录
    {
        ask_login(fd);  
    }
    else if(strcmp(online[count].request,"quit") == 0)  //如果要求退出
    {
        promise_quit(fd);
    }
}

int main(int argc, char **argv)
{

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SERV_PORT);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    Listen(listenfd, 20);

    maxfd = listenfd;		//maxfd为现在监听的文件描述符的最大值 
    maxi = -1;			/* client[]的下标*/ 
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;	/* -1 indicates available entry */
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);     // 设置allset（所有监听文件描述符集合最新状态）
    
    for ( ; ; ) 
    {
        rset = allset;	      //当前文件描述符集合 
        nready = select(maxfd+1, &rset, NULL, NULL, NULL); //select函数修改rset，nready为文件描述符改变的个数
        if (nready < 0)
            perr_exit("select error");

        if (FD_ISSET(listenfd, &rset))  // 判断listenfd是否在rset里，即是否监听到新的客户端
        { 
            cliaddr_len = sizeof(cliaddr);

            connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);

            printf("received from %s at PORT %d\n",
                    inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
                    ntohs(cliaddr.sin_port));

            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (client[i] < 0) 
                {
                    client[i] = connfd;    //将新的文件描述符添加到client里 
                    break;
                }
            }
            if (i == FD_SETSIZE) {
                fputs("too many clients\n", stderr);
                exit(1);		}

                FD_SET(connfd, &allset);     //将新的新的文件描述符添加到监控信号集里  
                if (connfd > maxfd)
                    maxfd = connfd; /* for select */
                if (i > maxi)
                    maxi = i;	/* max index in client[] array */

                if (--nready == 0)
                    continue;	/* no more readable descriptors */
        }

        for (i = 0; i <= maxi; i++) 
        {	 // 检测哪个clients 有数据就绪 
            if ( (sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset))       //对连接到服务器的每个客户机（文件描述符），检测是否有数据到达 
            {
                if ( (n = Read(sockfd, buf, MAXLINE)) == 0) 
                {
                    // 当client关闭链接时，服务器端也关闭对应链接 
                    Close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else 
                {
                    translate_buf();
                    if(strcmp(online[count].state,"watch") == 0)
                    {
                        printf("watch\n");
                        watch_user(sockfd);
                        memset(buf,0,MAXLINE);
                    }
                    else if(strcmp(online[count].state,"chat") == 0)
                    {
                        printf("chat\n");
                        group_chat(sockfd);
                        memset(buf,0,MAXLINE);
                    }
                    else if(strcmp(online[count].state,"kick") == 0)
                    {
                        printf("kick\n");
                        kick_handle(sockfd);
                        memset(buf,0,MAXLINE);
                    }
                    else if(strcmp(online[count].state,"ban") == 0)
                    {
                        printf("ban\n");
                        ban_handle(sockfd);
                        memset(buf,0,MAXLINE);
                    }
                    else if(strcmp(online[count].state,"pri_chat") == 0)
                    {
                        printf("pri_chat\n");
                        private_chat(sockfd);
                        memset(buf,0,MAXLINE);
                    }
                    else
                    {
                        printf("nihao\n");
                        choose(sockfd);
                        memset(buf,0,MAXLINE);
                    }
                }
                if (--nready == 0)
                    break;	//处理的那个fd不是最后一个 
            }
        }
    }
}
