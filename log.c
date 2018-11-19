/* Filename:	log.c
 * Author:	Sean DiGirolamo
 * Version	1.0.0
 * Date:	03-28-18
 * Purpose:	Contains function declarations for log fixed queue. Instructions
 *		for everything can be found in log.h. Most of this code is
 *		reused from fixed_q.c. I was too lazy to rewrite fixed_q.c for
 *		void pointers and instead just made a whole new one for strings
 *		because I'm on a deadline
 */

#include "log.h"

LOG_Q * init_lq(int capacity)
{
	if (capacity < 1) {
		return NULL;
	}

	LOG_Q * retval = malloc(sizeof(LOG_Q));
	if (retval == NULL)
		return NULL;

	retval->q = malloc(sizeof(char *) * capacity);
	if (retval->q == NULL) {
		free(retval);
		return NULL;
	}

	retval->head = -1;
	retval->tail = -1;
	retval->size = 0;
	retval->capacity = capacity;

	return retval;
}

void free_lq(LOG_Q * lq)
{
	for (int i = 0; i < lq->size; i++) {
		free(lq->q[(lq->head + i) % lq->capacity]);
	}
	free(lq->q);
	free(lq);
}

int lq_enqueue(LOG_Q * lq, char * data)
{
	// Check if there is space available
	if (lq->size >= lq->capacity) {
		return -1;
	}

	if (lq->size <= 0) {
		lq->head = 0;
	}
	lq->size++;
	lq->tail = (lq->tail + 1) % lq->capacity;
	lq->q[lq->tail] = data;

	return 0;
}

char * lq_dequeue(LOG_Q * lq)
{
	// Check if there is anything in the queue
	if (lq->size <= 0) {
		fprintf(stderr,
			"Error: Attempted to dequeue from empty queue\n");
		exit(0);
	}

	char * retval = lq->q[lq->head];
	lq->size--;
	if (lq->size <= 0) {
		lq->head = -1;
		lq->tail = -1;
	}
	lq->head = (lq->head + 1) % lq->capacity;

	return retval;
}

char * lq_peek(LOG_Q * lq)
{
	if (lq->size <= 0) {
		fprintf(stderr, "Error: Attempted to peek at an empty queue\n");
		exit(0);
	}

	return lq->q[lq->head];
}

bool lq_is_empty(LOG_Q * lq)
{
	if (lq->size <= 0) {
		return true;
	}
	return false;
}
