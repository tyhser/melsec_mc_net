#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "cjson_cli.h"
#include "cJSON.h"
#include "common.h"
#include "eeprom_flash_map.h"
#include "system.h"

struct _config_context {
    node_t device_list_header;
    pthread_rwlock_t rwlock;
} config_context;

void json_cli_init(void)
{
    node_init(&config_context.device_list_header);
    pthread_rwlock_init(&config_context.rwlock, NULL);
}

void json_config_device_list_read_lock(void)
{
    pthread_rwlock_rdlock(&config_context.rwlock);
}

void json_config_device_list_write_lock(void)
{
    pthread_rwlock_wrlock(&config_context.rwlock);
}

void json_config_device_list_unlock(void)
{
    pthread_rwlock_unlock(&config_context.rwlock);
}

int compare_device_name(const void *node_pos, const void *name)
{
    if (!strcmp(((device_t *)node_pos)->name, name)) {
        return 1;
    } else {
        return 0;
    }

}

int compare_data_point_address(const void *node_pos, const void *address)
{
    if (!strcmp(((device_t *)node_pos)->address, address)) {
        return 1;
    } else {
        return 0;
    }

}

static void delete_config_cache(void)
{
    node_t *device_pos = NULL;
    node_t *device_list_header = get_device_list_header();
    device_t *device = NULL;

    json_config_device_list_write_lock();
    list_for_each(device_pos, device_list_header) {
        device = (device_t *)device_pos;

        node_remove_all_node(&device->data_point_header);
    }
    node_remove_all_node(device_list_header);

    json_config_device_list_unlock();
}

int parse_config_json(char *json_str)
{
    int device_num = 0;
    cJSON *json_root = NULL;
    cJSON *json_inter_conf = NULL;
    cJSON *json_driver_conf = NULL;
    cJSON *json_points = NULL;

    cJSON *json_entry = NULL;
    cJSON *json_device = NULL;
    cJSON *json_point_item = NULL;
    device_t *device = NULL;
#ifdef FEATURE_BEFORE_PARSE_CONFIG_DELETE_CACHE
    delete_config_cache();
#endif
    json_root = cJSON_Parse(json_str);

    if (json_root) {
        LOG_I("JSON:\n%s", cJSON_Print(json_root));

        json_inter_conf  = cJSON_GetObjectItem(json_root, KEY_INTER_CONFIG);
        if (json_inter_conf == NULL) {
            LOG_E("[JSON] parse device info fail");
            return -1;
        }

        device_num = cJSON_GetArraySize(json_inter_conf);

        if (device_num > DEVICE_NUM_MAX) {
            device_num = DEVICE_NUM_MAX;
            LOG_W("[JSON] Device number over!");
        }
        for (int idx = 0; idx < device_num; idx++) {
            json_device = cJSON_GetArrayItem(json_inter_conf, idx);
            if (json_device == NULL) {
                continue;
            }

            json_entry = cJSON_GetObjectItem(json_device, KEY_DEVICE_NAME);

            if (json_entry != NULL) {
                if (json_entry->type == cJSON_String) {
#ifdef FEATURE_DELETE_UNIQUE_NAME_DEVICE
                    if (node_find_subdata(&config_context.device_list_header, json_entry->valuestring, compare_device_name) == NULL)
#endif

                    {
                        device = (device_t *)dlt_malloc(sizeof(device_t));
                        if (!device) {
                            LOG_E("[JSON] malloc fail");
                            continue;
                        }
                        strncpy(device->name, json_entry->valuestring, DEVICE_NAME_LEN_MAX);
                        LOG_I("[JSON] set device name: [%s]", device->name);
                    }

#ifdef FEATURE_DELETE_UNIQUE_NAME_DEVICE
                    else
                    {
                        LOG_I("[JSON] device unique");
                        continue;
                    }
#endif
                } else {
                    LOG_W("[JSON] json entry type incorrect");
                    continue;
                }
            } else {
                LOG_E("[JSON] check [%s] fail", KEY_DEVICE_NAME);
                continue;
            }

            json_entry = cJSON_GetObjectItem(json_device, KEY_INTERFACE);
            if (json_entry != NULL) {
                if (json_entry->type == cJSON_String) {
                    if (!strstr(json_entry->valuestring, "socket") ||
                        !strstr(json_entry->valuestring, "com")) {
                        device->interface = SERIAL_IF;
                    } else {
                        device->interface = SOCKET_IF;
                    }
                    LOG_I("[JSON] set device interface:[%d](socket:%d, serial:%d)", device->interface, SOCKET_IF, SERIAL_IF);
                } else {
                    LOG_W("[JSON] json entry type incorrect");
                }
            } else {
                LOG_E("[JSON] check [%s] fail", KEY_INTERFACE);
            }

            json_driver_conf = cJSON_GetObjectItem(json_device, KEY_DRIVER_CONFIG);

            if (json_driver_conf != NULL) {

                json_entry = cJSON_GetObjectItem(json_driver_conf, KEY_IP);
                if (json_entry->type == cJSON_String && json_entry != NULL) {
                    strncpy(device->ip, json_entry->valuestring, 20);
                } else {
                    LOG_W("[JSON] json entry type incorrect");
                }

                json_entry = cJSON_GetObjectItem(json_driver_conf, KEY_PORT);
                if (json_entry->type == cJSON_Number && json_entry != NULL) {
                    device->port = json_entry->valueint;
                    LOG_I("[JSON] set port:[%d]", device->port);
                } else {
                    LOG_W("[JSON] json entry type incorrect");
                }

                json_entry = cJSON_GetObjectItem(json_driver_conf, KEY_NETWORK_ADDR);
                if (json_entry->type == cJSON_Number && json_entry != NULL) {
                    device->network_addr = json_entry->valueint;
                    LOG_I("[JSON] set network_addr :[%d]", device->network_addr);
                } else {
                    LOG_W("[JSON] json entry type incorrect");
                }

                json_entry = cJSON_GetObjectItem(json_driver_conf, KEY_STATION_ADDR);
                if (json_entry->type == cJSON_Number && json_entry != NULL) {
                    device->station_addr = json_entry->valueint;
                    LOG_I("[JSON] set station_addr:[%d]", device->station_addr);
                } else {
                    LOG_W("[JSON] json entry type incorrect");
                }
            } else {
                LOG_E("[JSON] parse driver configure error");
            }
            json_points = cJSON_GetObjectItem(json_device, KEY_POINTS);
            if (json_points != NULL) {
                int points_num = cJSON_GetArraySize(json_points);
                if (points_num > POINTS_MAX) {
                    points_num = POINTS_MAX;
                    LOG_W("[JSON] Points number over!");
                }
                node_init(&device->data_point_header);
                for (int i = 0; i < points_num; i++) {
                    json_entry = cJSON_GetArrayItem(json_points, i);
                    if (json_entry == NULL) {
                        continue;
                    }

                    data_point_t *data_point = (data_point_t *)dlt_malloc(sizeof(data_point_t));

                    json_point_item = cJSON_GetObjectItem(json_entry, KEY_ADDRESS);
                    if (json_point_item->type == cJSON_String && json_point_item != NULL) {
#ifdef FEATURE_DELETE_UNIQUE_DATA_NAME_POINT
                        if (node_find_subdata(&device->data_point_header, json_point_item->valuestring,compare_data_point_address) != NULL) {
                            FREE(data_point);
                            data_point = NULL;
                            continue;
                        }
#endif
                        strncpy(data_point->address, json_point_item->valuestring, POINT_ADDRESS_SIZE_MAX);

                        LOG_I("[JSON] set data path: [%s]", data_point->path);
                    } else {
                        continue;
                        LOG_W("[JSON] json entry type incorrect");
                    }

                    json_point_item = cJSON_GetObjectItem(json_entry, KEY_PATH);
                    if (json_point_item->type == cJSON_String && json_point_item != NULL) {
                        strncpy(data_point->path, json_point_item->valuestring, DATA_NAME_LEN_MAX);
                    } else {
                        LOG_W("[JSON] json entry type incorrect");
                    }

                    json_point_item = cJSON_GetObjectItem(json_entry, KEY_OFFSET);
                    if (json_point_item->type == cJSON_Number && json_point_item != NULL) {
                        data_point->offset = json_point_item->valueint;
                            LOG_I("[JSON] set offset : [%d]", data_point->offset);
                        }
                    } else {
                        LOG_W("[JSON] json entry type incorrect");
                    }

                    json_point_item = cJSON_GetObjectItem(json_entry, KEY_TYPE);
                    if (json_point_item->type == cJSON_String && json_point_item != NULL) {
                        strncpy(data_point->type, json_point_item->valuestring, 10);
                        LOG_I("[JSON] set data type:[%s]", data_point->type);
                    } else {
                        LOG_W("[JSON] json entry type incorrect");
                    }

                    json_point_item = cJSON_GetObjectItem(json_entry, KEY_LEN);
                    if (json_point_item->type == cJSON_Number && json_point_item != NULL) {
                        data_point->len = json_point_item->valueint;
                            LOG_I("[JSON] set len : [%d]", data_point->len);
                        }
                    } else {
                        LOG_W("[JSON] json entry type incorrect");
                    }

                    json_point_item = cJSON_GetObjectItem(json_entry, KEY_DATA_CYCLE);
                    if (json_point_item->type == cJSON_Number && json_point_item != NULL) {
                        data_point->cycle = json_point_item->valueint;
                        LOG_I("[JSON] set data cycle: [%d]", data_point->cycle);
                    } else {
                        data_point->cycle = 1000;
                        LOG_W("[JSON] json entry type incorrect, set default value:[%d]ms", data_point->cycle);
                    }

                    data_point->time_count = 0;
                    if (data_point) {
                        node_insert(&device->data_point_header, &data_point->pointer);
                    }
                }
                if (device) {

                    json_config_device_list_write_lock();
                    node_insert(&config_context.device_list_header, &device->pointer);
                    json_config_device_list_unlock();
                }
            } else {
                LOG_E("[JSON] parse point fail");
                FREE(device);
            }
        }
    } else {
        LOG_E("[JSON] Parse json string fail!");
        return -1;
    }
    cJSON_free(json_root);
    return 0;
}

/* need free return value*/
char *create_publish_json(void)
{
    char *cjson_str = NULL;
    char path[DATA_NAME_LEN_MAX] = {0};
    char address[20] = {0};
    char sn[MAGIC_MAX_LENGTH] = {0};
    char time_str[11] = {0};
    uint32_t timestamp = 0;
    cJSON *json_root = cJSON_CreateObject();
    cJSON *json_point = cJSON_CreateArray();
    cJSON *json_device = NULL;
    cJSON *json_data = NULL;
    cJSON *json_data_item = NULL;

    node_t *device_pos = NULL;
    node_t *point_pos = NULL;
    node_t *device_list_header = get_device_list_header();

    if (-1 == eeprom_get_product_sn(sn)) {
        LOG_E("[JSON][PUBLISH] Read SN fail");
    }

    timestamp = get_unix_timestamp();
    sprintf(time_str,"%d",timestamp);

    cJSON_AddItemToObject(json_root, "SN", cJSON_CreateString(sn));
    cJSON_AddItemToObject(json_root, "UT", cJSON_CreateString(time_str));
    
    list_for_each(device_pos, device_list_header) {
        device_t *device = (device_t *)device_pos;
        json_data = cJSON_CreateArray();
        json_device = cJSON_CreateObject();

        list_for_each(point_pos, &device->data_point_header) {
            data_point_t *data_point = (data_point_t *)point_pos;
            json_data_item = cJSON_CreateObject();

            strcpy(path, data_point->path);
            strcpy(address, data_point->address);
            sprintf(time_str,"%d",data_point->time_stamp);

            json_config_device_list_read_lock();
            cJSON_AddItemToObject(json_data_item, "path", cJSON_CreateString(path));
            cJSON_AddItemToObject(json_data_item, "address", cJSON_CreateString(address));
            cJSON_AddItemToObject(json_data_item, "time", cJSON_CreateString(time_str));
            cJSON_AddItemToObject(json_data_item, "value", cJSON_CreateString((char *)data_point->data));
            json_config_device_list_unlock();

            cJSON_AddItemToArray(json_data, json_data_item);
        }

        json_config_device_list_read_lock();
        cJSON_AddItemToObject(json_device, "deviceName", cJSON_CreateString(device->name));
        cJSON_AddItemToObject(json_device, "ip", cJSON_CreateString(device->ip));
        cJSON_AddItemToObject(json_device, "network_addr", cJSON_CreateString(device->network_addr));
        cJSON_AddItemToObject(json_device, "station_addr", cJSON_CreateString(device->station_addr));
        cJSON_AddItemToObject(json_device, "data", json_data);
        json_config_device_list_unlock();

        cJSON_AddItemToArray(json_point, json_device);

    }

    cJSON_AddItemToObject(json_root, "POINT", json_point);
    
    cjson_str = cJSON_Print(json_root);

    LOG_I("\n[PUBLISH]:");
    printf(cjson_str, "\n");
    LOG_I("\n[PUBLISH END]");
    cJSON_Delete(json_root);
    return cjson_str;
}

void print_device(void)
{
    node_t *pos = NULL;
    node_t *point_pos = NULL;

    device_t *device_p;
    list_for_each(pos, &config_context.device_list_header) {
        device_p = (device_t *)pos;
        LOG_I("[JSON][LIST] path:[%s]", device_p->name);
        LOG_I("[JSON][LIST] interface:[%d]", device_p->interface);
        LOG_I("[JSON][LIST] ip address:[%s]", device_p->ip);
        LOG_I("[JSON][LIST] port:[%d]", device_p->port);
        LOG_I("[JSON][LIST] network_addr:[%d]", device_p->network_addr);
        LOG_I("[JSON][LIST] station_addr:[%d]", device_p->station_addr);

        data_point_t * point_p;
        list_for_each(point_pos, &device_p->data_point_header) {
            point_p = (data_point_t *)point_pos;
            LOG_I("[JSON][LIST] data path:[%s]", point_p->path);
            LOG_I("[JSON][LIST] data address:[%x]", point_p->address);
            LOG_I("[JSON][LIST] data cycle:[%d]", point_p->cycle);
        }
    }
}

node_t *dlt_get_device_list_header(void)
{
    return &config_context.device_list_header;
}
