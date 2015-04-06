/* Copyright (c) 2015 Brandon Goode */

#include "HubPCBAnt01HW.h"
#include "SparkIntervalTimer.h"


#define RF_STSREG      0x0000  // Status Read Register
#define RF_TXRXFIFO    0x8000  // FIFO state (1 = ready)
#define RF_POR         0x4000  // Power-on-Reset flag
#define RF_TXOWRXOF    0x2000  // Underrun/Overwrite/Overflow
#define RF_WUTINT      0x1000  // Wakeup timer overflow interrupt
#define RF_LCEXINT     0x0800  // Logic change interrupt
#define RF_LBTD        0x0400  // Low Battery threshold detect
#define RF_FIFOEM      0x0200  // Receiver FIFO empty (1 = empty)
#define RF_ATTRSSI     0x0100  // Antenna Tuning and RSSI indicator
#define RF_DQDO        0x0080  // Data Quality Detect/Indicate output
#define RF_CLKRL       0x0040  // Clock recovery lock bit
#define RF_AFCCT       0x0020  // Automatic frequency control cycle toggle
#define RF_OFFSV       0x0010  // Measured frequency offset of AFC cycle

#define RF_GENCREG     0x8000  // General Configuration Register
#define RF_TXDEN       0x0080  // TX Data Register Enable bit
#define RF_FIFOEN      0x0040  // FIFO Enable bit
#define RF_FBS_915     0x0030  // Frequency Band Select bits (915 MHz)
#define RF_LCS_10      0x0003  // Load Capacitance Select bits (10.0 pf)
#define RF_GENCREG_INIT (RF_GENCREG | RF_FBS_915 | RF_LCS_10)

#define RF_AFCCREG     0xC400  // AFC Configuration Register
#define RF_AUTOMS_INDP 0x00C0  // Offset independent for state of DIO sig.
#define RF_AUTOMS_RECV 0x0080  // Offset only during receive
#define RF_AUTOMS_ONCE 0x0040  // Offset once after power-cycle
#define RF_AUTOMS_OFF  0x0000  // Auto mode off
#define RF_ARFO_3to4   0x0030  // +3 to -4 Fres (tuning bits)
#define RF_ARFO_7to8   0x0020  // +7 to -8 Fres
#define RF_ARFO_15to16 0x0010  // +15 to -16 Fres
#define RF_ARFO_unlim  0x0000  // Unlimited
#define RF_MFCS        0x0008  // Manual Frequency control strobe
#define RF_HAM         0x0004  // High accuracy mode
#define RF_FOREN       0x0002  // Frequency Offset Register Enable
#define RF_FOFEN       0x0001  // Frequency Offset Enable
#define RF_AFCCREG_INIT (RF_AFCCREG | RF_AUTOMS_ONCE | RF_ARFO_3to4 | RF_HAM | RF_FOREN | RF_FOFEN)

#define RF_TXCREG      0x9800  // Transmit Configuration Register
#define RF_MODPLY      0x0100  // Modulation Polarity bit (for FSK)
#define RF_MODBW_30K   0x0001  // Modulation Bandwidth bits 30kHz
#define RF_MODBW_60K   0x0030  // Modulation Bandwidth bits 60kHz
#define RF_MODBW_75K   0x0040  // Modulation Bandwidth bits 75kHz
#define RF_MODBW_90K   0x0050  // Modulation Bandwidth bits 90kHz
#define RF_OTXPWR_0DB  0x0000  // Output Transmit Power Range bits - O dB
#define RF_TXCREG_INIT ( RF_TXCREG | RF_MODBW_75K | RF_OTXPWR_0DB)

#define RF_TXBREG      0xB8AA  // Transmit Byte Register

#define RF_CFSREG      0xA000  // Center Frequency Value Set Register
#define RF_FREQB_915   0x07CF  // Center Frequency Set bits 2000 = (915/30 - 30)*4000
#define RF_CFSREG_INIT (RF_CFSREG | RF_FREQB_915)

/*38.4 kbps

BW – 134

ΔfFSK – 90*/

#define RF_RXCREG     0x9000  // Receive Control Register
#define RF_DIOEN      0x0400  // DIO output
#define RF_DIORT_CONT 0x0300  // Continuous
#define RF_DIORT_SLOW 0x0200  // Slow
#define RF_DIORT_MED  0x0100  // Medium
#define RF_DIORT_FAST 0x0000  // Fast

#define RF_RXBW_67K  0x00C0  // Receiver Baseband Bandwidth bits 67KHz
#define RF_RXBW_134K 0x00A0  // Receiver Baseband Bandwidth bits 134KHz
#define RF_RXBW_200K 0x0080  // Receiver Baseband Bandwidth bits 200KHz
#define RF_RXBW_270K 0x0060  // Receiver Baseband Bandwidth bits 270KHz
#define RF_RXBW_340K 0x0040  // Receiver Baseband Bandwidth bits 340KHz
#define RF_RXBW_400K 0x0020  // Receiver Baseband Bandwidth bits 400KHz

// Receiver LNA Gain
#define RF_RXLNA_20DB 0x0018    // LNA Gain -20dB
#define RF_RXLNA_14DB 0x0010    // LNA Gain -14dB
#define RF_RXLNA_6DB  0x0008    // LNA Gain -6dB
#define RF_RXLNA_0DB  0x0000    // LNA Gain  0dB

// Digital RSSI threshold
#define RF_DRSSIT_73DB  0x0005    // -73dB Threshold
#define RF_DRSSIT_79DB  0x0004    // -79dB Threshold
#define RF_DRSSIT_85DB  0x0003    // -85dB Threshold
#define RF_DRSSIT_91DB  0x0002    // -91dB Threshold
#define RF_DRSSIT_97DB  0x0001    // -97dB Threshold
#define RF_DRSSIT_103DB 0x0000    // -103dB Threshold

#define RF_RXCREG_INIT (RF_RXCREG | RF_DIOEN | RF_DIORT_SLOW | RF_RXBW_200K | RF_RXLNA_0DB | RF_DRSSIT_103DB)


#define RF_BBFCREG    0xC228  // Baseband filter config. register address
#define RF_ACRLC      0x0080  // Automatic clock recovery lock control
#define RF_MCRLC      0x0040  // Manual clock recovery lock control
#define RF_FTYPE_EXT  0x0010  // Filter type (0 = digital, 1 = Ext. RC)
#define RF_DQTI_MASK  0x0007  // Data quality threshold indicator
#define RF_BBFCREG_INIT (RF_BBFCREG | RF_MCRLC | (RF_DQTI_MASK & 4))

#define RF_RXFIFOREG 0xB000  // Receiver FIFO Read Register

#define RF_FIFORSTREG 0xCA00  // FIFO and Reset mode Configuration Register
#define RF_FFBC_8     0x0080
#define RF_SYCHLEN    0x0008  // Synchronous Character Length bit (Byte long)
#define RF_FFSC       0x0004  // FIFO Fill Start Condition bit
#define RF_FSCF       0x0002  // FIFO Synchronous Character Fill bit
#define RF_DRSTM      0x0001  // Disable (Sensitive) Reset mode bit
#define RF_FIFORSTREG_INIT (RF_FIFORSTREG | RF_FFBC_8)

#define RF_SYNBREG    0xCE00  // Synchrnous byte config. register address
#define RF_SYNCB      0x00C3  // Sync byte configuration
#define RF_SYNBREG_INIT (RF_SYNBREG | RF_SYNCB)

#define RF_DRSREG      0xC600  // Data Rate Value Set Register
#define RF_DRPV_38_4   0x0008
#define RF_DRPV_76_8   0x0003
#define RF_DRSREG_INIT 0xC62A  // what insteon uses

#define RF_PMCREG  0x8200  // Power Management Configuration Register
#define RF_RXCEN   0x0080  // Receiver chain enable
#define RF_BBCEN   0x0040  // Baseband chain enable
#define RF_TXCEN   0x0020  // Transmitter chain enable
#define RF_SYNEN   0x0010  // Synthesier enable
#define RF_OSCEN   0x0008  // Oscillator enable
#define RF_LBDEN   0x0004  // Low Battery Detector Enable
#define RF_WUTEN   0x0002  // Wakeup timer enable
#define RF_CLKODIS 0x0001  // Clock output disable
#define RF_PMCREG_INIT (RF_PMCREG | RF_SYNEN | RF_OSCEN | RF_CLKODIS)
#define RF_PMCREG_RX (RF_PMCREG_INIT | RF_RXCEN)
#define RF_PMCREG_TX (RF_PMCREG | RF_TXCEN | RF_CLKODIS)

#define RF_WTSREG  0xE196  // Wake-up Timer Value Set Register

#define RF_DCSREG  0xC80E  // Duty Cycle Value Set Register

#define RF_BCSREG  0xC000  // Battery Threshold Detect and Clock Output Value Set Register
#define RF_LBDVB   0x000F  // Clock output disable
#define RF_BCSREG_INIT (RF_BCSREG | RF_LBDVB)

#define RF_PLLCREG    0xCC12  // PLL configuration register
#define RF_CBTC_5p    0x0060  // Clock buffer 5-10 Mhz
#define RF_CBTC_3     0x0040  // Clock buffer 3.3 Mhz
#define RF_CBTC_2p    0x0020  // Clock buffer > 2.5 Mhz
#define RF_CBTC_2m    0x0000  // Clock buffer < 2.5 Mhz
#define RF_PDDS       0x0008  // Phase detector delay
#define RF_PLLDD      0x0004  // PLL Dithering Disable
#define RF_PLLBWB     0x0001  // PLL Bandwidth (102dBc/Hz)
#define RF_PLLCREG_INIT (RF_PLLCREG | RF_CBTC_5p | RF_PLLDD)

#define SPI_SCLK_LOW_TIME 1  // 1 us = 1MHz


// 219.200 microseconds
#define BIT_TIME      219
#define BIT_TIME_50   109
#define BIT_TIME_75   164
#define BIT_TIME_125  274
#define BIT_TIME_175  384
#define BIT_TIME_225  493


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

#define FIRMWARE_VERSION      0x00

enum rxState_t {
  RX_IDLE,
  RX_SYNC,
  RX_SLEEP_CODE,
  RX_DATA_BYTE
};

enum txState_t {
  TX_IDLE,
  TX_STARTING,
  TX_SYNC,
  TX_START,
  TX_SLEEP_CODE,
  TX_BYTE
};

const int PLM_REQUEST_LENGTHS[] = {
  2,   // 0x60 - Get IM Info
  5,   // 0x61 - Send ALL-Link Command
  8,   // 0x62 - Send INSTEON Message (22 for extended)
  4,   // 0x63 - Send X10
  4,   // 0x64 - Start ALL-Linking
  2,   // 0x65 - Cancel ALL-Linking
  5,   // 0x66 - Set Host Device Category
  2,   // 0x67 - Reset the IM
  3,   // 0x68 - Set INSTEON ACK Message Byte
  2,   // 0x69 - Get First ALL-Link Record
  2,   // 0x6A - Get Next ALL-Link Record
  3,   // 0x6B - Set IM Configuration
  2,   // 0x6C - Get ALL-Link Record for Sender
  2,   // 0x6D - LED On
  2,   // 0x6E - LED Off
  11,  // 0x6F - Manage ALL-Link Record
  3,   // 0x70 - Set INSTEON NAK Message Byte
  4,   // 0x71 - Set INSTEON ACK Message Two Bytes
  2,   // 0x72 - RF Sleep
  2    // 0x73 - Get IM Configuration
};

IntervalTimer txTimer;

volatile bool rfDataWaiting;

volatile bool rfPacket = false;
volatile bool rfDataInt = false;
volatile uint8_t rfDataPin = LOW;
volatile uint8_t rfData = 0;
volatile uint8_t rfDio = LOW;
volatile uint8_t rfCount = 0;

volatile uint8_t rfPacketEnd = 0;

volatile uint32_t rxPulse = 0;
volatile uint32_t rxStart = 0;
volatile uint32_t currentTime = 0;
volatile rxState_t rxState = RX_IDLE;
volatile uint8_t rxBitCount = 0;
volatile uint8_t rxByteCount = 0;
volatile uint8_t rxSleepCode = 0;
volatile uint8_t rxDataByte = 0;
volatile uint8_t rxPacketLength = 0;


volatile uint8_t rxQueue[32][24];
volatile uint8_t rxQueueCurrent = 0;
volatile uint8_t rxQueueEnd = 0;
volatile uint8_t rxQueueNext = 1;

volatile uint8_t rxcrcBit = 0;
volatile uint8_t rxcrc = 0;


volatile uint8_t txQueue[32][32];
volatile uint8_t txQueueCurrent = 0;
volatile uint8_t txQueueEnd = 0;
volatile txState_t txState = TX_IDLE;
volatile uint8_t txBitCount = 0;
volatile uint8_t txByteCount = 0;
volatile uint8_t txByte = 0;
volatile uint8_t txSyncCount;
volatile uint8_t txSyncBit;
volatile uint8_t txClock;
volatile uint8_t txExtended;
volatile uint8_t txLength;
volatile uint8_t txSleepCode;
volatile uint8_t * txCmd;

byte mac[6];

void setup() {
  WiFi.macAddress(mac);

  Spark.function("insteon", receiveInsteonCommand);

  pinMode(RF_RESET, OUTPUT);
  digitalWrite(RF_RESET, LOW);
  delay(10);
  digitalWrite(RF_RESET, HIGH);
  delay(10);

  SPI.setDataMode(SPI_MODE1);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.setBitOrder(MSBFIRST);
  SPI.begin();

  rfReg(RF_FIFORSTREG_INIT);
  rfReg(RF_GENCREG_INIT);
  rfReg(RF_CFSREG_INIT);
  rfReg(RF_AFCCREG_INIT);
  rfReg(RF_BBFCREG_INIT);
  rfReg(RF_RXCREG_INIT);
  rfReg(RF_TXCREG_INIT);
  rfReg(RF_PMCREG_INIT);

  delay(1);

  rfReg(RF_DRSREG_INIT);

  rfReg(RF_PMCREG_TX);
  delay(5);

  enableRx();

  Serial.begin(9600);
}

void loop() {
  while (rxQueueCurrent != rxQueueEnd) {
    processRxCommand();
  }
}

void processRxCommand() {
  if (rxQueueCurrent == rxQueueEnd) {
    return;
  }

  rxQueueCurrent++;
  rxQueueCurrent &= 0x1F;

  uint8_t length;
  uint8_t cmd;
  uint8_t rxCmd[24];

  for (int i = 0; i < 24; i++) {
    rxCmd[i] = rxQueue[rxQueueCurrent][i];
  }

  uint8_t type = rxCmd[INSTEON_FLAGS_INDEX] & INSTEON_TYPE_MASK;
  uint8_t extended = rxCmd[INSTEON_FLAGS_INDEX] & INSTEON_EXT_FLAG;

  if (type != INSTEON_TYPE_BROADCAST && type != INSTEON_TYPE_ALL_LINK) {
    if (rxCmd[3] != mac[2] || rxCmd[4] != mac[1] || rxCmd[5] != mac[0]) {
      // invalid address
      return;
    }
  }


  if (extended) {
    cmd = 0x51;
    length = 23;
  } else {
    cmd = 0x50;
    length = 9;
  }

  uint8_t crc = crcGen(rxCmd, length);
  if (crc == rxCmd[length] || true) {  // FIXME crc not checked
    sendInsteonCommand(cmd, rxCmd, length);
  }
}

void sendInsteonCommand(uint8_t cmd, uint8_t *data, uint8_t length) {
  String commandStr = "02";

  commandStr += String(cmd, HEX);

  for (uint8_t i = 0; i < length; i++) {
    if (data[i] <= 0x0f) {
      commandStr += "0";
    }
    commandStr += String(data[i], HEX);
  }

  Serial.println(commandStr);

  Spark.publish("insteon", commandStr, 60, PRIVATE);
}

int receiveInsteonCommand(String args) {
  unsigned int argsLen = args.length();

  if (argsLen % 2 != 0) {
    return -1;
  }

  int cmdLength = argsLen / 2;
  if (cmdLength < 2 || cmdLength > 11) {
    return -2;
  }

  uint8_t cmd[11];


  for (int i = 0; i < cmdLength; i++) {
    int j = 2*i;
    uint8_t val1 = hexToInt(args.charAt(j++));
    uint8_t val2 = hexToInt(args.charAt(j));

    if (val1 > 0x0F || val2> 0x0F) {
      return -3;
    }

    uint8_t val = (val1 << 4) | val2;
    cmd[i] = val;
  }

  if (cmd[0] != INSTEON_START_CMD) {
    return -4;
  }

  if (cmd[1] < 0x60 || cmd[1] > 0x73) {
    return -5;
  }

  int expectedLength = PLM_REQUEST_LENGTHS[cmd[1] - 0x60];
  if (cmd[1] == INSTEON_SEND_MSG &&
    cmdLength >= 6 && cmd[5] & INSTEON_EXT_FLAG) {
    expectedLength = 22;
  }

  if (expectedLength != cmdLength) {
    return -6;
  }

  switch (cmd[1]) {
    case INSTEON_GET_IM_INFO:
      insteonInfo();
      break;
    case INSTEON_SEND_MSG:
      txCommand(cmd+2, cmdLength-2);
      break;
    default:
      Spark.publish("insteon", args + INSTEON_NACK_STR, 60, PRIVATE);
  }

  return cmdLength;
}


uint8_t hexToInt(char hex) {
  if (hex >= '0' && hex <= '9') {
    return hex - '0';
  }

  if (hex >= 'A' && hex <= 'F') {
    return hex - 'A' + 10;
  }

  if (hex >= 'a' && hex <= 'f') {
    return hex - 'a' + 10;
  }

  return 0xFF;
}

void insteonInfo() {
  uint8_t info[] = {
    mac[2],
    mac[1],
    mac[0],
    INSTEON_CATEGORY,
    INSTEON_SUBCATEGORY,
    FIRMWARE_VERSION,
    INSTEON_ACK
  };
  sendInsteonCommand(INSTEON_GET_IM_INFO, info, 7);
}

void txCommand(uint8_t cmd[], uint8_t length) {
  volatile uint8_t * txQueueNext = txQueue[txQueueEnd + 1];
  uint8_t crc = crcGen(cmd, length);

  int i = 0;
  txQueueNext[i++] = cmd[INSTEON_FLAGS_INDEX];
  txQueueNext[i++] = cmd[INSTEON_FROM_L_INDEX];
  txQueueNext[i++] = cmd[INSTEON_FROM_M_INDEX];
  txQueueNext[i++] = cmd[INSTEON_FROM_H_INDEX];
  txQueueNext[i++] = cmd[INSTEON_TO_L_INDEX];
  txQueueNext[i++] = cmd[INSTEON_TO_M_INDEX];
  txQueueNext[i++] = cmd[INSTEON_TO_H_INDEX];
  txQueueNext[i++] = cmd[INSTEON_CMD1_INDEX];
  txQueueNext[i++] = cmd[INSTEON_CMD2_INDEX];

  for (; i < length; i++) {
    txQueueNext[INSTEON_USER_DATA_OFFSET - i] = cmd[i];
  }

  txQueueNext[i++] = crc;

  for (; i < 31; i++) {
    txQueueNext[i] = 0xAA;  // Sync Filler
  }

  txQueueEnd++;

  enableTx();
}

void enableTx() {
  while (rxState != RX_IDLE) {
    delay(1);
  }
  if (txState == TX_IDLE) {
    rfReg(RF_PMCREG_TX);

    txState = TX_STARTING;
    detachInterrupt(RF_DATA);
    pinMode(RF_DATA, OUTPUT);
    txTimer.begin(txTimerInterrupt, BIT_TIME_50, uSec);
  }
}

void enableRx() {
  if (txState != TX_IDLE) {
    txTimer.end();
    txState = TX_IDLE;
  }

  rfReg(RF_PMCREG_RX);

  rfReg(RF_GENCREG_INIT);

  rfReg(RF_STSREG);

  pinMode(RF_DATA, INPUT);
  attachInterrupt(RF_DATA, rfDataInterrupt, CHANGE);
}


void txTimerInterrupt() {
  txClock ^= 1;

  switch (txState) {
    case TX_STARTING:
      digitalWrite(RF_DATA, LOW);
      txState = TX_SYNC;
      txSyncCount = 8;
            txSyncBit = 1;
      txClock = 0;
      break;

    case TX_SYNC:
      digitalWrite(RF_DATA, txSyncBit & txClock);
      if (!txClock) {
        txSyncBit ^= 1;

        if (txSyncCount-- == 0) {
          if (txQueueCurrent != txQueueEnd) {
            txCmd = txQueue[++txQueueCurrent];
            txByteCount = 0;
            txExtended = INSTEON_EXT_FLAG & txCmd[0];
            txNextByte();
          } else {
            enableRx();
          }
        }
      }
      break;

    case TX_START:
      digitalWrite(RF_DATA, LOW);

      if (!txClock) {
        txState = TX_SLEEP_CODE;
      }
      break;

    case TX_BYTE:
    case TX_SLEEP_CODE:
      digitalWrite(RF_DATA, txByte & txClock);

      if (!txClock) {
        txByte >>= 1;
        txBitCount--;

        if (txBitCount == 0) {
          if (txState == TX_BYTE) {
            txNextByte();

          } else {
            if (txByteCount == txLength) {
              txState = TX_SYNC;
              txSyncCount = 60;  // Sync Gap 4 x 15
              txSyncBit = 1;

            } else {
              txState = TX_BYTE;
              txByte = txCmd[txByteCount++];
              txBitCount = 8;
            }
          }
        }
      }
      break;

    default:
      // should never happen
      enableRx();
  }
}

void txNextByte() {
  txState = TX_START;

  if (txByteCount) {
    txByte = txSleepCode--;
  } else {
    txLength = txCmd[0] & INSTEON_EXT_FLAG ? 31 : 12;
    txByte = 31;
    txSleepCode = txExtended ? 30 : 11;
  }
  txBitCount = 5;
}

void rfDataInterrupt() {
  rfDataPin = digitalRead(RF_DATA);
  currentTime = micros();

  switch (rxState) {
  case RX_SYNC:
    rxPulse = currentTime - rxStart;
    if (rxPulse > BIT_TIME_225) {
      // Invalid sync -- back to idle state
      rxByteCount = 0;
      rxState = RX_IDLE;
    } else if (rxPulse > BIT_TIME_175) {
      // SYNC bit recieved
      rxcrc = 0;
      rxBitCount = 0;
      rxSleepCode = 1 & (~rfDataPin);
      rxDataByte = 0;
      rxState = RX_SLEEP_CODE;
      rxStart = currentTime;
    } else if (rxPulse >= BIT_TIME_75 && rxPulse < BIT_TIME_125) {
      rxStart = currentTime;
    }
    break;

  case RX_SLEEP_CODE:
  case RX_DATA_BYTE:
    rxPulse = currentTime - rxStart;
    if (rxPulse > BIT_TIME_125) {
      // ERROR - back to Idle
      rxByteCount = 0;
      rxState = RX_IDLE;
    } else if (rxPulse > BIT_TIME_75) {
      rxStart = currentTime;

      if (rxState == RX_SLEEP_CODE) {
        rxBitCount++;
        rxSleepCode |= (1 & (~rfDataPin)) << rxBitCount;

        if (rxBitCount == 4) {
          rxBitCount = 0;
          rxState = RX_DATA_BYTE;
        }
      } else {
        rxDataByte |= (1 & (~rfDataPin)) << rxBitCount;
        if (rxBitCount++ == 7) {
          switch (rxByteCount) {
          case 0:
            if (rxDataByte & INSTEON_EXT_FLAG) {
              rxPacketLength = 24;
            } else {
              rxPacketLength = 10;
            }
            rxQueue[rxQueueNext][INSTEON_FLAGS_INDEX] = rxDataByte;
            break;

          case 1:
            rxQueue[rxQueueNext][2] = rxDataByte;
            break;

          case 2:
            rxQueue[rxQueueNext][1] = rxDataByte;
            break;

          case 3:
            rxQueue[rxQueueNext][0] = rxDataByte;
            break;

          case 4:
            rxQueue[rxQueueNext][5] = rxDataByte;
            break;

          case 5:
            rxQueue[rxQueueNext][4] = rxDataByte;
            break;

          case 6:
            rxQueue[rxQueueNext][3] = rxDataByte;
            break;

          case 7:
            rxQueue[rxQueueNext][7] = rxDataByte;
            break;

          case 8:
            rxQueue[rxQueueNext][8] = rxDataByte;
            break;

          default:
            if (rxByteCount + 1 == rxPacketLength) {
              rxQueue[rxQueueNext][rxByteCount] = rxDataByte;
            }
            if (rxByteCount < rxPacketLength) {
              rxQueue[rxQueueNext][32 - rxByteCount] = rxDataByte;
            }
            break;
          }
          rxByteCount++;
          if (rxByteCount == rxPacketLength) {
            rxQueueEnd = rxQueueNext++;
            rxQueueNext = rxQueueNext % 20;
          }
          rxState = RX_SYNC;
        }
      }
    }
    break;

  case RX_IDLE:
  default:
    rxStart = currentTime;
    rxState = RX_SYNC;
    break;
  }
}


uint16_t rfReg(uint16_t regOut) {
  digitalWrite(SS, LOW);
  uint8_t msByteOut = (regOut >> 8) & 0xFF;
  uint8_t lsByteOut = regOut & 0xFF;
  uint8_t msByteIn = SPI.transfer(msByteOut);
  uint8_t lsByteIn = SPI.transfer(lsByteOut);
  digitalWrite(SS, HIGH);

  uint16_t regIn = 0x0000;

  regIn = msByteIn;
  regIn = regIn << 8;
  regIn |= lsByteIn;

  return regIn;
}



uint8_t crcGen(uint8_t *p, uint8_t len) {
  // FIXME: Insteon CRC

  /*
   * Insteon Developers Guide:
   *
   * The last field in an INSTEON message is a one-byte CRC, or Cyclic
   * Redundancy Check. The INSTEON transmitting device computes the CRC
   * over all the bytes in a message beginning with the From Address.
   * INSTEON uses a software-implemented 7-bit linear-feedback shift
   * register with taps at the two most-significant bits. The CRC covers
   * 9 bytes for Standard-length messages and 23 bytes for Extended-length
   * messages. An INSTEON receiving device computes its own CRC over the
   * same message bytes as it receives them. If the message is corrupt,
   * the receiver’s CRC will not match the transmitted CRC.
   *
   */

  return 0x00;
}

