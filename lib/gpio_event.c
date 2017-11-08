#include "gpio_event.h"

int epfd = -1;
pthread_t thread;
gpio_list_t *gpio_list = NULL;
evt_queue_t *evt_queue = NULL;
volatile int thread_running = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
new_gpio (unsigned int gpio,
          int direction,
          edge_t edge,
          void (*callback) (unsigned int gpio, int value))
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
  new->value = -1;

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

  n = 0;
  value = 0; 

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

      if (g->value == value)
      {
        continue;
      }

      g->value = value;
      g->callback(g->gpio, g->value);
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
add_gpio_event (unsigned int gpio,
                edge_t edge,
                void (*callback) (unsigned int gpio, int value))
{
  gpio_t *g;
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

evt_queue_t *
new_queue ()
{
  evt_queue_t *new;

  new = NULL;

  new = (evt_queue_t *) malloc(sizeof(evt_queue_t));

  if (new == NULL)
  {
    fprintf(stderr, "Malloc error\n");
    return NULL;
  }

  new->head = NULL;
  new->tail = NULL;
  new->size = 0;

  return new;
}

gpio_evt_t *
new_gpio_evt (unsigned int gpio, int value)
{
  gpio_evt_t *new;

  new = NULL;

  new = (gpio_evt_t *) malloc(sizeof(gpio_evt_t));

  if (new == NULL)
  {
    fprintf(stderr, "Malloc error\n");
    return NULL;
  }

  new->gpio = gpio;
  new->value = value;
  new->next = NULL;

  return new;
}

void
enqueue (gpio_evt_t *gpio_evt)
{
  evt_queue_t *queue;

  pthread_mutex_lock(&lock);
  queue = evt_queue;

  if (queue == NULL)
  {
    queue = new_queue();
    assert(queue);
    queue->head = gpio_evt;
    evt_queue = queue;
  }
  else if (queue->size == MAX_SIZE)
  {
    pthread_mutex_unlock(&lock);
    free(gpio_evt);
    return;
  }
  else if (queue->tail == NULL)
  {
    queue->head = gpio_evt;
  }
  else
  {
    queue->tail->next = gpio_evt;
  }

  queue->tail = gpio_evt;
  queue->size++;
  pthread_mutex_unlock(&lock);
}

gpio_evt_t *
dequeue ()
{
  gpio_evt_t *evt;

  pthread_mutex_lock(&lock);

  if (evt_queue == NULL || evt_queue->head == NULL)
  {
    pthread_mutex_unlock(&lock);
    return NULL;
  }

  evt = evt_queue->head;
  evt_queue->tail = evt->next ? evt_queue->tail : NULL;
  evt_queue->head = evt->next ? evt->next : NULL;
  evt_queue->size--;

  pthread_mutex_unlock(&lock);

  evt->next = NULL;

  return evt;
}

void
free_all()
{
  gpio_evt_t *evt;
  gpio_list_t *list, *aux;

  thread_running = 0;

  if (gpio_list != NULL)
  {
    list = gpio_list;

    while (list != NULL)
    {
      aux = list;
      list = aux->next;
      free(aux->current);
      free(aux);
    }
  }

  while ((evt = dequeue()) != NULL)
  {
    free(evt);
  }

  free(evt_queue);
}

/* XXX EOF XXX */
