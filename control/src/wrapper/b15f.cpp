#include "b15f.h"
#include "../drv/b15f.h"

b15f_t get_instance(char* err, size_t len) {
	try {
		return reinterpret_cast<b15f_t>(&B15F::getInstance());
	} catch(DriverException& e) {
		if (err) {
			strncpy(err, e.what(), len);
		}
	}

	return NULL;
}