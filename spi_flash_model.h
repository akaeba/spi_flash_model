/*************************************************************************
 @author:     Andreas Kaeberlein
 @copyright:  Copyright 2022
 @credits:    AKAE

 @license:    BSDv3
 @maintainer: Andreas Kaeberlein
 @email:      andreas.kaeberlein@web.de

 @file:       spi_flash_model.h
 @date:       2022-12-18

 @brief:      spi flash
              spi flash model, input is a spi packet
*************************************************************************/



// Define Guard
#ifndef __SPI_FLASH_MODEL_H
#define __SPI_FLASH_MODEL_H


/**
 * @defgroup SFM_HELP constants
 *
 * Commen used help definitions.
 *
 * @{
 */
#ifndef SFM_WIP_RETRY_IDLE
    #define SFM_WIP_RETRY_IDLE  (3)     /**<  Number of WIP registers poll until after Page write / Erase the SFM is ready for new requests */
#endif
/** @} */



/* C++ compatibility */
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus



/**
 *  @typedef t_sfm_type
 *
 *  @brief  Emulated SPI Flash
 *
 *  Defines Instruction set and topology of emulated flash
 *
 *  @since  December 18, 2022
 *  @author Andreas Kaeberlein
 */
typedef struct t_sfm_type {
    char        charFlashName[15];              /**<  Flash: Name                                       */
    char        charFlashIdHex[20];             /**<  Flash: ID in Ascii-Hex                            */
    uint8_t     uint8FlashIstRdID;              /**<  Flash IST: Read ID                                */
    uint8_t     uint8FlashIstWrEnable;          /**<  Flash IST: write enable                           */
    uint8_t     uint8FlashIstWrDisable;         /**<  Flash IST: write disable                          */
    uint8_t     uint8FlashIstEraseBulk;         /**<  Flash IST: Bulk erase                             */
    uint8_t     uint8FlashIstEraseSector;       /**<  Flash IST: erase smallest possible sector         */
    uint8_t     uint8FlashIstRdStateReg;        /**<  Flash IST: Read status reg                        */
    uint8_t     uint8FlashIstRdData;            /**<  Flash IST: Read data from flash                   */
    uint8_t     uint8FlashIstWrPage;            /**<  Flash IST: Write page                             */
    uint8_t     uint8FlashTopoAdrBytes;         /**<  Flash Topo: Number of address bytes               */
    uint32_t    uint32FlashTopoSectorSizeByte;  /**<  Flash Topo: Flash Sector Size in Byte             */
    uint32_t    uint32FlashTopoPageSizeByte;    /**<  Flash Topo: Flash Page Size in Byte               */
    uint32_t    uint32FlashTopoTotalSizeByte;   /**<  Flash Topo: Total Flash Size in Byte              */
    uint8_t     uint8FlashTopoRdIdDummyByte;    /**<  Flash Topo: Number of Dummy bytes after RD ID IST */
    uint8_t     uint8FlashMngWipMsk;            /**<  Flash MNG: Write-in-progress                      */
    uint8_t     uint8FlashMngWrEnaMsk;          /**<  Flash MNG: Write enable latch, 1: set, 0: clear   */
} t_sfm_type;



/**
 *  @typedef t_sfm
 *
 *  @brief  spi flash model
 *
 *  handle for SPI Flash Model
 *
 *  @since  December 18, 2022
 *  @author Andreas Kaeberlein
 */
typedef struct {
    uint8_t     uint8MsgLevel;              /**<  Message Level, 0: no messages */
    uint8_t*    uint8PtrMem;                /**<  Flash memory, allocated memory corresponds to flash size */
    uint32_t    uint32SelFlash;             /**<  Flash Type is selected */
    uint8_t     uint8StatusReg1;            /**<  Status Register */
    uint8_t     uint8WipRdAfterWriteCnt;    /**<  Number of WIP Flag Reads until new write i spossible, emulates timing behaviour of flash */
} t_sfm;



/**
 *  @brief init
 *
 *  initializes spi flash model
 *
 *  @param[in,out]  self                handle
 *  @param[in]      flashType           name of emulated flash, see #SPI_FLASH
 *  @return         int                 state
 *  @retval         0                   OKAY
 *  @retval         1                   non registered memory selected, add to #SPI_FLASH table
 *  @retval         2                   memory allocation failed
 *  @since          2022-12-19
 *  @author         Andreas Kaeberlein
 */
int sfm_init (t_sfm *self, char flashType[]);



/**
 *  @brief dump
 *
 *  dump flash content to console
 *
 *  @param[in,out]  self                handle
 *  @param[in]      start               start address of dump, aligned to 16byte, -1: default start
 *  @param[in]      stop                stop address of dump, aligned to 16byte, -1: default stop, end of flash
 *  @return         int                 state
 *  @retval         0                   OKAY
 *  @retval         1                   no flash selected
 *  @retval         2                   no memory to dump
 *  @retval         4                   provided addresses out of memory range
 *  @since          2022-12-19
 *  @author         Andreas Kaeberlein
 */
int sfm_dump (t_sfm *self, int32_t start, int32_t stop);



/**
 *  @brief store
 *
 *  stores spi flash memory into file
 *
 *  @param[in,out]  self                handle
 *  @param[in]      fileName            file name for save
 *  @return         int                 state
 *  @retval         0                   OKAY
 *  @retval         1                   no flash selected
 *  @retval         2                   no memory allocated
 *  @retval         4                   no file name provided
 *  @retval         8                   failed to open file
 *  @since          2022-12-25
 *  @author         Andreas Kaeberlein
 */
int sfm_store (t_sfm *self, char fileName[]);



/**
 *  @brief load
 *
 *  loads file into flash
 *
 *  @param[in,out]  self                handle
 *  @param[in]      fileName            file name for save
 *  @return         int                 state
 *  @retval         0                   OKAY
 *  @retval         1                   no flash selected
 *  @retval         2                   no memory allocated
 *  @retval         4                   no file name provided
 *  @retval         8                   failed to open file
 *  @since          2022-12-27
 *  @author         Andreas Kaeberlein
 */
int sfm_load (t_sfm *self, char fileName[]);



/**
 *  @brief compare
 *
 *  compares file with flash meomory content
 *
 *  @param[in,out]  self                handle
 *  @param[in]      fileName            file name for save
 *  @return         int                 state
 *  @retval         0                   SFM and file matches
 *  @retval         1                   no flash selected
 *  @retval         2                   no memory allocated
 *  @retval         4                   no file name provided
 *  @retval         8                   failed to open file
 *  @retval         16                  ´mismatch file/sfm
 *  @since          2022-12-28
 *  @author         Andreas Kaeberlein
 */
int sfm_cmp (t_sfm *self, char fileName[]);



/**
 *  @brief access flash
 *
 *  interacts with flash model
 *
 *  @param[in,out]  self                handle
 *  @param[in]      *spi                spi packet, request and response in same packet
 *  @param[in]      len                 spi packet length
 *  @return         int                 state
 *  @retval         0                   OKAY
 *  @retval         1                   no flash selected
 *  @retval         2                   no flash memory
 *  @retval         4                   unknown instruction
 *  @retval         8                   conversion error
 *  @retval         16                  write enable bit not set
 *  @retval         32                  address out of range
 *  @since          2022-12-19
 *  @author         Andreas Kaeberlein
 */
int sfm (t_sfm *self, uint8_t* spi, uint32_t len);



#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __SPI_FLASH_MODEL_H
