/*
 * @Author: cijliu
 * @Date: 2021-02-11 17:01:24
 * @LastEditTime: 2021-02-23 17:50:10
 */
#ifndef __NET_H__
#define __NET_H__
#include "stdio.h"
#include "string.h"
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "errno.h"
#include <sys/socket.h>
#define TCP_MAX_CLIENT 128
typedef struct {
    char ip[16];
    int port;
} ip_t;
typedef struct {
    int sock;
    int port;
    struct sockaddr_in addr;
    int client[TCP_MAX_CLIENT];
} tcp_t;

typedef struct {
    int sock;
    int port;
    struct sockaddr_in addr;
} udp_t;
int tcp_server_init(tcp_t *tcp, int port);
int tcp_server_wait_client(tcp_t *tcp);
int tcp_server_close_client(tcp_t *tcp, int client);
int tcp_server_send_msg(tcp_t *tcp, int client, unsigned char *data, int len);
int tcp_server_receive_msg(tcp_t *tcp, int client, unsigned char *data, int len);
int tcp_server_deinit(tcp_t *tcp);

int udp_server_init(udp_t *udp, int port);
int udp_server_send_msg(udp_t *udp, const char *ip, const int port, unsigned char *data, int len);
int udp_server_recive_msg(udp_t *udp, ip_t *ip, unsigned char *data, int len);
int udp_server_deinit(udp_t *udp);
#endif