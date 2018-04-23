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

#include "udpecho.h"

#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/api.h"
#include "lwip/sys.h"

#include "MK64F12.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "genericFunctions.h"
#include "fsl_pit.h"

#define QUEUE_ELEMENTS	1
#define N_ELEMENTS		2
#define UDP_PORT		50500

SemaphoreHandle_t g_semaphore_UDP;
SemaphoreHandle_t g_semaphore_Buffer;
SemaphoreHandle_t g_semaphore_NewPORT;

QueueHandle_t g_data_Buffer;

typedef struct
{
	uint16_t dataLow;
	uint16_t dataHigh;
}dataBuffer_t;

/*-----------------------------------------------------------------------------------*/
static void
buffers_Audio(void *arg)
{
	const uint32_t frequencyPIT = 4800;
	const uint32_t f_tx_Python = 100;
	const uint32_t sizeBuffer = ((frequencyPIT)/(f_tx_Python));

	uint16_t bufferA[sizeBuffer][N_ELEMENTS];
	uint16_t bufferB[sizeBuffer][N_ELEMENTS];
	dataBuffer_t data_Queue;
	uint8_t flagPingPong = pdFALSE;
	static uint32_t counterBlock = 0;

	pitInit();
	dacInit();
	/**Fix the float*/
	pitSetPeriod(0.0002);
	while(1)
	{
		xSemaphoreTake(g_semaphore_Buffer, portMAX_DELAY);
		xQueueReceive(g_data_Buffer, &data_Queue, portMAX_DELAY);

		if(sizeBuffer > counterBlock)
		{
			switch(flagPingPong)
			{
			case pdFALSE:
				bufferA[counterBlock][0] = data_Queue.dataLow;
				bufferA[counterBlock][1] = data_Queue.dataHigh;

				PRINTF("%d\r\n", bufferB[counterBlock][0]);
				PRINTF("%d\r\n", bufferB[counterBlock][1]);
				PRINTF("\r\n");

				dacSetValue((uint8_t)bufferB[counterBlock][0]);
				dacSetValue((uint8_t)bufferB[counterBlock][1]);

				break;
			case pdTRUE:
				bufferB[counterBlock][0] = data_Queue.dataLow;
				bufferB[counterBlock][1] = data_Queue.dataHigh;

				PRINTF("%d\r\n", bufferA[counterBlock][0]);
				PRINTF("%d\r\n", bufferA[counterBlock][1]);
				PRINTF("\r\n");

				dacSetValue((uint8_t)bufferA[counterBlock][0]);
				dacSetValue((uint8_t)bufferA[counterBlock][1]);
				break;
			default:
				break;
			}
			counterBlock++;
		}
		else if(sizeBuffer == counterBlock)
		{
			counterBlock = 0;
			flagPingPong = !flagPingPong;
		}
		xSemaphoreGive(g_semaphore_UDP);
	}
}
/*-----------------------------------------------------------------------------------*/
static void
server_thread(void *arg)
{
	struct netconn *conn;
	struct netbuf *buf;

	uint32_t *packet;
	uint16_t len;
	uint16_t dataLow = 0;
	uint16_t dataHigh = 0;
	dataBuffer_t data_Queue;

	LWIP_UNUSED_ARG(arg);
	conn = netconn_new(NETCONN_UDP);
	netconn_bind(conn, IP_ADDR_ANY, UDP_PORT);

	while (1)
	{
		xSemaphoreTake(g_semaphore_UDP, portMAX_DELAY);

		pitStartTimer();
		netconn_recv(conn, &buf);
		netbuf_data(buf, (void**)&packet, &len);
		partBlock(&dataHigh, &dataLow, *packet);

		data_Queue.dataLow = dataLow;
		data_Queue.dataHigh = dataHigh;

		if(pdTRUE == PIT_GetStatusFlags(PIT, kPIT_Chnl_0))
		{
			PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
			xQueueSend(g_data_Buffer, &data_Queue, portMAX_DELAY);
			xSemaphoreGive(g_semaphore_Buffer);
		}
		netbuf_delete(buf);
	}
}

/*-----------------------------------------------------------------------------------*/
static void
client_thread(void *arg)
{
	ip_addr_t dst_ip;
	struct netconn *conn;
	struct netbuf *buf;

	LWIP_UNUSED_ARG(arg);
	conn = netconn_new(NETCONN_UDP);

	char *msg = "Hello!";
	buf = netbuf_new();
	netbuf_ref(buf,msg,10);

	IP4_ADDR(&dst_ip, 192, 168, 1, 65);
	while (1)
	{
		netconn_sendto(conn, buf, &dst_ip, UDP_PORT);
		vTaskDelay(1000);
	}
}
/*-----------------------------------------------------------------------------------*/
void
udpecho_init(void)
{
	g_semaphore_UDP = xSemaphoreCreateBinary();
	g_semaphore_Buffer = xSemaphoreCreateBinary();
	g_semaphore_NewPORT = xSemaphoreCreateBinary();
	g_data_Buffer = xQueueCreate(QUEUE_ELEMENTS, sizeof(dataBuffer_t*));

	xTaskCreate(client_thread, "ClientUDP", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);
	xTaskCreate(server_thread, "ServerUDP", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);
	xTaskCreate(buffers_Audio, "Audio", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);

	xSemaphoreGive(g_semaphore_UDP);
	vTaskStartScheduler();
}

#endif /* LWIP_NETCONN */
