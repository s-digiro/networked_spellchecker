/* Filename:	main.c
 * Author:	Sean DiGirolamo
 * Version	1.0.0
 * Date:	03-25-18
 * Purpose:	Main program for networked spell checker. This program runs a
 *		a server which clients can connect to and send words to. The
 *		server then compares the words to a dict_path and informs the
 *		client whether or not their word was in the dict_path.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netinet/in.h>


#include "fixed_q.h"
#include "command.h"
#include "log.h"



#define DEFAULT_DICTIONARY "/usr/share/dict/words"
#define DEFAULT_PORT 9002
#define NUM_WORKERS 3
#define CONNECTION_CAPACITY 3
#define LOG_ARRAY_LEN 10



/* Handles arguments. Puts dict_path argument in dict_path string. Puts port
 * int port integer. If dict_path is null, then data will be malloced
 * for it. Don't forget to free it. Return -1 on failure, 0 on success
 *
 * Use this like so:
 *	handle_args(argc, argv, &dict_path, &port);
 */
int handle_args(int argc, char ** argv, char ** dict_path, int * port);

// returns true if strint p is a valid integer. returns false otherwise
bool is_int(char * s);

// The function that worker threads execute
void * worker(void * x);

// Function that log thread executes
void * logger(void * x);

// Reads a line from client and puts it in dest. Returns size of string dest
int readline(int client, char ** dest);

// Parses dictionary file into array of words
int parse_dictionary(char *** dest, FILE * source);

bool is_in_dictionary(char * s);

bool read_word(int client, char ** retval);

bool custom_strcmp(char * a, char * b);

void free_dictionary();



static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t log_m = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t fq_avail = PTHREAD_COND_INITIALIZER;
static pthread_cond_t work_avail = PTHREAD_COND_INITIALIZER;
static pthread_cond_t event_avail = PTHREAD_COND_INITIALIZER;
static pthread_cond_t log_avail = PTHREAD_COND_INITIALIZER;

static FIXED_Q * work = NULL;
static LOG_Q * log_q = NULL;
static char ** dictionary;
static int dict_word_count = 0;

static bool custom_dict_path = false;



int main(int argc, char **argv)
{
	printf("Starting server...\n");

	char * dict_path = NULL;
	int port;

	// Parse args into proper variables
	if (handle_args(argc, argv, &dict_path, &port)) {
		fprintf(stderr, "Error: Invalid args\n");
		fprintf(stderr,
			"usage: %s [-d dict_path] [-p port]\n",
			argv[0]);
		return 0;
	}
	printf("- Args parsed.\n");

	// Parse dictionary file into dictionary array
	FILE * dict_fp = fopen(dict_path, "r");
	if (dict_fp == NULL) {
		if (custom_dict_path) {
			free(dict_path);
		}
		fprintf(stderr, "Error: Dictionary not found\n");
		return 0;
	}
	dict_word_count = parse_dictionary(&dictionary, dict_fp);
	free(dict_path);
	fclose(dict_fp);



	// Create socket
	int serv_sock;
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_sock < 0) {
		fprintf(stderr, "Error: Could not create socket\n");
		fprintf(stderr, "	%s\n", strerror(errno));
		free_dictionary();
		return 0;
	}
	printf("- Socket created.\n");

	// Define Server Address
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	// Bind sock to specified IP and port
	if (bind(serv_sock, (struct sockaddr *) &addr, sizeof(addr))) {
		fprintf(stderr, "Error: Bind Error\n");
		fprintf(stderr, "	%s\n", strerror(errno));
		free_dictionary();
		return 0;
	}
	printf("- Socket bound\n");



	// Set up threads and stuff
	pthread_t tid[NUM_WORKERS];		// Tid array of thread ids
	work = init_fq(CONNECTION_CAPACITY);	// Work Queue

	// Set up threads
	for (int i = 0; i < NUM_WORKERS; i++) {
		if (pthread_create(tid + i, NULL, worker, NULL)) {
			fprintf(stderr, "Error creating thread %d\n", i);
			fprintf(stderr, "	%s\n", strerror(errno));
			free_dictionary();
			free_fq(work);
			return 0;
		}
		printf("thread created\n");
	}
	printf("- Worker threads created.\n");

	// Create log thread
	pthread_t log_tid;			// log thread tid
	log_q = init_lq(LOG_ARRAY_LEN);		// log Queue
	if (pthread_create(&log_tid, NULL, logger, NULL)) {
		fprintf(stderr, "Error creating log thread\n");
		fprintf(stderr, "	%s\n", strerror(errno));
		free_dictionary();
		free_fq(work);
		free_lq(log_q);
		return 0;
	}

	// Listen for connections
	listen(serv_sock, CONNECTION_CAPACITY);
	printf("Listening for connections...\n");

	while (true) {
		int new_connection = accept(serv_sock, NULL, NULL);
		pthread_mutex_lock(&mutex);
		while (work->size >= CONNECTION_CAPACITY) {
			printf("work->size = %d\n", work->size);
			pthread_cond_wait(&fq_avail, &mutex);
		}
		fq_enqueue(work, new_connection);
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&work_avail);
		printf("- recieved request from %d\n", new_connection);
	}

	return 0;
}

void * logger(void * x)
{
	FILE * logfp = fopen("log.txt", "a");
	if (logfp == NULL) {
		fprintf(stderr, "Couldn't open log.txt\n");
		exit(0);
	}
	char * cur;
	fprintf(logfp, "\nNEW INSTANCE\n~~~~~~~~~~~~\n");
	fclose(logfp);
	while (true) {
		logfp = fopen("log.txt", "a");

		pthread_mutex_lock(&log_m);
		while (log_q->size <= 0) {
			pthread_cond_wait(&event_avail, &log_m);
		}
		cur = lq_dequeue(log_q);
		pthread_mutex_unlock(&log_m);
		pthread_cond_signal(&log_avail);
		printf("- Writing to log...\n");
		printf("	");
		printf("- writing %s", cur);
		fprintf(logfp, "%s", cur);

		fclose(logfp);
		printf("- Finished writing to log\n");
		free(cur);
	}
}

void * worker(void * x)
{
	char server_message[256] = "You have reached the server!\n";
	int cur;	// Current Socket being worked on
	while(true) {
		pthread_mutex_lock(&mutex);
		while (work->size <= 0) {
			printf("A worker is now waiting\n");
			pthread_cond_wait(&work_avail, &mutex);
		}
		printf("A thread woke up\n");
		cur = fq_dequeue(work);
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&fq_avail);
		printf("- A worker is working on %d\n", cur);

		send(cur, server_message, sizeof(server_message), 0);
		int msg_size;
		char * msg;
		char * line = NULL;
		char ** words = NULL;
		send(cur, "Enter a line: ", 15, 0);
		readline(cur, &line);
		printf("- Worker recieved line %s\n", line);
		int wordc = parse_line_w(&words, line);
		for (int i = 0; i < wordc; i++) {
			if (is_in_dictionary(words[i])) {
				msg_size = strlen(words[i]) + 5;
				msg = malloc(sizeof(char) * msg_size);
				strcpy(msg, words[i]);
				strcat(msg, " OK\n");
				send(cur, msg, msg_size, 0);
				pthread_mutex_lock(&log_m);
				while (log_q->size >= log_q->capacity) {
					pthread_cond_wait(&log_avail, &mutex);
				}
				lq_enqueue(log_q, msg);
				pthread_mutex_unlock(&log_m);
				pthread_cond_signal(&event_avail);
				printf("	");
				printf("- word %s in dictionary\n", words[i]);
			} else {
				msg_size = strlen(words[i]) + 13;
				msg = malloc(sizeof(char) * msg_size);
				strcpy(msg, words[i]);
				strcat(msg, " MISSPELLED\n");
				send(cur, msg, msg_size, 0);
				pthread_mutex_lock(&log_m);
				while (log_q->size >= log_q->capacity) {
					pthread_cond_wait(&log_avail, &mutex);
				}
				lq_enqueue(log_q, msg);
				pthread_mutex_unlock(&log_m);
				pthread_cond_signal(&event_avail);
				printf("	");
				printf("- word %s is NOT in dictionary\n",
					words[i]);
			}
		}

		printf("- A worker finished working on %d\n", cur);
		close(cur);
	}

	return NULL;
}

bool is_in_dictionary(char * s)
{
	for (int i = 0; i < dict_word_count; i++) {
		if (strcmp(dictionary[i], s) == 0) {
			return true;
		}
	}
	return false;
}

int readline(int client, char ** dest)
{
	if (*dest != NULL) {
		return -1;
	}

	int size = 1;
	*dest = malloc(sizeof(char) * size);
	*dest[0] = ' ';
	while (true) {
		char * inbuf = malloc(sizeof(char) * 1);
		read(client, inbuf, 1);
		if (*inbuf == '\n' || *inbuf == '\0') {
			free(inbuf);
			break;
		}

		size = size + 1;
		*dest = realloc(*dest, size);
		*(*dest + (size - 1)) = *inbuf;

		free(inbuf);
	}
	int i = 0;
	while (true) {
		if (*(*dest + i) == 13) {
			*(*dest + i) = '\0';
			break;
		}
		i++;
	}

	return size;
}

int handle_args(int argc, char ** argv, char ** dict_path, int * port)
{
	int dict_path_set = false;
	int port_set = false;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0) {
			// -d found, assume next arg is dict_path
			if (dict_path_set) {
				// Dictionary is already set
				return -1;
			}
			if (i + 1 >= argc) {
				// There is no argument after -d
				return -1;
			}
			if (*dict_path == NULL) {
				// dict_path is NULL, must malloc data
				int size = strlen(argv[i + 1]);
				*dict_path = malloc(sizeof(char) * (size + 1));
			}

			strcpy(*dict_path, argv[i + 1]);
			dict_path_set = true;
			i++;
		} else if (strcmp(argv[i], "-p") == 0) {
			// -p found, assume next arg is port
			if (port_set) {
				// Port already set
				return -1;
			}
			if (i + 1 >= argc) {
				// There is no argument after -p
				return -1;
			}
			if (!is_int(argv[i + 1])) {
				// Not an integer
				return -1;
			}

			char *p;
			*port = strtol(argv[i + 1], &p, 0);
			port_set = true;
			i++;
		} else {
			// Argument without a -p or -d in front of it
			return -1;
		}
	}

	// Set values to default if they haven't ben manually defined
	if (!dict_path_set) {
		// Choose default dict_path
		if (*dict_path == NULL) {
			// Dictionary is NULL, gotta malloc some data
			*dict_path = malloc(sizeof(char)
				* (strlen(DEFAULT_DICTIONARY) + 1));
			custom_dict_path = true;
		}

		strcpy(*dict_path, DEFAULT_DICTIONARY);
	}
	if (!port_set) {
		// Choose default port
		*port = DEFAULT_PORT;
	}

	return 0;
}

bool is_int(char * s)
{
	char *p = s;
	int i = 0;
	while (*p != '\0') {
		if (!isdigit(s[i])) {
			return false;
		}
		p++;
	}
	return true;
}

int parse_dictionary(char *** dest, FILE * source)
{
	if (*dest != NULL) {
		return -1;
	}
	if (source == NULL) {
		return -1;
	}

	int size = 0;
	char line[128];
	while (fgets(line, sizeof(line), source)) {
		int len = strlen(line);
		char * string = malloc(sizeof(char) * len);
		strncpy(string, line, len - 1);

		if (dest == NULL) {
			size = 1;
			*dest = malloc(sizeof(char *) * size);
		} else {
			size++;
			*dest = realloc(*dest, sizeof(char *) * size);
		}
		*(*dest + size - 1) = string;
	}
	*dest = realloc(*dest, sizeof(char *) * (size + 1));
	*(*dest + size) = NULL;

	return size;
}

void free_dictionary()
{
	char **p = dictionary;
	while (*p != NULL) {
		free(p);
		p++;
	}
	free(dictionary);
}
