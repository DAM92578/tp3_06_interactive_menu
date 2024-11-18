/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @file   : task_menu.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
/* Project includes. */
#include "main.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "display.h"

/********************** macros and definitions *******************************/
#define G_TASK_MEN_CNT_INI			0ul
#define G_TASK_MEN_TICK_CNT_INI		0ul

#define DEL_MEN_XX_MIN				0ul
#define DEL_MEN_XX_MED				50ul
#define DEL_MEN_XX_MAX				500ul

#define MENU_1_MAX      			1ul
#define MENU_2_MAX      			2ul
#define MENU_3_POWER_MAX    		1ul
#define MENU_3_SPEED_MAX    		9ul
#define MENU_3_SPIN_MAX    			1ul
/********************** internal data declaration ****************************/
task_menu_dta_t task_menu_dta = {DEL_MEN_XX_MIN, ST_MAIN_MENU, EV_MEN_MEN_IDLE, false};

task_menu_motor_dta_t task_menu_motor_dta_1={0,0,0,0};

task_menu_motor_dta_t task_menu_motor_dta_2={0,0,0,0};

task_sub_menu_dta_t task_sub_menu_dta = {0,0,0};


#define MENU_DTA_QTY	(sizeof(task_menu_dta)/sizeof(task_menu_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_menu 		= "Task Menu (Interactive Menu)";
const char *p_task_menu_ 		= "Non-Blocking & Update By Time Code";


/********************** external data declaration ****************************/
uint32_t g_task_menu_cnt;
volatile uint32_t g_task_menu_tick_cnt;
task_menu_motor_dta_t   *p_task_menu_motor_dta;

/********************** external functions definition ************************/
void task_menu_init(void *parameters)
{
	task_menu_dta_t   *p_task_menu_dta;
	task_menu_st_t	state;
	task_menu_ev_t	event;
	bool b_event;


	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_menu_init), p_task_menu);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_menu), p_task_menu_);

	g_task_menu_cnt = G_TASK_MEN_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_menu_cnt), g_task_menu_cnt);

	init_queue_event_task_menu();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_menu_dta = & task_menu_dta;


	/* Print out: Task execution FSM */
	state = p_task_menu_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_menu_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_menu_dta->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

    displayInit( DISPLAY_CONNECTION_GPIO_4BITS );


	HAL_GPIO_WritePin(LED_A_PORT, LED_A_PIN, LED_A_ON);

	g_task_menu_tick_cnt = G_TASK_MEN_TICK_CNT_INI;
}

void task_menu_update(void *parameters)
{
	task_menu_dta_t         *p_task_menu_dta;
	task_sub_menu_dta_t     *p_task_sub_menu_dta;



	bool b_time_update_required = false;
    char menu_str[16];


	p_task_sub_menu_dta = &task_sub_menu_dta;

	g_task_menu_cnt++;

	/* Protect shared resource (g_task_menu_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
    {
    	g_task_menu_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_menu_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
		{
			g_task_menu_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task Menu Data Pointer */
		p_task_menu_dta = &task_menu_dta;

    	if (DEL_MEN_XX_MIN < p_task_menu_dta->tick)
		{
			p_task_menu_dta->tick--;
		}
		else
		{

			p_task_menu_dta->tick = DEL_MEN_XX_MAX;

			if (true == any_event_task_menu())
			{
				p_task_menu_dta->flag = true;
				p_task_menu_dta->event = get_event_task_menu();
			}

			switch (p_task_menu_dta->state)
			{
				case ST_MAIN_MENU:




	            	  	  	  	  displayCharPositionWrite(0,0);
	            	  	  	  	  displayStringWrite("Motor 1:");
	            	  	  	      p_task_menu_motor_dta = & task_menu_motor_dta_1;
	            	  	  	  snprintf(menu_str, sizeof(menu_str), "%lu",( p_task_menu_motor_dta->motor_power)/1ul);
	            	  	  	      displayCharPositionWrite(10,0);
	            	  	  	  	  displayStringWrite(menu_str);
	            	  	  	  snprintf(menu_str, sizeof(menu_str), "%lu",( p_task_menu_motor_dta->motor_speed)/1ul);
            	  	  	          displayCharPositionWrite(12,0);
            	  	  	  	      displayStringWrite(menu_str);
            	  		  	  snprintf(menu_str, sizeof(menu_str), "%lu",( p_task_menu_motor_dta->motor_spin)/1ul);
            	  	              displayCharPositionWrite(14,0);
            	  	              displayStringWrite(menu_str);


	            	  	  	  	  displayCharPositionWrite(0, 1);
	            	  	  	  	  displayStringWrite("Motor 2:");
	             	  	  	      p_task_menu_motor_dta = & task_menu_motor_dta_2;
	        	            	  snprintf(menu_str, sizeof(menu_str), "%lu",( p_task_menu_motor_dta->motor_power)/1ul);
	        	            	  displayCharPositionWrite(10,1);
	        	            	  displayStringWrite(menu_str);
	        	            	  snprintf(menu_str, sizeof(menu_str), "%lu",( p_task_menu_motor_dta->motor_speed)/1ul);
	        	            	  displayCharPositionWrite(12,1);
	        	            	  displayStringWrite(menu_str);
	            	  		  	  snprintf(menu_str, sizeof(menu_str), "%lu",( p_task_menu_motor_dta->motor_spin)/1ul);
	            	  	           displayCharPositionWrite(14,1);
	            	  	           displayStringWrite(menu_str);





	            	  	  	  	  if ((true == p_task_menu_dta->flag) && (EV_MEN_MEN_ACTIVE == p_task_menu_dta->event)){
	            	  	  	  		  p_task_menu_dta->flag = false;
	            	  	  	  		  p_task_menu_dta->state = ST_01_MENU;
	            	  	  	  		  displayClear();}
	            	  	  	  	  break;

				case ST_01_MENU:
					 	 	 	 displayCharPositionWrite(0, 0);
					 	 	 	 displayStringWrite("Enter/Next/Esc");

					 	 	 	 if(p_task_sub_menu_dta->sub_menu_1 == MOTOR_ID_1_MEN_1){
					 	 	 		p_task_menu_motor_dta = & task_menu_motor_dta_1;
					 	 	 		p_task_menu_motor_dta->motor_id=MOTOR_ID_1_MEN_1+1;

					 	 	 		 displayCharPositionWrite(0,1);
					 	 	 		 displayStringWrite("Motor");
						 	 		 snprintf(menu_str, sizeof(menu_str), "%lu",( p_task_menu_motor_dta->motor_id)/1ul);
								 	         displayCharPositionWrite(6,1);
								 	 	     displayStringWrite(menu_str);}

					 	 	 	 if(p_task_sub_menu_dta->sub_menu_1== MOTOR_ID_2_MEN_1){
					 	 	 		 p_task_menu_motor_dta =& task_menu_motor_dta_2;
					 	 	 		 p_task_menu_motor_dta->motor_id = MOTOR_ID_2_MEN_1+1;

					 	 	 		 displayCharPositionWrite(0,1);
					 	 	 		 displayStringWrite("Motor ");
						 	 		 snprintf(menu_str, sizeof(menu_str), "%lu", (p_task_menu_motor_dta->motor_id)/1ul);
									 	 	  displayCharPositionWrite(6,1);
									 	 	  displayStringWrite(menu_str);}
					 	 	 	/*-------------------------------------------------------------------------------------------*/
					 	 	 	 if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)){
					 	 	 		p_task_sub_menu_dta->sub_menu_1++;}

					 	 	 	 if (p_task_sub_menu_dta->sub_menu_1> MENU_1_MAX ){
					 	 	 		p_task_sub_menu_dta->sub_menu_1=0;}

								 if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)){
									p_task_menu_dta->flag = false;
									p_task_menu_dta->state = ST_02_MENU;
									displayClear();}

								 if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)){
									p_task_menu_dta->flag = false;
									p_task_menu_dta->state = ST_MAIN_MENU;
									displayClear();}
								 break;

				     case ST_02_MENU:

							     displayCharPositionWrite(0, 0);
								 displayStringWrite("Enter/Next/Esc:");
						 	 		snprintf(menu_str, sizeof(menu_str), "%lu", (p_task_menu_motor_dta->motor_id)/1ul);
								 	 		displayCharPositionWrite(15, 0);
								 	 	    displayStringWrite(menu_str);

								 if(p_task_sub_menu_dta->sub_menu_2 == POWER_MEN_2){
									 displayCharPositionWrite(0,1);
									 displayStringWrite("Power");}


								 if(p_task_sub_menu_dta->sub_menu_2== SPEED_MEN_2){
									 displayCharPositionWrite(0,1);
									 displayStringWrite("Speed");}

								 if(p_task_sub_menu_dta->sub_menu_2== SPIN_MEN_2){
									 displayCharPositionWrite(0,1);
									 displayStringWrite("Spin");}
                             /*-------------------------------------------------------------------------------------------*/
								 if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)){
									p_task_sub_menu_dta->sub_menu_2++;
									displayClear();}

								 if (p_task_sub_menu_dta->sub_menu_2> MENU_2_MAX ){
									p_task_sub_menu_dta->sub_menu_2=0;}

								 if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)){
									p_task_menu_dta->flag = false;
									p_task_menu_dta->state = ST_03_MENU;
									displayClear();}

								 if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)){
									p_task_menu_dta->flag = false;
									p_task_menu_dta->state = ST_01_MENU;
									displayClear();}
								 break;


				     case ST_03_MENU:
					     	 	 displayCharPositionWrite(0, 0);
								 displayStringWrite("Enter/Next/Esc:");
							     displayCharPositionWrite(0, 0);
								 snprintf(menu_str, sizeof(menu_str), "%lu", (p_task_menu_motor_dta->motor_id)/1ul);
								 displayCharPositionWrite(15, 0);
								 displayStringWrite(menu_str);
								 switch (p_task_sub_menu_dta->sub_menu_2)	{

								 	 	 case POWER_MEN_2 :
								 				if (p_task_sub_menu_dta->sub_menu_3 > MENU_3_POWER_MAX){
								 	 		 		 p_task_sub_menu_dta->sub_menu_3=0;}
								 	 		 	p_task_menu_motor_dta->motor_power = p_task_sub_menu_dta->sub_menu_3;

								 				snprintf(menu_str, sizeof(menu_str), "%lu", (p_task_menu_motor_dta->motor_power)/1ul);
								 				        displayCharPositionWrite(0, 1);
								 				        displayStringWrite(menu_str);
								 	 	  break;

								 	 	 case SPEED_MEN_2 :

												  if (p_task_sub_menu_dta->sub_menu_3 > MENU_3_SPEED_MAX){
													 p_task_sub_menu_dta->sub_menu_3=0;}
												  p_task_menu_motor_dta->motor_speed= p_task_sub_menu_dta->sub_menu_3;

								 	 		  snprintf(menu_str, sizeof(menu_str), "%lu", (p_task_menu_motor_dta->motor_speed)/1ul);
												       displayCharPositionWrite(0, 1);
												       displayStringWrite(menu_str);
									     break;
								 	 	 case SPIN_MEN_2 :


								 	 		 	 if (p_task_sub_menu_dta->sub_menu_3 > MENU_3_SPIN_MAX){
								 	 		 		 p_task_sub_menu_dta->sub_menu_3=0;}
								 	 		 	 p_task_menu_motor_dta->motor_spin= p_task_sub_menu_dta->sub_menu_3;

								 	 		 	 snprintf(menu_str, sizeof(menu_str), "%lu", (p_task_menu_motor_dta->motor_spin)/1ul);
								 	 		 	          displayCharPositionWrite(0, 1);
								 	 		 	          displayStringWrite(menu_str);
								 	     break;

					                     default:break;}
					 /*-------------------------------------------------------------------------------------------*/
	 	 	 	 	 	 	 if ((true == p_task_menu_dta->flag) && (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)){
	 	 	 	 	 	 		p_task_sub_menu_dta->sub_menu_3++;
	 	 	 	 	 	 		displayClear();}


							 if ((true == p_task_menu_dta->flag) && (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)){
								p_task_menu_dta->flag = false;
								p_task_menu_dta->state = ST_02_MENU;
								displayClear();}

							  if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)){
								  p_task_menu_dta->flag = false;
								  p_task_menu_dta->state = ST_MAIN_MENU;
								  displayClear();}
							   break;




				default:break;
			}
		}
	}
}

/********************** end of file ******************************************/
