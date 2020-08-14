#ifndef URART_DATA_H
#define URART_DATA_H
#include "stm32f30x.h"

#ifdef  DEFIN_URARTDATA_EXTERN
	#define  URARTDATA_EXTERN
#else 
	#define  URARTDATA_EXTERN extern 
#endif

typedef struct{
    
  int16_t iCurrent[2];
  int16_t SensorAngle[2];
  int16_t i_qd_data[2];
  int16_t FocAngle;
  
}t_MotoDebugData1;

typedef struct {
  uint8_t CMD;
  uint8_t dLen;
  uint8_t Flag;
  uint8_t Flag1;  
  
  t_MotoDebugData1 MotoData[3];

  int16_t Imu_Gyro[3];
  int16_t Imu_Acc[3];
  int16_t Imu_EurAngle[3];
  
}t_MotorDebugData;





/*ÿ10MS ����һ��
���������������� ��̨�ٻظ�������ȥ
Ҫ������̨��ֱ�ӽ� tGimbalCtrData ��������䵽 data ����ͨ�� ���ڷ��͡�
*/


/* 
  ����Ӳ��Э�� 115200��1ֹͣλ����У�� 

  ������ݰ���ʽ ��ÿһ������ռ��һ���ֽ�
  [0xa5][length][CMD][dLength][..data..][checksum][0x5a]
  0xa5    :
  0x5a    :
  length  : ��0xa5��0x5a�ĳ���
  checksum: data����checksum
  CMD     : ������ 
  dLength : CMD �� data �ĳ��� 
  checksum = 0x00^CMD^dLength^data[0]....^data[n]
*/


/* ��̨�������� ���ɷɿط��͵���̨ CMD=192 */
/* CMD=193�����ɿط����������ݻᷢ��ȥ */

  typedef struct
  {
    /*����ģʽ 0:�������ģʽ 1:��������ģʽ  */
    uint8_t ctr_Mode;
    /* У׼���� 0:������ 1:У׼������  2:׼��У׼ˮƽ(��ʼ����,׼����̨��ˮƽ��Ϻ��� 3) 
    3:��ʼУ׼ˮƽ  4:У׼FOC
    У׼��ʱ�򱣳�һ������һ������ ��Ȼ�����䵽0��У׼��ʱ��ʼ��Ч*/
    uint8_t Calibrate;
    /* �ɿص�ǰ����̬�ǣ���λ =  �Ƕ� * 100 */
    int16_t Pitch;
    int16_t Roll;
    int16_t Yaw; 
    /* ��̨Ŀ����̬�ǣ���λ =  �Ƕ� * 100 */ 
    int16_t ctr_Pitch;
    int16_t ctr_Roll; 
    int16_t ctr_Yaw; 
    
    int16_t gps[2];//������ٶ�
  }
  tGimbalCtrData;

  /* ��̨�������� CMD=191 */
  typedef struct
  {
    /* У׼״̬ */
    uint16_t Calibrate;
    /* ��̨��ǰ����̬�ǣ���λ =  �Ƕ� * 100 */
    int16_t Pitch;
    int16_t Roll;
    int16_t Yaw;
  }
  tGimbalRetData; 

  
  

void UartSend(void);
void UartRec(void);//5ms

#endif