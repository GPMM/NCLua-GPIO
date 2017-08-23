#include "PIN.h"

PIN
get_pin (int id, int mode)
{
  PIN p = malloc(sizeof(PIN));

  assert(p);

  p->id = id;
  p->status = GET_GPIO(id);

  switch (mode)
  {
    case INPUT:
      INP_GPIO(id);
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
setup_pin (PIN p, int status)
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

  p->status = GET_GPIO(p->id);

  return 1;
}

void
cleanup_pin (PIN p)
{
  if (GET_GPIO(p->id) == HIGH)
   {
     GPIO_CLR = 1 << p->id;
   }

  free(p);
}

/* XXX EOF XXX */
