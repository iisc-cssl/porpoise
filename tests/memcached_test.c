#include <libmemcached/memcached.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

char buf[4 * 1024];

int main(int argc, char **argv) {
  //memcached_servers_parse (char *server_strings);
  memcached_server_st *servers = NULL;
  memcached_st *memc;
  memcached_return rc;
  char *key = "key";
  char *value = "Keep Calm Stay Strong";

  char *retrieved_value;
  size_t value_length;
  uint32_t flags;

  memc = memcached_create(NULL);

  int pipefd[2];

  if(pipe(pipefd)){
	  perror("pipe: ");
  }

  pid_t pid = fork();
  if(pid == 0){
  	// child process;
	int i;
	for(i = 0; i < sizeof(buf); i++)
		buf[i] = 'a';

	char *argv[] = {"./memcached", (char*)0};
	write(pipefd[1], buf, sizeof(buf));
	if( execve("./memcached", argv, NULL) == -1){
		perror("execve failed! ");
	}
  }

  read(pipefd[0], buf, sizeof(buf));

  sleep(5);

  servers = memcached_server_list_append(servers, "localhost", 11211, &rc);
  rc = memcached_server_push(memc, servers);

  if (rc == MEMCACHED_SUCCESS)
    fprintf(stderr, "Added server successfully\n");
  else
    fprintf(stderr, "Couldn't add server: %s\n", memcached_strerror(memc, rc));

  rc = memcached_set(memc, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);

  if (rc == MEMCACHED_SUCCESS)
    fprintf(stderr, "Key stored successfully\n");
  else
    fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc));

  retrieved_value = memcached_get(memc, key, strlen(key), &value_length, &flags, &rc);
  printf("Yay!\n");

  if (rc == MEMCACHED_SUCCESS) {
    fprintf(stderr, "Key retrieved successfully\n");
    printf("The key '%s' returned value '%s'.\n", key, retrieved_value);
    free(retrieved_value);
  }
  else
    fprintf(stderr, "Couldn't retrieve key: %s\n", memcached_strerror(memc, rc));
  kill(pid, SIGKILL);

  return 0;
}
