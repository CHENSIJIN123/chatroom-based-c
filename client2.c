/* client.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "wrap.h"
#include <pthread.h>
#include <errno.h>

#define MAX_CLIENT 20
#define MAXLINE 240
#define SERV_PORT 8000

struct client_msg
{
    char name[MAX_CLIENT];
    char password[MAX_CLIENT];
    char mobile_phone[MAX_CLIENT];
    char request[MAX_CLIENT];
    char id[MAX_CLIENT];
    char conversation[5*MAX_CLIENT];
    char state[MAX_CLIENT]; //存放是踢人/禁言/聊天....
    char is_private[MAX_CLIENT]; //存放是否为私聊
};

struct sockaddr_in servaddr;
pthread_t tid,tid1;
int ret ,ret1;

void *do_thread(void *arg);
void *do_do_thread(void *arg);

char buf[MAXLINE];
int sockfd,n;
int flag;
char all[MAXLINE];
char temp[MAX_CLIENT];
char client_choose[MAX_CLIENT];
char choose[MAX_CLIENT];  //选择 one:私聊 two:群聊
char chose_name[MAX_CLIENT]; //选择私聊的名字
int choose_flag = 0;
int login_succeed_flag=0;
void login_succeed();
void login(struct client_msg p);
struct client_msg t;
struct client_msg t2; //暂时存放群聊客户信息
//注册
void regist(struct client_msg p)
{
    memset(buf, 0,sizeof(buf));

    printf("please input your name(不超过20)\n");
    scanf("%s",p.name);

    printf("please input your password(不超过20,字母/数字/符号)\n");
    scanf("%s",p.password);

    printf("please input your mobile_phone(不超过11)\n");
    scanf("%s",p.mobile_phone);

    strcpy(p.request,"regist");
    strcpy(p.state,"regist");
    memcpy(buf, &p, sizeof(p));               //将结构体转为字符串

    send(sockfd, buf, sizeof(struct client_msg), 0);  //将结构体发给服务器 
    printf("正在注册，请稍等……\n");
    memset(buf, 0, sizeof(buf));
    Read(sockfd, buf, MAXLINE);
    printf("你的账号为: %s\n",buf);
    printf("\n选择是否现在登录(yes/no)\n");
    char n[MAX_CLIENT];

    scanf("%s",n);
    if(strcmp(n,"yes") == 0)
    {
        struct client_msg new;   //进行登录
        login(new);
    }
    else if(strcmp(n,"no") == 0)
    {
        exit(0);
    }
    else           //输入非法字符,处理
    {
        printf("请输入(yes/no)\n");
        memset(n,0,sizeof(n));
        scanf("%s",n);
        if(strcmp(n,"yes") == 0)
        {   struct client_msg new;   //进行登录
            login(new);
        }   
        else if(strcmp(n,"no") == 0)
        {}   
        else
        {
            printf("请输入(yes/no)\n");
            memset(n,0,sizeof(n));
            scanf("%s",n);
        }
    }
}

//登录
void login(struct client_msg p)
{
    char temp[MAX_CLIENT];
    memset(buf, 0, sizeof(buf));
    printf("请输入账号:   ");
    scanf("%s", p.id);
    strcpy(temp,p.id);
    printf("\n");

    printf("请输入密码:   ");
    scanf("%s", p.password);
    printf("\n");

    strcpy(p.request,"login");
    memcpy(buf, &p, sizeof(p));
    send(sockfd, buf, sizeof(struct client_msg), 0);   //将结构体发给服务器
    printf("正在登录，请稍等……\n");
    memset(buf, 0, MAXLINE);
    Read(sockfd, buf, sizeof(struct client_msg));
    memcpy(&p,buf,sizeof(struct client_msg));
    if(strcmp(p.state,"not") == 0)
    {
        printf("您输入的账号不存在，请先注册\n");
        struct client_msg p;      //注册
        regist(p);
    }
    else if(strcmp(p.state,"yes") == 0)
    {
        printf("登录成功!\n");
        memset(buf,0,MAXLINE);
        //Read(sockfd,buf,sizeof(struct client_msg));
        memcpy(&t2,&p,sizeof(struct client_msg));  //暂时存放群聊客户信息
        //login_succeed_flag = 1;       
        login_succeed(p);         //登录成功后选择其他功能
        memcpy(&t,&p,sizeof(t));
    }
    else if(strcmp(p.state,"ys") == 0)
    {
        printf("密码错误，请重新输入\n");
        struct client_msg p;                //重新输入
        login(p);
    }
}

void watch_online(struct client_msg p)    //查看在线用户
{
    int i;
    strcpy(p.state,"watch");
    memset(buf, 0, MAXLINE);
    memcpy(buf,&p,sizeof(struct client_msg));
    Write(sockfd,buf,MAXLINE);
    printf("在线用户为：\n");
    memset(buf, 0, MAXLINE);
    Read(sockfd,buf,MAXLINE);
    for(i = 0;i<MAXLINE;i++)
    {printf("%c",buf[i]);}
}

void chat(struct client_msg p)
{
    printf("欢迎进入聊天！！:)\n");
    printf("请选择 one:私聊 two:群聊\n");
    //char choose[MAX_CLIENT];  //选择 one:私聊 two:群聊
    scanf("%s",choose);
   // memset(t2.state,0,MAX_CLIENT);
    char temp[5*MAX_CLIENT];
   //创建一个线程 
    ret = pthread_create(&tid , NULL ,  do_thread , NULL);  
    if(ret != 0)  
    {  
        fprintf(stderr , "创建线程1失败!\n");  
        exit(1);   
    }

    ret1 = pthread_create(&tid1 , NULL ,  do_do_thread , NULL);  
    if(ret1 != 0)  
    {  
        fprintf(stderr , "创建线程2失败!\n");  
        exit(1) ;   
    }
    while(1)
    {
        if(strcmp(all,"#") == 0)
        {break;}
    }
}

void *do_thread(void *arg)     //接收线程
{  
    while(1)  
    {
        n = Read(sockfd, buf, MAXLINE);
        memcpy(&t, buf, sizeof(struct client_msg));
//        printf("recv:%s\n",t.name);
//        printf("self%s\n",t2.name);
//        printf("input%s\n",chose_name);
        if (n == 0)
           printf("the other side has been closed.\n");
        else
        {
            if(strcmp(buf,"!") == 0)
            {
                printf("您输入的用户不在线\n");
                break;
            }
            if(strcmp(buf,"@") == 0)
            {
                printf("您输入的用户在线\n");
            }
            else
            {
                if(strcmp(all,"#") == 0)
                {break;}
             //   if((strcmp(t.name,chose_name) == 0) || strcmp(t.name,t2.name) == 0) 
             //   {
                  if((strcmp(t.is_private,"is") == 0) || (strcmp(t.name,t2.name) == 0))
                  {
                    printf("< %s >说: %s\n",t.name,t.conversation);
                  }
             //   }
                memset(all,0,sizeof(all));
            }
        }
        //            sleep(1);  
    }  
}  

void *do_do_thread(void *arg)   //接收键盘输入并发送线程
{
    while(1)
    {
        memset(t2.state,0,sizeof(MAX_CLIENT));
        if(strcmp(choose,"two") == 0)
        {
            strcpy(t2.state,"chat");
        }
        else if(strcmp(choose,"one") == 0)
        {
            strcpy(t2.state,"pri_chat");
            if(choose_flag == 0)
            {
                choose_flag = 1;
            strcpy(t2.state,"pri_chat");
            printf("请选择私聊的用户\n");
            scanf("%s",chose_name);
            strcpy(t2.conversation,chose_name);
          
            memset(buf,0,MAXLINE);
                   
            memcpy(buf,&t2,sizeof(struct client_msg));
            Write(sockfd,buf,sizeof(struct client_msg));
            continue;
            }
        }
        memset(temp,0,5*MAX_CLIENT);
        //        fgets(temp,sizeof(temp),stdin);
        scanf("%s",temp);
        if(strlen(temp) > 98)   //如果输入内容过长，则减少内容重新输入
        {
            printf("不支持长内容发送\n");
            continue;
        }
        strcpy(t2.conversation,temp);
        strcpy(all,temp);
        if(strcmp(all,"#") == 0)
        {break;}
        memset(buf,0,MAXLINE);

        memcpy(buf,&t2,sizeof(struct client_msg));
        Write(sockfd,buf,sizeof(struct client_msg));
    }
    //            sleep(1);
}

void ban_speak(struct client_msg p)
{
    watch_online(p);
    printf("请选择要禁言的用户:\t");
    char ban_name[MAX_CLIENT];
    scanf("%s",ban_name);
    memset(t2.conversation,0,5*MAX_CLIENT);
//    memset(t2.state,0,MAX_CLIENT);
    strcpy(t2.conversation,ban_name);
    //strcpy(t2.state,"ban");
    memset(buf,0,MAXLINE);
    memcpy(buf,&t2,sizeof(struct client_msg));
    Write(sockfd,buf,sizeof(struct client_msg));

    memset(buf,0,MAXLINE);
    Read(sockfd,buf,sizeof(MAXLINE));
    if(strcmp(buf,t2.name) == 0)
    {
        printf("抱歉，您已被禁言！\n");
        exit(0);
    }
    else
    {
        printf("%s 已被禁言\n",buf);
    }
}

void kick(struct client_msg p)
{
    int i;
    watch_online(p);
    printf("请选择要踢出的用户:\t");
    char kick_name[MAX_CLIENT];
    scanf("%s",kick_name);
    memset(t2.conversation,0,5*MAX_CLIENT);
    memset(t2.state,0,MAX_CLIENT);
    strcpy(t2.conversation,kick_name);
    //strcpy(t2.state,"kick");
    memset(buf,0,MAXLINE);
    memcpy(buf,&t2,sizeof(struct client_msg));
    Write(sockfd,buf,sizeof(struct client_msg));
    //printf("%s\n",buf);
    memset(buf,0,MAXLINE);
    Read(sockfd,buf,sizeof(MAXLINE));
    if(strcmp(buf,t2.name) == 0)
    {
        printf("抱歉，您已被移出聊天室！\n");
        exit(0);
    }
    else
    {
        printf("%s 已被踢出聊天室\n",buf);
    }
}

void watch_record(struct client_msg p)
{
    char c;
    FILE *fp;
    fp=fopen("./1.txt","r");
    while((c= fgetc(fp)) != EOF)
    {
        printf("%c",c);
    }
    fclose(fp);
}

void login_succeed(struct client_msg p)
{
    int choose;
    while(1)
    {
        printf("------------------------------------------------------------------\n");
        printf("1.踢人  2.禁言  3.查看在线用户  4.聊天  5.退出聊天  6.查看聊天记录\n"); 
        printf("------------------------------------------------------------------\n");
        memset(t2.state,0,MAX_CLIENT);
        scanf("%d",&choose);
        switch(choose)
        {
            case 1:{
                       strcpy(t2.state,"kick");
                       kick(p);             //1.踢人
                       break;
                   }
            case 2:{
                       strcpy(t2.state,"ban");
                       ban_speak(p);        //2.禁言
                       break;
                   }
            case 3:{
                       strcpy(p.state,"watch");
                       watch_online(p);     //3.查看在线用户
                       break;
                   }
            case 4:{
                       strcpy(t2.state,"chat");
                       strcpy(all,"0");
                       chat(p);              //4聊天
                       break;
                   }
            case 5:exit(0);                  //5.退出聊天
            case 6:{
                    //   strcpy(t2.state,"record");
                    //   strcpy(all,"0");
                       watch_record(p);
                       break;                //6.查看聊天记录
                   }
        }
    }
}

void back(struct client_msg p)
{
    strcpy(p.request,"quit");
    memcpy(buf, &p, sizeof(p));
    send(sockfd, buf, sizeof(struct client_msg), 0);   //将结构体发给服务器
    Close(sockfd);
    printf("离开聊天室！\n");
}

//界面显示
int login_show()
{
    printf("----------------------------------\n");  
    printf("        欢迎进入聊天室            \n");  
    printf("        1.登录                    \n");  
    printf("        2.注册                    \n");  
    printf("        3.退出                    \n");  
    printf("----------------------------------\n");  
    printf("please choose  one                \n");
    int m ;
    scanf("%d",&m);
    return m;
}

int main(int argc,char *argv[])
{
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(SERV_PORT);

    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("正在连接，请稍等……\n");  

    //register  and  login
    flag = login_show();
    switch(flag)
    {
        case 1:{  
                   struct client_msg p;
                   login(p);                    //登录
                   break;
               }
        case 2:{    
                   struct client_msg p;       //注册
                   regist(p);
                   break;
               }
        case 3:{
                   struct client_msg p;
                   back(p);
                   break;                     //退出
               }
    }

    //当线程结束时自动
    ret = pthread_detach(tid); 
    ret1 =pthread_detach(tid1);

    Close(sockfd);
}
