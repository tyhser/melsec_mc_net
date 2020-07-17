#include <sys/time.h>
#include <sys/types.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h> 
#include <errno.h>
#include <stdarg.h>
#include <pthread.h>
#include <time.h>
#include "common.h"

void *dlt_malloc(size_t size)
{
    void *ret = malloc(size);
    memset(ret, 0, size);
    return ret;
}

void print_timestamp(void)
{
    static long start_sec = 0;
    static long start_usec = 0;

    struct timeval tv;
    if (start_sec == 0) {
        gettimeofday(&tv, NULL);
        start_sec = tv.tv_sec;
        start_usec = tv.tv_usec;
    }
    gettimeofday(&tv, NULL);
    printf("[%6u.%10u]", (unsigned int)(tv.tv_sec - start_sec), (unsigned int)(tv.tv_usec - start_usec));
}

void print_start_time(void)
{
    time_t timep;
    time (&timep);
    printf("Start: %s",ctime(&timep));
}

/* list operation start*/

void node_init(node_t *node)
{
    node->previous = node;
    node->next = node;
}

void node_insert(node_t *head, node_t *node)
{
    node->next = head;
    node->previous = head->previous;
    head->previous->next = node;
    head->previous = node;
}

void node_remove(node_t *node)
{
    node->previous->next = node->next;
    node->next->previous = node->previous;
}

void *node_find_subdata(node_t *head, void *key, int (*compare)(const void *, const void *))
{
    node_t *current_node = head->next;
    node_t *result = NULL;

    while (current_node != head) {
        if (!compare(current_node, key)) {
            result = current_node;
            break;
        }
        current_node = current_node->next;
    }
    return result;
}

void node_remove_all_node(node_t *head)
{
    node_t *cur = head;
    while (head->next != head) {
        cur = head->next;
        node_remove(cur);
        FREE(cur);
    }
}


#ifdef FEATURE_FILE_LOG
static pthread_mutex_t fileMutex;

/*
 * safe_asprintf();
 */
int safe_asprintf(char **strp, const char *fmt, ...) 
{
  va_list ap;
  int retval;

  va_start(ap, fmt);
  retval = safe_vasprintf(strp, fmt, ap);
  va_end(ap);

  return retval;
}

/*
 * safe_vasprintf();
 */
int safe_vasprintf(char **strp, const char *fmt, va_list ap) 
{
  int retval;

  retval = vasprintf(strp, fmt, ap);
  if (retval == -1) 
  {
    printf("Failed to vasprintf: %s. Bailing out\n", strerror(errno));
    return 1;
  }
  return retval;
}

/*
 * plog();
 */
void log_debug(const char *format, ...)
{

  pthread_mutex_lock(&fileMutex);

  FILE *fp = NULL;
  va_list vlist;
  char *fmt = NULL;

  // Open debug info output file.
  if (!(fp = fopen("log.txt", "a+"))) {
    pthread_mutex_unlock(&fileMutex);
    return;
  }

  va_start(vlist, format);
  safe_vasprintf(&fmt, format, vlist);
  va_end(vlist);
  if (!fmt) {
    pthread_mutex_unlock(&fileMutex);
    return;
  }

  time_t timep;
  struct tm *ptm = NULL;
  time(&timep);
  ptm = localtime(&timep);
  fprintf(fp, "[%04d-%02d-%02d-%02d-%02d-%02d] [Line:%d Function:%s]%s",
    ptm->tm_year + 1900,
    ptm->tm_mon + 1,
    ptm->tm_mday,
    ptm->tm_hour,
    ptm->tm_min,
    ptm->tm_sec,
     __LINE__,
     __func__,
    fmt);

  free(fmt);
  fsync(fileno(fp));
  fclose(fp);

  pthread_mutex_unlock(&fileMutex);
}

/*
 * pinfo();
 */
void log_info(const char *format, ...)
{
  pthread_mutex_lock(&fileMutex);

  FILE *fp = NULL;
  va_list vlist;
  char *fmt = NULL;

  // Open debug info output file.
  if (!(fp = fopen("log.txt", "a+"))) {
    pthread_mutex_unlock(&fileMutex);
    return;
  }

  va_start(vlist, format);
  safe_vasprintf(&fmt, format, vlist);
  va_end(vlist);
  if (!fmt) {
    pthread_mutex_unlock(&fileMutex);
    return;
  }

  time_t timep;
  struct tm *ptm = NULL;
  time(&timep);
  ptm = localtime(&timep);
  fprintf(fp, "[%04d-%02d-%02d-%02d-%02d-%02d] %s",
    ptm->tm_year + 1900,
    ptm->tm_mon + 1,
    ptm->tm_mday,
    ptm->tm_hour,
    ptm->tm_min,
    ptm->tm_sec,
    fmt);

  free(fmt);
  fsync(fileno(fp));
  fclose(fp);

  pthread_mutex_unlock(&fileMutex);
}
#endif
