#!/bin/bash

case "$1" in
    0)  I2C1=0020 && I2C2=0021 ;;
    1)  I2C1=0040 && I2C2=0041 ;;
    2)  I2C1=0008 && I2C2=0009 ;;
    3)  I2C1=0044 && I2C2=0045 ;;
    4)  I2C1=0046 && I2C2=0047 ;;
    5)  I2C1=0062 && I2C2=0063 ;;
    6)  I2C1=0066 && I2C2=0067 ;;
    7)  I2C1=0028 && I2C2=0029 ;;
    *)  echo "Usage: $0 [PA #]"
        echo "  PA #: 0 - 7"
        exit 66
        ;;
esac

echo "Current PA App Version: `cat /sys/bus/i2c/drivers/pca954x/*-$I2C1/channel-5/*-$I2C2/eeprom | head -1`"
echo ""
echo "Current PA BL Version: `cat /sys/bus/i2c/drivers/pca954x/*-$I2C1/channel-6/*-$I2C2/eeprom | head -1`"
echo ""

echo "Reboot into Bootloader"
echo "13371337 0x133710ad" > /sys/bus/i2c/drivers/pca954x/*-$I2C1/channel-2/*-$I2C2/eeprom
echo ""

echo "Starting update"
sleep 2
./stm32flash -a 0x70 /dev/i2c-0 -S 0x8006000 -w galloway-v3.2.5.bin -g 0x8006000

sleep 10
echo ""
echo "New PA App Version: `cat /sys/bus/i2c/drivers/pca954x/*-$I2C1/channel-5/*-$I2C2/eeprom | head -1`"
echo ""
