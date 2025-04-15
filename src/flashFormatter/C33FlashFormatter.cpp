/*
  Copyright (c) 2025 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#if defined(ARDUINO_PORTENTA_C33)
#include "C33FlashFormatter.h"
#define BD_ERROR_OK 0

C33FlashFormatter::C33FlashFormatter():
  _root(BlockDevice::get_default_instance()),
  _sys_bd(_root, 1),
  _sys_fs("sys"),
  _user_bd(_root, 2),
  _kvStore_bd(_root, 3) {
}

bool C33FlashFormatter::checkPartition()
{
  if (_root->init() != BD_ERROR_OK)
  {
    return false;
  }

  if (_sys_bd.init() != BD_ERROR_OK || _sys_fs.mount(&_sys_bd) != FR_OK)
  {
    return false;
  }

  _sys_fs.unmount();
  _sys_bd.deinit();

  if (_user_bd.init() != BD_ERROR_OK)
  {
    return false;
  }

  _user_bd.deinit();

  if (_kvStore_bd.init() != BD_ERROR_OK)
  {
    return false;
  }

  _kvStore_bd.deinit();
  _root->deinit();

  return true;
}

bool C33FlashFormatter::formatPartition() {
  MBRBlockDevice::partition(_root, 1, 0x0B, 0, 5 * 1024 * 1024);
  MBRBlockDevice::partition(_root, 2, 0x0B, 5 * 1024 * 1024, 15 * 1024 * 1024);
  MBRBlockDevice::partition(_root, 3, 0x0B, 15 * 1024 * 1024, 16 * 1024 * 1024);

  int err = _sys_fs.reformat(&_sys_bd);
  if (err) {
    return false;
  }

  _sys_fs.unmount();
  _user_data_fs = new LittleFileSystem("user");
  err = _user_data_fs->reformat(&_user_bd);
  if (err) {
    return false;
  }
  _user_data_fs->unmount();
  _root->deinit();
  return true;
}

#endif
