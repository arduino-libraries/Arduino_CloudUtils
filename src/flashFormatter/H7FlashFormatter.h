/*
  Copyright (c) 2025 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once
#include "FlashFormatterBase.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "LittleFileSystem.h"
#include "FATFileSystem.h"

class MBEDH7FlashFormatter : public FlashFormatterClass {
public:
  MBEDH7FlashFormatter();

protected:
  bool checkPartition() override;
  bool formatPartition() override;
private:
  QSPIFBlockDevice _root;
  mbed::MBRBlockDevice _wifi_data;
  mbed::FATFileSystem _wifi_data_fs;
  mbed::MBRBlockDevice _ota_data;
  mbed::FATFileSystem _ota_data_fs;
  mbed::MBRBlockDevice _kvstore_data;

  long getFileSize(FILE *fp);
  bool checkWifiPartition();
  bool formatWifiPartition();
};
