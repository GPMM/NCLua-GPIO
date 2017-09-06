#include "GPIO.h"

bcm_peripheral gpio;

int
detect_by_device_tree (unsigned int *peri_base)
{
  FILE *fp;
  char buffer[4];

  if ((fp = fopen("/proc/device-tree/soc/ranges", "rb")) != NULL)
  {
    fseek(fp, 4, SEEK_SET);

    if(fread(buffer, 1, sizeof(buffer), fp) == sizeof(buffer))
    {
      *peri_base = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3] << 0; 
    }
    else
    {
      fprintf(stderr, "Failed to read file: \"/proc/device-tree/soc/ranges\"\n");
      return 0;
    }

    fclose(fp);
  }
  else
  {
    fprintf(stderr, "Cannot open file: \"/proc/device-tree/soc/ranges\"\n");
    return 0;
  }

  return -1;
}

int
detect_by_cpu_info (unsigned int *peri_base)
{
  FILE *fp;
  char buffer[256];
  unsigned int revision = REV_UNDEF;

  if ((fp = fopen("/proc/cpuinfo", "r")) != NULL)
  {
    while(!feof(fp))
    {
      fgets(buffer, sizeof(buffer), fp);
      if(sscanf(buffer, "Revision\t: %x", &revision)) break;
    }

    fclose(fp);

    switch (revision) {
      /*
        case REV_XXXX:
          *peri_base = BCM2708_PERI_BASE;
          break;
      */
      case REV_RPI3:
        *peri_base = BCM2709_PERI_BASE;
        break;
      case REV_UNDEF:
      default:
        fprintf(stderr, "Undefined revision cannot detect peri_base\n");
        return 0;
    }
  }
  else
  {
    fprintf(stderr, "Cannot open file: \"/proc/cpuinfo\"\n");
    return 0;
  }

  return -1;
}

int
detect_gpio_base (off_t *gpio_base)
{
  unsigned int peri_base;

  if ((access("/proc/device-tree/soc/ranges", F_OK)) != -1)
  {
    if ((detect_by_device_tree(&peri_base)) != -1)
    {
      return 0;
    }
  }
  else if ((access("/proc/cpuinfo", F_OK)) != 1)
  {
    if ((detect_by_cpu_info(&peri_base)) != -1)
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }

  *gpio_base = peri_base + GPIO_BASE_OFFSET;
  return -1;
}

int
map_peripheral (bcm_peripheral *p)
{
  int mem_fd;
  off_t gpio_base;

  if ((mem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC)) != -1)
  {
    gpio_base = 0;
  }
  else if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) != -1)
  {
    if (detect_gpio_base(&gpio_base) != -1)
    {
      fprintf(stderr, "Failed to detect gpio_base\n");
      return -1;
    }
  }
  else
  {
    fprintf(stderr, "Failed to open dev mem\n");
    return -1;
  }

  p->map = mmap(
      NULL,
      BLOCK_SIZE,
      PROT_READ | PROT_WRITE,
      MAP_SHARED,
      mem_fd,
      gpio_base
    );

  close(mem_fd);

  if (p->map == MAP_FAILED)
  {
    fprintf(stderr, "Map failed\n");
    return -1;
  }

  p->addr = (volatile unsigned int *) p->map;

  return 0;
}

void
unmap_peripheral (bcm_peripheral *p)
{
  munmap(p->map, BLOCK_SIZE);
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
