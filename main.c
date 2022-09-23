/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. 115200*/
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 256000 ) 

TaskHandle_t Button_1_Monitor_Handler = NULL;
TaskHandle_t Button_2_Monitor_Handler = NULL;
TaskHandle_t Periodic_Transmitter_Handler = NULL;
TaskHandle_t Uart_Receiver_Handler = NULL;
TaskHandle_t Load_1_Simulation_Handler = NULL;
TaskHandle_t Load_2_Simulation_Handler = NULL;

QueueHandle_t Button_1_Queue = NULL;
QueueHandle_t Button_2_Queue = NULL;
QueueHandle_t Periodic_Transmitter_Queue = NULL;
 

int Button_1_in_time = 0, Button_1_out_time = 0, Button_1_total_time = 0;
int Button_2_in_time = 0, Button_2_out_time = 0, Button_2_total_time = 0;
int Periodic_in_time = 0, Periodic_out_time = 0, Periodic_total_time = 0;
int UART_in_time = 0, UART_out_time = 0, UART_total_time = 0;
int Load_1_in_time = 0, Load_1_out_time = 0, Load_1_total_time = 0;
int Load_2_in_time = 0, Load_2_out_time = 0, Load_2_total_time = 0;

int system_time = 0;
int cpu_load = 0;

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
static void configTimer1(void);
void timer1Reset(void);

void Button_1_Monitor ( void * pvParameters);
void Button_2_Monitor ( void * pvParameters);
void Periodic_Transmitter ( void * pvParameters);
void Uart_Receiver(void * pvParameters);
void Load_1_Simulation( void * pvParameters );
void Load_2_Simulation( void * pvParameters );

/*-----------------------------------------------------------*/


/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	    
	
	/* Create Tasks here */

	 xTaskPeriodicCreate(
                    Button_1_Monitor,       /* Function that implements the task. */
                    "Button 1 Monitor",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Button_1_Monitor_Handler, /* Used to pass out the created task's handle. */
										50);      /*Used to pass task periodicity */	
		
		vTaskSetApplicationTaskTag(Button_1_Monitor_Handler,(void *)1);
		/* Create queue for button 1 state in order to send it to uart task */
		Button_1_Queue = xQueueCreate(1,sizeof(char));		
										
		xTaskPeriodicCreate(
                    Button_2_Monitor,       /* Function that implements the task. */
                    "Button 2 Monitor",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Button_2_Monitor_Handler, /* Used to pass out the created task's handle. */
										50);      /*Used to pass task periodicity */	
								
		vTaskSetApplicationTaskTag(Button_2_Monitor_Handler,(void *)2);
		/* Create queue for button 1 state in order to send it to uart task */
		Button_2_Queue = xQueueCreate(1,sizeof(char));
		
		xTaskPeriodicCreate(
                    Periodic_Transmitter,       /* Function that implements the task. */
                    "Periodic Transmitter",          /* Text name for the task. */
                    200,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Periodic_Transmitter_Handler, /* Used to pass out the created task's handle. */
										100);      /*Used to pass task periodicity */
			
		vTaskSetApplicationTaskTag(Periodic_Transmitter_Handler,(void *)3);
		/* Create queue for button 1 state in order to send it to uart task */										
		Periodic_Transmitter_Queue = xQueueCreate(1,22*sizeof(char));
						

		xTaskPeriodicCreate(
                    Uart_Receiver,       /* Function that implements the task. */
                    "Uart Receiver",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Uart_Receiver_Handler, /* Used to pass out the created task's handle. */
										20);      /*Used to pass task periodicity */
										
		vTaskSetApplicationTaskTag(Uart_Receiver_Handler,(void *)4);
			



		xTaskPeriodicCreate(
                    Load_1_Simulation,       /* Function that implements the task. */
                    "Load 1 Simulation",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Load_1_Simulation_Handler, /* Used to pass out the created task's handle. */
										10);      /*Used to pass task periodicity */
		
		vTaskSetApplicationTaskTag(Load_1_Simulation_Handler,(void *)5);
										
		xTaskPeriodicCreate(
                    Load_2_Simulation,       /* Function that implements the task. */
                    "Load 2 Simulation",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Load_2_Simulation_Handler, /* Used to pass out the created task's handle. */
										100);      /*Used to pass task periodicity */
	
	vTaskSetApplicationTaskTag(Load_2_Simulation_Handler,(void *)6);
										
										


	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/*
	 *Configure timer 1 in order to use it in run time analysis, timer tick configured to represent 1 micro second
	 * Read timer value from T1TC
	 */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t xTask,char * pcTaskName )
{	
	int i;
	for( ;;)
	{
		i = i;
	}	
}

void vApplicationTickHook(void)
{
	GPIO_write(PORT_0,PIN2,PIN_IS_HIGH);
	GPIO_write(PORT_0,PIN2,PIN_IS_LOW);
}

void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

static void configTimer1(void)
{
	/* In order to be able to count time of small tasks that take few micro seconds
	 * so tick time is prescaled to be 1 micro second	
	 */
	T1PR = 59;
	T1TCR |= 0x1;
	
}

void Button_1_Monitor ( void * pvParameters)
{
		TickType_t xLastWakeTime;
		const TickType_t xFrequency = 50;
		pinState_t prevPinState = GPIO_read(PORT_0,PIN0);
		pinState_t currPinState;
		volatile char button_1;
		xLastWakeTime = xTaskGetTickCount();
		
		for( ;; )
	  {
			currPinState = GPIO_read(PORT_0,PIN0);
			if(currPinState != prevPinState)
			{
				if(currPinState == PIN_IS_HIGH)
				{
					button_1 = '+';
				}
				else
				{
					button_1 = '-';
				}
				(void)xQueueSend(Button_1_Queue,( void * ) &button_1,( TickType_t )0);
			}
			prevPinState = currPinState;
			vTaskDelayUntil( &xLastWakeTime, xFrequency );		
		}
}

void Button_2_Monitor ( void * pvParameters)
{
		TickType_t xLastWakeTime;
		const TickType_t xFrequency = 50;
		pinState_t prevPinState = GPIO_read(PORT_0,PIN1);
		pinState_t currPinState;
	  volatile char button_2;
		xLastWakeTime = xTaskGetTickCount();
	  
		for( ;; )
	  {
			currPinState = GPIO_read(PORT_0,PIN1);
			if(currPinState != prevPinState)
			{
				if(currPinState == PIN_IS_HIGH)
				{
					button_2 = '+';
				}
				else
				{
					button_2 = '-';
				}
				(void)xQueueSend(Button_2_Queue,( void * ) &button_2,( TickType_t )0);
			}
			prevPinState = currPinState;
			vTaskDelayUntil( &xLastWakeTime, xFrequency );		
		}
}



void Periodic_Transmitter (void * pvParameters)
{
		TickType_t xLastWakeTime;
		const TickType_t xFrequency = 100;
		volatile char Periodic_message[20]="100ms has passed\n";
		xLastWakeTime = xTaskGetTickCount();
	
		for( ;; )
		{
			(void)xQueueSend(Periodic_Transmitter_Queue,( void * ) &Periodic_message,( TickType_t )0);
			vTaskDelayUntil( &xLastWakeTime, xFrequency );
		}
		
}

void Uart_Receiver(void * pvParameters)
{
		TickType_t xLastWakeTime;
		int i =0;
		const TickType_t xFrequency = 20;
		char button_1,button_2; 
		char Periodic_message[20];
	  char button_1_message[] = "B1: ";
	  char button_2_message[] = "B2: ";
		xLastWakeTime = xTaskGetTickCount();
		
		for( ;; )
		{
			if(xQueueReceive( Button_1_Queue,&( button_1 ),( TickType_t ) 0 ) == pdPASS )
			{
				for(i =0; i<strlen(button_1_message);i++)
				{
					xSerialPutChar(button_1_message[i]);
				}
				xSerialPutChar(button_1);
				xSerialPutChar('\n');
				(void)xQueueReset(Button_1_Queue);
			}
			
			if(xQueueReceive( Button_2_Queue,&( button_2 ),( TickType_t ) 0 ) == pdPASS )
			{
				for(i =0; i<strlen(button_2_message);i++)
				{
					xSerialPutChar(button_2_message[i]);
				}
				xSerialPutChar(button_2);
				xSerialPutChar('\n');
				(void)xQueueReset(Button_2_Queue);
			}
			
			if(xQueueReceive( Periodic_Transmitter_Queue,&( Periodic_message ),( TickType_t ) 0 ) == pdPASS )
			{
				vSerialPutString((signed char *)Periodic_message,strlen(Periodic_message));
			  (void)xQueueReset(Periodic_Transmitter_Queue);
			}
			
			vTaskDelayUntil(&xLastWakeTime, xFrequency );
		}
	
}

void Load_1_Simulation( void * pvParameters )
{
	  int i;
		TickType_t xLastWakeTime;
		const TickType_t xFrequency = 10;
		xLastWakeTime = xTaskGetTickCount();
		
    for( ;; )
    {
				
				for(i = 0;i<33100;i++)
				{
					  i = i;
				}     	
				vTaskDelayUntil(&xLastWakeTime, xFrequency );
    }
}


void Load_2_Simulation( void * pvParameters )
{
		int i;
		TickType_t xLastWakeTime;
		const TickType_t xFrequency = 100;
		xLastWakeTime = xTaskGetTickCount();
		
    for( ;; )
    {
				for(i = 0;i<79550;i++)
				{
					 i = i;
				}
				
				vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

