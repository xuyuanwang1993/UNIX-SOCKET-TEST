#include <string>
#include<iostream>
#include "unix_socket_helper.h"
#include <signal.h>
using namespace std;
void unix_socket_test(int argc,char *argv[]);
int main(int argc,char *argv[])
{
    unix_socket_test(argc,argv);
    return 0;
}

void unix_socket_test(int argc,char *argv[])
{
    auto helper_func=[](){
        printf("option with <option>\r\n");
        printf("1-------------------------udp_echo_server\r\n");
        printf("2-------------------------udp_client\r\n");
        printf("3-------------------------tcp_server\r\n");
        printf("4-------------------------tcp_client\r\n");
        exit(0);
    };
    if(argc<2)
    {
        helper_func();
    }
    int opt=stoi(argv[1]);
    if(opt<1||opt>4)
    {
        helper_func();
    }
    const char *const server_domain="/tmp/test_server.domain";
    const char *const udp_client_domain_base="/tmp/test_client";
    string udp_client_domain=string(udp_client_domain_base)+to_string(getpid())+".domain";
    int path_len=108;
    if(opt==1)
    {//udp echo server
        printf("udp echo server!\r\n");
        unix_dgram_socket socket(server_domain);
        socket.build();
        char buf[4096]={0};
        while(1)
        {
            auto len=socket.recv(buf,4096);
            if(len<=path_len)continue;
            string message(buf+path_len,len-path_len);
            string from(buf,path_len);
            printf("recv %s from %s\r\n",message.c_str(),from.c_str());
            socket.send(message.c_str(),message.length(),from);
        }
    }
    else if (opt==2) {
        printf("udp echo client!\r\n");
        unix_dgram_socket socket(udp_client_domain);
        socket.build();
        socket.set_peer_path(server_domain);
        char buf[4096]={0};
        strncpy(buf,udp_client_domain.c_str(),path_len);
        while(1)
        {
            printf("input the message you want to send:");
            cin.getline(buf+path_len,4096-path_len);
            socket.send(buf,4096);
            auto len =socket.recv(buf+path_len,4096-path_len);
            printf("recv from peer with a message:%s\r\n",string(buf+path_len,len).c_str());
        }
    }
    else if (opt==3) {
        signal(SIGCHLD,SIG_IGN);
        printf("tcp echo server!\r\n");
        auto handle_func=[](SOCKET fd){
            char buf[4096];
            unix_stream_socket socket;
            socket.reset(fd);
            while(1)
            {
                auto len=socket.recv(buf,4096);
                if(len<=0)break;
                string message(buf,len);
                printf("recv len %d  : %s\r\n",len,message.c_str());
                if(socket.send(buf,len)<=0)break;
            }
            ::close(fd);
        };
        unix_stream_socket server_socket(server_domain);
        server_socket.build();
        server_socket.listen();
        while(1)
        {
            auto fd=server_socket.aacept();
            auto pid=fork();
            if(pid==0)
            {
                handle_func(fd);
                printf("unix_stream_socket connecttion %d exit!\n",fd);
            }
            else {
                ::close(fd);
            }
        }
    }
    else if (opt==4) {
        printf("tcp echo client!\r\n");
        unix_stream_socket socket;
        socket.build();
        if(!socket.connect(server_domain))
        {
            perror("connect failed!");
            exit(EXIT_FAILURE);
        }
        char buf[4096]={0};
        while(1)
        {
            printf("input the message you want to send:");
            cin.getline(buf,4096);
            string send_str(buf);
            if(socket.send(send_str.c_str(),send_str.length())<=0)break;
            auto len =socket.recv(buf,4096);
            if(len<=0)break;
            printf("recv from peer:%s\r\n",string(buf,len).c_str());
        }
    }
}
