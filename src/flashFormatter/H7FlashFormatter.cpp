/*
  Copyright (c) 2025 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) \
  || defined(ARDUINO_OPTA) || defined(ARDUINO_GIGA)
#include "H7FlashFormatter.h"
#include "wiced_resource.h"
#include "certificates.h"

MBEDH7FlashFormatter::MBEDH7FlashFormatter():
_root(mbed::BlockDevice::get_default_instance()),
_wifi_data(_root, 1),
_wifi_data_fs("wlan"),
_ota_data(_root, 2),
_ota_data_fs("fs"),
_kvstore_data(_root, 3)
{
}

bool MBEDH7FlashFormatter::checkPartition()
{
    if (_root->init() != mbed::BD_ERROR_OK)
    {
        return false;
    }

    if (!checkWifiPartition())
    {
        return false;
    }

    if (_ota_data.init() !=  mbed::BD_ERROR_OK || _ota_data_fs.mount(&_ota_data) != 0)
    {
        return false;
    }

    if (_ota_data.size() < 5 * 1024 * 1024)
    {
        return false;
    }
    _ota_data_fs.unmount();
    _ota_data.deinit();

    if (_kvstore_data.init() !=  mbed::BD_ERROR_OK)
    {
        return false;
    }

    _kvstore_data.deinit();
    _root->deinit();

    return true;
}

bool MBEDH7FlashFormatter::formatPartition() {
  _root->erase(0x0, _root->get_erase_size());
  /* Default partitioning of OPTA boards includes a 4th partition for PLC ide runtime
   * This partition is not used in the context of ArduinoCloud and is not needed,
   * but we try to preserve the default partitioning for compatibility.
   */
#if defined(ARDUINO_OPTA)
  mbed::MBRBlockDevice::partition(_root, 1, 0x0B, 0, 1024 * 1024);
  mbed::MBRBlockDevice::partition(_root, 2, 0x0B, 1024 * 1024, 6 * 1024 * 1024);
  mbed::MBRBlockDevice::partition(_root, 3, 0x0B, 6 * 1024 * 1024, 7 * 1024 * 1024);
#else
  mbed::MBRBlockDevice::partition(_root, 1, 0x0B, 0, 1024 * 1024);
  mbed::MBRBlockDevice::partition(_root, 2, 0x0B, 1024 * 1024, 13 * 1024 * 1024);
  mbed::MBRBlockDevice::partition(_root, 3, 0x0B, 13 * 1024 * 1024, 14 * 1024 * 1024);
#endif

  if(_ota_data_fs.mount(&_ota_data) != 0) {
    if(_ota_data_fs.reformat(&_ota_data) != 0) {
      return false;
    }
  }else {
    _ota_data_fs.unmount();
  }

  if(!formatWifiPartition()) {
    return false;
  }
  _root->deinit();
  return true;
}

long MBEDH7FlashFormatter::getFileSize(FILE *fp) {
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  return size;
}

bool MBEDH7FlashFormatter::checkWifiPartition() {
  int err =  _wifi_data_fs.mount(&_wifi_data);
  if (err) {
    return false;
  }

  DIR *dir;
  struct dirent *ent;


  if ((dir = opendir("/wlan")) == NULL) {
    return false;
  }

  bool found = false;
  while ((ent = readdir (dir)) != NULL) {
    String fullname = "/wlan/" + String(ent->d_name);
    if (fullname == "/wlan/4343WA1.BIN") {
      found = true;
      break;
    }
  }

  closedir (dir);
  _wifi_data_fs.unmount();
  return found;
}

bool MBEDH7FlashFormatter::formatWifiPartition() {
  _wifi_data_fs.reformat(&_wifi_data);
  extern const unsigned char wifi_firmware_image_data[];
  extern const resource_hnd_t wifi_firmware_image;
  FILE* fp = fopen("/wlan/4343WA1.BIN", "wb");
  const int file_size = 421098;
  int chunck_size = 1024;
  int byte_count = 0;

  while (byte_count < file_size) {
    if(byte_count + chunck_size > file_size)
      chunck_size = file_size - byte_count;
    int ret = fwrite(&wifi_firmware_image_data[byte_count], chunck_size, 1, fp);
    if (ret != 1) {
      return false;
    }
    byte_count += chunck_size;
  }
  fclose(fp);

  chunck_size = 1024;
  byte_count = 0;
  const uint32_t offset = 15 * 1024 * 1024 + 1024 * 512;

  while (byte_count < file_size) {
    if(byte_count + chunck_size > file_size)
      chunck_size = file_size - byte_count;
    int ret = _root->program(wifi_firmware_image_data, offset + byte_count, chunck_size);
    if (ret != 0) {
      return false;
    }
    byte_count += chunck_size;
  }

  chunck_size = 128;
  byte_count = 0;
  fp = fopen("/wlan/cacert.pem", "wb");

  while (byte_count < cacert_pem_len) {
    if(byte_count + chunck_size > cacert_pem_len)
      chunck_size = cacert_pem_len - byte_count;
    int ret = fwrite(&cacert_pem[byte_count], chunck_size, 1 ,fp);
    if (ret != 1) {
      return false;
    }
    byte_count += chunck_size;
  }

  fclose(fp);
  _wifi_data_fs.unmount();
  return true;
}

#endif
