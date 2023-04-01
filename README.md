[![Test](https://github.com/akaeba/spi_flash_model/workflows/test/badge.svg)](https://github.com/akaeba/spi_flash_model/actions/workflows/test.yml) [![Doxygen](https://github.com/akaeba/spi_flash_model/workflows/doxygen/badge.svg)](https://github.com/akaeba/spi_flash_model/actions/workflows/doxygen.yml)

- [SPI-Flash-Model](#spi-flash-model)
  * [Features](#features)
    + [Limits](#limits)
  * [Releases](#releases)
  * [Emulated Flash](#emulated-flash)
  * [How to use](#how-to-use)
    + [API](#api)
      - [Init](#init)
      - [Dump](#dump)
      - [Store](#store)
      - [Load](#load)
      - [Compare](#compare)
      - [SFM (Spi Flash Model)](#SFM)
    + [Example](#example)
  * [References](#references)


# SPI-Flash-Model

Arbitrary SPI Flash C model. With the scope of an SPI Flash driver unit test emulates this module SPI Flashes on logic level.


## Features

The [model](./spi_flash_model.c) emulates an SPI Flash on logic level. As input are SPI packets used. In a real
world application would be sent this SPI packet to the physically SPI core instead.


### Limits

* No timing behavior
    * emulated with [WIP](https://github.com/akaeba/spi_flash_model/blob/main/spi_flash_model.h#L32) poll constant


## Releases

| Version                                                         | Date       | Source                                                                                              | Change log                                              |
| --------------------------------------------------------------- | ---------- | --------------------------------------------------------------------------------------------------- | ------------------------------------------------------- |
| latest                                                          |            | <a id="raw-url" href="https://github.com/akaeba/spi_flash_model/archive/master.zip ">latest.zip</a> |                                                         |
| [v0.1.0](https://github.com/akaeba/spi_flash_model/tree/v0.1.0) | tbd.       | <a id="raw-url" href="https://github.com/akaeba/spi_flash_model/archive/v0.1.0.zip ">v0.1.0.zip</a> | initial draft                                           |


## [Emulated Flash](./spi_flash_types.h)

* [W25Q16JV](https://www.winbond.com/resource-files/w25q16jv%20spi%20revh%2004082019%20plus.pdf)


## How to use

The [unittest](./test/spi_flash_model_test.c) gives a good flavor how to use the model.


### API

To interact with the _sfm_ (SPI Flash Model) are the following functions available.


#### Init

Initializes the SFM and selects the emulated flash.

```c
int sfm_init (t_sfm *self, char flashType[]);
```


#### Dump

Dumps Flash memory in hex values to console. Setting of start/stop ```-1``` will print whole
memory content to console.

```c
int sfm_dump (t_sfm *self, int32_t start, int32_t stop);
```


#### Store

Stores flash model internal data buffer as file. Supported formats:
* [.dif](./test/flash_read.dif) : difference to empty flash ```0xff``` in ascii-hex format

```c
int sfm_store (t_sfm *self, char fileName[]);
```


#### Load

Restore flash model internal data buffer from File. Supported formats:
* [.dif](./test/flash_read.dif) : difference to empty flash ```0xff``` in ascii-hex format

```c
int sfm_load (t_sfm *self, char fileName[]);
```

#### Compare

Compares SFM internal flash buffer with file. Supported formats:
* [.dif](./test/flash_read.dif) : difference to empty flash ```0xff``` in ascii-hex format

```c
int sfm_cmp (t_sfm *self, char fileName[]);
```


#### SFM

Access SPI Flash memory. SPI request and response are placed in the same SPI buffer variable.

```c
int sfm (t_sfm *self, uint8_t* spi, uint32_t len);
```


### Example

The ```c``` snippet below shows an minimal example to interact with the _sfm_. The variable _spi_ represents
the packet sent to the SPI flash.

```c
#include <stdlib.h> // EXIT codes, malloc
#include <stdio.h>  // f.e. printf
#include <stdint.h> // defines fixed data types: int8_t...
#include "spi_flash_model.h"  // function prototypes

int main ()
{
  /* variables */
  uint8_t spi[10];  // spi packet to interact with sfm
  t_sfm   spiFlash; // handle to SPI Flash

  /* define used flash model */
  sfm_init (&spiFlash, "W25Q16JV");

  /* write enable */
  spi[0] = 0x06;            // W25Q16JV: write enable instruction
  sfm (&spiFlash, spi, 1);  // access flash model

  /* write page */
  spi[0] = 0x02;  // W25Q16JV: write page instruction
  spi[1] = 0x00;  // address high byte
  spi[2] = 0x00;  // address middle byte
  spi[3] = 0x00;  // address low byte
  spi[4] = 0x01;  // data
  spi[5] = 0x23;
  spi[6] = 0x45;
  spi[7] = 0x67;
  spi[8] = 0x89;
  spi[9] = 0xAB;
  sfm (&spiFlash, spi, 10); // access flash model
    // poll for WIP
  for ( uint8_t i = 0; i < SFM_WIP_RETRY_IDLE; i++ ) {
    spiLen = 2;
    spi[0] = 0x05;
    sfm(&spiFlash, spi, spiLen);  // read state reg, needed for WIP poll
  }

  /* dump current flash content to check write */
  sfm_dump (&spiFlash, 0x0, 0x10);

  /* normal end */
  exit(0);
}
```

This example compiled and executed leads to following output:

```bash
gcc -c -O spi_flash_model.c -o spi_flash_model.o
gcc -c -O main.c -o main.o
gcc spi_flash_model.o main.o -lm -o main

./main

00: 01 23 45 67 89 ab ff ff  ff ff ff ff ff ff ff ff
10: ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff
```


## References

 * [W25Q16JV](https://www.winbond.com/resource-files/w25q16jv%20spi%20revh%2004082019%20plus.pdf)
