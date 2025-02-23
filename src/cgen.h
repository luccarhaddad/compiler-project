#ifndef _CGEN_H_
#define _CGEN_H_

#include "globals.h"

/**
 * @brief Generates code to a code file by traversal of the syntax tree.
 *
 * @param syntaxTree The root of the syntax tree to traverse.
 */
void generateCode(TreeNode* syntaxTree);

#endif
