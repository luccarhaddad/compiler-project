#ifndef _UTIL_H_
#define _UTIL_H_

#include "globals.h"

void printToken(TokenType token, const char* tokenString);
TreeNode* newStmtNode(StmtKind kind);
TreeNode* newExpNode(ExpKind kind);
char* copyString(char* string);
void printTree(TreeNode* tree);
void printLine();

#endif
