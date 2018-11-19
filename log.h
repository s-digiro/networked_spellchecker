/* Filename:	fixed_q.h
 * Author:	Sean DiGirolamo
 * Version	1.0.0
 * Date:	03-28-18
 * Purpose:	Header file for log fixed queue. Basically, it's a queue that's
 *		size cannot be changed. Contains instructions for each function
 *		This queue holds pointers to strings
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct log_fixed_queue
{
	char ** q;	// Array of data
	int head;	// Points to head
	int tail;	// Points to tail
	int size;	// Current size
	int capacity;	// Max size
};

#define LOG_Q struct log_fixed_queue

// returns pointer to a fixed queue of size capacity or NULL upon error
LOG_Q * init_lq(int capacity);

// Gracefully frees the queue
void free_lq(LOG_Q * lq);

// Add an item to the queue returns -1 on failure and 0 on success
int lq_enqueue(LOG_Q * fq, char * data);

// Remove item from queue and return it
char * lq_dequeue(LOG_Q * lq);

// Returns data at front of queue without removing it from the queue
char * lq_peek(LOG_Q * lq);

// Returns true if fq is empty, false otherwise
bool lq_is_empty(LOG_Q * lq);

#endif
