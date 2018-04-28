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
#include "genericFunctions.h"
#include "fsl_pit.h"

#define QUEUE_ELEMENTS	1
#define N_ELEMENTS		2
#define UDP_PORT		50500

SemaphoreHandle_t g_semaphore_UDP;
SemaphoreHandle_t g_semaphore_Buffer;
SemaphoreHandle_t g_semaphore_NewPORT;
SemaphoreHandle_t g_semaphore_TCP;
SemaphoreHandle_t g_semaphore_MenuPressed;

QueueHandle_t g_data_Buffer;
QueueHandle_t g_data_Menu;

typedef struct
{
	uint16_t dataLow;
	uint16_t dataHigh;
}dataBuffer_t;

void PIT0_IRQHandler()
{
	dataBuffer_t data_QueueReceived;
	dataBuffer_t data_QueueSend;
	BaseType_t xHigherPriorityTaskWoken;
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);

    xHigherPriorityTaskWoken = pdFALSE;
	PIT_StopTimer(PIT, kPIT_Chnl_0);
    xSemaphoreGiveFromISR(g_semaphore_Buffer, &xHigherPriorityTaskWoken);
	xQueueReceiveFromISR(g_data_Buffer, &data_QueueReceived, &xHigherPriorityTaskWoken);
    data_QueueSend.dataHigh = data_QueueReceived.dataHigh;
    data_QueueSend.dataLow = data_QueueReceived.dataLow;
	xQueueSendFromISR(g_data_Buffer, &data_QueueSend, &xHigherPriorityTaskWoken);
	PRINTF("DATA SEND\r\n");

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

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
		PIT_StopTimer(PIT, kPIT_Chnl_0);
		xQueueReceive(g_data_Buffer, &data_Queue, portMAX_DELAY);

		if(sizeBuffer > counterBlock)
		{
			switch(flagPingPong)
			{
			case pdFALSE:
				bufferA[counterBlock][0] = ((data_Queue.dataLow)>>3);
				//bufferA[counterBlock][1] = ((data_Queue.dataHigh+32768)>>3);

				dacSetValue((uint16_t)bufferB[counterBlock][0]);
				//dacSetValue((uint16_t)bufferB[counterBlock][1]);

				break;
			case pdTRUE:
				bufferB[counterBlock][0] = ((data_Queue.dataLow)>>3);
				//bufferB[counterBlock][1] = ((data_Queue.dataHigh+32768)>>3);

				dacSetValue((uint16_t)bufferA[counterBlock][0]);
				//dacSetValue((uint16_t)bufferA[counterBlock][1]);
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
	uint8_t blockSent;
	uint16_t dataLow = 0;
	uint16_t dataHigh = 0;
	dataBuffer_t data_Queue;

	LWIP_UNUSED_ARG(arg);
	conn = netconn_new(NETCONN_UDP);
	netconn_bind(conn, IP_ADDR_ANY, UDP_PORT);

	while (1)
	{	xSemaphoreTake(g_semaphore_UDP, portMAX_DELAY);
		blockSent = pdFALSE;
		netconn_recv(conn, &buf);
		netbuf_data(buf, (void**)&packet, &len);
		partBlock(&dataHigh, &dataLow, *packet);

		data_Queue.dataLow = dataLow;
		data_Queue.dataHigh = dataHigh;

		pitStartTimer();
		if(pdFALSE == blockSent)
		{
			xQueueSend(g_data_Buffer, &data_Queue, portMAX_DELAY);
			blockSent = pdTRUE;
		}
#if 0
		if(pdTRUE == PIT_GetStatusFlags(PIT, kPIT_Chnl_0))
		{
			PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
			xQueueSend(g_data_Buffer, &data_Queue, portMAX_DELAY);
			xSemaphoreGive(g_semaphore_Buffer);
		}
#endif
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
#if 0
	NVIC_EnableIRQ(PIT0_IRQn);
	NVIC_SetPriority(PIT0_IRQn,6);
#endif

	g_semaphore_UDP = xSemaphoreCreateBinary();
	g_semaphore_Buffer = xSemaphoreCreateBinary();
	g_semaphore_NewPORT = xSemaphoreCreateBinary();
	g_semaphore_TCP = xSemaphoreCreateBinary();
	g_semaphore_MenuPressed = xSemaphoreCreateBinary();
	g_data_Buffer = xQueueCreate(QUEUE_ELEMENTS, sizeof(dataBuffer_t*));
	g_data_Menu = xQueueCreate(QUEUE_ELEMENTS, sizeof(uint8_t));

#if 0
	xTaskCreate(client_thread, "ClientUDP", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);
	xTaskCreate(server_thread, "ServerUDP", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);
	xTaskCreate(buffers_Audio, "Audio", (3*configMINIMAL_STACK_SIZE), NULL, 4, NULL);
#endif
	xSemaphoreGive(g_semaphore_TCP);
	vTaskStartScheduler();
}

#endif /* LWIP_NETCONN */
