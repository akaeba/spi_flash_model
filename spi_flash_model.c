/*************************************************************************
 @author:     Andreas Kaeberlein
 @copyright:  Copyright 2022
 @credits:    AKAE

 @license:    BSDv3
 @maintainer: Andreas Kaeberlein
 @email:      andreas.kaeberlein@web.de

 @file:       spi_flash_model.c
 @date:       2022-12-18
 @see:        https://github.com/akaeba/spi_flash_model

 @brief:      spi flash
              spi flash model, input is a spi packet
*************************************************************************/



/** Includes **/
/* Standard libs */
#include <stdlib.h>     // EXIT codes, malloc
#include <stdio.h>      // f.e. printf
#include <stdint.h>     // defines fixed data types: int8_t...
#include <stddef.h>     // various variable types and macros: size_t, offsetof, NULL, ...
#include <string.h>     // string operation: memset, memcpy
#include <strings.h>    // strcasecmp
/* Self */
#include "spi_flash_model.h"    // function prototypes
#include "spi_flash_types.h"    // supported spi flashes




/** @brief sfm_asciihex_to_uint8
 *
 *  converts ASCII hex string to array of uint8 values
 *
 *  @param[in]      asciiHex        string for conversion
 *  @param[out]     *vals           to numbers converted string input
 *  @param[out]     *len            number of elements in *vals
 *  @param[in]      max             maximum number of elements in *vals
 *  @return         int             state
 *  @retval         #SFM_OK         OKAY; @see #SFM_E
 *  @retval         #SFM_E_MALLOC   FAIL; @see #SFM_E
 *
 */
static int sfm_asciihex_to_uint8 (const char asciiHex[], uint8_t* vals, uint32_t* len, uint32_t max)
{
    /** Variables **/
    char    hexByte[3];
    int     val;

    /* get length of converted string */
    if ( strlen(asciiHex) > max ) {
        printf("  ERROR:%s: not enough memory\n", __FUNCTION__);
        return SFM_E_MALLOC;
    }
    /* convert */
    *len = 0;
    hexByte[2] = '\0';
    for ( uint32_t i = 0; i < strlen(asciiHex); i += 2 ) {
        memcpy(hexByte, asciiHex+i, 2);
        sscanf(hexByte, "%02x", &val);
        vals[(*len)++] = (uint8_t) (val & 0xff);
    }
    /* finish function */
    return SFM_OK;
}



/** @brief sfm_spi_to_adr
 *
 *  converts spi packet to 32bit address
 *
 *  @param[in]      *vals           spi input packet with flash address
 *  @param[in]      *len            number of bytes in spi packet
 *  @return         uint32_t        32bit flash address
 *
 */
static uint32_t sfm_spi_to_adr (uint8_t* vals, uint8_t len)
{
    /** Variables **/
    uint32_t    adr;

    /* convert to address */
    adr = 0;
    for ( uint8_t i = 0; i < len; i++ ) {
        adr |= (uint32_t) (vals[i] << (len - i - 1) * 8);
    }
    return adr;
}



/** @brief sfm_adr_digits
 *
 *  determine number of digits for full address
 *
 *  @param[in]      adr             spi flash address
 *  @return         uint8_t         number of digits in asciihex converted address
 *
 */
static uint8_t sfm_adr_digits (uint32_t adr)
{
    /** variables **/
    char charHexAdr[10];    // adr converted to hexadecimal address

    /* determine number of digits for hex address */
    charHexAdr[0] = '\0';   // empty string
    snprintf( charHexAdr, sizeof(charHexAdr)/sizeof(charHexAdr[0]), "%x", adr );
    return ((uint8_t) strlen(charHexAdr));  // zero padding at lower addresses
}



/** @brief sfm_write_dif
 *
 *  write buffer to file in dif format, differences to 0xff default are in 16 bytes lines written out
 *
 *  @param[in]      *buf            buffer to write out
 *  @param[in]      len             number of elements in buffer
 *  @param[in]      fileName[]      file name to file
 *  @return         int             state
 *  @retval         #SFM_OK         OKAY; @see #SFM_E
 *  @retval         #SFM_E_ACCESS   FAIL; @see #SFM_E
 *
 */
static int sfm_write_dif (uint8_t *buf, uint32_t len, char fileName[])
{
    /** Variables **/
    char        line[128];          // buffer line
    char        charHex[5];         // hex digit
    uint32_t    i, j;               // iterator
    FILE*       fp;                 // file pointer
    uint8_t     uint8AdrDigits;     // number of address digits

    /* determine number of hex digits for full address */
    uint8AdrDigits = sfm_adr_digits( len );
    if (uint8AdrDigits > 8) {   // limit size according to uint32_t for len
        uint8AdrDigits = 8;
    }
    /* open file for write */
    fp = fopen(fileName, "w+");
    if ( NULL == fp ) {
        return SFM_E_ACCESS;    // failed to open for write
    }
    /* iterate over array in multiples of 16 */
    for ( i = 0; i < len; i += 16 ) {
        /* data write out required? */
        for ( j = 0; j < 16; j++ ) {    // check for non empty fields
            if ( 0xff != buf[i+j] ) {
                break;  // write out data line
            }
        }
        if ( j >= 16 ) {
            continue;   // go one with next 16 data bytes
        }
        /* write out data */
        line[0] = '\0'; // empty line
        snprintf( line, sizeof(line)/sizeof(line[0]), "%0*x:", uint8AdrDigits, i ); // write address to buffer line
        for ( j = 0; j < 16; j++ ) {
            charHex[0] = '\0';  // empty
            snprintf( charHex, sizeof(charHex)/sizeof(charHex[0]), " %02x", buf[i+j] ); // convert to ascii
            strncat( line, charHex, sizeof(line)/sizeof(line[0]) - strlen(line) - 1 );  // overflow save cat
        }
        fprintf( fp, "%s\n", line );    // file write
    }
    fclose(fp); // close file handle
    /* finish function */
    return SFM_OK;
}



/** @brief sfm_read_dif
 *
 *  read file into buffer in dif format
 *
 *  @param[in]      *buf            read in buffer
 *  @param[in]      len             number of elements in buffer
 *  @param[in]      fileName[]      file name to file
 *  @return         int             state
 *  @retval         #SFM_OK         OKAY; @see #SFM_E
 *  @retval         #SFM_E_ACCESS   FAIL; @see #SFM_E
 *
 */
static int sfm_read_dif (uint8_t *buf, uint32_t len, char fileName[])
{
    /** Variables **/
    FILE*       fp;             // file pointer
    char        *line = NULL;   // read buffer line
    char        *sep = NULL;    // separated
    size_t      lineLen = 0;    // number of elements in read buffer
    uint8_t     vals[16];       // read values in line
    uint32_t    adr;            // start address
    uint32_t    i, j;           // iterator
    int         intTemp;        // helper variable for sscanf
    int         intNumChr;      // number read chars

    /* open file for read */
    fp = fopen(fileName, "r");
    if ( NULL == fp ) {
        return SFM_E_ACCESS;    // failed to open for read
    }
    /* make given buffer empty */
    memset(buf, 0xff, len);
    /* read line by line */
    while( -1 != getline(&line, &lineLen, fp) ) {
        /* prepare */
        sep = line; // save anchto for processing
        intNumChr = 0;
        /* extract address */
        sscanf(sep, "%x:%n", &intTemp, &intNumChr);
        sep += intNumChr;   // go one with next element
        adr = (uint32_t) intTemp;
        /* extract data bytes */
        memset(vals, 0xff, sizeof(vals)/sizeof(vals[0]));   // make empty
        for ( i = 0; i < sizeof(vals)/sizeof(vals[0]); i++ ) {
            /* line shorter then expected */
            if ( 0 == strlen(sep) ) {
                break;
            }
            /* get byte */
            sscanf(sep, "%x%n", &intTemp, &intNumChr);
            sep += intNumChr;                       // go one with next element
            vals[i] = (uint8_t) (intTemp & 0xFF);   // cast to byte
        }
        /* check and write to big array */
        if ( adr + i < len ) {
            for ( j = 0; j < i; j++ ) {
                buf[adr+j] = vals[j];
            }
        }
    }
    /* finish function */
    return SFM_OK;
}



/** @brief min
 *
 *  return smaller number
 *
 *  @param[in]      val1            comparison value 1
 *  @param[in]      val2            comparison value 2
 *  @return         uint32_t        bigger number of both inputs
 *
 */
static uint32_t sfm_min_uint32 (uint32_t val1, uint32_t val2)
{
    if ( val1 < val2 ) {
        return val1;
    }
    return val2;
}



/** @brief subtract
 *
 *  overflow save subtraction
 *
 *  @param[in]      minuend         value from which something is removed
 *  @param[in]      subtrahend      the value of subtraction
 *  @return         uint32_t        saturated difference of 'minuend - subtrahend'
 *
 */
static uint32_t sfm_subtract_uint32 (uint32_t minuend, uint32_t subtrahend)
{
    /* underflow */
    if ( subtrahend > minuend ) {
        return (uint32_t) 0;
    }
    /* subtract */
    return (uint32_t) (minuend - subtrahend);
}



/** @brief hexdump
 *
 *  dumps uint8 array to console with 16 values per row
 *
 *  @param[in,out]  *data           uint8 data to printed
 *  @param[in]      start           start address, aligned to multiples of 16
 *  @param[in]      stop            stop address, aligned to multiples of 16
 *  @param[in,out]  rowlead         leading string in each row of dump
 *  @return         uint32_t        saturated difference of 'minuend - subtrahend'
 *
 */
static void sfm_hexdump_uint8 (uint8_t *data, uint32_t start, uint32_t stop, char rowlead[])
{
    /** Variables **/
    uint32_t    uint32Start;
    uint32_t    uint32Stop;
    uint8_t     uint8AdrDigits;

    /* check */
    if ( start > stop ) {
        return;
    }
    /* assign address to multiples of 16 */
    uint32Start = start & (uint32_t) ~0xF;  // start at 0
    uint32Stop = stop | (uint32_t) 0xF;     // stop at 15
    /* leading zeros in adr */
    uint8AdrDigits = sfm_adr_digits( uint32Stop );
    /* dump to console */
    for ( uint32_t i = uint32Start; i < uint32Stop; i += 16 ) {
        /* address */
        printf("%s%0*x: ", rowlead, uint8AdrDigits, i);  // memory address
        /* 16 byte per row */
        for ( uint32_t j = 0; j < 16; j++ ) {
            /* hex number */
            printf("%02x ", data[i+j]);
            /* divide high/low bytes */
            if ( 7 == j ) {
                printf(" ");
            }
        }
        printf("\n");   // new row
    }
    /* end */
    return;
}



/**
 *  sfm_init
 *    initialises spi flash model handle
 */
int sfm_init (t_sfm *self, char flashType[])
{
    /** variables **/
    uint32_t    i;  // iterator

    /* init */
    self->intMsgLevel = 0;                  // no messages
    self->uint8PtrMem = NULL;               // not initialised
    self->flashType = NULL;                 // no  memory selected
    self->uint8StatusReg1 = 0;              // status register
    self->uint8WipRdAfterWriteCnt = 0;      // ready for write access
    /* determine SPI flash by name */
    for ( i = 0; i < sizeof(SPI_FLASH)/sizeof(SPI_FLASH[0]) - 1; i++ ) {
        if ( 0 == strcasecmp(flashType, SPI_FLASH[i].charFlashName) ) { // match
            self->flashType = &SPI_FLASH[i];    // assign selected flash
            break;
        }
    }
    /* no memory found? */
    if ( NULL == self->flashType ) {
        return SFM_E_NO_FLASH;
    }
    /* allocate memory */
    self->uint8PtrMem = (uint8_t*) malloc(self->flashType->uint32FlashTopoTotalSizeByte);
    if ( NULL == self->uint8PtrMem ) {
        return SFM_E_MALLOC;    // memory allocation fail
    }
    /* make memory empty */
    memset(self->uint8PtrMem, 0xff, self->flashType->uint32FlashTopoTotalSizeByte);
    /* finish function */
    return SFM_OK;
}



/**
 *  sfm_dump
 *    dumps flash to console
 */
int sfm_dump (t_sfm *self, int32_t start, int32_t stop)
{
    /** Variables **/
    uint32_t    uint32Start;    // stop address
    uint32_t    uint32Stop;     // start address


    /* Function Call Message */
    if ( 0 != self->intMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* flash type selected */
    if ( NULL == self->flashType ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: no flash selected\n", __FUNCTION__); };
        return SFM_E_NO_FLASH;;
    }

    /* memory allocated */
    if ( NULL == self->uint8PtrMem ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: no memory for flash emulation allocated\n", __FUNCTION__); };
        return SFM_E_MALLOC;
    }

    /* default args */
    uint32Start = (uint32_t) start;
    uint32Stop = (uint32_t) stop;
    if ( 0 > start ) {
        uint32Start = 0;
    }
    if ( 0 > stop ) {
        uint32Stop = self->flashType->uint32FlashTopoTotalSizeByte;
    }

    /* inside address range */
    if ( uint32Stop  > self->flashType->uint32FlashTopoTotalSizeByte - 1 ||
         uint32Start > self->flashType->uint32FlashTopoTotalSizeByte - 1
    ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: flash address out of range\n", __FUNCTION__); }
        return SFM_E_ACCESS;
    }

    /* dump flash to console */
    sfm_hexdump_uint8 (self->uint8PtrMem, uint32Start, uint32Stop, "");

    /* finish function */
    return SFM_OK;
}



/**
 *  sfm_store
 *    stores spi flash memory into file
 *    .dif -> difference to empty flash, full initialized with 0xff
 */
int sfm_store (t_sfm *self, char fileName[])
{
    /** Variables **/
    char*       charPtrFileExt;     // pointer to file extension


    /* Function Call Message */
    if ( 0 != self->intMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* flash type selected */
    if ( NULL == self->flashType ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: no flash selected\n", __FUNCTION__); }
        return SFM_E_NO_FLASH;;
    }

    /* memory allocated */
    if ( NULL == self->uint8PtrMem ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: no memory for flash emulation allocated\n", __FUNCTION__); }
        return SFM_E_MALLOC;
    }

    /* check desired file extension */
    charPtrFileExt = strrchr(fileName, '.') + 1;
    /* no file extension */
    if ( !charPtrFileExt ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: No file name\n", __FUNCTION__); }
        return SFM_E_ACCESS;

    /* dif extension */
    } else if ( 0 == strcasecmp("dif", charPtrFileExt) ) {
        /* entry message */
        if ( 0 != self->intMsgLevel ) { printf("  INFO:%s: '.%s' file type used\n", __FUNCTION__, charPtrFileExt); }
        /* File write */
        if ( 0 != sfm_write_dif ( self->uint8PtrMem,
                                  self->flashType->uint32FlashTopoTotalSizeByte,
                                  fileName
                                )
        ) {
            if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: failed to open file '%s'\n", __FUNCTION__, fileName); }
            return SFM_E_ACCESS;
        }

    /* Unknown file extension */
    } else {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: unsupported file type '%s'\n", __FUNCTION__, charPtrFileExt); }
        return SFM_E_ACCESS;
    }

    /* finish function */
    return SFM_OK;
}



/**
 *  sfm_load
 *    loads file into flash
 *    .dif -> difference to empty flash, full initialised with 0xff
 */
int sfm_load (t_sfm *self, char fileName[])
{
    /** Variables **/
    char*       charPtrFileExt;         // pointer to file extension
    uint8_t*    uint8PtrLdBuf = NULL;   // load buffer


    /* Function Call Message */
    if ( 0 != self->intMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* flash type selected */
    if ( NULL == self->flashType ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: no flash selected\n", __FUNCTION__); }
        return SFM_E_NO_FLASH;;
    }

    /* memory allocated */
    uint8PtrLdBuf = (uint8_t*) malloc(self->flashType->uint32FlashTopoTotalSizeByte);   // allocate intermediate buffer
    if ( (NULL == self->uint8PtrMem) || (NULL == uint8PtrLdBuf) ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: no memory for flash emulation allocated\n", __FUNCTION__); }
        return SFM_E_MALLOC;
    }

    /* check desired file extension */
    charPtrFileExt = strrchr(fileName, '.') + 1;
    /* no file extension */
    if ( !charPtrFileExt ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: No file name\n", __FUNCTION__); }
        return SFM_E_ACCESS;

    /* dif extension */
    } else if ( 0 == strcasecmp("dif", charPtrFileExt) ) {
        /* entry message */
        if ( 0 != self->intMsgLevel ) { printf("  INFO:%s: '.%s' file type used\n", __FUNCTION__, charPtrFileExt); }
        /* File read */
        if ( 0 != sfm_read_dif ( uint8PtrLdBuf,
                                 self->flashType->uint32FlashTopoTotalSizeByte,
                                 fileName
                               )
        ) {
            if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: failed to open file '%s'\n", __FUNCTION__, fileName); }
            return SFM_E_ACCESS;
        }
        /* write back to spi flash */
        memcpy( self->uint8PtrMem, uint8PtrLdBuf, self->flashType->uint32FlashTopoTotalSizeByte );

    /* Unknown file extension */
    } else {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: unsupported file type '%s'\n", __FUNCTION__, charPtrFileExt); }
        return SFM_E_ACCESS;
    }

    /* finish function */
    free(uint8PtrLdBuf);
    return SFM_OK;
}



/**
 *  sfm_cmp
 *    compares file with internal spi buffer
 */
int sfm_cmp (t_sfm *self, char fileName[])
{
    /** Variables **/
    char*       charPtrFileExt;         // pointer to file extension
    uint8_t*    uint8PtrLdBuf = NULL;   // load buffer


    /* Function Call Message */
    if ( 0 != self->intMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* flash type selected */
    if ( NULL == self->flashType ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: no flash selected\n", __FUNCTION__); }
        return SFM_E_NO_FLASH;;
    }

    /* memory allocated */
    uint8PtrLdBuf = (uint8_t*) malloc(self->flashType->uint32FlashTopoTotalSizeByte);   // allocate intermediate buffer
    if ( (NULL == self->uint8PtrMem) || (NULL == uint8PtrLdBuf) ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: no memory for flash emulation allocated\n", __FUNCTION__); }
        return SFM_E_MALLOC;
    }

    /* check desired file extension */
    charPtrFileExt = strrchr(fileName, '.') + 1;
    /* no file extension */
    if ( !charPtrFileExt ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: No file name\n", __FUNCTION__); }
        return SFM_E_ACCESS;

    /* dif extension */
    } else if ( 0 == strcasecmp("dif", charPtrFileExt) ) {
        /* entry message */
        if ( 0 != self->intMsgLevel ) { printf("  INFO:%s: '.%s' file type used\n", __FUNCTION__, charPtrFileExt); }
        /* File read */
        if ( 0 != sfm_read_dif ( uint8PtrLdBuf,
                                 self->flashType->uint32FlashTopoTotalSizeByte,
                                 fileName
                               )
        ) {
            if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: failed to open file '%s'\n", __FUNCTION__, fileName); }
            return SFM_E_ACCESS;
        }

    /* Unknown file extension */
    } else {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: unsupported file type '%s'\n", __FUNCTION__, charPtrFileExt); }
        return SFM_E_ACCESS;
    }

    /* compare memory content */
    for ( uint32_t i = 0; i < self->flashType->uint32FlashTopoTotalSizeByte; i++ ) {
        if ( self->uint8PtrMem[i] != uint8PtrLdBuf[i] ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: mismatch at 0x%x: is=0x%02x, exp=0x%02x\n", __FUNCTION__, i, self->uint8PtrMem[i], uint8PtrLdBuf[i]);
                printf("  ERROR:%s: IS dump\n", __FUNCTION__);
                sfm_hexdump_uint8 (self->uint8PtrMem, sfm_subtract_uint32(i, 16), sfm_min_uint32(i+16, self->flashType->uint32FlashTopoTotalSizeByte), "    ");
                printf("  ERROR:%s: EXP dump\n", __FUNCTION__);
                sfm_hexdump_uint8 (uint8PtrLdBuf, sfm_subtract_uint32(i, 16), sfm_min_uint32(i+16, self->flashType->uint32FlashTopoTotalSizeByte), "    ");
                free(uint8PtrLdBuf);    // free memory
            }
            return SFM_E_CMP;   // mismatch to file
        }
    }

    /* finish function */
    free(uint8PtrLdBuf);
    return SFM_OK;
}



/**
 *  sfm
 *    access SPI Flash Model
 */
int sfm (t_sfm *self, uint8_t* spi, uint32_t len)
{
    /** Variables **/
    uint32_t    uint32ExpLen;   // expected length of packet
    uint32_t    i;              // iterator
    uint32_t    spiCur = 0;     // current spi position
    uint8_t     hexId[10];      // buffer variable for hexID
    uint32_t    hexIdLen;       // length of hex id
    uint32_t    flashAdr;       // address in flash
    uint32_t    flashAdrBase;   // address in flash


    /* Function Call Message */
    if ( 0 != self->intMsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* flash type selected */
    if ( NULL == self->flashType) {
        printf("  ERROR:%s: no flash selected\n", __FUNCTION__);
        return SFM_E_NO_FLASH;;
    }

    /* memory allocated */
    if ( NULL == self->uint8PtrMem ) {
        if ( 0 != self->intMsgLevel ) { printf("  ERROR:%s: no memory for flash emulation allocated\n", __FUNCTION__); }
        return SFM_E_MALLOC;
    }

    /* empty SPI packet */
    if ( 0 == len ) {
        return SFM_OK;
    }

    /* Read Manufacturer / Device ID */
    if ( spi[0] == self->flashType->uint8FlashIstRdID ) {
        /* entry message */
        if ( 0 != self->intMsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Read Manufacturer / Device ID\n", __FUNCTION__, self->flashType->uint8FlashIstRdID);
        }
        /* check length */
        uint32ExpLen = (uint32_t) (1 + self->flashType->uint8FlashTopoRdIdDummyByte + (int) strlen(self->flashType->charFlashIdHex)/2);
        if ( len != uint32ExpLen ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Malformed 'Read Manufacturer / Device ID' instruction, expLen=%d, isLen=%d\n", __FUNCTION__, uint32ExpLen, len);
            }
            return SFM_E_IST_FLASH; // malformed instruction
        }
        /* convert to hex id */
        if ( 0 != sfm_asciihex_to_uint8 (self->flashType->charFlashIdHex, hexId, &hexIdLen, sizeof(hexId)/sizeof(hexId[0])) ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Convert %s\n", __FUNCTION__, self->flashType->charFlashIdHex);
            }
            return SFM_E_MALLOC;
        }
        /* response */
        spiCur = 0;
        spi[spiCur++] = 0;  // first assign and then increment
        for ( i = 0; i < self->flashType->uint8FlashTopoRdIdDummyByte; i++ ) {
            spi[spiCur++] = 0;
        }
        /* copy hex values */
        for ( i = 0; i < hexIdLen; i++ ) {
            spi[spiCur++] = hexId[i];
        }
        /* exit */
        return SFM_OK;

    /* Write Enable (06h) */
    } else if ( spi[0] == self->flashType->uint8FlashIstWrEnable ) {
        /* entry message */
        if ( 0 != self->intMsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Write Enable\n", __FUNCTION__, self->flashType->uint8FlashIstWrEnable);
        }
        /* check length */
        if ( 1 != len ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Malformed 'Write Enable' instruction, expLen=1, isLen=%d\n", __FUNCTION__, len);
            }
            return SFM_E_IST_FLASH; // malformed instruction
        }
        /* set write enable */
        self->uint8StatusReg1 |= self->flashType->uint8FlashMngWrEnaMsk;
        /* spi response */
        memset(spi, 0, len);
        /* exit */
        return SFM_OK;

    /* Write Disable (04h) */
    } else if ( spi[0] == self->flashType->uint8FlashIstWrDisable ) {
        /* entry message */
        if ( 0 != self->intMsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Write Disable\n", __FUNCTION__, self->flashType->uint8FlashIstWrDisable);
        }
        /* check length */
        if ( 1 != len ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Malformed 'Write Disable' instruction, expLen=1, isLen=%d\n", __FUNCTION__, len);
            }
            return SFM_E_IST_FLASH; // malformed instruction
        }
        /* clear write enable */
        self->uint8StatusReg1 &= (uint8_t) ~(self->flashType->uint8FlashMngWrEnaMsk);
        /* spi response */
        memset(spi, 0, len);
        /* exit */
        return SFM_OK;

    /* Chip Erase */
    } else if ( spi[0] == self->flashType->uint8FlashIstEraseBulk ) {
        /* entry message */
        if ( 0 != self->intMsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Chip Erase\n", __FUNCTION__, self->flashType->uint8FlashIstEraseBulk);
        }
        /* check length */
        if ( 1 != len ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Malformed 'Chip Erase' instruction, expLen=1, isLen=%d\n", __FUNCTION__, len);
            }
            return SFM_E_IST_FLASH; // malformed instruction
        }
        /* check for write enable */
        if ( 0 == (self->uint8StatusReg1 & self->flashType->uint8FlashMngWrEnaMsk) ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Chip erase while write protection\n", __FUNCTION__);
            }
            return SFM_E_WP_FLASH;  // write protected
        }
        /* Write in progress? */
        if ( 0 != self->uint8WipRdAfterWriteCnt ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: WIP still in progress, read %i times for write access\n", __FUNCTION__, self->uint8WipRdAfterWriteCnt);
            }
            return SFM_E_WIP_FLASH; // Write in progress
        }
        /* erase */
        memset(self->uint8PtrMem, 0xff, self->flashType->uint32FlashTopoTotalSizeByte);
        /* clear write enable */
        self->uint8StatusReg1 &= (uint8_t) ~(self->flashType->uint8FlashMngWrEnaMsk);
        /* spi response */
        memset(spi, 0, len);
        /* set wait for write in progres */
        self->uint8WipRdAfterWriteCnt = SFM_WIP_RETRY_IDLE;
        /* exit */
        return SFM_OK;

    /* Sector Erase */
    } else if ( spi[0] == self->flashType->uint8FlashIstEraseSector ) {
        /* entry message */
        if ( 0 != self->intMsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Sector Erase\n", __FUNCTION__, self->flashType->uint8FlashIstEraseSector);
        }
        /* check length */
        uint32ExpLen = (uint32_t) (1 + self->flashType->uint8FlashTopoAdrBytes);
        if ( uint32ExpLen != len ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Malformed 'Sector Erase' instruction, expLen=%d, isLen=%d\n", __FUNCTION__, uint32ExpLen, len);
            }
            return SFM_E_IST_FLASH; // malformed instruction
        }
        /* check for write enable */
        if ( 0 == (self->uint8StatusReg1 & self->flashType->uint8FlashMngWrEnaMsk) ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Sector erase while write protection\n", __FUNCTION__);
            }
            return SFM_E_WP_FLASH;  // write protected
        }
        /* Write in progress? */
        if ( 0 != self->uint8WipRdAfterWriteCnt ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: WIP still in progress, read %i times for write access\n", __FUNCTION__, self->uint8WipRdAfterWriteCnt);
            }
            return SFM_E_WIP_FLASH; // Write in progress
        }
        /* assemble address */
        flashAdr = sfm_spi_to_adr (spi+1, self->flashType->uint8FlashTopoAdrBytes);      // spi packet to address
        flashAdr &= (uint32_t) ~(self->flashType->uint32FlashTopoSectorSizeByte - 1);    // align to sector
        /* in memory? */
        if ( self->flashType->uint32FlashTopoTotalSizeByte < flashAdr + self->flashType->uint32FlashTopoSectorSizeByte ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Address (0x%x) exceeds flash size (0x%x)\n", __FUNCTION__, flashAdr, self->flashType->uint32FlashTopoTotalSizeByte);
            }
            return SFM_E_ACCESS;    // address exceeds flash
        }
        /* erase */
        memset(self->uint8PtrMem+flashAdr, 0xff, self->flashType->uint32FlashTopoSectorSizeByte);
        /* clear write enable */
        self->uint8StatusReg1 &= (uint8_t) ~(self->flashType->uint8FlashMngWrEnaMsk);
        /* spi response */
        memset(spi, 0, len);
        /* set wait for write in progres */
        self->uint8WipRdAfterWriteCnt = SFM_WIP_RETRY_IDLE;
        /* exit */
        return SFM_OK;

    /* Read Status Register-1 (05h) */
    } else if ( spi[0] == self->flashType->uint8FlashIstRdStateReg ) {
        /* entry message */
        if ( 0 != self->intMsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Read Status Register\n", __FUNCTION__, self->flashType->uint8FlashIstRdStateReg);
        }
        /* check length */
        if ( 2 != len ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Malformed 'Read Status Register' instruction, expLen=2, isLen=%d\n", __FUNCTION__, len);
            }
            return SFM_E_IST_FLASH; // malformed instruction
        }
        /* state reg 1 has WIP flag */
        if ( 0 < self->uint8WipRdAfterWriteCnt ) {  // no write (erase/page programm) possible, more WIP polls are necessary
            --(self->uint8WipRdAfterWriteCnt);
            self->uint8StatusReg1 |= (uint8_t) (self->flashType->uint8FlashMngWipMsk);  // set WIP
        } else {    // no WIP
            self->uint8StatusReg1 &= (uint8_t) ~(self->flashType->uint8FlashMngWipMsk); // clear WIP flag
        }
        /* response */
        spi[0] = 0;
        spi[1] = self->uint8StatusReg1;
        /* exit */
        return SFM_OK;

    /* Read Data */
    } else if ( spi[0] == self->flashType->uint8FlashIstRdData ) {
        /* entry message */
        if ( 0 != self->intMsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Read Data\n", __FUNCTION__, self->flashType->uint8FlashIstRdData);
        }
        /* check length */
        if ( len < (uint32_t) (self->flashType->uint8FlashTopoAdrBytes + 1)) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Malformed 'Read Data' instruction, expLen>%d, isLen=%d\n", __FUNCTION__, self->flashType->uint8FlashTopoAdrBytes + 1, len);
            }
            return SFM_E_IST_FLASH; // malformed instruction
        }
        /* spi packet to address */
        flashAdr = sfm_spi_to_adr (spi+1, self->flashType->uint8FlashTopoAdrBytes);
        /* clear start of spi packet */
        spiCur = (uint32_t) self->flashType->uint8FlashTopoAdrBytes + 1;
        memset(spi, 0, (size_t) spiCur);
        /* fetch out the data */
        for ( i = spiCur; i < len; i++ ) {
            spi[spiCur++] = self->uint8PtrMem[flashAdr];
            flashAdr++;
            flashAdr &= (uint32_t) (self->flashType->uint32FlashTopoTotalSizeByte - 1);  // address overoll
        }
        /* exit */
        return SFM_OK;

    /* Page Program */
    } else if ( spi[0] == self->flashType->uint8FlashIstWrPage ) {
        /* entry message */
        if ( 0 != self->intMsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Page Program\n", __FUNCTION__, self->flashType->uint8FlashIstWrPage);
        }
        /* check length */
        if ( len < (uint32_t) (self->flashType->uint8FlashTopoAdrBytes + 1)) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Malformed 'Page Program' instruction, expLen>%d, isLen=%d\n", __FUNCTION__, self->flashType->uint8FlashTopoAdrBytes + 1, len);
            }
            return SFM_E_IST_FLASH; // malformed instruction
        }
        /* check for write enable */
        if ( 0 == (self->uint8StatusReg1 & self->flashType->uint8FlashMngWrEnaMsk) ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: Page Program while write protection\n", __FUNCTION__);
            }
            return SFM_E_WP_FLASH;  // write protected
        }
        /* Write in progress? */
        if ( 0 != self->uint8WipRdAfterWriteCnt ) {
            if ( 0 != self->intMsgLevel ) {
                printf("  ERROR:%s: WIP still in progress, read %i times for write access\n", __FUNCTION__, self->uint8WipRdAfterWriteCnt);
            }
            return SFM_E_WIP_FLASH; // Write in progress
        }
        /* spi packet to address */
        flashAdr     = sfm_spi_to_adr (spi+1, self->flashType->uint8FlashTopoAdrBytes);
        flashAdrBase = flashAdr;
        flashAdrBase &= (uint32_t) ~(self->flashType->uint32FlashTopoPageSizeByte - 1);  // base address, aligned to pages
        flashAdr     &= (uint32_t) self->flashType->uint32FlashTopoPageSizeByte - 1;     // in page address
        /* clear start of spi packet */
        spiCur = (uint32_t) self->flashType->uint8FlashTopoAdrBytes + 1;
        memset(spi, 0, (size_t) spiCur);
        /* page write */
        for ( i = spiCur; i < len; i++ ) {
            self->uint8PtrMem[flashAdrBase+flashAdr] &= spi[i]; // in flash can only bits swapped from 1s -> 0s, otherwise erase
            flashAdr++;
            flashAdr &= (uint32_t) self->flashType->uint32FlashTopoPageSizeByte - 1; // page overroll
        }
        /* set wait for write in progres */
        self->uint8WipRdAfterWriteCnt = SFM_WIP_RETRY_IDLE;
        /* exit */
        return SFM_OK;

    /* default */
    } else {
        if ( 0 != self->intMsgLevel ) {
            printf("  ERROR:%s: Unknown Instruction '0x%02x'\n", __FUNCTION__, spi[0]);
        };
        return SFM_E_IST_FLASH; // malformed instruction
    }

    /* finish function */
    return SFM_OK;
}
