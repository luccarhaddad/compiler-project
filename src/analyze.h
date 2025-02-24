#ifndef _ANALYZE_H_
#define _ANALYZE_H_

#include "globals.h"

/**
 * \brief Builds the symbol table for the given syntax tree.
 *
 * \param tree The root of the syntax tree.
 */
void buildSymbolTable(TreeNode* tree);

/**
 * \brief Performs type checking on the given syntax tree.
 *
 * \param tree The root of the syntax tree.
 */
void typeCheck(TreeNode* tree);

#endif
