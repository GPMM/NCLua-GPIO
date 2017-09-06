/*

 * Raspberry PI B+ pin mapping *    * Raspberry PI A/B pin mapping *
   _________________________           _________________________
   |            |           |          |            |           |
   |    3.3V    |     5V    |          |    3.3V    |     5V    |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |   GPIO2    |     5V    |          |   GPIO2    |     5V    |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |   GPIO3    |    GND    |          |   GPIO3    |    GND    |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |   GPIO4    |     TX    |          |   GPIO4    |     TX    |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |     GND    |     RX    |          |     GND    |     RX    |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |   GPIO17   |   GPIO18  |          |   GPIO17   |   GPIO18  |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |   GPIO27   |    GND    |          |   GPIO27   |    GND    |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |   GPIO22   |   GPIO23  |          |   GPIO22   |   GPIO23  |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |    3.3V    |   GPIO24  |          |    3.3V    |   GPIO24  |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |   GPIO10   |    GND    |          |   GPIO10   |    GND    |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |   GPIO9    |   GPIO25  |          |   GPIO9    |   GPIO25  |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |   GPIO11   |   GPIO8   |          |   GPIO11   |   GPIO8   |
   |____________|___________|          |____________|___________|
   |            |           |          |            |           |
   |    GND     |   GPIO7   |          |    GND     |   GPIO7   |
   |____________|___________|          |____________|___________|
   |            |           |
   |    RSVD    |   RSVD    |
   |____________|___________|
   |            |           |
   |   GPIO5    |    GND    |
   |____________|___________|
   |            |           |
   |   GPIO6    |   GPIO12  |
   |____________|___________|
   |            |           |
   |   GPIO13   |    GND    |
   |____________|___________|
   |            |           |
   |   GPIO19   |   GPIO16  |
   |____________|___________|
   |            |           |
   |   GPIO26   |   GPIO20  |
   |____________|___________|
   |            |           |
   |    GND     |   GPIO21  |
   |____________|___________|

 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BCM2708_PERI_BASE        0x20000000
#define BCM2709_PERI_BASE        0x3F000000
#define GPIO_BASE                (BCM2709_PERI_BASE + 0x200000) /* GPIO controller */
#define GPIO_BASE_OFFSET         0x200000

#define BLOCK_SIZE (4*1024)

#define INP_GPIO(g) *(gpio.addr+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio.addr+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio.addr+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio.addr+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio.addr+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio.addr+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

#define GPIO_PULL *(gpio.addr+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio.addr+38) // Pull up/pull down clock

/* TODO Add other models revision code TODO */
#define REV_UNDEF   0x000000
#define REV_RPI3    0xa22082

typedef struct bcm_peripheral_t {
  void *map;
  volatile unsigned int *addr;
} bcm_peripheral;

extern bcm_peripheral gpio;

int detect_by_device_tree (unsigned int *peri_base);
int detect_by_cpu_info (unsigned int *peri_base);
int detect_gpio_base (off_t *gpio_base);
int map_peripheral ();
void unmap_peripheral ();
void init_gpio ();
void close_gpio ();

/* XXX EOF XXX */
