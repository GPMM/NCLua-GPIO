#include "../lib/PIN.h"
#include <getopt.h>
#include <signal.h>

static struct option long_options[] = {
  { "pin",      required_argument,    NULL,    'p' },
  {  0,         0,                    0,        0  }
};

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
  init_gpio();
  signal(SIGINT, &sig_handler);

  int opt, pin, data;

  while ((opt = getopt_long(argc, argv, "p:", long_options, NULL)) != -1)
   {
     switch(opt)
      {
        case 'p':
          pin = atoi(optarg);
          break;
      }
   }

  PIN p = get_pin(pin, INPUT);

  while (running)
   {
     data = GET_GPIO(pin); 

     if(!data)
      {
        printf("button pressed %lu\n", data);
      }
     else
      {
        printf("button released %lu\n", data);
      }

     sleep(1);
   }

  cleanup_pin(p);
  close_gpio();

  return 0;
}
