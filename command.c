/* Filename:	command.h
 * Author:	Sean DiGirolamo
 * Version:	1.0.0
 * Date:	03-06-18
 * Purpose:	This file contains the function definitions for the functions
 *		declared in command.h. Explanations for each function can be
 *		found in command.h
 */

#include "command.h"

int parse_line(char *** argv, char * line, char fence)
{
        // Calculates how many arguments there are
        int argc = 1;
        for (int i = 0; i < strlen(line); i++) {
                if (line[i] == fence) {
                        argc++;
                }
        }

        // Allocates data for retval (array of pointers to strings). Has one
        // extra pointer so that it can be null terminated
        *argv = malloc(sizeof(char *) * (argc + 1));

        // Sets all pointers to null
        for (int i = 0; i < argc + 1; i++) {
                *(*argv + i) = NULL;
        }

        int argv_size = 0;      // Number of strings in argv
        char * start = line;    // Start of current string
        char * end = line;      // End of current string/current character
        /* Iterate through each character */
        while (*end != '\n' && *end != '\0') {
                /* Stops when it finds a fence, to copy all characters from
                 * either the start, or the last fence, up until the new fence
                 * to argv.
                 * In other words, extracts the word from the line
                 */
                if (end[1] == fence || end[1] == '\n' || end[1] == '\0') {
                        int string_size = (end - start) + 1;

                        // Allocates data for string
                        char * string = malloc(sizeof(char)
                                * (string_size + 1));

                        // Writes string, copying proper chars and setting last
                        // byte to NULL
                        strncpy(string, start, string_size);
                        string[string_size] = '\0';

                        // Makes the proper pointer in argv point to string
                        *(*argv + argv_size) = string;

                        // Set start to proper char after fence and incr number
                        // of strings in argv
                        start = end + 2;
                        ++argv_size;

                }
                end++;
        }

	return argc;
}

int parse_line_w(char *** argv, char * line)
{
	line = pre_parse(line);
        int argc = count_args(line);

        // Allocates data for retval (array of pointers to strings). Has one
        // extra pointer so that it can be null terminated
        *argv = malloc(sizeof(char *) * (argc + 1));

        // Sets all pointers to null
        for (int i = 0; i < argc + 1; i++) {
                *(*argv + i) = NULL;
        }

        int argv_size = 0;      // Number of strings in argv
        char * start = line;    // Start of current string
	// We don't count whitespace, so skip it
	while (is_whitespace(start[0])) {
			start++;
	}
        char * end = start;      // End of current string/current character

        /* Iterate through each character */
        while (*end != '\0' && *end != '\n' && *end != EOF) {
		/* Stops when it finds a fence, to copy all characters from
		 * either the start, or the last fence, up until the new fence
		 * to argv.
		 * In other words, extracts the word from the line
		 */
                if (((!is_whitespace(end[0]) && is_whitespace(end[1]))
		     || end[1] == '\n'
		     || end[1] == '\0')
		    && start[0] != '\n'
		    && start[0] != '\0') {
                        int string_size = (end - start) + 1;

                        // Allocates data for string
                        char * string = malloc(sizeof(char)
                                * (string_size + 1));

			// Writes string, copying proper chars and setting last
			// byte to NULL
                        strncpy(string, start, string_size);
                        *(string + string_size) = '\0';

			// Makes the proper pointer in argv point to string
                        *(*argv + argv_size) = string;

			// Set start to proper char after fence and incr number
			// of strings in argv
                        start = end + 1;
			while (is_whitespace(start[0]) && start[0] != '\n') {
				start++;
			}
                        ++argv_size;
                }
                end++;
        }

        return argc;
}

char * pre_parse(char * s)
{
	char * temp1;
	char * temp2;
	char * help_path;
	int size;

	// Iterate through each char in string
	for(int i = 0; i < strlen(s); i++) {
		switch (s[i]) {
		case ' ' :
			// Case ' ': If "help " is next, then we need to replace
			// that bit with "more <path to readme>"
			if (s[i + 1] == 'h'
			    && s[i + 2] == 'e'
			    && s[i + 3] == 'l'
			    && s[i + 4] == 'p'
			    && is_whitespace(s[i + 5])) {
				temp1 = malloc(sizeof(char) * (i + 1));
				strncpy(temp1, s, i);
				temp1[i] = '\0';

				temp2 = s + i + 5;
				while (is_whitespace(temp2[0])) {
					temp2++;
				}

				size = strlen(getenv("shelldir")) + 9;
				help_path = malloc(sizeof(char) * (size + 1));
				strcpy(help_path, getenv("shelldir"));
				strcat(help_path, "/help.txt");
				help_path[size] = '\0';

				size = strlen(temp1) + 1 + 4 + 1
					+ strlen(help_path) + 1 + strlen(temp2);
				s = malloc(sizeof(char) * (size + 1));

				strcpy(s, temp1);
				strcat(s, " less ");
				strcat(s, help_path);
				strcat(s, " ");
				strcat(s, temp2);
				s[size] = '\0';

				free(temp1);

			// Otherwise, if / ./ is next, we need to replace it
			// with the current directory
			} else if (s[i + 1] == '.' && s[i + 2] == '/') {
				temp1 = malloc(sizeof(char) * (i + 1));
				strncpy(temp1, s, i);
				temp1[i] = '\0';

				temp2 = s + i + 1;

				char * pwd_path;
				size = strlen(getenv("PWD"));
				pwd_path = malloc(sizeof(char) * (size + 1));
				strcpy(pwd_path, getenv("PWD"));
				pwd_path[size] = '\0';

				size = strlen(temp1) + strlen(pwd_path)
					+ strlen(temp2);
				s = malloc(sizeof(char) * (size + 1));

				strcpy(s, temp1);
				strcat(s, pwd_path);
				strcat(s, temp2);
				s[size] = '\0';

				free(temp1);
			}

			break;
		case 'h' :
			// If h is detected, and the next chars are "elp ", we
			// need to replace it with "more <path to readme>" this
			// is different from the one above because this one
			// accounts for the case in which help is the first
			// string in the string. The other assumes there is a
			// preceeding space
			if (i == 0) {
				if (s[1] == 'e'
				    && s[2] == 'l'
				    && s[3] == 'p'
				    && is_whitespace(s[4])) {
					temp2 = s + 4;

					size = strlen(getenv("shelldir")) + 7;
					help_path = malloc(sizeof(char)
							* (size + 1));
					strcpy(help_path, getenv("shelldir"));
					strcat(help_path, "/readme");
					help_path[size] = '\0';

					size = 5 + strlen(help_path)
						+ strlen(temp2);
					s = malloc(sizeof(char) * (size + 1));

					strcat(s, "more ");
					strcat(s, help_path);
					strcat(s, temp2);
					s[size] = '\0';

					free(help_path);
				}
			}

			break;
		case '~' :
			// If ~ is detected, expand it to the home directory
			temp1 = malloc(sizeof(char) * (i + 1));
			strncpy(temp1, s, i);
			temp1[i] = '\0';

			temp2 = s + i + 1;

			char * home_path;
			size = strlen(getenv("HOME"));
			home_path = malloc(sizeof(char) * (size + 1));
			strcpy(home_path, getenv("HOME"));
			home_path[size] = '\0';

			size = strlen(temp1) + strlen(home_path)
				+ strlen(temp2);
			s = malloc(sizeof(char) * (size + 1));

			strcpy(s, temp1);
			strcat(s, home_path);
			strcat(s, temp2);
			s[size] = '\0';

			free(temp1);
			free(home_path);

			break;
		default :
			break;
		}
	}

	return s;
}

int count_args(char * s)
{
	// Count args. Account for multiple whitespace in a row and tabs and
	// stuff like that
	int count = 0;
	if (!is_whitespace(s[0])) {
		count = 1;
	}
	for (int i = 0; i < strlen(s) - 1; i++) {
		if (is_whitespace(s[i]) && !is_whitespace(s[i + 1])) {
			count++;
		}
	}

	return count;
}

int is_whitespace(char c)
{
	if (c == ' '|| c == '	'|| c == '\n') {
		return 1;
	}
	return 0;
}

int commandify(COMMAND * com, char * line, char fence)
{
	// Calculates number of segments in command separated by fence
	int segc = 1;
	for (int i = 0; i < strlen(line); i++) {
		if (line[i] == fence) {
			segc++;
		}
	}
	// Associates pointer in com with proper data
	com->segc = segc;

	// Allocates data for pointers to command segments
	com->segs = malloc(sizeof(COM_SEG *) * (segc + 1));

	// Allocates data for each command segment and then sets last pointer to
	// NULL
	for (int i = 0; i < segc; i++) {
		com->segs[i] = malloc(sizeof(COM_SEG) * 1);
	}
	com->segs[segc] = NULL;

	int segs_size = 0;	// Tracks number of segs in command
	char * start = line;	// Start of current string
	char * end = line;	// End of current string
	/* Iterate through each string */
	while (*end != '\n' && *end != '\0') {
		/* Stops when a fence is found (because we found the end of
		 * the segment or when '\0' is found
		 */
		if (end[1] == fence || end[1] == '\0' || end[1] == '\n') {
			// Points to proper string count holder for readability
			int * strcp = &(com->segs[segs_size]->strc);
			// Points to proper segment holder for readability
			char *** segp = &(com->segs[segs_size]->seg);

			int string_size = (end - start) + 2;

			// Allocates data for temporary string to parse
			char * string = malloc(sizeof(char)
				* (string_size + 1));

			// Copies proper line piece to string and sets last byte
			// to NULL
			strncpy(string, start, string_size);
			*(string + string_size) = '\0';

			// Parses line, sending retval and parsed line to proper
			// locations in COMMAND com
			*strcp = parse_line_w(segp, string);

			// Properly sets next start, increments seg_size, and
			// frees the no longer needed string, now that it has
			// been parsed and stored elsewhere
			start = end + 2;
			while (is_whitespace(*start)
			       && *start != '\n'
			       && *start != '\0') {
				start++;
			}
			if (end[1] == '|'
			    && (*start == '\n' || *start == '\0') 
			    ) {
				return -1;
			}

			free(string);

			++segs_size;

			end = start;
		} else {
			end++;
		}
	}

	return segc;
}

int search_seg(COM_SEG * seg, char * target)
{
	for (int i = 0; i < seg->strc; i++) {
		if (strcmp(seg->seg[i], target) == 0) {
			return i;
		}
	}
	return -1;
}

void print_seg(COM_SEG * seg)
{
	for (int i = 0; i < seg->strc; i++) {
		printf("%s\n", seg->seg[i]);
	}
}

void print_com(COMMAND * com)
{
	for (int i = 0; i < com->segc; i++) {
		printf("SEG %d\n", i);
		for (int j = 0; j < com->segs[i]->strc; j++) {
			printf("	%s\n", com->segs[i]->seg[j]);
		}
	}
}

void free_seg(COM_SEG * seg)
{
	// Frees each string
	int i = 0;
	while (seg->seg[i] != NULL) {
		free(seg->seg[i]);
		i++;
	}

	// Frees array of pointers to strings
	free(seg->seg);

	// Frees segment itself
	free(seg);
}

void free_com(COMMAND * com)
{
	// Frees each segment
	int i = 0;
	while (com->segs[i] != NULL) {
		free_seg(com->segs[i]);
		i++;
	}

	// Free seg array of pointers
	free(com->segs);

	// Frees the command itself
	free(com);
}
