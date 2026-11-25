/*
  Copyright (c) 2025 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once
#include "FlashFormatterBase.h"

/*
 * This Flash formatter is expected to be run on a board that is performing a migration from mbedos
 * to zephyr, it won't erase the partitions, it will check if the partition table is correct,
 * adjust it and check if arduino partitions are compliant with zephyr. It is expected to have
 * partitions already formatted adequately, the only thing that will be performed eventually in
 * format is write the wifi fw in the partition from the file already present in /wlan/4343WA1.BIN
 */

class FlashFormatterZephyrMigrate : public FlashFormatterBase {
public:
  FlashFormatterZephyrMigrate ();
  bool checkPartition() override;
  bool formatPartition() override;

protected:
  struct {
    bool mbr_ok;
    bool certificates_ok;
    bool wififw_file_ok;
    bool wififw_ok;
  } qspi_fs_conditions;
};

