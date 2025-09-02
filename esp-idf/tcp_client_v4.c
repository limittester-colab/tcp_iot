#if 1
/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "sdkconfig.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>            // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"
#if defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
#include "addr_from_stdin.h"
#endif

#if defined(CONFIG_EXAMPLE_IPV4)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
#define HOST_IP_ADDR ""
#endif

#define PORT CONFIG_EXAMPLE_PORT
typedef unsigned char UCHAR;
static const char *TAG = "example";
static const char *payload = "Message from ESP32 ";
static const char *client_name = "wifi client\0";

static UCHAR pre_preamble[] = {0xF8,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00};
static void read_sock_task(void *pvParameters);
static int send_msg(int sd, int msg_len, UCHAR *msg, UCHAR msg_type, UCHAR dest);
static int get_msg(int sd);
#endif
void tcp_client(void)
{
	char host_ip[] = HOST_IP_ADDR;
	int addr_family = 0;
	int ip_protocol = 0;
	UCHAR tempx[30];
	int msg_len;
	int ret;

    while (1) 
	{
#if defined(CONFIG_EXAMPLE_IPV4)
		struct sockaddr_in dest_addr;
		inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(PORT);
		addr_family = AF_INET;
		ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
		struct sockaddr_storage dest_addr = { 0 };
		ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_STREAM, &ip_protocol, &addr_family, &dest_addr));
#endif

		int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
		if (sock < 0) 
		{
			ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
			break;
		}
		ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

		int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
		if (err != 0) 
		{
			ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
			break;
		}
		ESP_LOGI(TAG, "Successfully connected");

		xTaskCreate(read_sock_task, "read sock", 4096, (void *)sock, 5, NULL);

		strcpy((char *)&tempx[0],client_name);
		msg_len = strlen((const char *)tempx);

		send_msg(sock, msg_len, tempx, 72, 5);		// 72 is SET_CLIENT_NAME

		strcpy((char *)tempx,"testing IOT\0");

		while(1)
		{
			ret = send_msg(sock, msg_len, tempx, 73, 5);
			if(ret < 0)
			{
				ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
				vTaskDelay(5000);
			}else vTaskDelay(1000);
		}
/*
        while (1) 
		{
            int err = send(sock, payload, strlen(payload), 0);
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }

            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
            }
        }
*/
		if (sock != -1) 
		{
			ESP_LOGE(TAG, "Shutting down socket and restarting...");
			shutdown(sock, 0);
			close(sock);
		}
	}
}
/****************************************************************************************************/
static void read_sock_task(void *pvParameters)
{
	UCHAR tempx[100];
	int msg_len;
	int ret;
	UCHAR cmd;

	int sock = (int)pvParameters;
	ESP_LOGI(TAG, "sock: %d",sock);
	if(sock > 100)
	{
		ESP_LOGE(TAG, "error in read_sock_task - bad sock: %d",sock);
		vTaskDelete(NULL);
	}
	for(;;)
	{
		msg_len = get_msg(sock);
		if(msg_len < 0)
		{
			ESP_LOGE(TAG, "error in read_sock_task - msg_len: %d",msg_len);
			vTaskDelete(NULL);
		}
		ret = recv(sock, (char *)&tempx[0], msg_len+1, MSG_WAITALL);
		if(ret < 0)
		{
			ESP_LOGE(TAG, "error in read_sock_task  2 - ret: %d",ret);
			vTaskDelete(NULL);
		}
		cmd = tempx[0];
		ESP_LOGI(TAG, "Received %d bytes; cmd: %d", msg_len,cmd);
		memmove(tempx,tempx+1,msg_len);
		tempx[msg_len] = 0;
		ESP_LOGI(TAG, "tempx: %s", tempx);
		if(cmd == 58)		// SEND_STATUS
		{
			strcpy((char *)tempx,client_name);
			msg_len = strlen((char *)tempx);
			send_msg(sock, msg_len, tempx, 62, 5);		// send UPDATE_STATUS to server
		}
		// later we do something with cmd & the msg in tempx 
	}
//    ESP_LOGE(TAG, "%s with netif desc:%s Failed! exiting", __func__, netif_desc);
	vTaskDelete(NULL);
}

/****************************************************************************************************/
static int send_msg(int sd, int msg_len, UCHAR *msg, UCHAR msg_type, UCHAR dest)
{
	int ret;
	int i;
	UCHAR temp[5];

	ret = send(sd, &pre_preamble[0],8,0);
	if(ret < 0)
		return -1;
	temp[0] = (UCHAR)(msg_len & 0x0F);
	temp[1] = (UCHAR)((msg_len & 0xF0) >> 4);
	temp[2] = msg_type;
	temp[3] = dest;
	ret = send(sd, (UCHAR *)&temp[0],4,0);
	if(ret < 0)
		return -2;

	for(i = 0;i < msg_len;i++)
	{
		ret = send(sd, (UCHAR *)&msg[i],1,0);
		if(ret < 0)
			return -3;
	}
	return 0;
}
/****************************************************************************************************/
// get preamble & msg len from client
// preamble is: {0xF8,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,
// msg_len(lowbyte),msg_len(highbyte),0x00,0x00,0x00,0x00,0x00,0x00}
// returns message length
static int get_msg(int sd)
{
	int len;
	UCHAR low, high;
	int ret;
	int i;

	if(sd < 0)
	{
//		printf("sd: %d\n",sd);
		ESP_LOGE(TAG, "error in get_msg - sock: %d",sd);
		return -2;
	}
	UCHAR preamble[10];
//	ret = recv_tcp(sd, preamble,8,1);
	ret = recv(sd, preamble, 8, MSG_WAITALL);
	if(ret <= 0)
	{
		ESP_LOGE(TAG, "error in get_msg - ret: %d",ret);
		return -3;
	}
	if(memcmp(preamble,pre_preamble,8) != 0)
	{
		ESP_LOGE(TAG,"bad preamble");
//		printf("bad preamble\n");
//		for(i = 0;i < 10;i++)
//			printf("%02x ",preamble[i]);
//		printf("\n");
//		uSleep(1,0);
		return -1;
	}
	ret = recv(sd, &low, 1, MSG_WAITALL);
	ret = recv(sd, &high, 1, MSG_WAITALL);

	//printf("%02x %02x\n",low,high);
	len = 0;
	len = (int)(high);
	len <<= 4;
	len |= (int)low;

	return len;
}
