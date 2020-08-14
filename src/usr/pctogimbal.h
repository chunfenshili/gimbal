#ifndef	PCTOGIMBAL_H
#define	PCTOGIMBAL_H

#include "stm32f30x.h"

#ifdef  DEFIN_PCTOGIMBAL_EXTERN
	#define  PCTOGIMBAL_EXTERN
#else 
	#define  PCTOGIMBAL_EXTERN extern 
#endif
/*
IIC_States ����PC����slave2��ʱ�������״̬����slave2��������
�����жϽ�������״̬	
*/

#define PID_SERI_NUMBER   6	
	
PCTOGIMBAL_EXTERN struct
{
	u8 CMD;
	u8 dLen;
	u8 Flag;
	u8 Version;
	u16 motor1pid[3][2][PID_SERI_NUMBER];
	u16 speedpid[3][6];	
	u16 eulerpid[3][6];
    
  u8 ucExpSet[4][3];//���������Ƕ� �����С����  ������
    
    
}Tuning_data;//Ϊ�˼��ݴ������� ����ǰ��������
#define  g_SPidPra Tuning_data


#define slave1paralen  24
#define slave2paralen  24
#define masterparalen  72+24+12

#define motor1dataparalen	masterparalen
#define motor2dataparalen	slave1paralen
#define motor3dataparalen	slave1paralen



#define motor1pidptr    (&Tuning_data.motor1pid[2][0][0])
#define motor2pidptr    (&Tuning_data.motor1pid[1][0][0])
#define motor3pidptr    (&Tuning_data.motor1pid[0][0][0])


#define motor1pidTU     (Tuning_data.motor1pid)
#define motor2pidTU     (Tuning_data.motor1pid)
#define motor3pidTU     (Tuning_data.motor1pid)




/**/

//g_usart2pc_state

#define   USART2PC_TUN_1	1	//��ѯ�ڶ���
#define   USART2PC_TUN_2	2	//��ѯ������
#define   USART2PC_TUN_0	0	//��ѯ���


PCTOGIMBAL_EXTERN  u8 g_usart2pc_state;
PCTOGIMBAL_EXTERN uint8_t 		g_ucUpdatePraFlag;


void set_gimbalpra(uint8_t *data ,uint16_t lenth);
#endif


