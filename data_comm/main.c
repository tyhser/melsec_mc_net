#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include "port.h"
#include "common.h"
#include "cjson_cli.h"
#include "driver_redis.h"
#include "cjson_cli.h"
#include "system.h"

#define DATA_BUF_SIZE 10000

static pthread_t collect_tid;

void print_db_data(data_t *data)
{
    LOG_I("DS:[0x%x]\tTotal length: [%d]\tData name:[%s]\tData optype:[%c]\tSubdata count:[%d]\n", data->ds, data->total_len, data->data_name, data->data_optype, data->subdata_cnt);
}

static int bkdrhash(char *str) {
    int seed = 131;
    int hash = 0;
    for (int i = 0; i < strlen(str); i++) {
        hash = hash * seed + (int)str[i];
    }
   return hash & 0x7FFFFFFF;
}

extern void dump(void);
extern void signal_handler(int signo);
int main(void)
{
    static int config_json_hash = 0;
    int err;
    signal(SIGSEGV, signal_handler);
    print_start_time();
    dev_init();
    data_base_init();
    dlt_json_init();

    while (1) {
        char *config_json = redis_get_dlt_config();
        if (config_json_hash != bkdrhash(config_json)) {
            if (dlt_parse_config_json(config_json) < 0) {
                LOG_E("[MAIN] Fatal error: can't init configure json");
                exit(-1);
            }
            config_json_hash = bkdrhash(config_json);
        }
        FREE(config_json);

        if (!collect_tid) {
            err = pthread_create(&collect_tid, NULL, dlt_collect_thread, NULL);
            if (err != 0) {
                LOG_E("[MAIN] Fatal error: Can't Create collect thread!");
                exit(-1);
            }
        }

        char *pub = dlt_create_publish_json();

        redis_cli_publish_msg(pub);
        FREE(pub);
        sleep(2);
    }
    dev_deinit();
}
