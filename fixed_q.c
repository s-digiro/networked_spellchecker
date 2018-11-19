/* Filename:	fixed_q.c
 * Author:	Sean DiGirolamo
 * Version	1.0.0
 * Date:	03-28-18
 * Purpose:	Contains function declarations for fixed queue. Instructions for
 *		everything can be found in fixed_q.h
 */

#include "fixed_q.h"

FIXED_Q * init_fq(int capacity)
{
	if (capacity < 1) {
		return NULL;
	}

	FIXED_Q * retval = malloc(sizeof(FIXED_Q));
	if (retval == NULL)
		return retval;

	retval->q = malloc(sizeof(int) * capacity);
	if (retval->q == NULL)
		return retval;

	retval->head = -1;
	retval->tail = -1;
	retval->size = 0;
	retval->capacity = capacity;

	return retval;
}

void free_fq(FIXED_Q * fq)
{
	free(fq->q);
	free(fq);
}

int fq_enqueue(FIXED_Q * fq, int data)
{
	// Check if there is space available
	if (fq->size >= fq->capacity) {
		return -1;
	}

	if (fq->size <= 0) {
		fq->head = 0;;
	}
	fq->size++;
	fq->tail = (fq->tail + 1) % fq->capacity;
	fq->q[fq->tail] = data;

	return 0;
}

int fq_dequeue(FIXED_Q * fq)
{
	// Check if there is anything in the queue
	if (fq->size <= 0) {
		fprintf(stderr,
			"Error: Attempted to dequeue from empty queue\n");
		exit(0);
	}

	int retval = fq->q[fq->head];
	fq->size--;
	if (fq->size <= 0) {
		fq->head = -1;
		fq->tail = -1;
	}
	fq->head = (fq->head + 1) % fq->capacity;

	return retval;
}

int fq_peek(FIXED_Q * fq)
{
	if (fq->size <= 0) {
		fprintf(stderr, "Error: Attempted to peek at an empty queue\n");
		exit(0);
	}

	return fq->q[fq->head];
}

bool fq_is_empty(FIXED_Q * fq)
{
	if (fq->size <= 0) {
		return true;
	}
	return false;
}

void print_fq(FIXED_Q * fq)
{
	printf("capacity: %d\n", fq->capacity);
	printf("size: %d\n", fq->size);
	printf("head: %d\n", fq->head);
	printf("tail: %d\n", fq->tail);
	for (int i = 0; i < fq->capacity; i++) {
		printf("%d: %d", i, fq->q[i]);
		if (i == fq->head) {
			printf(" head");
		}
		if (i == fq->tail) {
			printf(" tail");
		}
		printf("\n");
	}
}
