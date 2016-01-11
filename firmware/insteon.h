/* Copyright (c) 2015 Brandon Goode */
#ifndef FIRMWARE_INSTEON_H_
#define FIRMWARE_INSTEON_H_

#define INSTEON_START_CMD       0x02

#define INSTEON_GET_IM_INFO        0x60
#define INSTEON_SEND_ALL_LINK      0x61
#define INSTEON_SEND_MSG           0x62
#define INSTEON_SEND_X10           0x63
#define INSTEON_START_ALL_LINK     0x64
#define INSTEON_CANCEL_ALL_LINK    0x65
#define INSTEON_SET_HOST_CAT       0x66
#define INSTEON_RESET              0x67
#define INSTEON_SET_ACK            0x68
#define INSTEON_GET_FRIST_RECORD   0x69
#define INSTEON_GET_NEXT_RECORD    0x6A
#define INSTEON_GET_RECORD_SENDER  0x6B
#define INSTEON_SET_CONFIG         0x6C
#define INSTEON_LED_ON             0x6D
#define INSTEON_LED_OFF            0x6E
#define INSTEON_MANAGE_RECORD      0x6F
#define INSTEON_SET_NACK           0x70
#define INSTEON_SET_ACK_TWO        0x71
#define INSTEON_RF_SLEEP           0x72
#define INSTEON_GET_CONIG          0x73

#define INSTEON_ACK                0x06
#define INSTEON_NACK               0x15
#define INSTEON_NACK_STR           "15"

#define INSTEON_FROM_H_INDEX       0
#define INSTEON_FROM_M_INDEX       1
#define INSTEON_FROM_L_INDEX       2
#define INSTEON_TO_H_INDEX         3
#define INSTEON_TO_M_INDEX         4
#define INSTEON_TO_L_INDEX         5
#define INSTEON_FLAGS_INDEX        6
#define INSTEON_CMD1_INDEX         7
#define INSTEON_CMD2_INDEX         8
#define INSTEON_USER_DATA_INDE     9
#define INSTEON_USER_DATA_LENGTH   14
#define INSTEON_USER_DATA_OFFSET   32

#define INSTEON_RF_FLAGS_INDEX     0

#define INSTEON_EXT_FLAG           0x10
#define INSTEON_TYPE_MASK          0xE0
#define INSTEON_TYPE_BROADCAST     0x80
#define INSTEON_TYPE_ALL_LINK      0xC0
#define INSTEON_TYPE_CLEANUP       0x40
#define INSTEON_TYPE_CLEANUP_ACK   0x60
#define INSTEON_TYPE_CLEANUP_NACK  0xE0
#define INSTEON_TYPE_DIRECT        0x00
#define INSTEON_TYPE_ACK           0x20
#define INSTEON_TYPE_NACK          0xA0

#define INSTEON_CATEGORY      0x00
#define INSTEON_SUBCATEGORY   0x00



#endif  // FIRMWARE_INSTEON_H_
