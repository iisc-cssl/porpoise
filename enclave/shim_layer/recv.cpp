#include <string.h>
#include "shim_layer.h"

/*
 * enclave/shim_layer/recv.cpp
 *
 * Kripa Shanker <kripashanker@iisc.ac.in>
 *
 * recv_user() 	-recv from user, copy buffer from user memory to enclave memory.
 * @user_addr 	- pointer to buffer in user memory (const: content of source should not be modifed)
 * @encl_addr 	- pointer to buffer in enclave memory
 * @len 	- length of buffer in user memory
 * @prot	- Protection, is user buffer encrypted or plain text
 *
 * Return: 0 on success
 */


int recv_user(const void *user_addr, void *encl_addr, int len, int prot = 0)
{	
	if(prot == 0){
		//printf("unprotected\n");
	} else {
		/*
		 * Decryption code goes here
		 */
		//printf("protected\n");
	}

	memcpy(encl_addr, user_addr, len);

	return 0;
}


