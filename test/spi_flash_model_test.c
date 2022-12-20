/*************************************************************************
 @author:     Andreas Kaeberlein
 @copyright:  Copyright 2022
 @credits:    AKAE

 @license:    BSDv3
 @maintainer: Andreas Kaeberlein
 @email:      andreas.kaeberlein@web.de

 @file:       spi_flash_model_test.c
 @date:       2022-12-18

 @brief:      unit test
              tests spi flash model
*************************************************************************/



/** Includes **/
/* Standard libs */
#include <stdlib.h>     // EXIT codes, malloc
#include <stdio.h>      // f.e. printf
#include <stdint.h>     // defines fixed data types: int8_t...
#include <stddef.h>     // various variable types and macros: size_t, offsetof, NULL, ...
#include <string.h>     // string operation: memset, memcpy
#include <unistd.h>     // system call wrapper functions such as fork, pipe and I/O primitives (read, write, close, etc.).
#include <fcntl.h>      // manipulate file descriptor
#include <ctype.h>      // used for testing and mapping characters
/* Self */
#include "spi_flash_model.h"    // function prototypes



/**
 *  Main
 *  ----
 */
int main ()
{
    /** Variables **/
    t_sfm       spiFlash;   // handle to SPI Flash
    uint8_t     spi[1024];  // spi buffer
    uint32_t    spiLen;     // spiLen


    /* entry message */
    printf("INFO:%s: unit test started\n", __FUNCTION__);

    /* sfcb_init */
    printf("INFO:%s: sfm_init\n", __FUNCTION__);
    if ( 0 != sfm_init( &spiFlash, "W25q16JV" ) ) {
        printf("ERROR:%s:sfm_init\n", __FUNCTION__);
        goto ERO_END;
    }

    /* enable advanced output */
    spiFlash.uint8MsgLevel = 1;

    /* sfm_dump */
    printf("INFO:%s: sfm_dump\n", __FUNCTION__);
    if ( 0 != sfm_dump( &spiFlash, 0, 256 ) ) {
        printf("ERROR:%s:sfm_dump\n", __FUNCTION__);
        goto ERO_END;
    }

    /* sfm: Read Manufacturer / Device ID */
    printf("INFO:%s:sfm: Read Manufacturer / Device ID\n", __FUNCTION__);
    spiLen = 6;
    memset(spi, 0, spiLen);
    spi[0] = 0x90;
    if ( 0 != sfm(&spiFlash, spi, spiLen) ) {
        printf("ERROR:%s:sfm: Read Manufacturer / Device ID\n", __FUNCTION__);
        goto ERO_END;
    }
    if ( (0xef != spi[4]) || (0x14 != spi[5]) ) {
        printf("ERROR:%s:sfm: Wrong ID %02x%02x\n", __FUNCTION__, spi[4], spi[5]);
        goto ERO_END;
    }

    /* sfm: Write Enable */
    printf("INFO:%s:sfm: Write Enable\n", __FUNCTION__);
    spiLen = 1;
    memset(spi, 0, spiLen);
    spi[0] = 0x06;
    if ( 0 != sfm(&spiFlash, spi, spiLen) ) {
        printf("ERROR:%s:sfm: Write Enable\n", __FUNCTION__);
        goto ERO_END;
    }

    /* sfm: Write Disable */
    printf("INFO:%s:sfm: Write Disable\n", __FUNCTION__);
    spiLen = 1;
    memset(spi, 0, spiLen);
    spi[0] = 0x04;
    if ( 0 != sfm(&spiFlash, spi, spiLen) ) {
        printf("ERROR:%s:sfm: Write Disable\n", __FUNCTION__);
        goto ERO_END;
    }

    /* sfm: chip erase */
    printf("INFO:%s:sfm: Chip erase\n", __FUNCTION__);
    spiLen = 1;
    spi[0] = 0x06;
    if ( 0 != sfm(&spiFlash, spi, spiLen) ) {
        printf("ERROR:%s:sfm: Write Enable\n", __FUNCTION__);
        goto ERO_END;
    }
    spi[0] = 0xc7;
    if ( 0 != sfm(&spiFlash, spi, spiLen) ) {
        printf("ERROR:%s:sfm: chip erase\n", __FUNCTION__);
        goto ERO_END;
    }

    /* sfm: Sector erase */
    printf("INFO:%s:sfm: Sector erase\n", __FUNCTION__);
    spiLen = 1;
    spi[0] = 0x06;
    if ( 0 != sfm(&spiFlash, spi, spiLen) ) {
        printf("ERROR:%s:sfm: Write Enable\n", __FUNCTION__);
        goto ERO_END;
    }
    spiLen = 4;
    spi[0] = 0x20;
    spi[1] = 0x1F;  // last sector in flash
    spi[2] = 0xF0;
    spi[3] = 0x10;
    if ( 0 != sfm(&spiFlash, spi, spiLen) ) {
        printf("ERROR:%s:sfm: sector erase\n", __FUNCTION__);
        goto ERO_END;
    }

    /* sfm: Read Status Register */
    printf("INFO:%s:sfm: Read Status Register\n", __FUNCTION__);
    spiLen = 2;
    spi[0] = 0x05;
    if ( 0 != sfm(&spiFlash, spi, spiLen) ) {
        printf("ERROR:%s:sfm: Read Status Register\n", __FUNCTION__);
        goto ERO_END;
    }
    if ( 0 != spi[1] ) {
        printf("ERROR:%s:sfm: Invalid Status Register value\n", __FUNCTION__);
        goto ERO_END;
    }

    /* sfm: Read Data */
    printf("INFO:%s:sfm: Read Data\n", __FUNCTION__);
    spiLen = 6;
    spi[0] = 0x03;  // instruction
    spi[1] = 0x0F;  // address high byte
    spi[2] = 0xFF;  // address middle byte
    spi[3] = 0x00;  // address low byte
    if ( 0 != sfm(&spiFlash, spi, spiLen) ) {
        printf("ERROR:%s:sfm: Read Data\n", __FUNCTION__);
        goto ERO_END;
    }
    if ( (0 != spi[0]) || (0 != spi[1]) || (0 != spi[2]) || (0 != spi[3]) || (0xff != spi[4]) || (0xff != spi[5]) ) {
        printf("ERROR:%s:sfm: Invalid Read Data value\n", __FUNCTION__);
        goto ERO_END;
    }

    /* sfm: Page Program */
    printf("INFO:%s:sfm: Page Program\n", __FUNCTION__);
    spiLen = 1;
    spi[0] = 0x06;
    if ( 0 != sfm(&spiFlash, spi, spiLen) ) {
        printf("ERROR:%s:sfm: Write Enable\n", __FUNCTION__);
        goto ERO_END;
    }
    spiLen = 8;
    spi[0] = 0x02;  // instruction
    spi[1] = 0x00;  // address high byte
    spi[2] = 0x10;  // address middle byte
    spi[3] = 0x20;  // address low byte
    spi[4] = 0x01;  // data
    spi[5] = 0x23;
    spi[6] = 0x45;
    spi[7] = 0x67;
    if ( 0 != sfm(&spiFlash, spi, spiLen) ) {
        printf("ERROR:%s:sfm: Page Program\n", __FUNCTION__);
        goto ERO_END;
    }
    if ( (0x01 != spiFlash.uint8PtrMem[0x1020]) ||
         (0x23 != spiFlash.uint8PtrMem[0x1021]) ||
         (0x45 != spiFlash.uint8PtrMem[0x1022]) ||
         (0x67 != spiFlash.uint8PtrMem[0x1023])
    ) {
        printf("ERROR:%s:sfm: Invalid Read Data value\n", __FUNCTION__);
        goto ERO_END;
    }
    sfm_dump( &spiFlash, 0x1010, 0x1040 );

    /* gracefull end */
    printf("INFO:%s: Module test SUCCESSFUL :-)\n", __FUNCTION__);
    exit(EXIT_SUCCESS);

    /* abnormal end */
    ERO_END:
        printf("FAIL:%s: Module test FAILED :-(\n", __FUNCTION__);
        exit(EXIT_FAILURE);

}
