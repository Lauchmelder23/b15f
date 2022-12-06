#ifndef _B15F_H_
#define _B15F_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef void* b15f_t;

	/**
	 * @brief Returns an instance of the B15 driver
	 * 
	 * @param err Buffer to write the exception message to (can be NULL)
	 * @param len Length of the error buffer
	 * @return b15_t Instance of the driver (or NULL on error)
	 */
	b15f_t get_instance(char* err, size_t len); 

	/**
	 * @brief Tries to reconnect to the B15
	 * 
	 * @param drv Instance of the B15 driver
	 * @param err Error buffer
	 * @param len Size of the error buffer
	 * @return int 0 on success
	 */
	int reconnect(b15f_t drv, char* err, size_t len);

	/**
	 * @brief Reads the values of the DIP switch
	 * 
	 * @param drv Instance of the B15 driver
	 * @param data Variable to store switch state in
	 * @param err Error buffer
	 * @param len Size of the error buffer
	 * @return int 0 on success
	 */
	int read_dip_switch(b15f_t drv, uint8_t* data, char* err, size_t len);

#ifdef __cplusplus
}
#endif

#endif // _B15F_H_