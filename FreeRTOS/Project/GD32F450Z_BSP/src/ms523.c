/*!
    \file  ms523.c
    \brief ms523 init read and write
*/

#include "ms523.h"
#include "i2c.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
// #include "debug.h"

/*!
    \brief      delay
    \param[in]  Delay_Time
    \param[out] none
    \retval     none
*/

void MFRC_Delay(uint16_t Delay_Time)
{
  uint16_t i, j;
  for (i = 40; i > 0; i--)
  {
    for (j = Delay_Time; j > 0; j--)
      ;
  }
}
/*!
    \brief      clear register bit
    \param[in]  reg:register
    \param[in]  mask:bit mask
    \param[out] none
    \retval     none
*/
void ClearBitMask(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t reg, uint8_t mask)
{
  uint8_t tmp = 0x0;
  i2c_read(i2c_periph, i2c_addr, reg, &tmp, 1);
  i2c_byte_write(i2c_periph, i2c_addr, reg, tmp & ~mask); // clear bit mask
}

/*!
    \brief      set register bit
    \param[in]  reg:register
    \param[in]  mask:bit mask
    \param[out] none
    \retval     none
*/
void SetBitMask(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t reg, uint8_t mask)
{
  uint8_t tmp = 0x0;
  i2c_read(i2c_periph, i2c_addr, reg, &tmp, 1);
  i2c_byte_write(i2c_periph, i2c_addr, reg, tmp | mask); // set bit mask
}

/*!
    \brief      turn on the antenna
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PcdAntennaOn(uint32_t i2c_periph, uint8_t i2c_addr)
{
  unsigned char i;
  i2c_read(i2c_periph, i2c_addr, TxControlReg, &i, 1);
  if (!(i & 0x03))
  {
    SetBitMask(i2c_periph, i2c_addr, TxControlReg, 0x03);
  }
}

/*!
    \brief      turn off the antenna
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PcdAntennaOff(uint32_t i2c_periph, uint8_t i2c_addr)
{
  ClearBitMask(i2c_periph, i2c_addr, TxControlReg, 0x03);
}
/*!
    \brief      Communication with iso14443 card
    \param[in]  Command:Command
    \param[in]  pInData:send data
    \param[in]  InLenByte:Send data length
    \param[out] pOutData:receive data
    \param[out] pOutLenBit:receive data bit lenth
    \retval     Communication status
*/

#define MAXRLEN 18
int8_t PcdComMS523(uint32_t i2c_periph, uint8_t i2c_addr,
                   uint8_t Command,
                   uint8_t *pInData,
                   uint8_t InLenByte,
                   uint8_t *pOutData,
                   uint16_t *pOutLenBit)
{
  int8_t status = MI_ERR;
  uint8_t irqEn = 0x00;
  uint8_t waitFor = 0x00;
  uint8_t lastBits;
  uint8_t n;
  uint16_t i;

  switch (Command)
  {
  case PCD_AUTHENT:
    irqEn = 0x12;
    waitFor = 0x10;
    break;
  case PCD_TRANSCEIVE:
    irqEn = 0x77;
    waitFor = 0x30;
    break;
  default:
    break;
  }

  i2c_byte_write(i2c_periph, i2c_addr, ComIEnReg, irqEn | 0x80);
  ClearBitMask(i2c_periph, i2c_addr, ComIrqReg, 0x80);
  i2c_byte_write(i2c_periph, i2c_addr, CommandReg, PCD_IDLE);
  SetBitMask(i2c_periph, i2c_addr, FIFOLevelReg, 0x80);

  i2c_write(i2c_periph, i2c_addr, FIFODataReg, pInData, InLenByte);

  i2c_byte_write(i2c_periph, i2c_addr, CommandReg, Command);

  if (Command == PCD_TRANSCEIVE)
  {
    SetBitMask(i2c_periph, i2c_addr, BitFramingReg, 0x80);
  }

  i = 800;
  do
  {
    i2c_read(i2c_periph, i2c_addr, ComIrqReg, &n, 1);
    i--;
  } while ((i != 0) && !(n & 0x01) && !(n & waitFor));
  ClearBitMask(i2c_periph, i2c_addr, BitFramingReg, 0x80);

  if (i != 0)
  {
    i2c_read(i2c_periph, i2c_addr, ErrorReg, &n, 1);
    if (!(n & 0x1B))
    {
      status = MI_OK;
      if (n & irqEn & 0x01)
      {
        status = MI_NOTAGERR;
      }
      if (Command == PCD_TRANSCEIVE)
      {
        i2c_read(i2c_periph, i2c_addr, FIFOLevelReg, &n, 1);
        i2c_read(i2c_periph, i2c_addr, ControlReg, &lastBits, 1);
        lastBits &= 0x07;
        if (lastBits)
        {
          *pOutLenBit = (n - 1) * 8 + lastBits;
        }
        else
        {
          *pOutLenBit = n * 8;
        }
        if (n == 0)
        {
          n = 1;
        }
        if (n > MAXRLEN)
        {
          n = MAXRLEN;
        }
        i2c_read(i2c_periph, i2c_addr, FIFODataReg, pOutData, n);
      }
    }
    else
    {
      status = MI_ERR;
    }
  }
  SetBitMask(i2c_periph, i2c_addr, ControlReg, 0x80); // stop timer now
  i2c_byte_write(i2c_periph, i2c_addr, CommandReg, PCD_IDLE);
  return status;
}

/*!
    \brief      reset ms523
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PcdReset(uint32_t i2c_periph, uint8_t i2c_addr)
{

  i2c_byte_write(i2c_periph, i2c_addr, CommandReg, PCD_RESETPHASE);
  MFRC_Delay(2000);

  i2c_byte_write(i2c_periph, i2c_addr, ModeReg, 0x3D);
  i2c_byte_write(i2c_periph, i2c_addr, TReloadRegL, 30);
  i2c_byte_write(i2c_periph, i2c_addr, TReloadRegH, 0);
  i2c_byte_write(i2c_periph, i2c_addr, TModeReg, 0x8D);
  i2c_byte_write(i2c_periph, i2c_addr, TPrescalerReg, 0x3E);
  i2c_byte_write(i2c_periph, i2c_addr, TxAutoReg, 0x40);

  ClearBitMask(i2c_periph, i2c_addr, TestPinEnReg, 0x80); // off MX and DTRQ out
  i2c_byte_write(i2c_periph, i2c_addr, TxAutoReg, 0x40);
}

/*!
    \brief      Calulate CRC16
    \param[in]  pInData:send data
    \param[in]  InLenByte:Send data length
    \param[out] pOutData:receive data
    \retval     none
*/
void CalulateCRC(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t *pIndata, uint8_t len, uint8_t *pOutData)
{
  uint8_t i, n;

  ClearBitMask(i2c_periph, i2c_addr, DivIrqReg, 0x04);
  i2c_byte_write(i2c_periph, i2c_addr, CommandReg, PCD_IDLE);
  SetBitMask(i2c_periph, i2c_addr, FIFOLevelReg, 0x80);

  i2c_write(i2c_periph, i2c_addr, FIFODataReg, pIndata, len);
  i2c_byte_write(i2c_periph, i2c_addr, CommandReg, PCD_CALCCRC);
  i = 0xFF;
  do
  {
    i2c_read(i2c_periph, i2c_addr, DivIrqReg, &n, 1);
    i--;
  } while ((i != 0) && !(n & 0x04));
  i2c_read(i2c_periph, i2c_addr, CRCResultRegL, &pOutData[0], 1);
  i2c_read(i2c_periph, i2c_addr, CRCResultRegM, &pOutData[1], 1);
}

/*!
    \brief      Set the mode of ms523
    \param[in]  type:mode
    \param[out] none
    \retval     status
*/
int8_t M500PcdConfigISOType(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t type)
{
  if ('A' == type)
  {
    i2c_byte_write(i2c_periph, i2c_addr, Status2Reg, 0x08);
    i2c_byte_write(i2c_periph, i2c_addr, ModeReg, 0x3D);
    i2c_byte_write(i2c_periph, i2c_addr, RxSelReg, 0x86);
    i2c_byte_write(i2c_periph, i2c_addr, RFCfgReg, 0x7F);
    i2c_byte_write(i2c_periph, i2c_addr, TReloadRegL, 30);
    i2c_byte_write(i2c_periph, i2c_addr, TReloadRegH, 0);
    i2c_byte_write(i2c_periph, i2c_addr, TModeReg, 0x8D);
    i2c_byte_write(i2c_periph, i2c_addr, TPrescalerReg, 0x3E);
    MFRC_Delay(10000);
    PcdAntennaOn(i2c_periph, i2c_addr);
  }
  else
  {
    return -1;
  }

  return MI_OK;
}

/*!
    \brief      Initialization ms523
    \param[in]  none
    \param[out] none
    \retval     none
*/

void MFRC522_Init(uint32_t i2c_periph, uint8_t i2c_addr)
{
  PcdReset(i2c_periph, i2c_addr);
  PcdAntennaOff(i2c_periph, i2c_addr);
  MFRC_Delay(2000);
  PcdAntennaOn(i2c_periph, i2c_addr);
  M500PcdConfigISOType(i2c_periph, i2c_addr, 'A');
}

/*!
    \brief      Card search
    \param[in]  req_code:Card searching mode
    \param[in]  pTagType:Card type
                0x4400 = Mifare_UltraLight
                0x0400 = Mifare_One(S50)
                0x0200 = Mifare_One(S70)
                0x0800 = Mifare_Pro(X)
                0x4403 = Mifare_DESFire
    \param[out] none
    \retval     status
*/

int8_t PcdRequest(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t req_code, uint8_t *pTagType)
{
  int8_t status;
  uint16_t unLen;
  uint8_t ucComMS523Buf[MAXRLEN];

  ClearBitMask(i2c_periph, i2c_addr, Status2Reg, 0x08);
  i2c_byte_write(i2c_periph, i2c_addr, BitFramingReg, 0x07);
  SetBitMask(i2c_periph, i2c_addr, TxControlReg, 0x03);

  ucComMS523Buf[0] = req_code;

  status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 1, ucComMS523Buf,
                       &unLen);
  if ((status == MI_OK) && (unLen == 0x10))
  {
    *pTagType = ucComMS523Buf[0];
    *(pTagType + 1) = ucComMS523Buf[1];
  }
  else
  {
    status = MI_ERR;
  }

  return status;
}

/*!
    \brief      card anti-collision
    \param[out]  pSnr:serial number,4bytes
    \param[in] none
    \retval     status
*/

int8_t PcdAnticoll(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t *pSnr)
{
  int8_t status;
  uint8_t i, snr_check = 0;
  uint16_t unLen;
  uint8_t ucComMS523Buf[MAXRLEN];

  ClearBitMask(i2c_periph, i2c_addr, Status2Reg, 0x08);
  i2c_byte_write(i2c_periph, i2c_addr, BitFramingReg, 0x00);
  ClearBitMask(i2c_periph, i2c_addr, CollReg, 0x80);

  ucComMS523Buf[0] = PICC_ANTICOLL1;
  ucComMS523Buf[1] = 0x20;

  status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 2, ucComMS523Buf, &unLen);

  if (status == MI_OK)
  {
    for (i = 0; i < 4; i++)
    {
      *(pSnr + i) = ucComMS523Buf[i];
      snr_check ^= ucComMS523Buf[i];
    }
    if (snr_check != ucComMS523Buf[i])
    {
      status = MI_ERR;
    }
  }

  SetBitMask(i2c_periph, i2c_addr, CollReg, 0x80);
  return status;
}
/*!
    \brief      Selected card
    \param[in]  pSnr:serial number,4bytes
    \param[out] none
    \retval     status
*/
int8_t PcdSelect(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t *pSnr)
{
  int8_t status;
  uint8_t i;
  uint16_t unLen;
  uint8_t ucComMS523Buf[MAXRLEN];

  ucComMS523Buf[0] = PICC_ANTICOLL1;
  ucComMS523Buf[1] = 0x70;
  ucComMS523Buf[6] = 0;
  for (i = 0; i < 4; i++)
  {
    ucComMS523Buf[i + 2] = *(pSnr + i);
    ucComMS523Buf[6] ^= *(pSnr + i);
  }
  CalulateCRC(i2c_periph, i2c_addr, ucComMS523Buf, 7, &ucComMS523Buf[7]);

  ClearBitMask(i2c_periph, i2c_addr, Status2Reg, 0x08);

  status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 9, ucComMS523Buf, &unLen);

  if ((status == MI_OK) && (unLen == 0x18))
  {
    status = MI_OK;
  }
  else
  {
    status = MI_ERR;
  }

  return status;
}
/*!
    \brief      Enter sleep mode
    \param[in]  none
    \param[out] none
    \retval     status
*/

int8_t PcdHalt(uint32_t i2c_periph, uint8_t i2c_addr)
{
  uint16_t unLen;
  uint8_t ucComMS523Buf[MAXRLEN];

  ucComMS523Buf[0] = PICC_HALT;
  ucComMS523Buf[1] = 0;
  CalulateCRC(i2c_periph, i2c_addr, ucComMS523Buf, 2, &ucComMS523Buf[2]);

  PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 4, ucComMS523Buf, &unLen);

  return MI_OK;
}
/*!
    \brief      Verify card password
    \param[in]  auth_mode:Password authentication mode
    \param[in]  addr:Block address
    \param[in]  pKey:Password
    \param[in]  pSnr:serial number,4bytes
    \param[out] none
    \retval     status
*/
int8_t PcdAuthState(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t auth_mode, uint8_t addr, uint8_t *pKey, uint8_t *pSnr)
{
  int8_t status;
  uint16_t unLen;
  uint8_t i, ucComMS523Buf[MAXRLEN];
  uint8_t tmp;

  ucComMS523Buf[0] = auth_mode;
  ucComMS523Buf[1] = addr;
  for (i = 0; i < 6; i++)
  {
    ucComMS523Buf[i + 2] = *(pKey + i);
  }
  for (i = 0; i < 4; i++)
  {
    ucComMS523Buf[i + 8] = *(pSnr + i);
  }

  status = PcdComMS523(i2c_periph, i2c_addr, PCD_AUTHENT, ucComMS523Buf, 12, ucComMS523Buf, &unLen);
  i2c_read(i2c_periph, i2c_addr, Status2Reg, &tmp, 1);
  if ((status != MI_OK) || (!(tmp & 0x08)))
  {
    status = MI_ERR;
  }

  return status;
}
/*!
    \brief      read M1 card data of block
    \param[in]  addr:Block address
    \param[out] pData: data
    \retval     status
*/

int8_t PcdRead(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t addr, uint8_t *pData)
{
  int8_t status;
  uint16_t unLen;
  uint8_t i, ucComMS523Buf[MAXRLEN];

  ucComMS523Buf[0] = PICC_READ;
  ucComMS523Buf[1] = addr;
  CalulateCRC(i2c_periph, i2c_addr, ucComMS523Buf, 2, &ucComMS523Buf[2]);

  status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 4, ucComMS523Buf, &unLen);
  if ((status == MI_OK) && (unLen == 0x90))
  //   {   memcpy(pData, ucComMS523Buf, 16);   }
  {
    for (i = 0; i < 16; i++)
    {
      *(pData + i) = ucComMS523Buf[i];
    }
  }
  else
  {
    status = MI_ERR;
  }

  return status;
}
/*!
    \brief      write data of block to M1 card
    \param[in]  addr:Block address
    \param[in] pData: data
    \retval     status
*/
int8_t PcdWrite(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t addr, uint8_t *pData)
{
  int8_t status;
  uint16_t unLen;
  uint8_t i, ucComMS523Buf[MAXRLEN];

  ucComMS523Buf[0] = PICC_WRITE;
  ucComMS523Buf[1] = addr;
  CalulateCRC(i2c_periph, i2c_addr, ucComMS523Buf, 2, &ucComMS523Buf[2]);

  status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 4, ucComMS523Buf, &unLen);

  if ((status != MI_OK) || (unLen != 4) || ((ucComMS523Buf[0] & 0x0F) != 0x0A))
  {
    status = MI_ERR;
  }

  if (status == MI_OK)
  {
    // memcpy(ucComMS523Buf, pData, 16);
    for (i = 0; i < 16; i++)
    {
      ucComMS523Buf[i] = *(pData + i);
    }
    CalulateCRC(i2c_periph, i2c_addr, ucComMS523Buf, 16, &ucComMS523Buf[16]);

    status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 18, ucComMS523Buf, &unLen);
    if ((status != MI_OK) || (unLen != 4) || ((ucComMS523Buf[0] & 0x0F) != 0x0A))
    {
      status = MI_ERR;
    }
  }

  return status;
}
/*!
    \brief      Deduction and recharge
    \param[in]  dd_mode:command
    \param[in]  addr: Wallet address
    \param[in]  pValue: data,4-byte increase (decrease) low order data in front
    \retval     status
*/
int8_t PcdValue(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t dd_mode, uint8_t addr, uint8_t *pValue)
{
  int8_t status;
  uint16_t unLen;
  uint8_t ucComMS523Buf[MAXRLEN];

  ucComMS523Buf[0] = dd_mode;
  ucComMS523Buf[1] = addr;
  CalulateCRC(i2c_periph, i2c_addr, ucComMS523Buf, 2, &ucComMS523Buf[2]);

  status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 4, ucComMS523Buf, &unLen);

  if ((status != MI_OK) || (unLen != 4) || ((ucComMS523Buf[0] & 0x0F) != 0x0A))
  {
    status = MI_ERR;
  }

  if (status == MI_OK)
  {
    memcpy(ucComMS523Buf, pValue, 4);
    //       for (i=0; i<16; i++)
    //       {    ucComMS523Buf[i] = *(pValue+i);   }
    CalulateCRC(i2c_periph, i2c_addr, ucComMS523Buf, 4, &ucComMS523Buf[4]);
    unLen = 0;
    status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 6, ucComMS523Buf, &unLen);
    if (status != MI_ERR)
    {
      status = MI_OK;
    }
  }

  if (status == MI_OK)
  {
    ucComMS523Buf[0] = PICC_TRANSFER;
    ucComMS523Buf[1] = addr;
    CalulateCRC(i2c_periph, i2c_addr, ucComMS523Buf, 2, &ucComMS523Buf[2]);

    status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 4, ucComMS523Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMS523Buf[0] & 0x0F) != 0x0A))
    {
      status = MI_ERR;
    }
  }
  return status;
}
/*!
    \brief      Backup Wallet
    \param[in]  dd_mode:command
    \param[in]  sourceaddr: source address
    \param[in]  goaladdr: Destination address
    \retval     status
*/
int8_t PcdBakValue(uint32_t i2c_periph, uint8_t i2c_addr, uint8_t sourceaddr, uint8_t goaladdr)
{
  int8_t status;
  uint16_t unLen;
  uint8_t ucComMS523Buf[MAXRLEN];

  ucComMS523Buf[0] = PICC_RESTORE;
  ucComMS523Buf[1] = sourceaddr;
  CalulateCRC(i2c_periph, i2c_addr, ucComMS523Buf, 2, &ucComMS523Buf[2]);

  status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 4, ucComMS523Buf, &unLen);

  if ((status != MI_OK) || (unLen != 4) || ((ucComMS523Buf[0] & 0x0F) != 0x0A))
  {
    status = MI_ERR;
  }

  if (status == MI_OK)
  {
    ucComMS523Buf[0] = 0;
    ucComMS523Buf[1] = 0;
    ucComMS523Buf[2] = 0;
    ucComMS523Buf[3] = 0;
    CalulateCRC(i2c_periph, i2c_addr, ucComMS523Buf, 4, &ucComMS523Buf[4]);

    status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 6, ucComMS523Buf, &unLen);
    if (status != MI_ERR)
    {
      status = MI_OK;
    }
  }

  if (status != MI_OK)
  {
    return MI_ERR;
  }

  ucComMS523Buf[0] = PICC_TRANSFER;
  ucComMS523Buf[1] = goaladdr;

  CalulateCRC(i2c_periph, i2c_addr, ucComMS523Buf, 2, &ucComMS523Buf[2]);

  status = PcdComMS523(i2c_periph, i2c_addr, PCD_TRANSCEIVE, ucComMS523Buf, 4, ucComMS523Buf, &unLen);

  if ((status != MI_OK) || (unLen != 4) || ((ucComMS523Buf[0] & 0x0F) != 0x0A))
  {
    status = MI_ERR;
  }

  return status;
}

/*!
   \brief      ms523 data test
   \param[in]  none
   \param[out] none
   \retval     none
*/
uint8_t Type[2];
uint8_t Cardnum[4];
uint8_t PS[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t RdVal[16];

uint8_t MS523_Data_test(uint32_t i2c_periph, uint8_t i2c_addr)
{
  uint8_t i;
  uint8_t sts = 0;
  if (PcdRequest(i2c_periph, i2c_addr, PICC_REQALL, Type) == MI_OK)
  {
    printf("NFC Card search succeeded!\n");
    if (PcdAnticoll(i2c_periph, i2c_addr, Cardnum) == MI_OK)
    {
      printf("card anti-collision succeeded!\n");
      if (PcdSelect(i2c_periph, i2c_addr, Cardnum) == MI_OK)
      {
        printf("card select succeeded!\n");
        if (PcdAuthState(i2c_periph, i2c_addr, PICC_AUTHENT1A, 0, PS, Cardnum) == MI_OK)
        {
          printf("Verify card password succeeded!\n");
          if (PcdRead(i2c_periph, i2c_addr, 0, RdVal) == MI_OK)
          {
            printf("data read succeeded!the data is:\n");
            for (i = 0; i < 16; i++)
            {
              printf("%d", RdVal[i]);
              printf("\n");
            }
            sts = 1;
          }
          else
          {
            printf("data read failed!\n");
          }
        }
        else
        {
          printf("Verify card password failed!\n");
        }
      }
      else
      {
        printf("card select failed!\n");
      }
    }
    else
    {
      printf("card anti-collision failed!\n");
    }
  }
  else
  {
    // printf("Card search failed!\n");
  }

  PcdHalt(i2c_periph, i2c_addr);
  return sts;
}

// 第1块当作钱包块
// 初始金额100元
uint8_t RFID1[16] = {0x64, 0x00, 0x00, 0x00, 0x9B, 0xFF, 0xFF, 0xFF, 0x64, 0x00, 0x00, 0x00, 0x01, 0xFE, 0x01, 0xFE};
uint8_t MS523_Wallet_Init(uint32_t i2c_periph, uint8_t i2c_addr)
{
  uint8_t i;
  uint8_t sts = 0;
  if (PcdRequest(i2c_periph, i2c_addr, PICC_REQALL, Type) == MI_OK)
  {
    if (PcdAnticoll(i2c_periph, i2c_addr, Cardnum) == MI_OK)
    {
      if (PcdSelect(i2c_periph, i2c_addr, Cardnum) == MI_OK)
      {
        if (PcdAuthState(i2c_periph, i2c_addr, PICC_AUTHENT1A, 1, PS, Cardnum) == MI_OK)
        {
          if (PcdWrite(i2c_periph, i2c_addr, 1, RFID1) == MI_OK)
          {
            if (PcdRead(i2c_periph, i2c_addr, 1, RdVal) == MI_OK)
            {
              for (i = 0; i < 16; i++)
              {
                printf("%02x", RdVal[i]);
              }
              printf("\n");
              sts = 1;
            }
          }
        }
      }
    }
    PcdHalt(i2c_periph, i2c_addr);
    return sts;
  }
}
// 连接到卡片
uint8_t MS523_Connect(uint32_t i2c_periph, uint8_t i2c_addr)
{
  uint8_t i;
  uint8_t sts = 0;
  if (PcdRequest(i2c_periph, i2c_addr, PICC_REQALL, Type) == MI_OK)
  {
    printf("NFC Card search succeeded!\n");
    if (PcdAnticoll(i2c_periph, i2c_addr, Cardnum) == MI_OK)
    {
      printf("card anti-collision succeeded!\n");
      if (PcdSelect(i2c_periph, i2c_addr, Cardnum) == MI_OK)
      {
        printf("card select succeeded!\n");
        if (PcdAuthState(i2c_periph, i2c_addr, PICC_AUTHENT1A, 1, PS, Cardnum) == MI_OK)
        {
          printf("Verify card password succeeded!\n");
          sts = 1;
          if (PcdRead(i2c_periph, i2c_addr, 1, RdVal) == MI_OK)
          {
            printf("data read succeeded!the data is:\n");
            for (i = 0; i < 16; i++)
            {
              printf("%02x", RdVal[i]);
            }
            printf("\n");
            sts = 1;
          }
          else
          {
            printf("data read failed!\n");
          }
        }
        else
        {
          printf("Verify card password failed!\n");
        }
      }
      else
      {
        printf("card select failed!\n");
      }
    }
    else
    {
      printf("card anti-collision failed!\n");
    }
  }
  else
  {
    printf("Card search failed!\n");
  }
  return sts;
}
// 一次扣1元
uint8_t decrement[4] = {0x01, 0x00, 0x00, 0x00}; // 扣1元
uint8_t MS523_Decrement(uint32_t i2c_periph, uint8_t i2c_addr, int *value)
{
  uint8_t sts = 0;
  if (PcdRead(i2c_periph, i2c_addr, 1, RdVal) == MI_OK)
  {
    int v = RdVal[0] + RdVal[1] * 256 + RdVal[2] * 256 * 256 + RdVal[3] * 256 * 256 * 256;
    if (v <= 0)
    {
      return 0;
    }
  }
  if (PcdValue(i2c_periph, i2c_addr, PICC_DECREMENT, 1, decrement) == MI_OK)
  {
    if (PcdRead(i2c_periph, i2c_addr, 1, RdVal) == MI_OK)
    {
      sts = 1;
      *value = RdVal[0] + RdVal[1] * 256 + RdVal[2] * 256 * 256 + RdVal[3] * 256 * 256 * 256;
    }
  }
  return sts;
}
