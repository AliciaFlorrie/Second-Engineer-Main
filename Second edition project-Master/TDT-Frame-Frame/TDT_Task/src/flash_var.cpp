#include "flash_var.h"
#include "stm32f4xx_flash.h"
/**FreeRTOS*START***************/
#include "FreeRTOS.h" //FreeRTOSʹ��
#include "timers.h"
#include "list.h"
#include "queue.h"
#include "task.h"
/**FreeRTOS*END***************/

void FlashSave_Task(void *pvParameters)
{
	while(1)
	{
		if(ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
		{
			IFlash.saving = true;
			IFlash.save();
			IFlash.saving = false;
		}

	}
}

/**
 * @brief ��ȡָ����ַ�İ���(16λ����) 
 * @param  faddr            ��ȡ�ĵ�ַ
 * @return u32 				��Ӧ����
 */
u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu32 *)faddr;
}

/**
 * @brief ��ȡĳ����ַ���ڵ�flash����
 * @param  addr             flash��ַ
 * @return uint16_t 		0~11,��addr���ڵ�����
 */
uint16_t STMFLASH_GetFlashSector(u32 addr)
{
	if (addr < ADDR_FLASH_SECTOR_1)
		return FLASH_Sector_0;
	if (addr < ADDR_FLASH_SECTOR_2)
		return FLASH_Sector_1;
	if (addr < ADDR_FLASH_SECTOR_3)
		return FLASH_Sector_2;
	if (addr < ADDR_FLASH_SECTOR_4)
		return FLASH_Sector_3;
	if (addr < ADDR_FLASH_SECTOR_5)
		return FLASH_Sector_4;
	if (addr < ADDR_FLASH_SECTOR_6)
		return FLASH_Sector_5;
	if (addr < ADDR_FLASH_SECTOR_7)
		return FLASH_Sector_6;
	if (addr < ADDR_FLASH_SECTOR_8)
		return FLASH_Sector_7;
	if (addr < ADDR_FLASH_SECTOR_9)
		return FLASH_Sector_8;
	if (addr < ADDR_FLASH_SECTOR_10)
		return FLASH_Sector_9;
	if (addr < ADDR_FLASH_SECTOR_11)
		return FLASH_Sector_10;
	return FLASH_Sector_11;
}

/**
 * @brief  ��ָ����ַ��ʼд��ָ�����ȵ�����
 * @param  WriteAddr        ��ʼ��ַ(�˵�ַ����Ϊ4�ı���!!)
 * @param  pBuffer          ����ָ��
 * @param  NumToWrite       ��(32λ)��(����Ҫд���32λ���ݵĸ���.)
 * @note   //�ر�ע��:��ΪSTM32F4������ʵ��̫��,û�취���ر�����������,���Ա�����
 *         д��ַ�����0XFF,��ô���Ȳ������������Ҳ�������������.����
 *         д��0XFF�ĵ�ַ,�����������������ݶ�ʧ.����д֮ǰȷ��������
 *         û����Ҫ����,��������������Ȳ�����,Ȼ����������д. 
 */
void STMFLASH_Write(u32 WriteAddr, u32 *pBuffer, u32 NumToWrite)
{
	FLASH_Status status = FLASH_COMPLETE;
	u32 addrx = 0;
	u32 endaddr = 0;
	if (WriteAddr < STM32_FLASH_BASE || WriteAddr % 4)
		return;					 //�Ƿ���ַ
	FLASH_Unlock();				 //����
	FLASH_DataCacheCmd(DISABLE); //FLASH�����ڼ�,�����ֹ���ݻ���

	addrx = WriteAddr;					  //д�����ʼ��ַ
	endaddr = WriteAddr + NumToWrite * 4; //д��Ľ�����ַ
	if (addrx < 0X1FFF0000)				  //ֻ�����洢��,����Ҫִ�в�������!!
	{
		while (addrx < endaddr) //ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
		{
			if (STMFLASH_ReadWord(addrx) != 0XFFFFFFFF) //�з�0XFFFFFFFF�ĵط�,Ҫ�����������
			{
				status = FLASH_EraseSector(STMFLASH_GetFlashSector(addrx), VoltageRange_3); //VCC=2.7~3.6V֮��!!
				if (status != FLASH_COMPLETE)
					break; //����������
			}
			else
				addrx += 4;
		}
	}
	if (status == FLASH_COMPLETE)
	{
		while (WriteAddr < endaddr) //д����
		{
			if (FLASH_ProgramWord(WriteAddr, *pBuffer) != FLASH_COMPLETE) //д������
			{
				break; //д���쳣
			}
			WriteAddr += 4;
			pBuffer++;
		}
	}
	FLASH_DataCacheCmd(ENABLE); //FLASH��������,�������ݻ���
	FLASH_Lock();				//����
}

/**
 * @brief ��ָ����ַ��ʼ����ָ�����ȵ�����
 * @param  ReadAddr         ��ʼ��ַ
 * @param  pBuffer          ����ָ��
 * @param  NumToRead        ��(4λ)��
 */
void STMFLASH_Read(u32 ReadAddr, u32 *pBuffer, u32 NumToRead)
{
	u32 i;
	for (i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = STMFLASH_ReadWord(ReadAddr); //��ȡ4���ֽ�.
		ReadAddr += 4;							  //ƫ��4���ֽ�.
	}
}

/*�������洢����*/
void SaveToMEMORY(u32 *pBuff, u32 size)
{
	STMFLASH_Write(InnerSECTOR, pBuff, size);
}

/**
 * @brief �Ӵ洢���ʶ���������new����������ָ��
 * @return u32* 			���ش���������ָ��
 * @note  
 */
u32 *ReadFromMEMORY()
{
	u32 size = 0;
	STMFLASH_Read(InnerSECTOR, &size, 1);
	if (size == 0 || size == 0xFFFFFFFF)
		return NULL;
	u32 *pBuff = new u32[size];
	STMFLASH_Read(InnerSECTOR, pBuff, size);
	return pBuff;
}

void (*FlashVar::pSaveFunc)(u32 *saveBuffer, u32 buffsize) = SaveToMEMORY;
u32 *(*FlashVar::pReadFunc)() = ReadFromMEMORY;

FlashVar IFlash;
