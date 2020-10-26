/*
 * $Id: td_getline.h,v 1.3 2020/10/25 18:54:47 tom Exp $
 *
 * This header provides "getline" for systems which lack that function.
 */

#ifndef TD_GETLINE_H_incl
#define TD_GETLINE_H_incl 1

#ifndef HAVE_GETLINE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ssize_t
td_getline(char **lineptr, size_t *length, FILE *fp)
{
    char *buffer;
    const size_t chunk = 1024;
    size_t offset = 0;
    ssize_t result = -1;

    do {
	if (*lineptr == 0 || *length == 0) {
	    buffer = malloc(*length = chunk);
	} else if (*length <= (offset + chunk)) {
	    buffer = realloc(*lineptr, *length += chunk);
	} else {
	    buffer = *lineptr;
	}

	if (buffer == NULL) {
	    free(*lineptr);
	    *lineptr = 0;
	    *length = 0;
	    break;
	}
	*lineptr = buffer;

	buffer = *lineptr + offset;
	memset(buffer, 0, chunk);
	if (fgets(buffer, (int) chunk, fp) != NULL) {
	    int n;
	    int after = -1;
	    int ended = 0;
	    for (n = 0; n < (int) chunk; ++n) {
		if (buffer[n] == '\n') {
		    ended = 1;
		    after = n + 1;
		    break;
		} else if (buffer[n] == '\0') {
		    after = n;
		    break;
		}
	    }
	    if (after >= 0) {
		if (after == (int) chunk) {
		    buffer = realloc(*lineptr, *length += chunk);
		    if (buffer == NULL) {
			free(*lineptr);
			*lineptr = 0;
			*length = 0;
			break;
		    }
		    *lineptr = buffer;
		}
		result = (ssize_t) (offset + (size_t) after);
		(*lineptr)[result] = '\0';
		if (ended)
		    break;
		else
		    offset += after;
	    } else {
		offset += chunk;
	    }
	}
    } while (!feof(fp) && !ferror(fp));
    return result;
}
#define getline(lineptr, length, fp) td_getline(lineptr, length, fp)
#endif

#endif /* TD_GETLINE_H_incl */
