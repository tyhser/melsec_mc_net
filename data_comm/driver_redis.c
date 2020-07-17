#include <string.h>
#include <stdlib.h>
#include "dlt645.h"
#include "cjson_cli.h"
#include "dlt_redis.h"

#define REDIS_UNIX_DOMAIN_SOCKET_PATH "/tmp/redis-naos.sock"

#ifdef FEATURE_USE_UNIX_SOCKET_CONNECT_REDIS
static dlt_redis_context_t redis_context = {
    .host = "127.0.0.1",
    .port = 6379,
};
#endif

redisContext *redis_connect(void)
{
    struct timeval timeout = {3, 0}; 

#ifdef FEATURE_USE_UNIX_SOCKET_CONNECT_REDIS
    redisContext *ret = redisConnectWithTimeout(redis_context.host, redis_context.port, timeout);
#else
    redisContext *ret = redisConnectUnixWithTimeout(REDIS_UNIX_DOMAIN_SOCKET_PATH, timeout);
#endif

    if (ret == NULL || ret->err) {
        if (ret) {
            redisFree(ret);
        }
        LOG_E("[Redis] Cannot connect to Redis");
        ret = NULL;
    }
    return ret;
}

int redis_cli_update_database(redisContext *rds,
                              char *devicename,
                              char *data_address,
                              long time,
                              char *value_str,
                              char *path,
                              char *type,
                              int cycle)
{
    redisReply *reply = NULL;

    if ((NULL == rds) ||
        (NULL == devicename) ||
        (NULL == data_address) ||
        (NULL == path) ||
        (NULL == type)) {
        return -1;
    }

    reply = redisCommand(rds,
            "HMSET s2n/melsec/%s/%s path %s value %s type %s time %ld",
            devicename, data_address, path, value_str, type, time);
            //LOG_I("HMSET s2n/melsec/%s/%s name %s value %s time %ld", devicename, ds, dataname, value_str, type, time);
    if (reply != NULL) {
        if ((reply->type == REDIS_REPLY_ERROR) && (reply->len != 0)) {
            freeReplyObject(reply);
            return -1;
        }
    }
    freeReplyObject(reply);
    reply = redisCommand(rds, "EXPIRE s2n/melsec/%s/%s %d", devicename, data_address, 3 * cycle);
    //LOG_D("[REDIS] reply:%p", reply);

    if (0 == reply) {
        freeReplyObject(reply);
    }
    return 0;
}

int redis_cli_get_device_list(char (*dev_list)[DEVICE_NAME_LEN_MAX], int device_num)
{
    int dev_num = 0;
    int get_num = 0;
    redisReply *reply = NULL;
    char (*dev)[DEVICE_NAME_LEN_MAX] = dev_list;

    if (dev == NULL || device_num == 0) {
        get_num = 1;
    }

    redisContext *rds = redis_connect();
    if (NULL == rds) {
        return -1;
    }
    reply = redisCommand(rds, "KEYS s2n/dlt/*");
    if ((reply != NULL) && (reply->type == REDIS_REPLY_ARRAY)) {

        char (*str_temp)[DEVICE_NAME_LEN_MAX] = (char (*)[DEVICE_NAME_LEN_MAX])dlt_malloc(reply->elements * DEVICE_NAME_LEN_MAX);
        if (get_num) {
            dev = (char (*)[DEVICE_NAME_LEN_MAX])dlt_malloc(reply->elements * DEVICE_NAME_LEN_MAX);
        }

        for (int i = 0; i < reply->elements; i++) {
            char *str = reply->element[i]->str;
            char *p;
            for (int idx = 0; ((p = strsep(&str, "/")) != NULL); idx++) {

                /* e.g. s2n/dlt/m0/00000100, m0 is device name, position is 2*/
                if (idx == 2) {
                    if (strlen(p) < DEVICE_NAME_LEN_MAX) {
                        strcpy((str_temp[i]), p);
                    } else {
                        memcpy(str_temp[i], p, DEVICE_NAME_LEN_MAX - 1);
                    }
                }
            }
            FREE(str);
        }


        for (int idx = 0; idx < reply->elements; idx++) {
            if (idx == 0) {
                memset(dev[0], 0, DEVICE_NAME_LEN_MAX);
                strcpy(dev[0], str_temp[0]);
                dev_num = 1;
            }
            int same_cnt = 0;
            int dev_idx = 0;
            for (dev_idx = 0, same_cnt = 0; dev_idx < dev_num; dev_idx++) {
                if (strstr(dev[dev_idx], str_temp[idx]) != NULL) {
                   same_cnt++;
                }
            }
            if (same_cnt == 0) {
                memset(dev[dev_num], 0, DEVICE_NAME_LEN_MAX);
                strcpy(dev[dev_num], str_temp[idx]);
                dev_num++;
            }
        }
        FREE(str_temp);
        freeReplyObject(reply);
    } else {
        freeReplyObject(reply);
        return -1;
    }

    return dev_num;
}

/*TODO*/
#if 0
int redis_cli_get_device_data_list(char dev[DEVICE_NAME_LEN_MAX], char *data_num)
{
    int dev_num = 0;
    int get_num = 0;
    redisReply *reply = NULL;
    char (*dev)[DEVICE_NAME_LEN_MAX] = dev_list;

    redisContext *rds = redis_connect();
    if (dev == NULL || data_num == 0) {
        get_num = 1;
    }

    if (NULL == rds) {
        return -1;
    }
    reply = redisCommand(rds, "KEYS s2n/dlt/*");
    if ((reply != NULL) && (reply->type == REDIS_REPLY_ARRAY)) {

        char (*str_temp)[DEVICE_NAME_LEN_MAX] = (char (*)[DEVICE_NAME_LEN_MAX])dlt_malloc(reply->elements * DEVICE_NAME_LEN_MAX);
        if (get_num) {
            dev = (char (*)[DEVICE_NAME_LEN_MAX])dlt_malloc(reply->elements * DEVICE_NAME_LEN_MAX);
        }

        for (int i = 0; i < reply->elements; i++) {
            char *str = reply->element[i]->str;
            char *p;
            for (int idx = 0; ((p = strsep(&str, "/")) != NULL); idx++) {

                /* e.g. s2n/dlt/m0/00000100, m0 is device name, position is 2*/
                if (idx == 2) {
                    if (strlen(p) < DEVICE_NAME_LEN_MAX) {
                        strcpy((str_temp[i]), p);
                    } else {
                        memcpy(str_temp[i], p, DEVICE_NAME_LEN_MAX - 1);
                    }
                }
            }
            FREE(str);
        }


        for (int idx = 0; idx < reply->elements; idx++) {
            if (idx == 0) {
                memset(dev[0], 0, DEVICE_NAME_LEN_MAX);
                strcpy(dev[0], str_temp[0]);
                dev_num = 1;
            }
            int same_cnt = 0;
            int dev_idx = 0;
            for (dev_idx = 0, same_cnt = 0; dev_idx < dev_num; dev_idx++) {
                if (strstr(dev[dev_idx], str_temp[idx]) != NULL) {
                   same_cnt++;
                }
            }
            if (same_cnt == 0) {
                memset(dev[dev_num], 0, DEVICE_NAME_LEN_MAX);
                strcpy(dev[dev_num], str_temp[idx]);
                dev_num++;
            }
        }
        FREE(str_temp);
        freeReplyObject(reply);
    } else {
        freeReplyObject(reply);
        return -1;
    }

    return dev_num;
}
#endif

void redis_cli_publish_msg(char *str)
{
    redisReply *reply = NULL;

    if (NULL == str) {
        return;
    }
    redisContext *rds = redis_connect();
    if (NULL == rds) {
        return;
    }

    reply = redisCommand(rds, "PUBLISH publish.DriverUpData.json %s", str);
    freeReplyObject(reply);

    LOG_D("[REDIS] publish succ : %s\n", str);
}

char *redis_get_dlt_config(void)
{
    char *ret_str = NULL;
    redisReply *reply = NULL;

    redisContext *rds = redis_connect();
    if (NULL == rds) {
        return NULL;
    }

    reply = redisCommand(rds, "GET cfg_DLT");

    if ((reply != NULL) && (reply->type == REDIS_REPLY_STRING)) {

        ret_str = (char *)dlt_malloc((reply->len+1) * sizeof(char));
        strcpy(ret_str, reply->str);

        freeReplyObject(reply);
        return ret_str;
    } else {
        freeReplyObject(reply);
        return NULL;
    }
}
