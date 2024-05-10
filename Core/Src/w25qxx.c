/*

*/
#include "w25qxx.h"
#include "spi.h"

uint16_t FlashType = 0;//
uint32_t ConfigStartAddr = 0; //

static void HAL_W25QXX_CS_ENABLE(void);
static void HAL_W25QXX_CS_DISABLE(void);
static void HAL_SPI_Send_Byte(uint8_t byte);
static uint8_t HAL_SPI_Receive_Byte(void);
void HAL_W25QXX_Wait_Busy(void);

uint8_t W25QXX_BUFFER[4096];

const uint32_t FlashSpace[6] = {
	W25Q80SPACE   ,
	W25Q16SPACE   ,
	W25Q32SPACE   ,
	W25Q64SPACE 	,
	W25Q128SPACE	,
	W25Q256SPACE	,
};


static void HAL_W25QXX_CS_ENABLE(void) {
	HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port,W25Q64_CS_Pin, GPIO_PIN_RESET);
}

static void HAL_W25QXX_CS_DISABLE(void) {
	HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port,W25Q64_CS_Pin, GPIO_PIN_SET);
}

static void HAL_SPI_Send_Byte(uint8_t byte) {
	HAL_SPI_Transmit(&hspi1,&byte,1,0xFFFF);
}

static uint8_t HAL_SPI_Receive_Byte(void) {
	uint8_t data = 0xFF;
	HAL_SPI_Receive(&hspi1,&data,1,0xFFFF);
	return data;
}

uint32_t HAL_W25QXX_ReadID(void) {
	uint32_t Temp = 0;
	HAL_W25QXX_CS_ENABLE();
	HAL_SPI_Send_Byte(W25X_ManufactDeviceID);
	HAL_SPI_Send_Byte(0x00);
	HAL_SPI_Send_Byte(0x00);
	HAL_SPI_Send_Byte(0x00);
	Temp |= HAL_SPI_Receive_Byte()<<8;
	Temp |= HAL_SPI_Receive_Byte();
	HAL_W25QXX_CS_DISABLE();
	return Temp;
}

uint8_t HAL_W25QXX_Read_SR(uint8_t regno) {
	uint8_t byte=0,command=0;
	switch(regno)
	{
		case 1:
			command=W25X_ReadStatusReg1;
			break;
		case 2:
			command=W25X_ReadStatusReg2;
			break;
		case 3:
			command=W25X_ReadStatusReg3;
			break;
		default:
			command=W25X_ReadStatusReg1;
			break;
	}
	HAL_W25QXX_CS_ENABLE();
	HAL_SPI_Send_Byte(command);
	byte=HAL_SPI_Receive_Byte();
	HAL_W25QXX_CS_DISABLE();
	return byte;
}


void HAL_W25QXX_Write_SR(uint8_t regno,uint8_t sr) {
	uint8_t command=0;
	switch(regno)
	{
		case 1:
			command=W25X_WriteStatusReg1;
			break;
		case 2:
			command=W25X_WriteStatusReg2;
			break;
		case 3:
			command=W25X_WriteStatusReg3;
			break;
		default:
			command=W25X_WriteStatusReg1;
			break;
	}
	HAL_W25QXX_CS_ENABLE();
	HAL_SPI_Send_Byte(command);
	HAL_SPI_Send_Byte(sr);
	HAL_W25QXX_CS_DISABLE();
}

void HAL_W25QXX_Wait_Busy(void) {
	while((HAL_W25QXX_Read_SR(1)&0x01)==0x01);
}

void HAL_W25QXX_Write_Enable(void) {
	HAL_W25QXX_CS_ENABLE();
    HAL_SPI_Send_Byte(W25X_WriteEnable);
	HAL_W25QXX_CS_DISABLE();
}

void HAL_W25QXX_Write_Disable(void) {
	HAL_W25QXX_CS_ENABLE();
	HAL_SPI_Send_Byte(W25X_WriteDisable);
	HAL_W25QXX_CS_DISABLE();
}

uint8_t HAL_W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead) {
 	uint16_t i;
	HAL_W25QXX_CS_ENABLE();
	HAL_SPI_Send_Byte(W25X_ReadData);
	HAL_SPI_Send_Byte((uint8_t)((ReadAddr)>>16));
	HAL_SPI_Send_Byte((uint8_t)((ReadAddr)>>8));
	HAL_SPI_Send_Byte((uint8_t)ReadAddr);
	for(i=0;i<NumByteToRead;i++) {
		pBuffer[i]=HAL_SPI_Receive_Byte();
	}
	HAL_W25QXX_CS_DISABLE();
	return 0;
}

void HAL_W25QXX_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite) {
 	uint16_t i;
 	HAL_W25QXX_Write_Enable();
	HAL_W25QXX_CS_ENABLE();
	HAL_SPI_Send_Byte(W25X_PageProgram);
	HAL_SPI_Send_Byte((uint8_t)((WriteAddr)>>16));
	HAL_SPI_Send_Byte((uint8_t)((WriteAddr)>>8));
	HAL_SPI_Send_Byte((uint8_t)WriteAddr);
	for(i=0;i<NumByteToWrite;i++)
		HAL_SPI_Send_Byte(pBuffer[i]);
	HAL_W25QXX_CS_DISABLE();
	HAL_W25QXX_Wait_Busy();
}

void HAL_W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite) {
	uint16_t pageremain;
	pageremain=256-WriteAddr%256;
	if(NumByteToWrite<=pageremain)
		pageremain=NumByteToWrite;
	while(1) {
		HAL_W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)
			break;
	 	else {
			pBuffer+=pageremain;
			WriteAddr+=pageremain;
			NumByteToWrite-=pageremain;
			if(NumByteToWrite>256)
				pageremain=256;
			else
				pageremain=NumByteToWrite;
		}
	}
}

uint8_t HAL_W25QXX_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite) {
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;
 	uint16_t i;
	uint8_t* W25QXX_BUF;
	W25QXX_BUF=W25QXX_BUFFER;
 	secpos=WriteAddr/4096;
	secoff=WriteAddr%4096;
	secremain=4096-secoff;
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;
	while(1) {
		HAL_W25QXX_Read(W25QXX_BUF,secpos*4096,4096);
		for(i=0;i<secremain;i++) {
			if(W25QXX_BUF[secoff+i]!=0XFF)break;
		}
		if(i<secremain) {
			HAL_W25QXX_Erase_Sector(secpos);
			for(i=0;i<secremain;i++)
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];
			}
			HAL_W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);
		}
		else
			HAL_W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);
		if(NumByteToWrite==secremain)
			break;
		else {
			secpos++;
			secoff=0;

			pBuffer+=secremain;
			WriteAddr+=secremain;
			NumByteToWrite-=secremain;
			if(NumByteToWrite>4096)secremain=4096;
			else secremain=NumByteToWrite;
		}
	};
	return 0;
}

void HAL_W25QXX_Erase_Chip(void) {
	HAL_W25QXX_Write_Enable();
	HAL_W25QXX_Wait_Busy();
	HAL_W25QXX_CS_ENABLE();
	HAL_SPI_Send_Byte(W25X_ChipErase);
	HAL_W25QXX_CS_DISABLE();
	HAL_W25QXX_Wait_Busy();
}

void HAL_W25QXX_Erase_Sector(uint32_t Dst_Addr) {
 	//printf("fe:%x\r\n",Dst_Addr);
	Dst_Addr*=4096;
	HAL_W25QXX_Write_Enable();
	HAL_W25QXX_Wait_Busy();
	HAL_W25QXX_CS_ENABLE();
	HAL_SPI_Send_Byte(W25X_SectorErase);
	HAL_SPI_Send_Byte((uint8_t)((Dst_Addr)>>16));
	HAL_SPI_Send_Byte((uint8_t)((Dst_Addr)>>8));
	HAL_SPI_Send_Byte((uint8_t)Dst_Addr);
	HAL_W25QXX_CS_DISABLE();
	HAL_W25QXX_Wait_Busy();
}

void HAL_W25QXX_PowerDown(void) {
	HAL_W25QXX_CS_ENABLE();
	HAL_SPI_Send_Byte(W25X_PowerDown);
	HAL_W25QXX_CS_DISABLE();
	HAL_Delay(1);
}

void HAL_W25QXX_WAKEUP(void) {
  	HAL_W25QXX_CS_ENABLE();
    HAL_SPI_Send_Byte(W25X_ReleasePowerDown);  //  send W25X_PowerDown command 0xAB
	HAL_W25QXX_CS_DISABLE();
    HAL_Delay(1);
}


