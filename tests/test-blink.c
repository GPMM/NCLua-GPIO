#include "../lib/PIN.h"
#include <getopt.h>

static struct option long_options[] = {
	{ "pin",		required_argument,	NULL,		'p' },
	{ "delay",	required_argument,	NULL,		'd' },
	{ "repeat",	required_argument,	NULL,		'r' },
	{	0, 				0, 									0,		 	 0 	}
};

int
main(int argc, char **argv)
{
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

	PIN p = setup(pin, OUTPUT);

	for (i = 0; i < repeat; i++)
	 {
		 change_status(p, HIGH);
		 sleep(delay);
		 change_status(p, LOW);
		 sleep(delay);
	 }

	clean_up(p);

	return 0;
}
