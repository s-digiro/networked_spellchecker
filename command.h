/* Filename:	command.h
 * Author:	Sean DiGirolamo
 * Version:	1.0.0
 * Date:	03-06-18
 * Purpose:	Header file for command.c
 *		Contains utilities mostly used for string parsing and preparing
 *		lines to be used as commands in myshell
 */

#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMMAND struct Command
#define COM_SEG struct Segment


/* Command Structure
 * Contains list of command segments (segments of command separated by |, <, or
 * >) and number of segments in command
 * For future reference, |/</> will be at the end of ever segment (unless it is
 * the last, in which case there won't be any |/</>
 * The array of segs is NULL terminated
 */
struct Command
{
	COM_SEG ** segs;
	int segc;
};

/* Segment Structure
 * Contains list of strings in segment and number of strings in segment
 * ^ This all includes the |/</> if they are at the end
 * The string array is NULL terminated
 */
struct Segment
{
	char ** seg;
	int strc;
};


/* Separates string into array of strings. Uses char 'fence' as border between
 * strings. The fence is not included in either the string before or after it.
 * The parsed string will be stored in char *** argv, and the function will
 * return the number of strings in the line
 */
int parse_line(char *** argv, char * line, char fence);

/* Parses a string into a command. Uses ' ' as the fence. The command will be
 * an array of segments. Each segment will be terminated by either a <, >, | or
 * /0 (usually if its the last segment)
 * The parsed command will be stored in COMMAND * com, and the function will
 * return the count of segments in COMMAND * com
 */
int commandify(COMMAND * com, char * line, char fence);

/* Returns index of target string in segment, or -1 if it is not in the segment
 */
int search_seg(COM_SEG * seg, char * target);

void print_seg(COM_SEG * seg);
void print_com(COMMAND * com);

/* The two functions below here free segments and commands conveniently */
void free_seg(COM_SEG * seg);
void free_com(COMMAND * com);

/* Returns true if char c is whitespace, false otherwise */
int is_whitespace(char c);

/* Parses line, except uses any and all whitespace as a deliminer. Much better
 * than the one above, but the one above still has uses, eg. parsing path
 * environment variable */
int parse_line_w(char *** argv, char * line);

int count_args(char * line);

/* Replaces certain strings found within s with other strings */
char * pre_parse(char * s);

#endif
