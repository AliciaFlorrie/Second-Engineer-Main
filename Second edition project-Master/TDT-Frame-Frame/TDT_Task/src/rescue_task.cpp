/************
@brief	��Ԯ����
@descirbe	1��ͬ��һ�泵����û������
			2��
@function 
************/



#include "rescue_task.h"

/**FreeRTOS*START***************/
#include "FreeRTOS.h"					//FreeRTOSʹ��	 
#include "timers.h"
#include "list.h"
#include "queue.h"
#include "task.h"
/**FreeRTOS*END***************/
#include "motor.h"
#include "dbus_task.h"
#include "led.h"
#include "can.h"
#include "can_calculate.h"
#include "switch.h"

u8 Travel_Switch_STA = 0;
u8 Travel_Switch_STA_Compulsory = 0;
_AirCmd AirCmd;
#define travel_on PCin(13)

int Flag_Help_Start = 3;
int travel_on_last = 0;
float error_help,error_unhelp;
int flaghelp = 0;
int flagunhelp = 0;
int up_help,down_help;
int rescue_flag = 0;
int unrescue_flag = 0;

Motor Relive(M2006,CAN1,0x205);
float Air_Cylinder[4];
void AirSend()
{
	Air_Cylinder[0]=(int16_t)(AirCmd.reliveAir<<5);
	Air_Cylinder[1]=(int16_t)(AirCmd.knifeAir<<4);
	Air_Cylinder[2]=1;
	Air_Cylinder[3]=1;
	
	canTx(Air_Cylinder,CAN1,0x112);	
	

}
void Travel_switch_init()
{
	Switch Travel = Switch(RCC_AHB1Periph_GPIOC,GPIOC,GPIO_Pin_13);
	Travel.init();	
}
void rescue()		//��������,Ҳ������ǿ�ƾ�Ԯ
{
	if(Travel_Switch_STA_Compulsory == 1 )
	{
		AirCmd.reliveAir = 1;	
	}
	if(AirCmd.reliveAir == 1)
	{
		unrescue_flag = 0;
		rescue_flag++;
		Relive.ctrlSpeed(Relive.getMotorSpeedLimit()/3.0,0);
		if(abs(Relive.pidInner.error) >= 2000 && rescue_flag > 30)
		{
			Relive.ctrlSpeed(0,0);							
		}
	
	}
	
	if(AirCmd.reliveAir == 0 )
	{
		if(abs(Relive.canInfo.totalRound) < 4)		//��ʼλ��Ϊ0ʱ--����
		{
			Relive.ctrlSpeed(0,0);
		}
		
		else						//��ʼλ�ò�Ϊ0ʱ--��
		{
			rescue_flag = 0;
			unrescue_flag++;
			Relive.ctrlSpeed(-Relive.getMotorSpeedLimit()/3.0,0);			
			if(abs(Relive.pidInner.error) >= 2000 && unrescue_flag > 30)
			{
				Relive.ctrlSpeed(0,0);				
			}
		
		}
	}
	
	
	
	
	

}

void Help_without_holding()
{
	/*�����־λ����*/
	if( Travel_Switch_STA == 1)
	{
		Flag_Help_Start = 1;
	}
	else if( Travel_Switch_STA == 0)
	{
		Flag_Help_Start = 0;
	}
	if(Flag_Help_Start == 1)
	{
		if(travel_on == 1 && travel_on_last == 0)	//ײһ���г̿���
		{
			AirCmd.reliveAir = 1;			//��Ԯצ�ӷ���
			flagunhelp = 0;					//��Ԯ��ȫ�����
			flaghelp++;
			Relive.ctrlSpeed(Relive.getMotorSpeedLimit()/3.0,0);
			error_help = Relive.pidInner.error;
			if(abs(error_help) >= 3000 && flaghelp > 30)
			{
				Relive.ctrlSpeed(0,0);
				Flag_Help_Start = 0;				
			}
			
			
		}
		
	
		
	}
	if(Flag_Help_Start == 0)
	{
		if(AirCmd.reliveAir == 0)
		{
			flaghelp = 0;
			flagunhelp++;
			Relive.ctrlSpeed(-Relive.getMotorSpeedLimit()/3.0,0);
			error_unhelp = Relive.pidInner.error;
			if(abs(Relive.pidInner.getFbValue()) <= 10 && flagunhelp > 30)
			{
				Relive.ctrlSpeed(0,0);
				Flag_Help_Start = 0;				
			}
		}
	}
	
	
	
}






void Rescue_Task(void *pvParameters)
{
	
	PidParam inRL[2],outRL[2];
	Travel_switch_init();
	Relive.pidInner.setPlanNum(2);
	Relive.pidOuter.setPlanNum(2);
	
	inRL[0].kp = 1;
	inRL[0].ki = 0;
	inRL[0].kd = 0;
	inRL[0].resultMax = Relive.getMotorCurrentLimit();
	
	outRL[0].kp = 0;
	outRL[0].ki = 0;
	outRL[0].kd = 0;
	outRL[0].resultMax = Relive.getMotorSpeedLimit();
	
	Relive.pidInner.paramPtr = inRL;
	Relive.pidOuter.paramPtr = outRL;
	
	Relive.pidInner.fbValuePtr[0] = &Relive.canInfo.speed;
	Relive.pidOuter.fbValuePtr[0] = &Relive.canInfo.totalEncoder;
	while(1)
	{
		if(RC.KeyPress.Q == 1 && RC.Key.CTRL == 0)
		{
			Travel_Switch_STA = !Travel_Switch_STA;
			
			if(Travel_Switch_STA == 0)
			{	
				AirCmd.reliveAir = 0;
			}
			
			Travel_Switch_STA_Compulsory = 0;
		}
		
		
		if(RC.KeyPress.Q == 1 && RC.Key.CTRL == 1)
		{
			Travel_Switch_STA_Compulsory= !Travel_Switch_STA_Compulsory;
			if(Travel_Switch_STA_Compulsory == 0)
			{	
				AirCmd.reliveAir = 0;
			}			
			Travel_Switch_STA = 0;
		}
			
		Help_without_holding();
		rescue();
		AirSend();
		
		travel_on_last = travel_on;
		vTaskDelay(pdMS_TO_TICKS(5));
	
	}
}