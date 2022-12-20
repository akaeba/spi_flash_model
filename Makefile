# ************************************************************************
#  @author:     Andreas Kaeberlein
#  @copyright:  Copyright 2022
#  @credits:    AKAE
# 
#  @license:    BSDv3
#  @maintainer: Andreas Kaeberlein
#  @email:      andreas.kaeberlein@web.de
# 
#  @file:       Makefile
#  @date:       2022-12-18
# 
#  @brief:      Build
# ************************************************************************



# select compiler
CC = gcc

# set linker
LINKER = gcc

# set compiler flags
ifeq ($(origin CFLAGS), undefined)
  CFLAGS = -c -O -Wall -Wextra -Wconversion -I . -I ../
endif

# linking flags here
ifeq ($(origin LFLAGS), undefined)
  LFLAGS = -Wall -Wextra -I. -lm
endif


all: spi_flash_model_test

spi_flash_model_test: spi_flash_model_test.o spi_flash_model.o
	$(LINKER) ./test/spi_flash_model_test.o ./test/spi_flash_model.o $(LFLAGS) -o ./test/spi_flash_model_test

spi_flash_model_test.o: ./test/spi_flash_model_test.c
	$(CC) $(CFLAGS) ./test/spi_flash_model_test.c -o ./test/spi_flash_model_test.o

spi_flash_model.o: ./spi_flash_model.c
	$(CC) $(CFLAGS) ./spi_flash_model.c -o ./test/spi_flash_model.o

ci: ./spi_flash_model.c
	$(CC) $(CFLAGS) -Werror ./spi_flash_model.c -o ./test/spi_flash_model.o

clean:
	rm -f ./test/*.o ./test/spi_flash_model_test
