#ifndef _SCAN_H_
#define _SCAN_H_

#include "globals.h"

/**
 * Maximum length of a token.
 */
#define MAXTOKENLEN 40

/**
 * String to store the current token.
 */
extern char tokenString[MAXTOKENLEN + 1];

/**
 * Retrieves the next token from the input.
 *
 * @return The type of the next token.
 */
TokenType getToken(void);

#endif
