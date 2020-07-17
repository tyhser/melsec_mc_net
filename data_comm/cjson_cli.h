#ifndef CJSON_CLI_H
#define CJSON_CLI_H
#include "cJSON.h"
#include "common.h"

#define KEY_INTER_CONFIG    "interConfg"
#define KEY_DRIVER_CONFIG   "driveConfig"
#define KEY_POINTS          "points"
#define KEY_DEVICE_NAME     "deviceName"
#define KEY_INTERFACE       "interface"

#define KEY_PROTOCOL    "protocolVersion"
#define KEY_IP          "ip"
#define KEY_PORT        "port"
#define KEY_NETWORK_ADDR    "network_addr"
#define KEY_STATION_ADDR    "station_addr"
#define KEY_ADDRESS     "address"
#define KEY_BAUD        "baud"
#define KEY_BYTE_INVL   "byteInterval"
#define KEY_REPLY_INVL  "replyInterval"
#define KEY_CYCLE       "Cycle"
#define KEY_TIMEOUT     "readTimeout"
#define KEY_DATA_NAME   "name"
#define KEY_DS          "ds"
#define KEY_DATA_CYCLE  "cycle"
#define KEY_PATH        "path"
#define KEY_OFFSET      "offset"
#define KEY_TYPE        "type"
#define KEY_LEN         "len"

#define DEVICE_NAME_LEN_MAX 128
#define DATA_NAME_LEN_MAX   128
#define DATA_STRING_SIZE_MAX 128

#define SERIAL_IF   0
#define SOCKET_IF   1

#define DEVICE_NUM_MAX 50
#define POINTS_MAX 10000

#define IP_ADDRESS_SIZE_MAX 20
#define POINT_ADDRESS_SIZE_MAX 20

typedef struct {
    node_t pointer;
    char name[DEVICE_NAME_LEN_MAX];
    int interface;
    char ip[20];
    int port;
    int network_addr;
    int station_addr;
    int fd;
    node_t data_point_header;
} device_t;

typedef struct {
    node_t pointer;
    char path[DATA_NAME_LEN_MAX];
    char address[20];
    int offset;
    char type[10];
    int len;
    uint8_t data[DATA_STRING_SIZE_MAX];
    uint32_t cycle;

    /*for time countdown*/
    uint32_t time_count;

    /*unix time stamp*/
    uint32_t time_stamp;
} data_point_t;

void json_cli_init(void);
void json_config_device_list_read_lock(void);
void json_config_device_list_write_lock(void);
void json_config_device_list_unlock(void);
int parse_config_json(char *json_str);

char *create_publish_json(void);
node_t *get_device_list_header(void);
#endif
