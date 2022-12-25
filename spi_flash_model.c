/*************************************************************************
 @author:     Andreas Kaeberlein
 @copyright:  Copyright 2022
 @credits:    AKAE

 @license:    BSDv3
 @maintainer: Andreas Kaeberlein
 @email:      andreas.kaeberlein@web.de

 @file:       spi_flash_model.c
 @date:       2022-12-18

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



/**
 *  sfm_asciihex_to_uint8
 *    converts ascii hex string to array of uint8 values
 */
static int sfm_asciihex_to_uint8 (const char asciiHex[], uint8_t* vals, uint32_t* len, uint32_t max)
{
    /** Variables **/
    char    hexByte[3];
    int     val;

    /* get length of converted string */
    if ( strlen(asciiHex) > max ) {
        printf("  ERROR:%s: not enough memory\n", __FUNCTION__);
        return -1;
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
    return 0;
}



/**
 *  sfm_spi_to_adr
 *    converts spi packet to 32bit address
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



/**
 *  sfm_adr_digits
 *    determine number of digits for full address
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



/**
 *  sfm_init
 *    initializes spi flash model handle
 */
int sfm_init (t_sfm *self, char flashType[])
{
    /** variables **/
    uint32_t    i;  // iterator

    /* init */
    self->uint8MsgLevel = 0;                // no messages
    self->uint8PtrMem = NULL;               // not initialized
    self->uint32SelFlash = (uint32_t) ~0;   // make invalid
    self->uint8StatusReg1 = 0;              // status register
    /* determine SPI flash by name */
    for ( i = 0; i < sizeof(SPI_FLASH)/sizeof(SPI_FLASH[0]) - 1; i++ ) {
        if ( 0 == strcasecmp(flashType, SPI_FLASH[i].charFlashName) ) { // match
            self->uint32SelFlash = i;
            break;
        }
    }
    /* found? */
    if ( i >= (sizeof(SPI_FLASH)/sizeof(SPI_FLASH[0]) - 1) ) {
        return 1;   // no memory found
    }
    /* allocate memory */
    self->uint8PtrMem = (uint8_t*) malloc(SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte);
    if ( NULL == self->uint8PtrMem ) {
        return 2;   // memory allocation fail
    }
    /* make memory empty */
    memset(self->uint8PtrMem, 0xff, SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte);
    /* finish function */
    return 0;
}



/**
 *  sfm_dump
 *    dumps flash to console
 */
int sfm_dump (t_sfm *self, int32_t start, int32_t stop)
{
    /** Variables **/
    uint8_t     uint8AdrDigits; // digits of hex address
    uint32_t    uint32Start;    // stop address
    uint32_t    uint32Stop;     // start address


    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* flash type selected */
    if ( (uint32_t) ~0 == self->uint32SelFlash ) {
        printf("  ERROR:%s: no flash selected\n", __FUNCTION__);
        return 1;
    }

    /* memory allocated */
    if ( NULL == self->uint8PtrMem ) {
        printf("  ERROR:%s: no flash memory allocated\n", __FUNCTION__);
        return 2;
    }

    /* determine number of hex digits for full address */
    uint8AdrDigits = sfm_adr_digits( SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte );
    if ( 0 != self->uint8MsgLevel ) {
        printf("  INFO:%s: uint8AdrDigits = %d\n", __FUNCTION__, uint8AdrDigits);
    }

    /* process input argguments */
    uint32Start = (uint32_t) start;
    uint32Stop = (uint32_t) stop;
    if ( 0 > start ) {
        uint32Start = 0;
    }
    if ( 0 > stop ) {
        uint32Stop = SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte;
    }
    uint32Start = uint32Start & (uint32_t) ~0xF;    // align to multiples of 16
    uint32Stop = uint32Stop & (uint32_t) ~0xF;      // multiples of 16
    if ( uint32Stop  > SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte - 1 ||
         uint32Start > SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte - 1
    ) {
        printf("  ERROR:%s: flash address out of range\n", __FUNCTION__);
        return 4;
    }

    /* dump flash to console */
    for ( uint32_t i = uint32Start; i < uint32Stop; i += 16 ) {
        /* address */
        printf("%0*x:   ", uint8AdrDigits, i);  // memory address
        /* 16 byte per row */
        for ( uint32_t j = 0; j < 16; j++ ) {
            /* hex number */
            printf("%02x ", self->uint8PtrMem[i+j]);
            /* divide high/low bytes */
            if ( 7 == j ) {
                printf(" ");
            }
        }
        printf("\n");   // new row
    }

    /* finish function */
    return 0;
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
    char        line[128];          // buffer line
    char        charHex[5];         // hex digit
    uint32_t    i, j;               // iterator
    FILE*       fp;                 // file pointer
    uint8_t     uint8AdrDigits;     // number of address digits


    /* Function Call Message */
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* flash type selected */
    if ( (uint32_t) ~0 == self->uint32SelFlash ) {
        printf("  ERROR:%s: no flash selected\n", __FUNCTION__);
        return 1;
    }

    /* memory allocated */
    if ( NULL == self->uint8PtrMem ) {
        printf("  ERROR:%s: no flash memory allocated\n", __FUNCTION__);
        return 2;
    }

    /* determine number of digits for address output */
    uint8AdrDigits = sfm_adr_digits( SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte );

    /* check desired file extension */
    charPtrFileExt = strrchr(fileName, '.') + 1;
    /* no file extsnion */
    if ( !charPtrFileExt ) {
        printf("  ERROR:%s: No file name\n", __FUNCTION__);
        return 4;

    /* dif extension */
    } else if ( 0 == strcasecmp("dif", charPtrFileExt) ) {
        /* entry message */
        printf("  INFO:%s: '.%s' file type used\n", __FUNCTION__, charPtrFileExt);
        /* open file for write */
        fp = fopen(fileName, "w+");
        if ( NULL == fp ) {
            printf("  ERROR:%s: failed to open file '%s'\n", __FUNCTION__, fileName);
            return 8;
        }
        /* iterate over array in multiples of 16 */
        for ( i = 0; i < SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte; i += 16 ) {
            /* data write out required? */
            for ( j = 0; j < 16; j++ ) {    // check for non empty fields
                if ( 0xff != self->uint8PtrMem[i+j] ) {
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
                snprintf( charHex, sizeof(charHex)/sizeof(charHex[0]), " %02x", self->uint8PtrMem[i+j] );   // convert to ascii
                strncat( line, charHex, sizeof(line)/sizeof(line[0]) - strlen(line) - 1 );  // overflow save cat
            }
            fprintf( fp, "%s\n", line );    // file write
        }
        fclose(fp); // close file handle

    /* Unknown file extension */
    } else {
        printf("  ERROR:%s: unsuppored file type '%s'\n", __FUNCTION__, charPtrFileExt);
        return 8;
    }

    /* finish function */
    return 0;
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
    if ( 0 != self->uint8MsgLevel ) { printf("__FUNCTION__ = %s\n", __FUNCTION__); };

    /* flash type selected */
    if ( (uint32_t) ~0 == self->uint32SelFlash ) {
        printf("  ERROR:%s: no flash selected\n", __FUNCTION__);
        return 1;
    }

    /* memory allocated */
    if ( NULL == self->uint8PtrMem ) {
        printf("  ERROR:%s: no flash memory allocated\n", __FUNCTION__);
        return 2;
    }

    /* empty SPI packet */
    if ( 0 == len ) {
        return 0;
    }

    /* Read Manufacturer / Device ID */
    if ( spi[0] == SPI_FLASH[self->uint32SelFlash].uint8FlashIstRdID ) {
        /* entry message */
        if ( 0 != self->uint8MsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Read Manufacturer / Device ID\n", __FUNCTION__, SPI_FLASH[self->uint32SelFlash].uint8FlashIstRdID);
        }
        /* check length */
        uint32ExpLen = (uint32_t) (1 + SPI_FLASH[self->uint32SelFlash].uint8FlashTopoRdIdDummyByte + (int) strlen(SPI_FLASH[self->uint32SelFlash].charFlashIdHex)/2);
        if ( len != uint32ExpLen ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Malformed 'Read Manufacturer / Device ID' instruction, expLen=%d, isLen=%d\n", __FUNCTION__, uint32ExpLen, len);
            }
            return 4;   // malformed instruction
        }
        /* convert to hex id */
        if ( 0!= sfm_asciihex_to_uint8 (SPI_FLASH[self->uint32SelFlash].charFlashIdHex, hexId, &hexIdLen, sizeof(hexId)/sizeof(hexId[0])) ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Convert %s\n", __FUNCTION__, SPI_FLASH[self->uint32SelFlash].charFlashIdHex);
            }
            return 8;
        }
        /* response */
        spiCur = 0;
        spi[spiCur++] = 0;  // first assign and then increment
        for ( i = 0; i < SPI_FLASH[self->uint32SelFlash].uint8FlashTopoRdIdDummyByte; i++ ) {
            spi[spiCur++] = 0;
        }
        /* copy hex values */
        for ( i = 0; i < hexIdLen; i++ ) {
            spi[spiCur++] = hexId[i];
        }
        /* exit */
        return 0;

    /* Write Enable (06h) */
    } else if ( spi[0] == SPI_FLASH[self->uint32SelFlash].uint8FlashIstWrEnable ) {
        /* entry message */
        if ( 0 != self->uint8MsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Write Enable\n", __FUNCTION__, SPI_FLASH[self->uint32SelFlash].uint8FlashIstWrEnable);
        }
        /* check length */
        if ( 1 != len ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Malformed 'Write Enable' instruction, expLen=1, isLen=%d\n", __FUNCTION__, len);
            }
            return 4;   // malformed instruction
        }
        /* set write enable */
        self->uint8StatusReg1 |= SPI_FLASH[self->uint32SelFlash].uint8FlashMngWrEnaMsk;
        /* spi response */
        memset(spi, 0, len);
        /* exit */
        return 0;

    /* Write Disable (04h) */
    } else if ( spi[0] == SPI_FLASH[self->uint32SelFlash].uint8FlashIstWrDisable ) {
        /* entry message */
        if ( 0 != self->uint8MsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Write Disable\n", __FUNCTION__, SPI_FLASH[self->uint32SelFlash].uint8FlashIstWrDisable);
        }
        /* check length */
        if ( 1 != len ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Malformed 'Write Disable' instruction, expLen=1, isLen=%d\n", __FUNCTION__, len);
            }
            return 4;   // malformed instruction
        }
        /* clear write enable */
        self->uint8StatusReg1 &= (uint8_t) ~(SPI_FLASH[self->uint32SelFlash].uint8FlashMngWrEnaMsk);
        /* spi response */
        memset(spi, 0, len);
        /* exit */
        return 0;

    /* Chip Erase */
    } else if ( spi[0] == SPI_FLASH[self->uint32SelFlash].uint8FlashIstEraseBulk ) {
        /* entry message */
        if ( 0 != self->uint8MsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Chip Erase\n", __FUNCTION__, SPI_FLASH[self->uint32SelFlash].uint8FlashIstEraseBulk);
        }
        /* check length */
        if ( 1 != len ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Malformed 'Chip Erase' instruction, expLen=1, isLen=%d\n", __FUNCTION__, len);
            }
            return 4;   // malformed instruction
        }
        /* check for write enable */
        if ( 0 == (self->uint8StatusReg1 & SPI_FLASH[self->uint32SelFlash].uint8FlashMngWrEnaMsk) ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Chip erase while write protection\n", __FUNCTION__);
            }
            return 16;  // write protected
        }
        /* erase */
        memset(self->uint8PtrMem, 0xff, SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte);
        /* clear write enable */
        self->uint8StatusReg1 &= (uint8_t) ~(SPI_FLASH[self->uint32SelFlash].uint8FlashMngWrEnaMsk);
        /* spi response */
        memset(spi, 0, len);
        /* exit */
        return 0;

    /* Sector Erase */
    } else if ( spi[0] == SPI_FLASH[self->uint32SelFlash].uint8FlashIstEraseSector ) {
        /* entry message */
        if ( 0 != self->uint8MsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Sector Erase\n", __FUNCTION__, SPI_FLASH[self->uint32SelFlash].uint8FlashIstEraseSector);
        }
        /* check length */
        uint32ExpLen = (uint32_t) (1 + SPI_FLASH[self->uint32SelFlash].uint8FlashTopoAdrBytes);
        if ( uint32ExpLen != len ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Malformed 'Sector Erase' instruction, expLen=%d, isLen=%d\n", __FUNCTION__, uint32ExpLen, len);
            }
            return 4;   // malformed instruction
        }
        /* check for write enable */
        if ( 0 == (self->uint8StatusReg1 & SPI_FLASH[self->uint32SelFlash].uint8FlashMngWrEnaMsk) ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Sector erase while write protection\n", __FUNCTION__);
            }
            return 16;  // write protected
        }
        /* assemble address */
        flashAdr = sfm_spi_to_adr (spi+1, SPI_FLASH[self->uint32SelFlash].uint8FlashTopoAdrBytes);      // spi packet to address
        flashAdr &= (uint32_t) ~(SPI_FLASH[self->uint32SelFlash].uint32FlashTopoSectorSizeByte - 1);    // allign to sector
        /* in memory? */
        if ( SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte < flashAdr + SPI_FLASH[self->uint32SelFlash].uint32FlashTopoSectorSizeByte ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Address (0x%x) exceeds flash size (0x%x)\n", __FUNCTION__, flashAdr, SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte);
            }
            return 32;  // address exceeds flash
        }
        /* erase */
        memset(self->uint8PtrMem+flashAdr, 0xff, SPI_FLASH[self->uint32SelFlash].uint32FlashTopoSectorSizeByte);
        /* clear write enable */
        self->uint8StatusReg1 &= (uint8_t) ~(SPI_FLASH[self->uint32SelFlash].uint8FlashMngWrEnaMsk);
        /* spi response */
        memset(spi, 0, len);
        /* exit */
        return 0;

    /* Read Status Register-1 (05h) */
    } else if ( spi[0] == SPI_FLASH[self->uint32SelFlash].uint8FlashIstRdStateReg ) {
        /* entry message */
        if ( 0 != self->uint8MsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Read Status Register\n", __FUNCTION__, SPI_FLASH[self->uint32SelFlash].uint8FlashIstRdStateReg);
        }
        /* check length */
        if ( 2 != len ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Malformed 'Read Status Register' instruction, expLen=2, isLen=%d\n", __FUNCTION__, len);
            }
            return 4;   // malformed instruction
        }
        /* response */
        spi[0] = 0;
        spi[1] = self->uint8StatusReg1;
        /* exit */
        return 0;

    /* Read Data */
    } else if ( spi[0] == SPI_FLASH[self->uint32SelFlash].uint8FlashIstRdData ) {
        /* entry message */
        if ( 0 != self->uint8MsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Read Data\n", __FUNCTION__, SPI_FLASH[self->uint32SelFlash].uint8FlashIstRdData);
        }
        /* check length */
        if ( len < (uint32_t) (SPI_FLASH[self->uint32SelFlash].uint8FlashTopoAdrBytes + 1)) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Malformed 'Read Data' instruction, expLen>%d, isLen=%d\n", __FUNCTION__, SPI_FLASH[self->uint32SelFlash].uint8FlashTopoAdrBytes + 1, len);
            }
            return 4;   // malformed instruction
        }
        /* spi packet to address */
        flashAdr = sfm_spi_to_adr (spi+1, SPI_FLASH[self->uint32SelFlash].uint8FlashTopoAdrBytes);
        /* clear start of spi packet */
        spiCur = (uint32_t) SPI_FLASH[self->uint32SelFlash].uint8FlashTopoAdrBytes + 1;
        memset(spi, 0, (size_t) spiCur);
        /* fetch out the data */
        for ( i = spiCur; i < len; i++ ) {
            spi[spiCur++] = self->uint8PtrMem[flashAdr];
            flashAdr++;
            flashAdr &= (uint32_t) (SPI_FLASH[self->uint32SelFlash].uint32FlashTopoTotalSizeByte - 1);  // address overoll
        }
        /* exit */
        return 0;

    /* Page Program */
    } else if ( spi[0] == SPI_FLASH[self->uint32SelFlash].uint8FlashIstWrPage ) {
        /* entry message */
        if ( 0 != self->uint8MsgLevel ) {
            printf("  INFO:%s: IST=0x%02x, Page Program\n", __FUNCTION__, SPI_FLASH[self->uint32SelFlash].uint8FlashIstWrPage);
        }
        /* check length */
        if ( len < (uint32_t) (SPI_FLASH[self->uint32SelFlash].uint8FlashTopoAdrBytes + 1)) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Malformed 'Page Program' instruction, expLen>%d, isLen=%d\n", __FUNCTION__, SPI_FLASH[self->uint32SelFlash].uint8FlashTopoAdrBytes + 1, len);
            }
            return 4;   // malformed instruction
        }
        /* check for write enable */
        if ( 0 == (self->uint8StatusReg1 & SPI_FLASH[self->uint32SelFlash].uint8FlashMngWrEnaMsk) ) {
            if ( 0 != self->uint8MsgLevel ) {
                printf("  ERROR:%s: Page Program while write protection\n", __FUNCTION__);
            }
            return 16;  // write protected
        }
        /* spi packet to address */
        flashAdr     = sfm_spi_to_adr (spi+1, SPI_FLASH[self->uint32SelFlash].uint8FlashTopoAdrBytes);
        flashAdrBase = flashAdr;
        flashAdrBase &= (uint32_t) ~(SPI_FLASH[self->uint32SelFlash].uint32FlashTopoPageSizeByte - 1);  // base address, aligned to pages
        flashAdr     &= (uint32_t) SPI_FLASH[self->uint32SelFlash].uint32FlashTopoPageSizeByte - 1;     // in page address
        /* clear start of spi packet */
        spiCur = (uint32_t) SPI_FLASH[self->uint32SelFlash].uint8FlashTopoAdrBytes + 1;
        memset(spi, 0, (size_t) spiCur);
        for ( i = spiCur; i < len; i++ ) {
            self->uint8PtrMem[flashAdrBase+flashAdr] = spi[i];
            flashAdr++;
            flashAdr &= (uint32_t) SPI_FLASH[self->uint32SelFlash].uint32FlashTopoPageSizeByte - 1; // page overroll
        }
        /* exit */
        return 0;

    /* default */
    } else {
        if ( 0 != self->uint8MsgLevel ) {
            printf("  ERROR:%s: Unknown Instruction '0x%02x'\n", __FUNCTION__, spi[0]);
        };
        return 4;   // malformed instruction
    }

    /* finish function */
    return 0;
}
