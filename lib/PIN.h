#include "GPIO.h"
#include <assert.h>

#define LOW 0
#define HIGH 1

#define INPUT 0
#define OUTPUT 1

typedef struct pin_t {
	int id;
	int mode;
	int status;
} *PIN;

PIN get_pin (int id, int mode);
int setup_pin  (PIN p, int status);
void cleanup_pin (PIN p);

/* XXX EOF XXX */
