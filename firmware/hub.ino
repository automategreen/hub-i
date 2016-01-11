/* Copyright (c) 2015 Brandon Goode */

#define HW_VERSION_PCB_ANT_01 // HW_VERSION_WIRE_ANT_01
#include "hw.h"

#include "insteon.h"
#include "insteonRF.h"

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

const uint8_t insteonStdToRF[] = {3, 2, 1, 6, 5, 4, 0, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};

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

  pinMode(RF_TX_LED, OUTPUT);
  digitalWrite(RF_TX_LED, LOW);

  pinMode(RF_RESET, OUTPUT);
  digitalWrite(RF_RESET, LOW);
  delay(10);
  digitalWrite(RF_RESET, HIGH);
  delay(10);


  pinMode(RF_MOSI, OUTPUT);
  pinMode(RF_SCK, OUTPUT);
  pinMode(RF_SS, OUTPUT);

  // SPI.setDataMode(SPI_MODE1);
  // SPI.setClockSpeed(1, MHZ);
  // SPI.setBitOrder(MSBFIRST);
  // SPI.begin();

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
  digitalWrite(RF_TX_LED, HIGH);

  rxQueueCurrent++;
  rxQueueCurrent &= 0x1F;

  uint8_t length;
  uint8_t cmd;
  uint8_t rxCmd[24];

  for (int i = 0; i < 24; i++) {
    rxCmd[i] = rxQueue[rxQueueCurrent][i];
  }

  uint8_t type = rxCmd[INSTEON_RF_FLAGS_INDEX] & INSTEON_TYPE_MASK;
  uint8_t extended = rxCmd[INSTEON_RF_FLAGS_INDEX] & INSTEON_EXT_FLAG;

  if (type != INSTEON_TYPE_BROADCAST && type != INSTEON_TYPE_ALL_LINK) {
    if (rxCmd[6] != mac[2] || rxCmd[5] != mac[1] || rxCmd[4] != mac[0]) {
      digitalWrite(RF_TX_LED, LOW);
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

  sendInsteonCommand(cmd, rxCmd, length);

  digitalWrite(RF_TX_LED, LOW);
}

void sendInsteonCommand(uint8_t cmd, uint8_t *data, uint8_t length) {

  String commandStr = "02";

  commandStr += String(cmd, HEX);

  for (uint8_t i = 0; i < length; i++) {
    uint8_t rfIndex = insteonStdToRF[i];
    uint8_t dataByte = data[rfIndex];
    if (dataByte <= 0x0f) {
      commandStr += "0";
    }
    commandStr += String(dataByte, HEX);
  }

  uint8_t crc = crcGen(data, length);
  if (crc == data[length]) {
    Spark.publish("insteon", commandStr, 60, PRIVATE);
  } else {
    Serial.println("CRC ERROR: rxCRC=0x" + String(data[length], HEX) + " crcCalc=0x" + String(crc, HEX));
  }
  Serial.println(commandStr);
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
      Serial.println("tx data");
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

  for(int j = 0; j < length; j++) {
    Serial.println(String(cmd[j], HEX));
  }

  Serial.println(" ");

  volatile uint8_t * txQueueNext = txQueue[txQueueEnd + 1];
  uint8_t crc = crcGen(cmd, length);

  int i = 0;
  txQueueNext[i++] = cmd[INSTEON_FLAGS_INDEX-3];
  txQueueNext[i++] = mac[0];
  txQueueNext[i++] = mac[1];
  txQueueNext[i++] = mac[2];
  txQueueNext[i++] = cmd[INSTEON_TO_L_INDEX-3];
  txQueueNext[i++] = cmd[INSTEON_TO_M_INDEX-3];
  txQueueNext[i++] = cmd[INSTEON_TO_H_INDEX-3];
  txQueueNext[i++] = cmd[INSTEON_CMD1_INDEX-3];
  txQueueNext[i++] = cmd[INSTEON_CMD2_INDEX-3];

  for (; i < length; i++) {
    txQueueNext[INSTEON_USER_DATA_OFFSET - i] = cmd[i];
  }

  txQueueNext[i++] = crc;

  for (; i < 31; i++) {
    txQueueNext[i] = 0xAA;  // Sync Filler
  }

  for(int j = 0; j < 31; j++) {
    Serial.println(String(txQueueNext[j], HEX));
  }

  txQueueEnd++;

  enableTx();
}

void enableTx() {
  while (rxState != RX_IDLE) {
    delay(1);
  }
  if (txState == TX_IDLE) {

    txState = TX_STARTING;
    detachInterrupt(RF_DATA);
    pinMode(RF_DATA, OUTPUT);

    digitalWrite(RF_TX_LED, HIGH);

    rfReg(RF_PMCREG_TX);
    delay(5);

    txTimer.begin(txTimerInterrupt, BIT_TIME_50, uSec);
  }
}

void enableRx() {
  if (txState != TX_IDLE) {
    txTimer.end();
    txState = TX_IDLE;
  }

  digitalWrite(RF_TX_LED, LOW);

  rfReg(RF_PMCREG_RX);

  rfReg(RF_GENCREG_INIT);

  rfReg(RF_STSREG);

  pinMode(RF_DATA, INPUT_PULLDOWN);
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
      digitalWrite(RF_DATA, txSyncBit ^ txClock);
      if (txClock) {
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

      if (txClock) {
        txState = TX_SLEEP_CODE;
      }
      break;

    case TX_BYTE:
    case TX_SLEEP_CODE:
      digitalWrite(RF_DATA, (txByte ^ txClock) & 1);

      if (txClock) {
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
          if (rxByteCount == 0) {
            if (rxDataByte & INSTEON_EXT_FLAG) {
              rxPacketLength = 24;
            } else {
              rxPacketLength = 10;
            }
          }
          rxQueue[rxQueueNext][rxByteCount] = rxDataByte;

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

// uint16_t rfReg(uint16_t regOut) {
//   digitalWrite(SS, LOW);
//   uint8_t msByteOut = (regOut >> 8) & 0xFF;
//   uint8_t lsByteOut = regOut & 0xFF;
//   uint8_t msByteIn = SPI.transfer(msByteOut);
//   uint8_t lsByteIn = SPI.transfer(lsByteOut);
//   digitalWrite(SS, HIGH);

//   uint16_t regIn = 0x0000;

//   regIn = msByteIn;
//   regIn = regIn << 8;
//   regIn |= lsByteIn;

//   return regIn;
// }

// #define SPISPEED 10
// void SPI_delay() {volatile int v; int i; for (i = 0; i < SPISPEED/2; i++) v; }

void rfReg(uint16_t regOut)
{

  digitalWrite(RF_SS, LOW);
  unsigned char bit;

  for (bit = 0; bit < 16; bit++) {
      /* delay between raise of clock */
      // SPI_delay();


      if (regOut & 0x8000)
        digitalWrite(RF_MOSI, HIGH);
      else
        digitalWrite(RF_MOSI, LOW);

      digitalWrite(RF_SCK, HIGH);

      // SPI_delay();

      digitalWrite(RF_SCK, LOW);

      regOut <<= 1;
  }

  digitalWrite(RF_SS, HIGH);
}



// Source https://github.com/evilpete/insteonrf/blob/master/Src/insteon_lib.c
unsigned char lsfr_table[] = {
  0x00, 0x30, 0x60, 0x50, // 0 1 2 3
  0xC0, 0xF0, 0xA0, 0x90, // 4 5 6 7
  0x80, 0xB0, 0xE0, 0xD0, // 8 9 A B
  0x40, 0x70, 0x20, 0x10  // C D E F
};

uint8_t crcGen(uint8_t *p, uint8_t len) {
  uint8_t crc = 0;

  for(uint8_t i = 0; i<len; i++) {
      crc ^= p[i];
      crc ^= lsfr_table[ crc & 0x0F ] ;
  }

  return crc;
}
