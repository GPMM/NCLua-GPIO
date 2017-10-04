#include "PIN.h"
#include <getopt.h>

static struct option long_options[] = {
  { "pin",      required_argument,    NULL,    'p' },
  { "delay",    required_argument,    NULL,    'd' },
  { "repeat",   required_argument,    NULL,    'r' },
  {  0,         0,                    0,        0  }
};

  int
main(int argc, char **argv)
{
  init_gpio();

  int opt, pin, delay, repeat, i;

  while ((opt = getopt_long(argc, argv, "p:d:r:", long_options, NULL)) != -1)
   {
     switch(opt)
      {
        case 'p':
          pin = atoi(optarg);
          break;
        case 'd':
          delay = atoi(optarg);
          break;
        case 'r':
          repeat = atoi(optarg);
          break;
      }
   }

  PIN p = get_pin(pin, OUTPUT);

  for (i = 0; i < repeat; i++)
   {
     setup_pin(p, HIGH);
     sleep(delay);
     setup_pin(p, LOW);
     sleep(delay);
   }

  cleanup_pin(p);
  close_gpio();

  return 0;
}
