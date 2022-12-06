#ifndef _B15F_H_
#define _B15F_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef void* b15f_t;

	/**
	 * @brief Returns an instance of the B15F driver
	 * 
	 * @param err Buffer to write the exception message to (can be NULL)
	 * @param len Length of the error buffer
	 * @return b15_t Instance of the driver (or NULL on error)
	 */
	b15f_t get_instance(char* err, size_t len); 

#ifdef __cplusplus
}
#endif

#endif // _B15F_H_