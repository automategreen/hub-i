/* Copyright (c) 2015 Brandon Goode */
#ifndef FIRMWARE_HW_H_
#define FIRMWARE_HW_H_

#ifdef HW_VERSION_PCB_ANT_01

#define RF_RESET    A1
#define RF_SS       A2
#define RF_SCK      A3
#define RF_MISO     A4
#define RF_MOSI     A5
#define RF_DATA     A6
#define RF_CLK      A7

#define RF_IRO      D2

#define RF_TX_LED   D7

#endif  // HW_VERSION_PCB_ANT_01


#ifdef HW_VERSION_WIRE_ANT_01

#define RF_DIO      D3
#define RF_RESET    D4
#define RF_SS       A2
#define RF_SCK      A3
#define RF_MISO     A4
#define RF_MOSI     A5
#define RF_DATA     D1
#define RF_CLK      D0

#define RF_IRO      D2

#define RF_TX_LED   D7

#endif

#endif  // FIRMWARE_HW_H_
