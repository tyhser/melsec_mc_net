#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <time.h>

#define ERROR   3
#define INFO    2
#define WARING  1
#define DEBUG   0

#define DEBUG_LEVEL 1

#ifndef FEATURE_FILE_LOG
#define log_print(level, fmt, args...) do { \
                                            print_timestamp(); \
                                        if (level == ERROR){\
                                            printf("ERROR: [Line:%d Function:%s] ", __LINE__, __func__);} \
                                        if (level >= DEBUG_LEVEL || level == ERROR) {\
                                            printf(fmt, ##args); \
                                            printf("\n");}\
                                    } while (0)
#define log_debug(level, fmt, args...) do { \
                                            print_timestamp(); \
                                            printf("[Line:%d Function:%s] ", __LINE__, __func__); \
                                            printf(fmt, ##args); \
                                            printf("\n");\
                                    } while (0)
#endif
#define MIN(a, b) ((a) < (b)?(a):(b))
#define MAX(a, b) ((a) > (b)?(a):(b))

#define container_of(ptr, type, member) ({ \
     const typeof( ((type *)0)->member ) *__mptr = (ptr); \
     (type *)( (char *)__mptr - offsetof(type,member) );})

#define list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

#define UNUSE __attribute__((unused))
#define FREE(p) do {free(p); p = NULL;} while(0)

typedef struct _node_t {
    struct _node_t *previous;
    struct _node_t *next;
} node_t;

void *dlt_malloc(size_t size);
void print_timestamp(void);
void print_start_time(void);
void node_init(node_t *node);
void node_insert(node_t *head, node_t *node);
void node_remove(node_t *node);
void node_remove_all_node(node_t *head);
void *node_find_subdata(node_t *head, void *key, int (*compare)(const void *, const void *));

#ifdef FEATURE_FILE_LOG
void log_info(const char *format, ...);
void log_debug(const char *format, ...);

#define LOG_D(fmt, args...) log_debug(fmt, ##args)
#define LOG_I(fmt, args...) log_info(fmt, ##args)
#define LOG_W(fmt, args...) log_info(fmt, ##args)
#define LOG_E(fmt, args...) log_debug(fmt, ##args)

#else
#define LOG_I(fmt, args...) log_print(INFO, fmt, ##args)
#define LOG_W(fmt, args...) log_print(WARING, fmt, ##args)
#define LOG_E(fmt, args...) log_print(ERROR, fmt, ##args)
#define LOG_D(fmt, args...) log_debug(DEBUG, fmt, ##args)
#endif

#endif
