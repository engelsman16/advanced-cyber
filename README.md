[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=engelsman16_advanced-cyber&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=engelsman16_advanced-cyber)[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=engelsman16_advanced-cyber&metric=bugs)](https://sonarcloud.io/summary/new_code?id=engelsman16_advanced-cyber)[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=engelsman16_advanced-cyber&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=engelsman16_advanced-cyber)


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
~/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate "../main/nvs.csv" certs.bin 16384
```

### Build
```sh
idf.py build
```

### Flash to esp32
```sh
esptool.py -p COM3 -b 460800 --before default_reset --after hard_reset --chip esp32 write_flash --flash_mode dio --flash_freq 40m --flash_size detect 0x10000 build/main.bin 0x1000 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x9000 build/certs.bin
```
### Monitor
```sh
idf.py monitor -p COM3
```