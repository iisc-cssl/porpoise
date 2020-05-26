#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

char buf[4 * 1024];

int main(int argc, char **argv) {
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

	char *argv[] = {"./h2o", "-m", "worker","-c","tests/h2o/examples/h2o/h2o.conf", 
		(char*)0};
	write(pipefd[1], buf, sizeof(buf));
	if( execve(argv[0], argv, NULL) == -1){
		perror("execve failed! ");
	}
  }

  read(pipefd[0], buf, sizeof(buf));

  sleep(3);

  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if(curl) {
	  curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8888");

	  /* Perform the request, res will get the return code */ 
	  res = curl_easy_perform(curl);
	  /* Check for errors */ 
	  if(res != CURLE_OK)
		  fprintf(stderr, "curl_easy_perform() failed: %s\n",
				  curl_easy_strerror(res));

	  /* always cleanup */ 
	  curl_easy_cleanup(curl);
  }

  kill(pid, SIGKILL);

  return 0;
}
