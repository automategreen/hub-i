/* Copyright (c) 2015 Brandon Goode */
#ifndef FIRMWARE_INSTEONRF_H_
#define FIRMWARE_INSTEONRF_H_

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
#define RF_AFCCREG_INIT (RF_AFCCREG | RF_AUTOMS_ONCE | RF_ARFO_3to4 | \
                         RF_HAM | RF_FOREN | RF_FOFEN)

#define RF_TXCREG      0x9800  // Transmit Configuration Register
#define RF_MODPLY      0x0100  // Modulation Polarity bit (for FSK)
#define RF_MODBW_30K   0x0001  // Modulation Bandwidth bits 30kHz
#define RF_MODBW_60K   0x0030  // Modulation Bandwidth bits 60kHz
#define RF_MODBW_75K   0x0040  // Modulation Bandwidth bits 75kHz
#define RF_MODBW_90K   0x0050  // Modulation Bandwidth bits 90kHz
#define RF_MODBW_105K  0x0060  // Modulation Bandwidth bits 105kHz
#define RF_MODBW_120K  0x0070  // Modulation Bandwidth bits 120kHz
#define RF_OTXPWR_0DB  0x0000  // Output Transmit Power Range bits - O dB
#define RF_TXCREG_INIT (RF_TXCREG | RF_MODBW_75K | RF_OTXPWR_0DB)

#define RF_TXBREG      0xB8AA  // Transmit Byte Register

#define RF_CFSREG      0xA000  // Center Frequency Value Set Register
#define RF_FREQB_915   0x07CF  // Center Frequency Set bits (915/30 - 30)*4000
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

#define RF_RXCREG_INIT (RF_RXCREG | RF_DIOEN | RF_DIORT_SLOW | RF_RXBW_200K | \
                        RF_RXLNA_0DB | RF_DRSSIT_73DB)


#define RF_BBFCREG    0xC228  // Baseband filter config. register address
#define RF_ACRLC      0x0080  // Automatic clock recovery lock control
#define RF_MCRLC      0x0040  // Manual clock recovery lock control
#define RF_FTYPE_EXT  0x0010  // Filter type (0 = digital, 1 = Ext. RC)
#define RF_DQTI_MASK  0x0007  // Data quality threshold indicator
#define RF_BBFCREG_INIT (RF_BBFCREG | RF_MCRLC | (RF_DQTI_MASK & 5))

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
#define RF_PMCREG_INIT (RF_PMCREG | RF_BBCEN | RF_SYNEN | RF_OSCEN | RF_CLKODIS)
#define RF_PMCREG_RX (RF_PMCREG_INIT | RF_RXCEN)
#define RF_PMCREG_TX (RF_PMCREG_INIT | RF_TXCEN)

#define RF_WTSREG  0xE196  // Wake-up Timer Value Set Register

#define RF_DCSREG  0xC80E  // Duty Cycle Value Set Register

#define RF_BCSREG  0xC000  // Battery Threshold and Clock Output Register
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

#endif  // FIRMWARE_INSTEONRF_H_
