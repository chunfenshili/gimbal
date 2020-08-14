#define DEFIN_URARTDATA_EXTERN
#include "usart.h"
#include "usart_data.h"
#include  "datacalibrate.h"
#include  "Control.h"
#include <string.h>
#include "device.h"
#include "UartPack.h"
#include "main.h"
#include "factory.h"


static tGimbalCtrData   UsartToGimbal;
static tGimbalRetData  GimbalSetData;
static uint8_t UsartRecSuc = 0;
static uint8_t SendCmd = 0;


extern uint8_t  GetFactoryState(void);
extern void UsartSetTeleClib(uint8_t states);
extern void SetClibrateFoc(uint8_t);


/**/
int udp_PackData(uint8_t* p_data,uint8_t wLen,uint8_t *p_out)
{
    /**/
    int wcnt;
    int k = 0;
    uint8_t chk;
    /*
    if ((wLen+4)>255) {
    return -1;
}*/
    /**/
    p_out[k++] = 0xa5;
    p_out[k++] = wLen + 6;
    p_out[k++] = 191;
    p_out[k++] = wLen+2;
    chk = 0;
    for (wcnt = 0; wcnt < wLen; wcnt++) {
        p_out[k++] = p_data[wcnt];
        chk ^= p_data[wcnt]; 
    }
    p_out[k++] = chk;
    p_out[k++] = 0x5a;
    /**/
    return 0;
    /**/
}



//
//
void UartRecCallBak(u8* data,u8 lenth)
{
    switch(*(data+2)){
    case 193:
        UsartRecSuc  =0;
        USART1_send_data(data,lenth);
        SendCmd = 193;
        break;
    case 192:
        SendCmd = 192;
        uint8_t *pdData = (uint8_t *)&UsartToGimbal.ctr_Mode;
        uint8_t *psData = data+4;
        
        uint8_t checkSum = 0;
        checkSum ^=data[2];
        checkSum ^=data[3];
        int i = 0;
        for(i = 0;i<sizeof(tGimbalCtrData);i++){
            pdData[i]     = psData[i];
            checkSum^=psData[i];    
        }
        //if(checkSum==psData[i]){
            UsartRecSuc  = 1;
       // }  
    
    }
}
//�װ�2�ŷ������ݵ�0��
void UsartSendToMainPcb(void)
{
    if(UsartRecSuc){
        UsartRecSuc = 0;
        uint8_t pdata[sizeof(tGimbalCtrData)+3];
        pdata[0] = 27;
        pdata[1] = sizeof(tGimbalCtrData);
        pdata[2] = 0x08;
        memcpy((void*)&pdata[3],(void*)&UsartToGimbal.ctr_Mode,sizeof(tGimbalCtrData)); 
        ComX_SendPack((uint8_t*)pdata,0,sizeof(tGimbalCtrData)+3);
    }
}

//����ɿ����� ����ҪҪ��5MS ����һ��




static  uint16_t calibrateConter = 0;
void DealWithClibrte(void)
{

    calibrateConter++;
}
void DealWithFCData(void)
{
    
    static  uint8_t oldCalibrateData= 0;
    uint8_t clibrtFoc = 0;

   // UsartSendToSubMainPcb();  
        //
    g_fAngleExp[0] = UsartToGimbal.ctr_Pitch/100.f;
    g_fAngleExp[1] = UsartToGimbal.ctr_Roll/100.f;
    g_fAngleExp[2] = UsartToGimbal.ctr_Yaw/100.f;    
    g_ucDeviceMode = UsartToGimbal.ctr_Mode;
        //
    if(UsartToGimbal.Calibrate!=oldCalibrateData){//�����䡣�ص�0��ʱ���˵��״̬�ȶ�
        if(UsartToGimbal.Calibrate==0){
            if(calibrateConter>=1000){
                UsartSetTeleClib(oldCalibrateData); //��ֵУ׼״̬ 
            }
        }else {
            calibrateConter  = 0;
        }
    }

    oldCalibrateData = UsartToGimbal.Calibrate;   
   
    uint8_t pdata[15];
    pdata[0] = 27;
    pdata[2] = 0x28;//���͸�2�� ��Ӧ8����   
    pdata[3] =clibrtFoc;
    GimbalSetData.Pitch = (int16_t)(Sensordata.pitch*100);
    GimbalSetData.Roll  = (int16_t)(Sensordata.roll*100 );
    GimbalSetData.Yaw   = (int16_t)(Sensordata.yaw*100 );
    GimbalSetData.Calibrate = GetFactoryState();
    
    memcpy((void*)&pdata[4],(void*)&GimbalSetData.Calibrate,sizeof(tGimbalRetData));  
    ComX_SendPack((uint8_t*)pdata,0,sizeof(tGimbalRetData)+4);

}
//�� 2�ŷ���0�� Ȼ�����������ﴦ��
//������ɺ� �ٷ��͸�1��Ȼ��1���ٷ���2;
//Ȼ��2���ڷ��� �ɿ�
void MemUsart0pcbRecDataToRam(uint8_t*pdata,uint8_t lenth)
{
    memcpy((void*)&UsartToGimbal.ctr_Mode,(void*)pdata,sizeof(tGimbalCtrData)); 
    DealWithFCData();
    UsartRecSuc = 1;
} 
//
void MemUsart1pcbRecDataToRam(uint8_t*pdata,uint8_t lenth)
{
//    pdata[2] = 0x28;//���͸�2�� ��ӦA����  ����������  
//    ComX_SendPack((uint8_t*)pdata,0,sizeof(tGimbalRetData)+4);
//    UsartRecSuc = 1;
} 
//
void MemUsart2pcbRecDataToRam(uint8_t*pdata,uint8_t lenth)
{
    uint8_t  data_Temp1[50]; 
    //2��Ҫ��0�ŷ����������� ���͸��ɿ� 
    memcpy((void*)&GimbalSetData.Calibrate,(void*)(pdata+4),sizeof(tGimbalRetData)); 
    udp_PackData((uint8_t *)(pdata+4),sizeof(tGimbalRetData),data_Temp1);
    USART1_send_data(data_Temp1,sizeof(tGimbalRetData)+6);   
}

void UartRec(void)//5ms
{ 
    if(g_ucDeviceNum==2){
        UsartSendToMainPcb();  
    }else if(g_ucDeviceNum==0){
        DealWithClibrte();
    }   
}









