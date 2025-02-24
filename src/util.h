#ifndef _UTIL_H_
#define _UTIL_H_

#include "globals.h"

/**
 * Prints a token and its string representation.
 *
 * @param token The token type.
 * @param tokenString The string representation of the token.
 */
void printToken(TokenType token, const char* tokenString);

/**
 * Creates a new statement node.
 *
 * @param kind The kind of statement.
 * @return A pointer to the new statement node.
 */
TreeNode* newStmtNode(StmtKind kind);

/**
 * Creates a new expression node.
 *
 * @param kind The kind of expression.
 * @return A pointer to the new expression node.
 */
TreeNode* newExpNode(ExpKind kind);

/**
 * Copies a string.
 *
 * @param string The string to copy.
 * @return A pointer to the copied string.
 */
char* copyString(const char* string);

/**
 * Prints the syntax tree.
 *
 * @param tree The syntax tree to print.
 */
void printTree(const TreeNode* tree);

/**
 * Prints a line.
 */
void printLine();

#endif
