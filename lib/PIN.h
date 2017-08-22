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

PIN setup (int id, int mode);
int change_status(PIN p, int status);
void clean_up (PIN p);
int main ();
