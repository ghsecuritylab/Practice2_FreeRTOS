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
#include "clock_config.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "event_groups.h"
#include "genericFunctions.h"
#include "fsl_pit.h"
#include "queue.h"

#define QUEUE_ELEMENTS	1
#define N_ELEMENTS		2
#define UDP_PORT		50500
#define LENGTH_ARRAY_UDP 200
#define TICKS_SECONDS	(1000)

#define EVENT_PLAY		(1<<0)
#define EVENT_STOP		(1<<1)
#define EVENT_SELECT	(1<<2)
#define EVENT_CONN		(1<<3)

SemaphoreHandle_t g_semaphore_UDP;
SemaphoreHandle_t g_semaphore_Buffer;
SemaphoreHandle_t g_semaphore_NewPORT;
SemaphoreHandle_t g_semaphore_TCP;
SemaphoreHandle_t g_semaphore_returnMenu;
SemaphoreHandle_t g_semaphore_Trans;
EventGroupHandle_t g_events_Menu;

QueueHandle_t g_data_Buffer;
QueueHandle_t g_data_Menu;

typedef struct
{
	int16_t data[LENGTH_ARRAY_UDP];
	uint16_t length;
}dataBuffer_t;

void PIT0_IRQHandler()
{
	setIsrFlag(pdTRUE);
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
}
/*-----------------------------------------------------------------------------------*/
static void
buffers_Audio(void *arg)
{
	const uint32_t sizeBuffer = 200;
	int16_t bufferA[sizeBuffer];
	int16_t bufferB[sizeBuffer];
	dataBuffer_t *data_Queue;
	uint8_t flagStop = pdFALSE;
	uint8_t flagPingPong = pdFALSE;
	uint16_t counter = 0;
	uint8_t flagTimer = pdFALSE;

	pitInit();
	dacInit();
	/**Fix the float*/
	pitSetPeriod(1);
	while(1)
	{
		xSemaphoreTake(g_semaphore_Buffer, portMAX_DELAY);
#if 0
		if(pdFALSE == flagStop)
		{
			xEventGroupWaitBits(g_events_Menu, EVENT_STOP,
				pdTRUE, pdTRUE, portMAX_DELAY);
			flagStop = pdTRUE;
		}
#endif
		xQueueReceive(g_data_Buffer, &data_Queue, portMAX_DELAY);

		if(pdFALSE == flagTimer)
		{
			pitStartTimer();
			flagTimer = pdTRUE;
		}
		while(counter < sizeBuffer)
		{
			switch(flagPingPong)
			{
			case pdFALSE:
				bufferA[counter] = data_Queue->data[counter];
				dacSetValue(bufferB[counter]);
				break;
			case pdTRUE:
				bufferB[counter] = data_Queue->data[counter];
				dacSetValue(bufferA[counter]);
				break;
			default:
				break;
			}
			if(getIsrFlag())
			{
				counter++;
				setIsrFlag(pdFALSE);
			}
		}
		counter = 0;
		flagPingPong = !flagPingPong;
		vPortFree(data_Queue);
#if 0
		if(pdTRUE == flagStop)
		{
			flagStop = pdFALSE;
			xSemaphoreGive(g_semaphore_returnMenu);
		}
#endif
		xSemaphoreGive(g_semaphore_UDP);
	}
}
/*-----------------------------------------------------------------------------------*/
static void
server_thread(void *arg)
{
	struct netconn *conn;
	struct netbuf *buf;

	ip_addr_t dst_ip;
	uint32_t *packet;
	uint16_t len;
	uint8_t blockSent;
	dataBuffer_t *data_Queue;
	uint32_t *directionData;

	LWIP_UNUSED_ARG(arg);
	IP4_ADDR(&dst_ip, 192, 168, 1, 66);
	conn = netconn_new(NETCONN_UDP);
#if 0
	xEventGroupWaitBits(g_events_Menu, EVENT_PLAY,
		pdTRUE, pdTRUE, portMAX_DELAY);
#endif
	while (1)
	{
		xSemaphoreTake(g_semaphore_UDP, portMAX_DELAY);
		netconn_bind(conn, &dst_ip, UDP_PORT);
		netconn_recv(conn, &buf);
		netbuf_data(buf, (void**)&packet, &len);

		data_Queue = pvPortMalloc(sizeof(dataBuffer_t));
		directionData = (uint32_t*)&packet;
		data_Queue->length = (len/2);

		partBlock(data_Queue->data, directionData, len);
		xQueueSend(g_data_Buffer, &data_Queue, portMAX_DELAY);
		netbuf_delete(buf);
		xSemaphoreGive(g_semaphore_Buffer);
	}
}

/*-----------------------------------------------------------------------------------*/
#if 0
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
#endif
/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/


void
udpecho_init(void)
{
	NVIC_EnableIRQ(PIT0_IRQn);
	NVIC_SetPriority(PIT0_IRQn,5);

	g_semaphore_UDP = xSemaphoreCreateBinary();
	g_semaphore_Buffer = xSemaphoreCreateBinary();
	g_semaphore_NewPORT = xSemaphoreCreateBinary();
	g_semaphore_TCP = xSemaphoreCreateBinary();
	g_semaphore_returnMenu = xSemaphoreCreateBinary();
	g_semaphore_Trans = xSemaphoreCreateBinary();

	g_events_Menu = xEventGroupCreate();
	g_data_Buffer = xQueueCreate(QUEUE_ELEMENTS, sizeof(dataBuffer_t*));
	g_data_Menu = xQueueCreate(QUEUE_ELEMENTS, sizeof(uint8_t));

#if 0
	xTaskCreate(client_thread, "ClientUDP", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);
#endif
	xTaskCreate(server_thread, "ServerUDP", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);
	xTaskCreate(buffers_Audio, "Audio", (20*configMINIMAL_STACK_SIZE), NULL, 5, NULL);

	xSemaphoreGive(g_semaphore_UDP);
	vTaskStartScheduler();

}

#endif /* LWIP_NETCONN */
