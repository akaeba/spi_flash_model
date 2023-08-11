/*************************************************************************
 @author:     Andreas Kaeberlein
 @copyright:  Copyright 2022
 @credits:    AKAE

 @license:    BSDv3
 @maintainer: Andreas Kaeberlein
 @email:      andreas.kaeberlein@web.de

 @file:       spi_flash_types.h
 @date:       2022-12-18
 @see:        https://github.com/akaeba/spi_flash_model

 @brief:      flash type definition table

*************************************************************************/



//--------------------------------------------------------------
// Define Guard
//--------------------------------------------------------------
#ifndef __SPI_FLASH_TYPES_H
#define __SPI_FLASH_TYPES_H


/* C++ compatibility */
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus



/** Look up table **/
/* Supported Flash Types */
const struct t_sfm_type SPI_FLASH[] = {
  /* W25Q16JV
   *   @see https://www.winbond.com/resource-files/w25q16jv%20spi%20revh%2004082019%20plus.pdf
   */
  {
    {"W25Q16JV"},   // charFlashName[15]
    {"ef14"},       // charFlashIdHex                       W25Q16JV_Rev_H      p.19, Manufacturer and Device Identification
    0x90,           // uint8FlashIstRdID                    W25Q16JV_Rev_H      p.44, Read Manufacturer / Device ID (90h)
    0x06,           // uint8FlashIstWrEnable                W25Q16JV_Rev_H      p.22, Write Enable (06h)
    0x04,           // uint8FlashIstWrDisable               W25Q16JV_Rev_H      p.23, Write Disable (04h)
    0xc7,           // uint8FlashIstEraseBulk               W25Q16JV_Rev_H      p.38, Chip Erase (C7h / 60h)
    0x20,           // uint8FlashIstEraseSector             W25Q16JV_Rev_H      p.35, Sector Erase (20h)
    0x05,           // uint8FlashIstRdStateReg              W25Q16JV_Rev_H      p.23, Read Status Register-1 (05h)
    0x03,           // uint8FlashIstRdData                  W25Q16JV_Rev_H      p.26, Read Data (03h)
    0x02,           // uint8FlashIstWrPage                  W25Q16JV_Rev_H      p.33, Page Program (02h)
    3,              // uint8FlashTopoAdrBytes
    4096,           // uint32FlashTopoSectorSizeByte
    256,            // uint32FlashTopoPageSizeByte
    2097152,        // uint32FlashTopoTotalSizeByte
    3,              // uint8FlashTopoRdIdDummyByte          W25Q16JV_Rev_H      p.44, Read Manufacturer / Device ID (90h)
    0x01,           // uint8FlashMngWipMsk
    0x02,           // uint8FlashMngWrEnaMsk
  },

  /* add new entry here ... */

  /* Protection entry */
  {
    {""},   // charFlashName
    {""},   // charFlashIdHex
    0,      // uint8FlashIstRdID
    0,      // uint8FlashIstWrEnable
    0,      // uint8FlashIstWrDisable
    0,      // uint8FlashIstEraseBulk
    0,      // uint8FlashIstEraseSector
    0,      // uint8FlashIstRdStateReg
    0,      // uint8FlashIstRdData
    0,      // uint8FlashIstWrPage
    0,      // uint8FlashTopoAdrBytes
    0,      // uint32FlashTopoSectorSizeByte
    0,      // uint32FlashTopoPageSizeByte
    0,      // uint32FlashTopoTotalSizeByte
    0,      // uint8FlashTopoRdIdDummyByte
    0,      // uint8FlashMngWipMsk
    0,      // uint8FlashMngWrEnaMsk
  }
};


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __SPI_FLASH_TYPES_H
