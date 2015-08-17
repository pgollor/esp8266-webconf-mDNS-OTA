# esp8266-webconf-mDNS-OTA
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

### Flashing Image
After that we need to flash this binary image to the file system address into flash:
```
esptool.py --port /dev/ttyUSB0 write_flash [Address] file.bin
```

### Addresses
The address is depending on your flash chip size. Here are some values for you:
- 512K (64K spiffs): `0x6B000`
- 1M (64K spiffs): `0xEB000`

For other flash sizes you can get the address information from your arduino esp8266 installtion linker files located in
`arduino_esp/hardware/esp8266com/esp8266/tools/sdk/ld/eagle.flash.[X]m[Y].ld`.
`[X]` means the flash size and `[Y]` the spiffs size. This files contains a variable like `_SPIFFS_start`.
The start address for your file system is: `_SPIFFS_start` - `0x40200000`


# License
For license information please see [License](LICENSE.md)
