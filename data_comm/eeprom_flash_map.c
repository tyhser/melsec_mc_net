#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include "eeprom_flash_map.h"
#include "common.h"

#define DEVICE_FILE_KV "/sys/bus/i2c/devices/0-0050/eeprom"

#define GetKerverVersioCMD  "cat  /proc/version"
#define GetRtcChipNameCMD   "cat  /sys/class/rtc/rtc0/name"
#define KV3  "3.2.0"
#define KV4  "4.14.40"
#define KV_RK3328  "4.4.114"
/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

static int eeprom_fb = 0;

static int ExecuteCMD(char *cmd, unsigned char *result, int size)
{
	FILE *fp = popen(cmd, "r");
	if (NULL == fp) {
		LOG_E("popen error");
		return -1;
	}

	int nbytes = fread(result, sizeof(unsigned char), size, fp);
	if (nbytes > 0 && nbytes < size - 1)
		result[nbytes] = '\0';

	pclose(fp);
	return nbytes;
}

static int GetKernelVerion(unsigned char *result, int size)
{
	int tmp = ExecuteCMD(GetKerverVersioCMD, result, size);
	return tmp;
}

static int GetRtcChipName(unsigned char *result, int size)
{
	int tmp = ExecuteCMD(GetRtcChipNameCMD, result, size);
	return tmp;
}

int isKV3(void)
{
	unsigned char result[512] = { 0 };
	int size = 512;
	int tmp = 0;
	tmp = GetKernelVerion(result, size);
	if (tmp < 0) {
		return -1;
	}
	if (strstr((char *) result, KV3) != NULL) {
		return 0;
	} else {
		return 1;
	}
}

int isKV4(void)
{
	unsigned char result[512] = { 0 };
	int size = 512;
	int tmp = 0;
	tmp = GetKernelVerion(result, size);
	if (tmp < 0) {
		return -1;
	}
	if (strstr((char *) result, KV4) != NULL) {
		return 0;
	} else {
		return 1;
	}
}

int isKV_RK3328(void)
{
	unsigned char result[512] = { 0 };
	int size = 512;
	int tmp = 0;
	tmp = GetKernelVerion(result, size);
	if (tmp < 0) {
		return -1;
	}
	if (strstr((char *) result, KV_RK3328) != NULL) {
		return 0;
	} else {
		return 1;
	}
}

static int isPcf8563RtcChip()
{
	unsigned char result[512] = { 0 };
	int size = 512;
	int tmp = 0;
	tmp = GetRtcChipName(result, size);
	if (tmp < 0) {
		return -1;
	}
	if (strstr((char *) result, "rtc-pcf8563") != NULL) {
		return 0;
	}

	return -1;
}

UNUSE static int isRx8025RtcChip()
{
	unsigned char result[512] = { 0 };
	int size = 512;
	int tmp = 0;
	tmp = GetRtcChipName(result, size);
	if (tmp < 0) {
		return -1;
	}
	if (strstr((char *) result, "rtc-rx8025") != NULL) {
		return 0;
	}

	return -1;
}

/**
 * @brief open file get fd
 *
 * @param [in] char* file_path
 * @param [out] None
 *
 * @return
 */
int eeprom_open(char *file_path)
{
	int fd;
	fd = open(file_path, O_RDONLY);
	if (fd < 0) {
		LOG_E("Open %s fail", file_path);
		return -1;
	}
	return fd;
}

/**
 * @brief close file release fd
 *
 * @param [in] int fd
 * @param [out] None
 *
 * @return
 */
int eeprom_close(int fd)
{
	if (fd > 0) {
		close(fd);
		return 0;
	} else {
		return -1;
	}
}

/**
 * @brief dump eeprom info
 *
 * @param [in] int fd
 * @param [in] int base
 * @param [in] int max_length
 * @param [out] buf
 *
 * @return
 */
int eeprom_dump(int fd, int base, int max_length, char *buf)
{

	char tmp_buf[256] = { 0 };
	int result = 0;
	int size = 0;

	if (fd < 0 || base < 0)
		return -1;

	if ((base + max_length + 1) > EEPROM_SIZE)
		return -1;

	lseek(fd, base, SEEK_SET);

	result = read(fd, tmp_buf, max_length + 1);

	if (result < 1)
		return -1;

	size = tmp_buf[0];

	if (size > max_length || size < 0)
		return -1;

	if (buf == NULL) {
		tmp_buf[1 + size] = '\0';
		LOG_I("base : %04x, length : %04d, context : %s\n", base, size,
			  (tmp_buf + 1));
	} else {
		memcpy(buf, tmp_buf + 1, size);
		return size;
	}
	return 0;
}

/**
 * @brief eeprom write
 *
 * @param [in] int fd
 * @param [in] int base
 * @param [in] int length
 * @param [in] char *buf
 *
 * @return int
 */
int eeprom_write(int fd, int base, int length, char *buf)
{

	int result = 0;
	char size = (char) length;

	if (fd < 0 || base < 0 || length < 0 || buf == NULL)
		return -1;

	if ((base + length + 1) > EEPROM_SIZE)
		return -1;

	// write length
	lseek(fd, base, SEEK_SET);
	result = write(fd, &size, 1);

	if (result != 1)
		return -1;

	// write body
	lseek(fd, base + 1, SEEK_SET);
	result = write(fd, buf, length);

	if (result != length)
		return -1;

	return 0;

}

/**
 * @brief dump flash spec section context
 *
 * @param [in] int base
 * @param [in] int max_length
 * @param [in] char *buf
 * @param [out] None
 *
 * @return
 */
int am335x_eeprom_dump(int base, int max_length, char *buf)
{
	int result;
	int eeprom_fb;
	eeprom_fb = eeprom_open(DEVICE_FILE_KV);
	if (eeprom_fb < 0) {
		return -1;
	}

	result = eeprom_dump(eeprom_fb, base, max_length, buf);

	eeprom_close(eeprom_fb);

	return result;
}

/**
 * @brief write eeprom spec section context
 *
 * @param [in] int base
 * @param [in] int max_length
 * @param [in] char* buf
 * @param [out] None
 *
 * @return
 */
int am335x_eeprom_write(int base, int max_length, char *buf)
{
	int result = 0, UNUSE ret = 0;

	eeprom_fb = eeprom_open(DEVICE_FILE_KV);

	if (eeprom_fb < 0) {
		return -1;
	}

	ret =
		system
		("echo 0  > /sys/class/leds/eeprom-write-control/brightness");
	if (strlen(buf) > max_length)
		result = eeprom_write(eeprom_fb, base, max_length, buf);
	else
		result = eeprom_write(eeprom_fb, base, strlen(buf), buf);

	ret =
		system
		("echo 255  > /sys/class/leds/eeprom-write-control/brightness");
	eeprom_close(eeprom_fb);
	return result;

}

/************2018.9.29 zw 重新封装外部文件调用接口*************/

/*获取dump_magic
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_get_magic(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_dump(MAGIC_BASE, MAGIC_MAX_LENGTH, buffer);
}

/*设置dump_magic
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_set_magic(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_write(MAGIC_BASE, MAGIC_MAX_LENGTH, buffer);
}

/*获取produce_time
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_get_produce_time(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_dump(PRODUCE_TIME_BASE, PRODUCE_TIME_MAX_LENGTH,
							  buffer);
}

/*设置produce_time
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_set_produce_time(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_write(PRODUCE_TIME_BASE, PRODUCE_TIME_MAX_LENGTH,
							   buffer);
}

/*获取produce_name
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_get_produce_name(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_dump(PRODUCT_NAME_BASE, PRODUCT_NAME_MAX_LENGTH,
							  buffer);
}

/*设置produce_name
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_set_produce_name(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_write(PRODUCT_NAME_BASE, PRODUCT_NAME_MAX_LENGTH,
							   buffer);
}

/*获取hardware_ver
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_get_hardware_ver(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_dump(HARDWARE_VER_BASE, HARDWARE_VER_MAX_LENGTH,
							  buffer);
}

/*设置hardware_ver
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_set_hardware_ver(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_write(HARDWARE_VER_BASE, HARDWARE_VER_MAX_LENGTH,
							   buffer);
}

/*获取product_type
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_get_product_type(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_dump(PRODUCT_TYPE_BASE, PRODUCT_TYPE_MAX_LENGTH,
							  buffer);
}

/*设置product_type
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_set_product_type(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_write(PRODUCT_TYPE_BASE, PRODUCT_TYPE_MAX_LENGTH,
							   buffer);
}

/*获取product_sn
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_get_product_sn(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_dump(PRODUCT_SN_BASE, PRODUCT_SN_MAX_LENGTH,
							  buffer);
}

/*设置product_sn
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_set_product_sn(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_write(PRODUCT_SN_BASE, PRODUCT_SN_MAX_LENGTH,
							   buffer);
}

/*获取product_key
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_get_product_key(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_dump(PRODUCT_KEY_BASE, PRODUCT_KEY_MAX_LENGTH,
							  buffer);
}

/*设置product_key
返回值：-1：失败，成功返回实际数据长度
*/
int eeprom_set_product_key(char *buffer)
{
	if (NULL == buffer)
		return -1;

	return am335x_eeprom_write(PRODUCT_KEY_BASE, PRODUCT_KEY_MAX_LENGTH,
							   buffer);
}
