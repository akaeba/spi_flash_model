

# SPI-Flash-Model

Arbitrary C model of an SPI Flash.


## Features

The [model](./spi_flash_model.c) emulates an SPI Flash on logic level. As input are SPI packets used. In the real
application would this packet sent to the SPI instead.


### Limits

* No timing behavior


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

#### Init

Initializes the SFM and selects the emulated flash

```c
int sfm_init (t_sfm *self, char flashType[]);
```


#### Dump

Dumps Flash memory in hex values to console. Setting of start/stop ```-1``` will print whole
memory content to console.

```c
int sfm_dump (t_sfm *self, int32_t start, int32_t stop);
```


#### SFM (Spi Flash Model)

Access SPI Flash memory. SPI request and response is placed in the same SPI buffer variable.

```c
int sfm (t_sfm *self, uint8_t* spi, uint32_t len);
```


### Example



## References

 * [W25Q16JV](https://www.winbond.com/resource-files/w25q16jv%20spi%20revh%2004082019%20plus.pdf)
