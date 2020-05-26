#include <string.h>
#include "shim_layer.h"
/*
 * enclave/shim_layer/send.cpp
 *
 * Kripa Shanker <kripashanker@iisc.ac.in>
 *
 * send_user() - send to user, copy buffer from enclave memory to user memory.
 * @user_addr 	- pointer to buffer in user memory
 * @encl_addr 	- pointer to buffer in enclave memory
 * @len 	- length of buffer in user memory
 * @prot	- Protection, is  buffer needed to be encrypted or plain text
 *
 * Return: 0 on success
 */


int send_user(void *user_addr, const void *encl_addr, int len, int prot)
{
	if(prot == 0){
		//printf("unprotected\n");
	} else {
		/*
		 * Encryption code goes here
		 */
		//printf("protected\n");
	}
	
	memcpy(user_addr, encl_addr, len);

	return 0;
}


