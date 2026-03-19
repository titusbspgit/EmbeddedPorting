#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include "app_types.h"
#include "storage.h"

LOG_MODULE_REGISTER(storage, LOG_LEVEL_INF);

/* Mount LittleFS on 'log' partition defined in overlay */
#ifdef FIXED_PARTITION_ID
/* nothing */
#endif

static struct fs_mount_t lfs_mnt;

static int storage_mount(void)
{
	/* Use fixed partition labeled "log" */
#define LOG_PARTITION DT_NODELABEL(log_partition)
	BUILD_ASSERT(DT_NODE_HAS_STATUS(LOG_PARTITION, okay), "log partition missing");

	static struct fs_littlefs lfs;
	lfs_mnt.type = FS_LITTLEFS;
	lfs_mnt.fs_data = &lfs;
	lfs_mnt.storage_dev = (void *)FIXED_PARTITION_ID(LOG_PARTITION);
	lfs_mnt.mnt_point = "/lfs";

	int rc = fs_mount(&lfs_mnt);
	if (rc == -EEXIST) rc = 0;
	if (rc) {
		LOG_ERR("fs_mount failed: %d", rc);
	}
	return rc;
}

static int append_json_line(const struct env_record *rec)
{
	struct fs_file_t file;
	fs_file_t_init(&file);

	int rc = fs_open(&file, "/lfs/log.txt", FS_O_CREATE | FS_O_WRITE | FS_O_APPEND);
	if (rc) {
		LOG_ERR("fs_open: %d", rc);
		return rc;
	}

	char line[128];
	int len = snprintk(line, sizeof(line),
	                   "{\"temperature\":%d,\"humidity\":%d,\"light\":%d,\"pressure\":%d}\n",
	                   rec->temperature, rec->humidity, rec->light, rec->pressure);
	if (len < 0) {
		fs_close(&file);
		return -EINVAL;
	}

	rc = fs_write(&file, line, len);
	if (rc < 0) {
		LOG_ERR("fs_write: %d", rc);
		fs_close(&file);
		return rc;
	}

	(void)fs_sync(&file);
	fs_close(&file);
	return 0;
}

void storage_thread(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

	(void)storage_mount();

	struct env_record rec;
	while (1) {
		if (k_msgq_get(&log_q, &rec, K_FOREVER) == 0) {
			(void)append_json_line(&rec);
			LOG_INF("logged: T=%d H=%d L=%d P=%d", rec.temperature, rec.humidity, rec.light, rec.pressure);
		}
	}
}
