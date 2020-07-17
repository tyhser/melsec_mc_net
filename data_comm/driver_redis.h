#ifndef DLT_REDIS_H
#define DLT_REDIS_H
#include "hiredis/hiredis.h"
#include "cjson_cli.h"

#define HOST_STR_MAX 40

typedef struct {
    redisContext *redis;
    char host[HOST_STR_MAX];
    int  port;

} dlt_redis_context_t;


redisContext *redis_connect(void);
int redis_cli_update_database(redisContext *rds,
                              char *devicename,
                              char *ds,
                              long time,
                              char *value_str,
                              char *dataname,
                              int cycle);

/**
 * @brief   Call this function to get unique device name list or unique device num.
 * @param[in] dev_list      memory for save device list.
 * @param[in] device_num   device_num for save device name.
 * @return unique device num
 */
int redis_cli_get_device_list(char (*dev_list)[DEVICE_NAME_LEN_MAX], int device_num);
int redis_cli_get_device_num(void);
void redis_cli_publish_msg(char *str);

char *redis_get_dlt_config(void);

#endif
