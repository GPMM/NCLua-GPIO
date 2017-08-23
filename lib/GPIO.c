#include "GPIO.h"

bcm2835_peripheral gpio = { GPIO_BASE };

int
map_peripheral (bcm2835_peripheral *p)
{
  if ((p->mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0)
   {
     return -1;
   }

  p->map = mmap(
      NULL,
      BLOCK_SIZE,
      PROT_READ | PROT_WRITE,
      MAP_SHARED,
      p->mem_fd,      // File descriptor to physical memory virtual file '/dev/mem'
      p->addr_p       // Address in physical map that we want this memory block to expose
      );

  if (p->map == MAP_FAILED)
   {
     perror("mmap");
     return -1;
   }

  p->addr = (volatile unsigned int *)p->map;

  return 0;
}

void
unmap_peripheral (bcm2835_peripheral *p)
{
  munmap(p->map, BLOCK_SIZE);
  close(p->mem_fd);
}

void
init_gpio ()
{
  if (map_peripheral(&gpio) == -1)
   {
     fprintf(stderr, "Error: permission denied!\n");
     exit(EXIT_FAILURE);
   }
}

void
close_gpio ()
{
  unmap_peripheral(&gpio);
}

/* XXX EOF XXX */
