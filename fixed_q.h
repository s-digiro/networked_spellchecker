/* Filename:	fixed_q.h
 * Author:	Sean DiGirolamo
 * Version	1.0.0
 * Date:	03-28-18
 * Purpose:	Header file for fixed queue. Basically, it's a queue that's size
 *		cannot be changed. Contains instructions for each function
 */

#ifndef FIXED_Q_H
#define FIXED_Q_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct fixed_queue
{
	int * q;	// Array of data
	int head;	// Points to head
	int tail;	// Points to tail
	int size;	// Current size
	int capacity;	// Max size
};

#define FIXED_Q struct fixed_queue

// returns pointer to a fixed queue of size capacity or NULL upon error
FIXED_Q * init_fq(int capacity);

// Gracefully frees the queue
void free_fq(FIXED_Q * fq);

// Add an item to the queue returns -1 on failure and 0 on success
int fq_enqueue(FIXED_Q * fq, int data);

// Remove item from queue and return it
int fq_dequeue(FIXED_Q * fq);

// Returns data at front of queue without removing it from the queue
int fq_peek(FIXED_Q * fq);

// Returns true if fq is empty, false otherwise
bool fq_is_empty(FIXED_Q * fq);

// Prints the queue and its stats. Useful for debugging
void print_fq(FIXED_Q * fq);

#endif
