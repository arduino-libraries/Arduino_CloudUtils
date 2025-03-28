/*
  Copyright (c) 2025 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once
#include "FlashFormatterInterface.h"
#include "BlockDevice.h"
#include "MBRBlockDevice.h"
#include "LittleFileSystem.h"
#include "FATFileSystem.h"

class C33FlashFormatter : public FlashFormatterClass {
public:
  C33FlashFormatter();
protected:
  bool checkPartition() override;
  bool formatPartition() override;
private:
  BlockDevice* _root;
  MBRBlockDevice _sys_bd;
  MBRBlockDevice _user_bd;
  FATFileSystem _sys_fs;
  FileSystem * _user_data_fs;
  MBRBlockDevice _kvStore_bd;
};
