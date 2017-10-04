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

#define IN   0
#define OUT  1

typedef enum _edge_t {
  EDGE_ERR = -1,
  NONE,
  RISING,
  FALLING,
  BOTH
} edge_t;

typedef struct _gpio_t {
  unsigned int gpio;
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

int epfd = -1;
pthread_t thread;
gpio_list_t *gpio_list = NULL;
volatile int thread_running = 0;

int
export (unsigned int gpio)
{
  int fd, len;
  char str_gpio[3];

  if ((fd = open("/sys/class/gpio/export", O_WRONLY)) == -1)
  {
    fprintf(stderr, "Failed to export GPIO %d\n", gpio);
    return -1;
  }

  len = snprintf(str_gpio, sizeof(str_gpio), "%d", gpio);
  write(fd, str_gpio, len);

  close(fd);
  return 0;
}

int
unexport (unsigned int gpio)
{
  int fd, len;
  char str_gpio[3];

  if ((fd = open("/sys/class/gpio/unexport", O_WRONLY)) == -1)
  {
    fprintf(stderr, "Failed to unexport GPIO %d\n", gpio);
    return -1;
  }

  len = snprintf(str_gpio, sizeof(str_gpio), "%d", gpio);
  write(fd, str_gpio, len);

  close(fd);
  return 0;
}

int
set_direction (unsigned int gpio, int flag)
{
  int fd, retry;
  char filename[33];
  struct timespec delay;

  snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/direction", gpio);

  delay.tv_sec = 0;
  delay.tv_nsec = 10000000L;

  for (retry = 0; retry < 100; retry++)
  {
    if ((fd = open(filename, O_WRONLY)) != -1)
    {
      break;
    }
    nanosleep(&delay, NULL);
  }

  if (retry >= 100)
  {
    fprintf(stderr, "Failed to open direction file\n");
    return -1;
  }

  switch (flag)
  {
    case IN:
      write(fd, "in", 3);
      break;
    case OUT:
      write(fd, "out", 4);
      break;
  }

  close(fd);
  return 0;
}

edge_t
set_edge (unsigned int gpio, edge_t flag)
{
  int fd;
  char filename[28];

  snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/edge", gpio);

  if ((fd = open(filename, O_WRONLY)) == -1)
  {
    fprintf(stderr, "Failed to open edge file\n");
    return EDGE_ERR;
  }

  switch (flag)
  {
    case RISING:
      write(fd, "rising", 7);
      break;
    case FALLING:
      write(fd, "falling", 8);
      break;
    case BOTH:
      write(fd, "both", 5);
      break;
    case NONE:
    default:
      write(fd, "none", 5);
      flag = NONE;
      break;
  }

  close(fd);
  return flag;
}

void
set_value (unsigned int gpio, int value)
{
  int fd;
  char filename[29], buffer[3];

  snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/value", gpio);

  if ((fd = open(filename, O_WRONLY)) == -1)
  {
    fprintf(stderr, "Failed to open edge file\n");
    return;
  }

  snprintf(buffer, sizeof(buffer), "%d", value);
  write(fd, buffer, 3);

  close(fd);
}

int
add_to_list (gpio_t *gpio)
{
  gpio_list_t *new;

  new = NULL;
  new = (gpio_list_t*) malloc(sizeof(gpio_list_t));

  if (new == NULL)
  {
    fprintf(stderr, "Malloc error\n");
    return -1;
  } 

  new->current = gpio;
  new->next = NULL;

  if (gpio_list != NULL)
  {
    new->next = gpio_list;
  }

  gpio_list = new;

  return 0;
}



gpio_t *
search_list (unsigned int gpio)
{
  gpio_list_t *list;

  if (gpio_list == NULL)
  {
    return NULL;
  }
  
  for(list = gpio_list; list != NULL; list = list->next)
  {
    if (list->current->gpio == gpio)
    {
      return list->current;
    }
  }

  return NULL;
}

int
open_gpio_file (unsigned int gpio)
{
  int fd;
  char filename[29];

  snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/value", gpio);
 
  if ((fd = open(filename, O_RDONLY | O_NONBLOCK)) == -1)
  {
    fprintf(stderr, "Cant open gpio %d value file\n", gpio);
    return -1;
  }

  return fd;
}

gpio_t *
new_gpio (unsigned int gpio, int direction, edge_t edge, void (*callback) (unsigned int gpio, int value))
{
  gpio_t *new;

  new = NULL;
  new = (gpio_t*) malloc(sizeof(gpio_t));

  if (new == NULL)
  {
    fprintf(stderr, "Malloc error\n");
    return NULL;
  }

  new->gpio = gpio;

  if (export(gpio) == -1)
  {
    free(new);
    return NULL;
  }

  if (set_direction(gpio, direction) == -1)
  {
    free(new);
    return NULL;
  }

  if ((new->fd = open_gpio_file(gpio)) == -1)
  {
    unexport(gpio);
    free(new);
    return NULL;
  }

  if ((new->edge = set_edge(gpio, edge)) == -1)
  {
    unexport(gpio);
    free(new);
    return NULL;
  }

  new->initial_thread = 1;
  new->thread_added = 0;
  new->callback = callback;

  if (add_to_list(new) == -1)
  {
    free(new);
    return NULL;
  }

  return new;
}

void
delete_gpio (unsigned int gpio)
{
  gpio_list_t *list, *prev;

  prev = NULL;

  if (gpio_list == NULL)
  {
    return;
  }
  
  for(list = gpio_list; list != NULL; list = list->next)
  {
    if (list->current->gpio == gpio)
    {
      if (prev != NULL)
      {
        prev->next = list->next;
      }
      else
      {
        gpio_list = list->next;
      }
      break;
    }
    prev = list;
  }

  free(list->current);
  free(list);
}

void
remove_gpio_event (unsigned int gpio)
{
  gpio_t *g;
  struct epoll_event ev;

  g = search_list(gpio);

  if (g == NULL)
  {
    return;
  }

  ev.events = EPOLLIN | EPOLLET | EPOLLPRI;
  ev.data.ptr = g;

  epoll_ctl(epfd, EPOLL_CTL_DEL, g->fd, &ev);

  g->edge = set_edge(gpio, NONE);

  unexport(gpio);
  close(g->fd);
  delete_gpio(gpio);
}

void *
epoll_loop (void *arg)
{
  gpio_t *g;
  char buffer;
  int n, value;
  struct epoll_event ev;

  thread_running = 1;

  while (thread_running)
  {
    n = epoll_wait(epfd, &ev, 1, -1);

    if (n > 0)
    {
      g = (gpio_t*) ev.data.ptr;

      if (g->initial_thread)
      {
        g->initial_thread = 0;
        continue;
      }

      lseek(g->fd, 0, SEEK_SET);  

      if (read(g->fd, &buffer, 1) != 1)
      {
        thread_running = 0;
        pthread_exit(NULL);
      }

      value = atoi(&buffer);
      g->callback(g->gpio, value);
    }
    else if (n == -1)
    {
        if (errno == EINTR)
        {
          continue;
        }
        thread_running = 0;
        pthread_exit(NULL);
    }
  }

  thread_running = 0;
  pthread_exit(NULL);
}

int
add_gpio_event (unsigned int gpio, edge_t edge, void (*callback) (unsigned int gpio, int value))
{
  gpio_t *g;
  pthread_t thread;
  struct epoll_event ev;

  g = search_list(gpio);

  if (g == NULL)
  {
    if((g = new_gpio(gpio, IN, edge, callback)) == NULL)
    {
      return -1;
    }
  }
  else if (g->edge != edge)
  {
    if ((g->edge = set_edge(gpio, edge)) == - 1)
    {
      return -1;
    }
  }

  if (epfd == -1)
  {
    if ((epfd = epoll_create1(0)) == -1)
    {
      fprintf(stderr, "Failed to create epoll fd\n");
      return -1;
    }
  }

  ev.events = EPOLLIN | EPOLLET | EPOLLPRI;
  ev.data.ptr = g;

  if (epoll_ctl(epfd, EPOLL_CTL_ADD, g->fd, &ev) == -1)
  {
    remove_gpio_event(gpio);
    return -1;
  }

  g->thread_added = 1;

  if (!thread_running)
  {
    if (pthread_create(&thread, NULL, epoll_loop, NULL) != 0)
    {
      remove_gpio_event(gpio);
      return -1;
    }
  }

  return 0;
}

void gpio_callback (unsigned int gpio, int value)
{
    printf("GPIO %d Value %d\n", gpio, value);
}

sig_atomic_t volatile running = 1;

void
sig_handler (int signum)
{
  if(signum == SIGINT)
   {
     running = 0;
   }
}

int
main(int argc, char **argv)
{
  signal(SIGINT, &sig_handler);

  add_gpio_event(25, BOTH, gpio_callback);

  while (running)
  {
    sleep(1);
    if (thread_running == 0)
      break;
  }

  remove_gpio_event(25);
  return EXIT_SUCCESS;
}
