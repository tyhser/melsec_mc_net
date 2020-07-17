#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "driver_redis.h"
#include "common.h"
#include "cjson_cli.h"
#include "melsec_mc_bin.h"

uint32_t get_unix_timestamp(void)
{
    uint32_t unix_timestamp = 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unix_timestamp = tv.tv_sec;
    return unix_timestamp;
}

int import_data_from_device(char *data_address, char *type, int fd, char *data, int len)
{
    if (NULL != strstr(type, "bool")) {
        val = false;
        ret = mc_read_bool(fd, data_address, &val);
        LOG_I("Read\t %s \tbool:\t %d\n", data_address, val);

    } else if (NULL != strstr(type, "short")) {
        short us_val = 0;
        ret = mc_read_short(fd, data_address, &us_val);
        LOG_I("Read\t %s \tshort:\t %d\n", data_address, us_val);
    } else if (NULL != strstr(type, "ushort")) {
        ushort us_val = 0;
        ret = mc_read_ushort(fd, data_address, &us_val);
        LOG_I("Read\t %s \tushort:\t %d\n", data_address, us_val);
   
    } else if (NULL != strstr(type, "int32")) {
        int32_t i_val = 0;
        ret = mc_read_int32(fd, data_address, &i_val);
        LOG_I("Read\t %s \tint32:\t %d\n", data_address, i_val);

    } else if (NULL != strstr(type, "uint32")) {
        uint32_t ui_val = 0;
        ret = mc_read_uint32(fd, data_address, &ui_val);
        LOG_I("Read\t %s \tuint32:\t %d\n", data_address, ui_val);

    } else if (NULL != strstr(type, "int64")) {
        int64 i64_val = 0;
        ret = mc_read_int64(fd, data_address, &i64_val);
        LOG_I("Read\t %s \tint64:\t %lld\n", data_address, i64_val);

    } else if (NULL != strstr(type, "uint64")) {
        int64 ui64_val = 0;
        ret = mc_read_uint64(fd, data_address, &ui64_val);
        LOG_I("Read\t %s \tuint64:\t %lld\n", data_address, ui64_val);

    } else if (NULL != strstr(type, "float")) {
        float f_val = 0;
        ret = mc_read_float(fd, data_address, &f_val);
        LOG_I("Read\t %s \tfloat:\t %f\n", data_address, f_val);

    } else if (NULL != strstr(type, "double")) {
        double d_val = 0;
        ret = mc_read_double(fd, data_address, &d_val);
        LOG_I("Read\t %s \tdouble:\t %lf\n", data_address, d_val);

    } else if (NULL != strstr(type, "string")) {
        char str_val[100];
        ret = mc_read_string(fd, data_address, length, str_val);
        LOG_I("Read\t %s \tstring:\t %s\n", data_address, str_val);
    } else {
        LOG_W("[SYSTEM] unknow type");
    }
}

void *dlt_collect_thread(void *arg)
{
    int ret = 0;
    node_t *device_pos = NULL;
    node_t *point_pos = NULL;
    node_t *device_list_header = get_device_list_header();
    char ds[9] = {0};
    char buf[DATA_STRING_SIZE_MAX] = {0};
    redisContext *rds = redis_connect();

    while (1) {
        list_for_each(device_pos, device_list_header) {
            device_t *device = (device_t *)device_pos;

            list_for_each(point_pos, &device->data_point_header) {
                data_point_t *data_point = (data_point_t *)point_pos;

                if (data_point->time_count == 0) {
                    if (NULL != strstr(data_point->type, "bool")) {
                        uint8_t val = false;
                        ret = mc_read_bool(fd, data_point->address, &val);
                        snprintf(buf, 2, "%d", val);
                        LOG_I("Read\t %s \tbool:\t %d", data_point->address, val);

                    } else if (NULL != strstr(data_point->type, "short")) {
                        int16_t us_val = 0;
                        ret = mc_read_short(fd, data_point->address, &us_val);
                        snprintf(buf, 6, "%d", val);
                        LOG_I("Read\t %s \tshort:\t %d", data_point->address, us_val);
                    } else if (NULL != strstr(data_point->type, "ushort")) {
                        uint16_t us_val = 0;
                        ret = mc_read_ushort(fd, data_point->address, &us_val);
                        snprintf(buf, 7, "%u", val);
                        LOG_I("Read\t %s \tushort:\t %d", data_point->address, us_val);
   
                    } else if (NULL != strstr(data_point->type, "int32")) {
                        int32_t i_val = 0;
                        ret = mc_read_int32(fd, data_point->address, &i_val);
                        snprintf(buf, 12, "%d", val);
                        LOG_I("Read\t %s \tint32:\t %d", data_point->address, i_val);

                    } else if (NULL != strstr(data_point->type, "uint32")) {
                        uint32_t ui_val = 0;
                        ret = mc_read_uint32(fd, data_point->address, &ui_val);
                        snprintf(buf, 12, "%u", val);
                        LOG_I("Read\t %s \tuint32:\t %d", data_point->address, ui_val);

                    } else if (NULL != strstr(data_point->type, "int64")) {
                        int64_t i64_val = 0;
                        ret = mc_read_int64(fd, data_point->address, &i64_val);
                        snprintf(buf, 22, "%d", val);
                        LOG_I("Read\t %s \tint64:\t %lld", data_point->address, i64_val);

                    } else if (NULL != strstr(data_point->type, "uint64")) {
                        uint64_t ui64_val = 0;
                        ret = mc_read_uint64(fd, data_point->address, &ui64_val);
                        snprintf(buf, 22, "%u", val);
                        LOG_I("Read\t %s \tuint64:\t %lld", data_point->address, ui64_val);

                    } else if (NULL != strstr(data_point->type, "float")) {
                        float f_val = 0;
                        ret = mc_read_float(fd, data_point->address, &f_val);
                        snprintf(buf, 10, "%f", val);
                        LOG_I("Read\t %s \tfloat:\t %f", data_point->address, f_val);

                    } else if (NULL != strstr(data_point->type, "double")) {
                        double d_val = 0;
                        ret = mc_read_double(fd, data_point->address, &d_val);
                        snprintf(buf, 20, "%lf", val);
                        LOG_I("Read\t %s \tdouble:\t %lf", data_point->address, d_val);

                    } else if (NULL != strstr(data_point->type, "string")) {
                        char str_val[100];
                        ret = mc_read_string(fd, data_point->data_address, length, str_val);
                        strcpy(buf, str_val);
                        LOG_I("Read\t %s \tstring:\t %s", data_point->data_address, str_val);
                    } else {
                        LOG_W("[SYSTEM] unknow type");
                    }

                    json_config_device_list_write_lock();
                    memcpy(data_point->data, buf, DATA_STRING_SIZE_MAX);
                    data_point->time_stamp = get_unix_timestamp();

                    json_config_device_list_unlock();
                    LOG_I("[CYCLE] data:[%s]", data_point->data);
                    snprintf(ds, 9, "%08x", data_point->ds);
                    redis_cli_update_database(rds, device->name, ds, data_point->time_stamp,
                                              (char *)data_point->data, data_point->name, data_point->cycle);

                    data_point->time_count = data_point->cycle/100;
                } else {
                    data_point->time_count--;
                }
            }
        }

        usleep(100000);
    }
}

