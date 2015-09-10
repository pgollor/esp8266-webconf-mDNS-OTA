# esp8266-webconf-mDNS-OTA
Arduino code sample to configure the ESP8266 via http and update Over-the-Air (OTA).

## Information
For using this Arduino sketch you need the [Arduino ESP8266 Environment](https://github.com/esp8266/Arduino)
<br>
The over the air (OTA) flashing stuff works only for flash size higher or equal than 1 MB.
You can check your flash size by getting the flash chip id with the esptool python script:
```
esptool.py --port [Serial port] flash_id
```
For more information please read the [esp8266 wiki page](http://www.esp8266.com/wiki/doku.php?id=esp8266-module-family#modules).

# Usage

## Flashing file system

### Create Image
First we have to crate a binary file from data folder.
I use the [mkspiffs](https://github.com/igrr/mkspiffs) tool for this with the following command:
```
mkspiffs -c ./data/ file.bin
```
This works for 64K SPIFFS and standard page and block size very well, but for higher SPIFFS size you have to customize the settings with the following options for the ESP8266 Generic module:

flash size | SPIFFS | options | real size (KBytes)
-----------|--------|---------|------------------
512kB | 64k | nothing | 64
1M | 64k | nothing | 64
1M | 128k | -s 0x20000 | 128
1M | 256k | -s 0x40000 | 256
1M | 512k | -s 0x80000 -b 0x2000 | 512
2M | 1M | -s 0xFB000 -b 0x2000 | 1004
4M | 1M | -s 0xFB000 -b 0x2000 | 1004
4M | 3M | -s 0x2FB000 -b 0x2000 | 3052

You can get more information at the [ESP8266 Arduino reference page](https://github.com/esp8266/Arduino/blob/esp8266/hardware/esp8266com/esp8266/doc/reference.md#file-system).


### Flashing Image
After that we need to flash this binary image to the file system address into flash:
```
esptool.py --port /dev/ttyUSB0 write_flash [Address] file.bin
```

### Addresses
The address is depending on your flash chip size. Here are some values for you:
- 512K (64K SPIFFS): `0x6B000`
- 1M (64K SPIFFS): `0xEB000`
- 4M (1M SPIFFS): `0x300000`

For other flash sizes you can get the address information from your arduino esp8266 installtion linker files located in
`arduino_esp/hardware/esp8266com/esp8266/tools/sdk/ld/eagle.flash.[X]m[Y].ld`.
`[X]` means the flash size and `[Y]` the spiffs size. This files contains a variable like `_SPIFFS_start`.
The start address for your file system is: `_SPIFFS_start` - `0x40200000`


# License
For license information please see [License](LICENSE.md)
