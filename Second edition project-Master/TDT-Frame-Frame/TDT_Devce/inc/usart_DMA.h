#ifndef _USART_DMA__H
#define _USART_DMA__H

#include "board.h"

#define CONTROL_RX_BUF_NUM 10u      //���ط����ֽ�����2�����޷������ʹ洢
#define CONTROL_FRAME_LENGTH 5u      //���ط����ֽ���    
#define CONTROL_TX_BUF_NUM 9u      //���͸����ص������ֽ���

static void Rx_data_processing(uint8_t *comm_buf);

extern u8 Translation_STA;
extern u8 uplift_STA;
extern u8 climb_STA;
extern u8 pumpTurn_STA;//���ñ���ת��ɱ�־
extern u8 pump_STA;//������ɱ�־


extern int16_t tranFeed;
extern u8 uplift_CMD;//�������̧����ʯ
extern u8 climb_CMD;//���ñ�����ǰ��
extern u8 pumpTurn_CMD;//���ñ���ת
extern u8 pump_CMD;//����
extern u8 turntable_CMD;//����ת��ת
extern u8 clip_CMD;//���Ӽ���
#endif

