#include "PIN.h"

PIN
setup (int id, int mode)
{
	PIN p = malloc(sizeof(PIN));

	assert(p);

	if (map_peripheral(&gpio) == -1)
	 {
		 fprintf(stderr, "Failed to map: permission denied!\n");
		 exit(1);
	 }

	p->id = id;
	p->status = LOW;

	switch (mode)
	 {
		 case INPUT:
			 p->mode = INPUT;
			 break;
		 case OUTPUT:
			 p->mode = OUTPUT;
			 INP_GPIO(id);
			 OUT_GPIO(id);
			 break;
	 }

	return p;	
}

int
change_status (PIN p, int status)
{
	if (p->status == status)
	 {
		 return 0;
	 }

	switch (status)
	 {
		 case HIGH: 
			 GPIO_SET = 1 << p->id;
			 break;
		 case LOW: 
			 GPIO_CLR = 1 << p->id;
			 break;
	 }

	p->status = status;

	return 1;
}

void
clean_up (PIN p)
{
	free(p);
	unmap_peripheral(&gpio);
}
