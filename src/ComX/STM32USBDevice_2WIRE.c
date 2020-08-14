/**/
#include "main.h"
#include "usb_lib.h"
/**/

#define HidUsbSecHLen 3
#define HidUsbSecFlag 1
#define HidUsbSecNum  2
#define HidUsbSectionDataLen (64 - HidUsbSecHLen)
#define HidUsbPackLen 64

#define ENDP1                   ((uint8_t)1)
#define ENDP2                   ((uint8_t)2)
#define EP1_IN                  ((uint8_t)0x81)
#define EP2_OUT                 ((uint8_t)0x02)
#define EP_RX_VALID             (0x3000) /* EndPoint RX VALID */
#define EP_TX_VALID             (0x0030) /* EndPoint TX VALID */

/* ���������� */
#define USBabuffLen             1024
#define USBCMD1_RBuffLen        (USBabuffLen)
#define USBCMD1_TBuffLen        (USBabuffLen)

  typedef enum  {
    USB_State_Connect,
    USB_State_DisConnect,
    USB_ReqConnect
  }USB_Connect_State;

/* * */
typedef struct  {
  //USB����˿� ���ջ�����
  uint8_t IoRxBuff[USBCMD1_RBuffLen];
  uint8_t IoTxBuff[USBCMD1_TBuffLen];
  //FIFO������
  //SoftFIFO1 USBCMD1FIFO_In;
  /*���ͼ�����*/
  uint16_t IoTxBuffWriteCnt,IoTxBuffReadCnt;
  /* ���ռ����� */
  uint16_t IoRecBuffWriteCnt;
  uint16_t IoRecBuffReadCnt;
  uint16_t IoRecLen;
  uint8_t IoRecOverflow;
  /* ��ǻ������е�����֡���� */
  uint8_t ValidFramNum;
  uint8_t IsReadyToSend;
}t_UsbDevice_Buff;

/**/
void EP2_OUT_Callback(void);
void EP1_IN_Callback(void);
uint32_t USB_SIL_Init(void);
uint32_t USB_SIL_Write(uint8_t bEpAddr, uint8_t *pBufferPointer, uint32_t wBufferSize);
uint32_t USB_SIL_Read(uint8_t bEpAddr, uint8_t *pBufferPointer);
void SetEPTxCount(uint8_t /*bEpNum*/, uint16_t /*wCount*/);
void SetEPRxCount(uint8_t /*bEpNum*/, uint16_t /*wCount*/);
void SetEPTxStatus(uint8_t /*bEpNum*/, uint16_t /*wState*/);
void SetEPRxStatus(uint8_t /*bEpNum*/, uint16_t /*wState*/);
void USB_Init(void);
void SOF_Callback(void);
static uint8_t Stm32UsbDevice_ReadyToSend(void);
static void Stm32UsbDevice_ReceiveBlock(void);
/**/
static uint8_t UserUSB_IoBuff1[64];
static uint32_t USBTXCnt = 0, USBRXCnt = 0;
static uint8_t SOPLostCount;
static uint8_t usbPrevFlag,usbFlag;
static t_UsbDevice_Buff UsbDevice_Buff;

static uint32_t CurrentPinState;

USB_Connect_State USBDevice_ConnectState;

/* CRC 8 λУ��� */
static uint8_t CRC8_Table[] =
{
  0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
  157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
  35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
  190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
  70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
  219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
  101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
  248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
  140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
  17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
  175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
  50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
  202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
  87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
  233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
  116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

/**/
static uint8_t Usb_Calcrc8(uint8_t *buffer, int32_t offset, int32_t len) {
  uint8_t crc;
  int32_t j;
  crc = 0;
  
  for (j = 0; j < len; j++) {
    crc = CRC8_Table[crc ^ buffer[offset]];
    offset++;
  }
  return (crc);
}

/**/
static uint8_t Usb_Calcrc8_2(uint8_t *buffer, int32_t offset, int32_t len, uint8_t PrvCrc) {
  uint8_t crc;
  int32_t j;
  crc = PrvCrc;
  
  for (j = 0; j < len; j++) {
    crc = CRC8_Table[crc ^ buffer[offset]];
    offset++;
  }
  return (crc);
}

/**/
static void CSP_USBDevice_ReadyToSend(uint8_t *UserMem, uint16_t SDataLen) {
  
  USB_SIL_Write(EP1_IN, (uint8_t*)UserMem, 64);
  SetEPTxCount(ENDP1, 64);
  SetEPTxStatus(ENDP1, EP_TX_VALID);
}

/* * */
void SOF_Callback(void) {
  SOPLostCount = 0;
}

/* * */
void EP1_IN_Callback(void) {
  USBTXCnt++;
  Stm32UsbDevice_ReadyToSend();
}

/* * */
void EP2_OUT_Callback(void) {
  
  USB_SIL_Read(ENDP2, UserUSB_IoBuff1);
  Stm32UsbDevice_ReceiveBlock();
  
  USBRXCnt++;
  SetEPRxStatus(ENDP2, EP_RX_VALID);
}

/*
����: �ڴ渴֮����,��Դ���ݸ��Ƶ�Ŀ������
����: Դ���ݣ�Ŀ���ڴ棬���ƵĴ�С��λ=�ֽ�
*/
void usbmemcpy(uint8_t *sdp, uint8_t *odp, uint32_t CSize) {
  for (; CSize > 0; CSize--) {
    *odp = *sdp;
    sdp++;
    odp++;
  }
}

/* ����ϵ�ģʽ */
static void UsbGoPowerDown(void) {
  uint16_t cntregval;
  cntregval = _GetCNTR();
  cntregval |= (CNTR_PDWN);  //����ϵ�ģʽ
  _SetCNTR(cntregval);
  
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/* �˳��ϵ�ģʽ */
static void UsbExitPowerDown(void) {
  uint16_t cntregval;
  cntregval = _GetCNTR();
  cntregval &= (~CNTR_PDWN);  //����ϵ�ģʽ
  _SetCNTR(cntregval);
  
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


/*
���յ�һ���� 
*/
static void Stm32UsbDevice_ReceiveBlock(void) {
  
  //int32_t inta;
  /*����Ƿ����ظ���*/
  if (usbPrevFlag == UserUSB_IoBuff1[HidUsbSecFlag])
    return;
  usbPrevFlag = UserUSB_IoBuff1[HidUsbSecFlag];
  /* �������� */
  int32_t intb;
  intb = UsbDevice_Buff.IoRecBuffWriteCnt + HidUsbSectionDataLen;
  
  UsbDevice_Buff.IoRecOverflow = 1;
  if (intb < USBabuffLen) {
    UsbDevice_Buff.IoRecOverflow = 0;
    usbmemcpy(&UserUSB_IoBuff1[HidUsbSecHLen],
              &UsbDevice_Buff.IoRxBuff[UsbDevice_Buff.IoRecBuffWriteCnt],
              HidUsbSectionDataLen);
    UsbDevice_Buff.IoRecBuffWriteCnt = intb;
  }
  
  /*��������һ��*/
  if ((UserUSB_IoBuff1[HidUsbSecFlag] & 0x80) == 0) {
    UsbDevice_Buff.ValidFramNum++; /*��ǽ��յ�һ����*/
  }
}


/**/
uint8_t Stm32UsbDevice_ReadyToSend(void) {
  
  if (UsbDevice_Buff.IoTxBuffWriteCnt == 0) {
    UsbDevice_Buff.IsReadyToSend = 0;
    return (0);
  }
  if (UsbDevice_Buff.IsReadyToSend == 0)
    return (0);
  CSP_USBDevice_ReadyToSend(&UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffReadCnt], HidUsbPackLen);
  UsbDevice_Buff.IoTxBuffWriteCnt -= HidUsbPackLen;
  UsbDevice_Buff.IoTxBuffReadCnt += HidUsbPackLen;
  //���� 1 ��ʾ������Ҫ����
  return (1);
}

/**/
static uint8_t ReadyToSend(void) {
  if (UsbDevice_Buff.IoTxBuffWriteCnt == 0) {
    UsbDevice_Buff.IsReadyToSend = 0;
    return (0);
  }
  if (UsbDevice_Buff.IsReadyToSend == 0)
    return (0);
  CSP_USBDevice_ReadyToSend(&UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffReadCnt], HidUsbPackLen);
  UsbDevice_Buff.IoTxBuffWriteCnt -= HidUsbPackLen;
  UsbDevice_Buff.IoTxBuffReadCnt += HidUsbPackLen;
  //����0 ��ʾ������Ҫ����
  return (1);
}

/* ��ʼ��PInΪͨ�ù��� */
static void SetPinToGeneral(void) {

}

/* ��ʼ��PIN ΪUSB���� */
static void SetPinToUSB(void) {

}

/**/
void USBDevice_WriteData(uint8_t Cmd, uint8_t *DataPoint, int32_t DataLen) {

  /**/
  UsbDevice_Buff.IsReadyToSend = 0;
  /**/
  UsbDevice_Buff.IoTxBuffWriteCnt = 0;
  /*����һ��*/
  /*���ñ���ID*/
  UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = 1;
  /*����flag*/
  UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = usbFlag | 0x80;
  usbFlag++;
  /*���öα��*/
  UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = 0;
  //�������ݵ�������
  UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = 0; //crc8
  DataLen += 4;
  UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = (DataLen & 0xff);
  UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = ((DataLen >> 8) & 0xff);
  DataLen -= 4;
  //
  UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = Cmd;
  /**/
  UsbDevice_Buff.IoTxBuff[3] = Usb_Calcrc8_2(&UsbDevice_Buff.IoTxBuff[4], 0, 3, 0);
  UsbDevice_Buff.IoTxBuff[3] = Usb_Calcrc8_2(DataPoint, 0, DataLen, UsbDevice_Buff.IoTxBuff[3]);
  /**/
  int32_t Inta;
  for (Inta = 0; UsbDevice_Buff.IoTxBuffWriteCnt < HidUsbPackLen; Inta++)
    UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = DataPoint[Inta];
  /**/
  int32_t Intb, Intc;
  Intc = 1;
  for (; Inta < DataLen;) {
    /* �ֶ� */
    //���ñ���ID
    UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = 1;
    //����flag
    UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = usbFlag | 0x80;
    usbFlag++;
    //���öα��
    UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = Intc++;
    /* ����һ������ */
    for (Intb = 0; Intb < HidUsbSectionDataLen; Intb++)
      UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt++] = DataPoint[Inta++];
  }
  UsbDevice_Buff.IoTxBuff[UsbDevice_Buff.IoTxBuffWriteCnt - (HidUsbPackLen - 1)] &= 0x7f;
  UsbDevice_Buff.IoTxBuffReadCnt = 0;
  UsbDevice_Buff.IsReadyToSend = 1;
  ReadyToSend();
}
/*
����: ��ȡ����
����: ���������[uint8_t]����ȡ����
����: ���������ݣ�[cmd][data]
*/
uint8_t* USBDevice_Read(int32_t* pOut_Length)
{
  /*ռ��*/
  //SemLock.Lock(0);
  /* ������ջ�������� */
  if (UsbDevice_Buff.IoRecOverflow > 0 || 
      UsbDevice_Buff.IoRecBuffReadCnt >= UsbDevice_Buff.IoRecBuffWriteCnt) {
   lp1:
    //__disable_irq();
    UsbDevice_Buff.IoRecBuffReadCnt = 0;
    UsbDevice_Buff.IoRecBuffWriteCnt = 0;
    UsbDevice_Buff.IoRecOverflow = 0;
    UsbDevice_Buff.ValidFramNum = 0;
    //__enable_irq();
    *pOut_Length = -1;
    return (0);
  }
  if(UsbDevice_Buff.ValidFramNum == 0)
  {
    *pOut_Length = -1;
    return 0;
  }
  
  UsbDevice_Buff.ValidFramNum--;
  /* * */
  uint32_t      mLen;
  uint8_t       rCrc8;
  uint8_t       lCrc8;
  /* * */
  rCrc8 = UsbDevice_Buff.IoRxBuff[UsbDevice_Buff.IoRecBuffReadCnt];
  mLen = UsbDevice_Buff.IoRxBuff[UsbDevice_Buff.IoRecBuffReadCnt + 1];
  mLen |= (UsbDevice_Buff.IoRxBuff[UsbDevice_Buff.IoRecBuffReadCnt + 2] << 8);
  if((mLen+UsbDevice_Buff.IoRecBuffReadCnt) > (USBabuffLen-1))
    goto lp1;
  
  lCrc8 = Usb_Calcrc8(&UsbDevice_Buff.IoRxBuff[UsbDevice_Buff.IoRecBuffReadCnt + 1], 0, mLen - 1);
  if (lCrc8 != rCrc8) {goto lp1;}
  /*��������*/

  int32_t intc;
  if ((mLen % HidUsbSectionDataLen) != 0)
    intc = ((mLen / HidUsbSectionDataLen) * HidUsbSectionDataLen) + HidUsbSectionDataLen;
  else
    intc = mLen;
  
  *pOut_Length = mLen - 3;
  
  mLen = intc; // - HidUsbSecHLen;

  
  uint8_t *p_data_cpy =  &UsbDevice_Buff.IoRxBuff[UsbDevice_Buff.IoRecBuffReadCnt + HidUsbSecHLen];
  UsbDevice_Buff.IoRecBuffReadCnt += intc;
  intc -= HidUsbSecHLen;
    
  return (p_data_cpy);
}

/* ���USB�Ƿ����� �ὫIOȫ���õ� ������IO���ر�״̬���� */
USB_Connect_State USBDevice_CheckUsbConnect(void) {
  return USBDevice_ConnectState;
}
uint8_t GetUsbConnectState(void)
{
    if(SOPLostCount++>=100)return 0;
    else return 1;
}

/**/
int32_t USBDevice_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  /**/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOA, GPIO_Pin_11);
  GPIO_ResetBits(GPIOA, GPIO_Pin_12);
  Timer_Delay_ms(300);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_SetBits(GPIOA, GPIO_Pin_11);
  GPIO_SetBits(GPIOA, GPIO_Pin_12);
    /*SET PA11,12 for USB: USB_DM,DP*/
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_14);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_14);
  
  
  /**/
  Set_USBClock();
  USB_Interrupts_Config();
  
  USB_Init();
  
//  while (bDeviceState != CONFIGURED)
//  {}
  return 1;
}

