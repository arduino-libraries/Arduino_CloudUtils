/*
  Copyright (c) 2025 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#if (defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) \
  || defined(ARDUINO_OPTA) || defined(ARDUINO_GIGA)) && \
  defined(ARDUINO_ARCH_ZEPHYR)
#include "FlashFormatterZephyrMigrate.h"
#include <Arduino_DebugUtils.h>

// #define AIROC_WIFI_INCLUDE_FW

#if defined(CONFIG_AIROC_WIFI_CUSTOM) && defined(AIROC_WIFI_INCLUDE_FW)
#include "firmware/4343WA1_bin.h"
#endif

#if defined(CONFIG_AIROC_WIFI_CUSTOM)
#define WIFI_FW_SIZE  421098
#define WIFI_FW_FILE  "/wlan:/4343WA1.BIN"
#define WIFI_FW_CRC32 0xEAFE5F02
#endif

#include "certificates/zephyr_certificates.h"

#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/fs.h>
#include <zephyr/sys/crc.h>

#define MBR_SECTOR_SIZE 512

#define CERTIFICATE_FILE "/wlan:/cacert.pem"

#define CHUNK_SIZE 256

struct __attribute__((packed)) mbrEntry {
	uint8_t status;
	uint8_t chsStart[3];
	uint8_t type;
	uint8_t chsStop[3];
	uint32_t lbaOffset;
	uint32_t lbaSize;
};

struct __attribute__((packed)) mbrTable {
	mbrEntry entries[4];
	uint8_t signature[2];
};

static const struct mbrTable expected_mbr_table = {
	.entries = {
		// Partition 1: WLAN (FAT32) - starts at 0x1000, size ~1020KB (255 blocks of 4KB)
		{
			.status    = 0x00,                        // Not bootable
			.chsStart  = { 0, 2, 0 },
			.type      = 0x0B,                        // FAT32
			.chsStop   = { 4, 0, 0 },
			.lbaOffset = 1,                           // 0x1000 / 4096 = 1
			.lbaSize   = 255,                         // (1MB - 4KB) / 4096 = 255
		},
		// Partition 2: OTA (FAT32) - starts at 0x100000, size 5MB (1280 blocks of 4KB)
		{
			.status    = 0x00,
			.chsStart  = { 4, 1, 0 },
			.type      = 0x0B,                         // FAT32
			.chsStop   = { 24, 0, 0 },
			.lbaOffset = 256,                          // 0x100000 / 4096 = 256
			.lbaSize   = 1280,                         // 5MB / 4096 = 1280
		},
		// Partition 3: KVS (FAT32) - starts at 0x600000, size 1MB (256 blocks of 4KB)
		{
			.status    = 0x00,
			.chsStart  = { 24, 1, 0 },
			.type      = 0x0B,                        // FAT32
			.chsStop   = { 28, 0, 0 },
			.lbaOffset = 1536,                        // 0x600000 / 4096 = 1536
			.lbaSize   = 256,                         // 1MB / 4096 = 256
		},
		// Partition 4: Storage (LittleFS/FAT32) - starts at 0x700000, size 7MB (1792 blocks of 4KB)
		{
			.status    = 0x00,
			.chsStart  = { 28, 1, 0 },
			.type      = 0x83,                        // FAT32 (could be 0x83 for LittleFS)
			.chsStop   = { 56, 0, 0 },
			.lbaOffset = 1792,                        // 0x700000 / 4096 = 1792
			.lbaSize   = 1792,                        // 7MB / 4096 = 1792
		},
	},
	.signature = { 0x55, 0xAA },
};


FlashFormatterZephyrMigrate::FlashFormatterZephyrMigrate()
: qspi_fs_conditions{false, false, false, false} {
}

bool FlashFormatterZephyrMigrate::checkPartition() {
	struct mbrTable table;
	const struct flash_area *fa;
	int rc = 0;

	// We read the MBR partition on the board and check it against the expected values
	if ((rc = flash_area_open(PARTITION_ID(mbr_partition), &fa)) != 0) {
		DEBUG_ERROR("Error while opening mbr partition, %d", rc);
		qspi_fs_conditions.mbr_ok = false;
		return false;
	}

	// Read MBR from sector 0
	// MBR is in the last 66 bytes of sector 0 (at offset 446)
	if ((rc = flash_area_read(fa, MBR_SECTOR_SIZE - sizeof(table),
				  &table, sizeof(table))) != 0) {
		DEBUG_ERROR("Error while reading mbr table: %d", rc);
		qspi_fs_conditions.mbr_ok = false;
		return false;
	}

	if (table.signature[0] != 0x55 || table.signature[1] != 0xAA) {
		DEBUG_ERROR("MBR partition invalid signature");
		qspi_fs_conditions.mbr_ok = false;
		return false;
	}

	for (int i = 0; i < 3; i++) {
		const struct mbrEntry *got = &table.entries[i], *exp = &expected_mbr_table.entries[i];
		if (got->type != exp->type || got->lbaOffset != exp->lbaOffset ||
		got->lbaSize != exp->lbaSize) {
			DEBUG_ERROR("MBR partition %d invalid: type 0x%02x offset %u size %u",
				i + 1, got->type, (unsigned)got->lbaOffset, (unsigned)got->lbaSize);
			qspi_fs_conditions.mbr_ok = false;
			return false;
		}
	}

#ifdef CHECK_USER_PARTITION
	// User storage partition, NOTE we can skip verifying this
	// The partition can be either fat32 or littlefs
	if (table.entries[3].type != 0x0B || table.entries[3].type != 0x83) {
		DEBUG_ERROR("MBR partition value invalid: partition 3 type %d",
			    table.entries[3].type);
		qspi_fs_conditions.mbr_ok = false;
		return false;
	}

	if (table.entries[3].lbaOffset != expected_mbr_table.entries[3].lbaOffset) {
		DEBUG_ERROR("MBR partition value invalid: partition 3 lbaOffset %d",
			    table.entries[3].lbaOffset);
		qspi_fs_conditions.mbr_ok = false;
		return false;
	}
	if (table.entries[3].lbaSize != expected_mbr_table.entries[3].lbaSize) {
		DEBUG_ERROR("MBR partition value invalid: partition 3 lbaSize %d",
			    table.entries[3].lbaSize);
		qspi_fs_conditions.mbr_ok = false;
		return false;
	}
#endif // CHECK_USER_PARTITION
	qspi_fs_conditions.mbr_ok = true;

	struct fs_dirent entry;
	struct fs_file_t file;
	fs_file_t_init(&file);

	rc = fs_stat(CERTIFICATE_FILE, &entry);

	if (rc != 0) {
		DEBUG_WARNING("WARN: certificates not present in qspi flash");
		qspi_fs_conditions.certificates_ok = false;
	} else if (entry.size != cacert_pem_len) {
		DEBUG_WARNING("WARN: certificates are not up to date %d %d", entry.size, cacert_pem_len);
		qspi_fs_conditions.certificates_ok = false;
	} else {
		qspi_fs_conditions.certificates_ok = true;
	}

#if defined(CONFIG_AIROC_WIFI_CUSTOM) // only in case of stm32h7
	// we don't need the wifi fw file to be present, when migrating from mbed to zephyr, but
	// if the fw is not present in the proper partition, we cannot use it
	if ((rc = fs_stat(WIFI_FW_FILE, &entry)) != 0) {
		DEBUG_WARNING("WARN: airoc_firmware file do not exist");
		qspi_fs_conditions.wififw_file_ok = false;
	} else if (entry.size != (size_t)WIFI_FW_SIZE) {
		DEBUG_WARNING("WARN: airoc_firmware file has incorrect size");
		qspi_fs_conditions.wififw_file_ok = false;
	} else {
		qspi_fs_conditions.wififw_file_ok = true;
	}

	if ((rc = flash_area_open(PARTITION_ID(airoc_firmware), &fa)) != 0) {
		DEBUG_ERROR("Error opening airoc_firmware partition: %d", rc);
		qspi_fs_conditions.wififw_ok = false;
		return false;
	}

	uint8_t raw[CHUNK_SIZE];
	uint32_t crc = 0;

	// We check that the wifi fw is correct by calculating the crc of it. WIFI_FW_CRC32 contains
	// the expected value
	for (size_t off = 0; off < WIFI_FW_SIZE && rc == 0; off += sizeof(raw)) {
		size_t n = MIN(sizeof(raw), WIFI_FW_SIZE - off);

		rc = flash_area_read(fa, off, raw, n);
		if (rc == 0) {
			crc = crc32_ieee_update(crc, raw, n);
		}
	}

	if (crc != WIFI_FW_CRC32) {
		DEBUG_ERROR("Error the wifi fw partition do not contain a valid wifi fw");
		qspi_fs_conditions.wififw_ok = false;
	} else {
		qspi_fs_conditions.wififw_ok = true;
	}
#endif // defined(CONFIG_AIROC_WIFI_CUSTOM)
out:
	return qspi_fs_conditions.mbr_ok &&
		qspi_fs_conditions.certificates_ok &&
		// qspi_fs_conditions.wififw_file_ok && // the wifi fw file is not required in zephyr
		qspi_fs_conditions.wififw_ok;
}


#if defined(CONFIG_AIROC_WIFI_CUSTOM) && defined(AIROC_WIFI_INCLUDE_FW)
static int flashWiFiFirmware() {
	DEBUG_VERBOSE("WiFi firmware size: %d bytes", sizeof(wifi_firmware_image_data));

	const struct flash_area *fa;
	int ret = flash_area_open(PARTITION_ID(airoc_firmware), &fa);
	if (ret) {
		DEBUG_ERROR("Error opening airoc_firmware partition: %d", ret);
		return ret;
	}

	DEBUG_VERBOSE("Erasing WiFi firmware partition...");
	ret = flash_area_erase(fa, 0, fa->fa_size);
	if (ret) {
		DEBUG_ERROR("Error erasing airoc_firmware partition: %d", ret);
		flash_area_close(fa);
		return ret;
	}

	DEBUG_VERBOSE("Writing WiFi firmware...");
	ret = flash_area_write(fa, 0, wifi_firmware_image_data, sizeof(wifi_firmware_image_data));
	if (ret) {
		DEBUG_ERROR("Error writing WiFi firmware: %d", ret);
		flash_area_close(fa);
		return ret;
	}

	flash_area_close(fa);
	DEBUG_VERBOSE("WiFi firmware written successfully!");

	return 0;
}
#endif // defined(CONFIG_AIROC_WIFI_CUSTOM) && defined(AIROC_WIFI_INCLUDE_FW)

static int copyWiFiFirmware() {
	static uint8_t buf[CHUNK_SIZE];
	const struct flash_area *fa;
	struct fs_dirent entry;
	struct fs_file_t file;
	size_t align;
	int rc;

	rc = fs_stat(WIFI_FW_FILE, &entry);
	if (rc) {
		return rc;
	}
	if (entry.type != FS_DIR_ENTRY_FILE || entry.size == 0) {
		return -EINVAL;
	}

	rc = flash_area_open(PARTITION_ID(airoc_firmware), &fa);
	if (rc) {
		return rc;
	}

	align = flash_area_align(fa);
	if (align == 0 || CHUNK_SIZE % align != 0) {
		rc = -ENOTSUP;
		goto out;
	}

	/* The whole partition is always a multiple of the erase size. */
	rc = flash_area_erase(fa, 0, fa->fa_size);
	if (rc) {
		goto out;
	}

	fs_file_t_init(&file);
	rc = fs_open(&file, WIFI_FW_FILE, FS_O_READ);
	if (rc) {
		goto out;
	}

	for (size_t off = 0; off < entry.size; off += CHUNK_SIZE) {
		ssize_t n = fs_read(&file, buf, CHUNK_SIZE);
		if (n < 0) {
			rc = n;
			break;
		}
		size_t write_len = ROUND_UP(n, align);

		/* Pad a short last chunk to the write block size with the erased value. */
		memset(buf + n, flash_area_erased_val(fa), write_len - n);
		rc = flash_area_write(fa, off, buf, write_len);
		if (rc) {
			break;
		}
	}
	fs_close(&file);
out:
	flash_area_close(fa);
	return rc;
}

static int flashCertificates() {
	DEBUG_VERBOSE("Certificate size: %d bytes", cacert_pem_len);

	struct fs_file_t file;
	fs_file_t_init(&file);

	fs_unlink(CERTIFICATE_FILE);

	DEBUG_VERBOSE("Opening " CERTIFICATE_FILE " for writing...");
	int rc = fs_open(&file, CERTIFICATE_FILE, FS_O_CREATE | FS_O_WRITE);
	if (rc != 0) {
		DEBUG_ERROR("Error opening cacert.pem: ", rc);
		return rc;
	}

	DEBUG_VERBOSE("Writing certificates...");
	ssize_t ret = fs_write(&file, cacert_pem, cacert_pem_len);
	if (ret < 0) {
		DEBUG_ERROR("Error writing certificates: %d", ret);
		fs_close(&file);
		return rc;
	}

	rc = fs_sync(&file);
	if (rc != 0) {
		DEBUG_ERROR("Warning: fs_sync failed: ", rc);
		fs_close(&file);
		return rc;
	}

	fs_close(&file);
	DEBUG_VERBOSE("Certificates written successfully!");
	return 0;
}

bool FlashFormatterZephyrMigrate::formatPartition() {
	int rc = 0;
	// In case of migration we only act when mbr partitions are ok
	if (!qspi_fs_conditions.mbr_ok) {
		DEBUG_ERROR("Unable to migrate, mbr table is not consistent");
		return false;
	}

	if (!qspi_fs_conditions.certificates_ok) {
		rc = flashCertificates();
		if (rc) {
			DEBUG_ERROR("Error while flashing certificates: %d", rc);
		} else {
			qspi_fs_conditions.certificates_ok = true;
		}
	}

#if defined(CONFIG_AIROC_WIFI_CUSTOM) // only in case of stm32h7
	// if the wififw is ok we have nothing to do
	if (qspi_fs_conditions.wififw_ok) {
		return true;
	}

	// AIROC_WIFI_INCLUDE_FW
	// if the wififw is not properly flashed in the partition we can try to recover it:
	//  - if AIROC_WIFI_INCLUDE_FW is defined, we can use the fw included in the binary to flash
	//  - if there is the wifi fw binary we will use that to flash
	//  - otherwise error is reported and the board cannot connect
#if defined(AIROC_WIFI_INCLUDE_FW)
	rc = flashWiFiFirmware();
	if (rc) {
		DEBUG_ERROR("Error while flashing certificates: %d", rc);
		return false;
	}
	qspi_fs_conditions.wififw_ok = true;
#else
	if (qspi_fs_conditions.wififw_file_ok) {
		rc = copyWiFiFirmware();
		if (rc) {
			DEBUG_ERROR("Error while flashing certificates: %d", rc);
			return false;
		}
		qspi_fs_conditions.wififw_ok = true;
	} else {
		DEBUG_ERROR("Wifi firmware not present, please compile with `AIROC_WIFI_INCLUDE_FW`"
			"enabled or run zephyr example in Storage/FlashFormat");
		return false;
	}
#endif // defined(AIROC_WIFI_INCLUDE_FW)
#endif // defined(CONFIG_AIROC_WIFI_CUSTOM)

	return true;
}

#endif /* (defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) \
  || defined(ARDUINO_OPTA) || defined(ARDUINO_GIGA)) && \
  defined(ARDUINO_ARCH_ZEPHYR) */
