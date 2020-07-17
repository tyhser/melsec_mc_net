/**
 * @brief eeprom flash map define file
 * @file eeprom_flash_map.h
 * @version 1.0
 * @author panzidong
 * @date 2018-9-27 10:19:59
 * @par Copyright:
 */

#ifndef __EEPROM_FLASH_MAP_H__
#define __EEPROM_FLASH_MAP_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif							// __cplusplus

#define EEPROM_SIZE 1024

#define MAGIC_BASE 0x0
#define MAGIC_MAX_LENGTH 0xf

#define PRODUCE_TIME_BASE (MAGIC_BASE + MAGIC_MAX_LENGTH + 1)
#define PRODUCE_TIME_MAX_LENGTH 0xf

#define PRODUCT_NAME_BASE  (PRODUCE_TIME_BASE + PRODUCE_TIME_MAX_LENGTH + 1)
#define PRODUCT_NAME_MAX_LENGTH 0x1f

#define HARDWARE_VER_BASE (PRODUCT_NAME_BASE + PRODUCT_NAME_MAX_LENGTH + 1)
#define HARDWARE_VER_MAX_LENGTH 0xf

#define PRODUCT_TYPE_BASE  (HARDWARE_VER_BASE + HARDWARE_VER_MAX_LENGTH + 1)
#define PRODUCT_TYPE_MAX_LENGTH 0xf

#define PRODUCT_SN_BASE  (PRODUCT_TYPE_BASE + PRODUCT_KEY_MAX_LENGTH + 1)
#define PRODUCT_SN_MAX_LENGTH 0xf

#define PRODUCT_KEY_BASE  (PRODUCT_SN_BASE + PRODUCT_SN_MAX_LENGTH + 1)
#define PRODUCT_KEY_MAX_LENGTH  0xf

#define ALI_PRODUCT_KEY_BASE  (PRODUCT_KEY_BASE + PRODUCT_KEY_MAX_LENGTH + 1)
#define ALI_PRODUCT_KEY_MAX_LENGTH  0x3f

#define ALI_DEVICE_SECRET_BASE  (ALI_PRODUCT_KEY_BASE + ALI_PRODUCT_KEY_MAX_LENGTH + 1)
#define ALI_DEVICE_SECRET_MAX_LENGTH  0x3f

	typedef struct magic_struct {
		unsigned char magic_len;
		unsigned char magic[MAGIC_MAX_LENGTH];
	} maigc_map;

	typedef struct produce_time_struct {
		unsigned char produce_time_len;
		unsigned char produce_time[PRODUCE_TIME_MAX_LENGTH];
	} produce_time_map;

	typedef struct product_name_struct {
		unsigned char product_name_len;
		unsigned char product_name[PRODUCT_NAME_MAX_LENGTH];
	} product_name_map;

	typedef struct hardware_ver_struct {
		unsigned char hardware_ver_len;
		unsigned char hardware_ver[HARDWARE_VER_MAX_LENGTH];
	} hardware_ver_map;

	typedef struct product_type_struct {
		unsigned char product_type_len;
		unsigned char product_type[PRODUCT_TYPE_MAX_LENGTH];
	} product_type_map;

	typedef struct product_sn_struct {
		unsigned char product_sn_len;
		unsigned char product_sn[PRODUCT_SN_MAX_LENGTH];
	} product_sn_map;

	typedef struct product_key_struct {
		unsigned char product_key_len;
		unsigned char product_key[PRODUCT_KEY_MAX_LENGTH];
	} product_key_map;

	typedef struct ali_product_key_struct {
		unsigned char ali_product_key_len;
		unsigned char ali_product_key[ALI_PRODUCT_KEY_MAX_LENGTH];
	} ali_product_key_map;

	typedef struct ali_device_secret_struct {
		unsigned char ali_device_secret_len;
		unsigned char ali_device_secret[ALI_DEVICE_SECRET_MAX_LENGTH];
	} ali_device_secret_map;

	typedef struct eeprom_flash_map {
		maigc_map e_magic_map;
		produce_time_map e_produce_time_map;
		product_name_map e_product_name_map;
		hardware_ver_map e_hardware_ver_map;
		product_type_map e_product_type_map;
		product_sn_map e_product_sn_map;
		product_key_map e_product_key_map;
		ali_product_key_map e_ali_product_key_map;
		ali_device_secret_map e_ali_device_secret_map;
	} eeprom_flash_struc;

/************2018.9.29 zw 重新封装外部文件调用接口*************/

//内核版本3.2.0
	int isKV3(void);
//内核版本4.14.40
	int isKV4(void);
//内核版本4.4.114
	int isKV_RK3328(void);

/*获取dump_magic，
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_get_magic(char *buffer);

/*设置dump_magic，
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_set_magic(char *buffer);

/*获取produce_time
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_get_produce_time(char *buffer);

/*设置produce_time
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_set_produce_time(char *buffer);

/*获取produce_name
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_get_produce_name(char *buffer);

/*设置produce_name
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_set_produce_name(char *buffer);

/*获取hardware_ver
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_get_hardware_ver(char *buffer);

/*设置hardware_ver
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_set_hardware_ver(char *buffer);

/*获取product_type
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_get_product_type(char *buffer);

/*设置product_type
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_set_product_type(char *buffer);

/*获取product_sn
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_get_product_sn(char *buffer);

/*设置product_sn
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_set_product_sn(char *buffer);

/*获取product_key
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_get_product_key(char *buffer);

/*设置product_key
返回值：-1：失败，成功返回实际数据长度
*/
	int eeprom_set_product_key(char *buffer);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif							// __cplusplus
#endif							// __EEPROM_FLASH_MAP_H__
