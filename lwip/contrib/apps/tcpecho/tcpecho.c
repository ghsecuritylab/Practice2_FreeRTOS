/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "tcpecho.h"

#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"

#include "MK64F12.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "event_groups.h"

#define TCP_PORT		50200
#define MENU_ELEMENTS	5
#define SIZE_LINE_0		28
#define SIZE_LINE_1		28
#define SIZE_LINE_2		28
#define SIZE_LINE_3		28
#define SIZE_LINE_4		28

extern SemaphoreHandle_t g_semaphore_TCP;
extern QueueHandle_t g_data_Menu;
extern EventGroupHandle_t g_events_Menu;
extern SemaphoreHandle_t g_semaphore_returnMenu;

/*-----------------------------------------------------------------------------------*/
static void 
tcp_server(void *arg)
{
	ip_addr_t dst_ip;
	struct netbuf *newbuf;
	uint8_t counterPrint;
    void *data;
    uint8_t flagMenu;
    uint8_t counterReceive;
    u16_t lenNew;

    struct netconn *conn, *newconn;
    err_t err;
    uint32_t optionPressed;
    LWIP_UNUSED_ARG(arg);

	char *menu_Line0 = "***************MENU***************";
	char *menu_Line1 = "Play                    [1]";
	char *menu_Line2 = "Stop                    [2]";
	char *menu_Line3 = "Select                  [3]";
	char *menu_Line4 = "Connection statistics   [4]";

	char *menus[MENU_ELEMENTS] =
	{
			menu_Line0,
			menu_Line1,
			menu_Line2,
			menu_Line3,
			menu_Line4
	};

	uint8_t sizes[MENU_ELEMENTS] =
	{
			SIZE_LINE_0,
			SIZE_LINE_1,
			SIZE_LINE_2,
			SIZE_LINE_3,
			SIZE_LINE_4
	};
	IP4_ADDR(&dst_ip, 192, 168, 1, 64);

    /* Create a new connection identifier. */
    conn = netconn_new(NETCONN_TCP);

    netconn_bind(conn, IP_ADDR_ANY, TCP_PORT);
    /* Tell connection to go into listening mode. */
    netconn_listen(conn);
    while (1)
    {
    	xSemaphoreTake(g_semaphore_TCP, portMAX_DELAY);
        flagMenu = pdFALSE;
        counterReceive = 0;

    	/* Grab new connection. */
    	err = netconn_accept(conn, &newconn);
    	/* Process the new connection. */
    	if (err == ERR_OK)
    	{
    		struct netbuf *buf;
    		uint32_t *packet;
    		u16_t len;

    		while ((err = netconn_recv(newconn, &buf)) == ERR_OK)
    		{
    			do
    			{
    				netbuf_data(buf, (void**)&packet, &len);

    			} while (netbuf_next(buf) >= 0);
    			optionPressed = *packet;
    			optionPressed &= 0xF;

    			netbuf_delete(buf);
    			if(pdFALSE == flagMenu)
    			{
        			newbuf = netbuf_new();
    	    		for(counterPrint = 0; counterPrint < MENU_ELEMENTS; counterPrint++)
    	    		{
    	    			netbuf_ref(newbuf, *(&menus[counterPrint]), sizes[counterPrint]);
    	    		    netbuf_data(newbuf, &data, &lenNew);
    	    			netconn_write(newconn, data, lenNew, NETCONN_COPY);
    	    			vTaskDelay(500);
    	    		}
        			netbuf_delete(newbuf);
        			flagMenu = pdTRUE;
    			}
    			counterReceive++;
    			if(2 == counterReceive)
    			{
    				//break;
    				xEventGroupSetBits(g_events_Menu, 1<<(optionPressed-1));
    				//xSemaphoreTake(g_semaphore_returnMenu, portMAX_DELAY);
    			}
    		}
    		/* Close connection and discard connection identifier. */
    		//netconn_close(newconn);
    		//netconn_delete(newconn);

    	}
    }
}
/*-----------------------------------------------------------------------------------*/
#if 0
static void
tcp_client(void *arg)
{
	ip_addr_t dst_ip;
	struct netconn *conn;
	struct netbuf *buf;
	uint8_t counterPrint;
    void *data;
    u16_t len;

	LWIP_UNUSED_ARG(arg);
	conn = netconn_new(NETCONN_TCP);

	char *menu_Line0 = "***************MENU***************";
	char *menu_Line1 = "Play                    [1]";
	char *menu_Line2 = "Stop                    [2]";
	char *menu_Line3 = "Select                  [3]";
	char *menu_Line4 = "Connection statistics   [4]";

	char *menus[MENU_ELEMENTS] =
	{
			menu_Line0,
			menu_Line1,
			menu_Line2,
			menu_Line3,
			menu_Line4
	};

	uint8_t sizes[MENU_ELEMENTS] =
	{
			SIZE_LINE_0,
			SIZE_LINE_1,
			SIZE_LINE_2,
			SIZE_LINE_3,
			SIZE_LINE_4
	};

	buf = netbuf_new();
	IP4_ADDR(&dst_ip, 192, 168, 1, 69);
	netconn_connect(conn, &dst_ip, TCP_PORT);
	while(1)
	{
		for(counterPrint = 0; counterPrint < MENU_ELEMENTS; counterPrint++)
		{
			netbuf_ref(buf, *(&menus[counterPrint]), sizes[counterPrint]);
		    netbuf_data(buf, &data, &len);
			netconn_write_partly(conn, data, len, NETCONN_COPY, NULL);
			vTaskDelay(1000);
		}
		xSemaphoreGive(g_semaphore_MenuPressed);
	}
}
#endif
/*-----------------------------------------------------------------------------------*/
void
tcpecho_init(void)
{
	xTaskCreate(tcp_server, "ServerTCP", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);
#if 0
	xTaskCreate(tcp_client, "ClientTCP", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);
#endif
}
/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */
