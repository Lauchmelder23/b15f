#include "b15f.h"
#include "../drv/b15f.h"

#define wrap(x) reinterpret_cast<b15f_t>(x)
#define unwrap(x) (*(reinterpret_cast<B15F*>(x)))

#define default_catch catch(std::exception& e) {\
	if (err) { \
		strncpy(err, e.what(), len); \
	} \
} 


b15f_t get_instance(char* err, size_t len) {
	try {
		return wrap(&B15F::getInstance());
	} default_catch;

	return NULL;
}

int reconnect(b15f_t drv, char* err, size_t len) {
	try {
		unwrap(drv).reconnect();
		return 0;
	} default_catch;

	return -1;
}

int read_dip_switch(b15f_t drv, uint8_t* data, char* err, size_t len) {
	if (data == NULL) {
		return -EINVAL;
	}

	try {
		*data = unwrap(drv).readDipSwitch();
		return 0;
	} default_catch;

	return -1;
}