/*

	* Raspberry PI 3 pin mapping *
		________________________
		|						|						|
		|		3.3V		|			5V		|
		|___________|___________|
		|						|						|
		|		GPIO2		|			5V		|
		|___________|___________|
		|						|						|
		|		GPIO3		|			GND		|
		|___________|___________|
		|						|						|
		|		GPIO4		|			TX		|
		|___________|___________|
		|						|						|
		|		GND			|			RX		|
		|___________|___________|
		|						|						|
		|		GPIO17	|		GPIO18	|
		|___________|___________|
		|						|						|
		|		GPIO27	|			GND		|
		|___________|___________|
		|						|						|
		|		GPIO22	|		GPIO23	|
		|___________|___________|
		|						|						|
		|		3.3V		|		GPIO24	|
		|___________|___________|
		|						|						|
		|		GPIO10	|			GND		|
		|___________|___________|
		|						|						|
		|		GPIO9		|		GPIO25	|
		|___________|___________|
		|						|						|
		|		GPIO11	|		GPIO8		|
		|___________|___________|
		|						|						|
		|		GND			|		GPIO7		|
		|___________|___________|
		|						|						|
		|		RSVD		|		RSVD		|
		|___________|___________|
		|						|						|
		|		GPIO5		|			GND		|
		|___________|___________|
		|						|						|
		|		GPIO6		|		GPIO12	|
		|___________|___________|
		|						|						|
		|		GPIO13	|			GND		|
		|___________|___________|
		|						|						|
		|		GPIO19	|		GPIO16	|
		|___________|___________|
		|						|						|
		|		GPIO26	|		GPIO20	|
		|___________|___________|
		|						|						|
		|		GND			|		GPIO21	|
		|___________|___________|

*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BCM2708_PERI_BASE        0x3F000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define BLOCK_SIZE (4*1024)

#define INP_GPIO(g) *(gpio.addr+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio.addr+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio.addr+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio.addr+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio.addr+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio.addr+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

#define GPIO_PULL *(gpio.addr+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio.addr+38) // Pull up/pull down clock

typedef struct bcm2835_peripheral_t {
	unsigned long addr_p;
	int mem_fd;
	void *map;
	volatile unsigned int *addr;
} bcm2835_peripheral;

extern bcm2835_peripheral gpio;

int map_peripheral (bcm2835_peripheral *p);
void unmap_peripheral (bcm2835_peripheral *p);

/* XXX EOF XXX */
