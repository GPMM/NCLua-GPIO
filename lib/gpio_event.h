#include <pthread.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#define MAX_SIZE 64
#define IN 1
#define OUT 0

typedef enum _edge_t {
  EDGE_ERR = -1,
  NONE,
  RISING,
  FALLING,
  BOTH
} edge_t;

typedef struct _gpio_t {
  unsigned int gpio;
  int value;
  int fd;
  edge_t edge;
  int initial_thread;
  int thread_added;
  void (*callback) (unsigned int gpio, int value);
} gpio_t;

typedef struct _gpio_list_t {
  gpio_t *current;
  struct _gpio_list_t *next;
} gpio_list_t; 

typedef struct __gpio_evt_t {
  unsigned int gpio;
  int value;
  struct __gpio_evt_t *next;
} gpio_evt_t;

typedef struct __evt_queue_t {
  gpio_evt_t *head;
  gpio_evt_t *tail;
  int size;
} evt_queue_t;

/* SYSFS */
int export (unsigned int gpio);
int unexport (unsigned int gpio);
int set_direction (unsigned int gpio, int flag);
edge_t set_edge (unsigned int gpio, edge_t flag);
void set_value (unsigned int gpio, int value);

/* LIST */
int open_gpio_file (unsigned int gpio);
gpio_t *new_gpio (unsigned int gpio, int direction, edge_t edge, void (*callback) (unsigned int gpio, int value));
int add_to_list (gpio_t *gpio);
gpio_t *search_list (unsigned int gpio);
void delete_gpio (unsigned int gpio);
void remove_gpio_event (unsigned int gpio);

/* QUEUE */
evt_queue_t *new_queue ();
gpio_evt_t *new_gpio_evt (unsigned int gpio, int value);
void enqueue (gpio_evt_t *gpio_evt);
int add_gpio_event (unsigned int gpio, edge_t edge, void (*callback) (unsigned int gpio, int value));
gpio_evt_t *dequeue ();

void free_all();

/* XXX EOF XXX */
