# Docs 

These docs assume you have a running esp-idf env with terminal.

* This project uses nvs storage for wifi and https credentials, for this make sure to generate the proper bins using the commands below. 

An example cvs file:  
| key          | type      | encoding | value   |
|--------------|-----------|----------|---------|
| wifi_storage | namespace | empty    | _empty_ |
| ssid         | data      | string   | "x"     |
| password     | data      | string   | "x"     |

```py
~/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate "../main/nvs.csv" wifi_storage.bin 16384
```

### Build
```sh
idf.py build
```

### Flash to esp32
```sh
esptool.py -p COM3 -b 460800 --before default_reset --after hard_reset --chip esp32 write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x10000 build/main.bin 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x9000 build/wifi_storage.bin
```
### Monitor
```sh
idf.py monitor -p COM3
```