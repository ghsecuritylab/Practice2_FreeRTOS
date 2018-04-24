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

#define TCP_PORT	50500

extern SemaphoreHandle_t g_semaphore_TCP;

/*-----------------------------------------------------------------------------------*/

static void 
tcp_server(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  /* Create a new connection identifier. */
  /* Bind connection to well known port number 7. */
#if LWIP_IPV6
  conn = netconn_new(NETCONN_TCP_IPV6);
  netconn_bind(conn, IP6_ADDR_ANY, TCP_PORT);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, TCP_PORT);
#endif /* LWIP_IPV6 */
  LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);

  while (1) {

    /* Grab new connection. */
    err = netconn_accept(conn, &newconn);
    /*printf("accepted new connection %p\n", newconn);*/
    /* Process the new connection. */
    if (err == ERR_OK) {
      struct netbuf *buf;
      void *data;
      u16_t len;
      
      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
        PRINTF("Recved\n");
        do {
             netbuf_data(buf, &data, &len);
             err = netconn_write(newconn, data, len, NETCONN_COPY);
#if 0
            if (err != ERR_OK) {
              printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
            }
#endif
        } while (netbuf_next(buf) >= 0);
        netbuf_delete(buf);
      }
      /*printf("Got EOF, looping\n");*/ 
      /* Close connection and discard connection identifier. */
      netconn_close(newconn);
      netconn_delete(newconn);
    }
  }
}
/*-----------------------------------------------------------------------------------*/

static void
tcp_client(void *arg)
{
	ip_addr_t dst_ip;
	struct netconn *conn, *newconn;
	struct netbuf *buf, *new_buff;
	err_t err;
    void *data;
    u16_t len;


	LWIP_UNUSED_ARG(arg);
	conn = netconn_new(NETCONN_TCP);

	char *msg = "Hello!";
	buf = netbuf_new();
	netbuf_ref(buf,msg,10);
	IP4_ADDR(&dst_ip, 192, 168, 1, 69);

	netconn_connect(conn, &dst_ip, TCP_PORT);

	data = &msg;
	len = 4;
	size_t *size;
	size = len;

	xSemaphoreTake(g_semaphore_TCP, portMAX_DELAY);
	while(1)
	{
		err = netconn_write_partly(conn, data, len, NETCONN_COPY, NULL);
		//netconn_write_partly(conn, data, len, NETCONN_COPY, NULL);
		vTaskDelay(1000);
		netconn_recv(conn, &new_buff);
	}

}

#if 0
static void
tcp_client(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  /* Create a new connection identifier. */
  /* Bind connection to well known port number 50500. */

  conn = netconn_new(NETCONN_TCP);
  netconn_connect(conn, IP_ADDR_ANY, 50500);
  /* Tell connection to go into listening mode. */

  while (1)
  {

    /* Grab new connection. */
    err = netconn_accept(conn, &newconn); //ACCEPT
    /*printf("accepted new connection %p\n", newconn);*/
    /* Process the new connection. */
    if (err == ERR_OK) {
      struct netbuf *buf;
      void *data;
      u16_t len;

      //PERFORM OPERATIONS
      while ((err = netconn_send(newconn, buf)) == ERR_OK) {
        /*printf("Recved\n");*/
        do {
             netbuf_data(buf, &data, &len);
             err = netconn_write(newconn, data, len, NETCONN_COPY);

       } while (netbuf_next(buf) >= 0);
      }
        while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
              /*printf("Recved\n");*/
              do {
                   netbuf_data(buf, &data, &len);
                   err = netconn_write(newconn, data, len, NETCONN_COPY);
              } while (netbuf_next(buf) >= 0);


        netbuf_delete(buf);
        }

      //
      /*printf("Got EOF, looping\n");*/
      /* Close connection and discard connection identifier. */
      netconn_close(newconn);
      netconn_delete(newconn);

    }
  }
}
#endif
/*-----------------------------------------------------------------------------------*/
void
tcpecho_init(void)
{
	xTaskCreate(tcp_server, "ServerTCP", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);
	xTaskCreate(tcp_client, "ClientTCP", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);

}
/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */
