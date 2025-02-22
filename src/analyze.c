#include "analyze.h"
#include "globals.h"
#include "log.h"
#include "symtab.h"

/**
 * @brief The current location in the source code.
 */
static int location = 0;

/**
 * @brief Offset for local memory allocation.
 */
static int localMemoryOffset = 0;

/**
 * @brief Offset for global memory allocation.
 */
static int globalMemoryOffset = MAX_MEMORY - 2;

/**
 * @brief Flag to check if the main function is already declared.
 */
static bool declaredMainFunction = FALSE;

/**
 * @brief Flag to check the compound scope is the one from function declaration
 */
static bool compoundScopeFromFunctionDeclaration = FALSE;

/**
 * @brief The name of the current function being processed.
 */
static char* currentFunction = NULL;

/**
 * @brief Counter for the number of compound scopes.
 */
static int numberOfCompoundScopes = 0;

/**
 * @brief Enters a new scope with the given name.
 *
 * @param name The name of the new scope.
 */
static void enterScope(const char* name) {
	Scope scope   = malloc(sizeof(struct ScopeRecord));
	scope->name   = strdup(name);
	scope->parent = currentScope;
	scope->next   = NULL;
	for (int i = 0; i < SIZE; i++) scope->hashTable[i] = NULL;

	if (!scopeList) {
		scopeList = scope;
	} else {
		Scope pointer = scopeList;
		while (pointer->next) pointer = pointer->next;
		pointer->next = scope;
	}
	currentScope = scope;
}

/**
 * @brief Exits the current scope if the node is a compound statement.
 *
 * @param node The syntax tree node.
 */
static void exitScope(TreeNode* node) {
	if (node->kind.stmt == CompoundK) {
		currentScope = currentScope->parent;
	}
}

/**
 * @brief Traverses the syntax tree and applies pre-order and post-order processing functions.
 *
 * @param node The syntax tree node to traverse.
 * @param preProc The function to apply before traversing the children.
 * @param postProc The function to apply after traversing the children.
 */
static void traverse(TreeNode* node, void (*preProc)(TreeNode*), void (*postProc)(TreeNode*)) {
	if (node) {
		preProc(node);
		{
			for (int i = 0; i < MAXCHILDREN; i++) {
				if (node->child[i]) node->child[i]->parent = node;
				traverse(node->child[i], preProc, postProc);
			}
		}
		postProc(node);
		traverse(node->sibling, preProc, postProc);
	}
}

/**
 * @brief A null processing function for tree traversal.
 *
 * @param node The syntax tree node (unused).
 */
static void nullProc(TreeNode* node) {
}

/**
 * @brief Inserts a node into the symbol table.
 *
 * @param node The syntax tree node to insert.
 */
static void insertNode(TreeNode* node) {
	switch (node->nodekind) {
		case StmtK: {
			switch (node->kind.stmt) {
				case FuncK: {
					compoundScopeFromFunctionDeclaration = TRUE;
					if (strcmp(node->attr.name, "main") == 0) declaredMainFunction = TRUE;

					localMemoryOffset = MAX_MEMORY - 2;

					if (symbolTableLookup(node->attr.name)) {
						pce("Semantic error at line %d: %s was already declared\n", node->attr.name);
						Error = TRUE;
						break;
					}
					symbolTableInsert(node->attr.name, node->lineno, MAX_MEMORY - 1, node->type,
									  node->kind.stmt, node->isArray, "global");
					enterScope(node->attr.name);
					currentFunction = node->attr.name;
					break;
				}
				// OK
				case ParamK: {
					if (!symbolTableLookupCurrentScope(node->attr.name)) {
						if (strcmp(currentScope->name, "global") == 0) {
							symbolTableInsert(node->attr.name, node->lineno, globalMemoryOffset++, node->type,
											  node->kind.stmt, node->isArray, currentScope->name);
							break;
						}
						symbolTableInsert(node->attr.name, node->lineno, localMemoryOffset--, node->type,
										  node->kind.stmt, node->isArray, currentScope->name);
						break;
					}
					pce("Semantic error at line %d: '%s' was already declared as a variable\n",
						node->lineno, node->attr.name);
					Error = TRUE;
					break;
				}
				// OK
				case VarK: {
					if (node->type == Void) {
						pce("Semantic error at line %d: variable declared void\n", node->lineno);
						Error = TRUE;
						break;
					}

					BucketList localSymbol  = symbolTableLookupCurrentScope(node->attr.name);
					BucketList globalSymbol = symbolTableLookup(node->attr.name);

					if (globalSymbol && globalSymbol->kind == FuncK) {
						pce("Semantic error at line %d: '%s' was already declared as a function\n",
							node->lineno, node->attr.name);
						Error = TRUE;
						break;
					}

					if (!localSymbol) {
						if (strcmp(currentScope->name, "global") == 0) {
							if (node->isArray) {
								globalMemoryOffset += node->child[0]->attr.val;
								symbolTableInsert(node->attr.name, node->lineno, globalMemoryOffset++,
												  node->type, node->kind.stmt, node->isArray,
												  currentScope->name);
								break;
							}
							symbolTableInsert(node->attr.name, node->lineno, globalMemoryOffset++,
											  node->type, node->kind.stmt, node->isArray,
											  currentScope->name);
							break;
						}

						if (node->isArray) {
							symbolTableInsert(node->attr.name, node->lineno, localMemoryOffset--,
											  node->type, node->kind.stmt, node->isArray,
											  currentScope->name);
							localMemoryOffset -= node->child[0]->attr.val;
							break;
						}
						symbolTableInsert(node->attr.name, node->lineno, localMemoryOffset--, node->type,
										  node->kind.stmt, node->isArray, currentScope->name);
						break;
					}

					if (strcmp(localSymbol->scope, currentScope->name) == 0) {
						pce("Semantic error at line %d: '%s' was already declared as a variable\n",
							node->lineno, node->attr.name);
						Error = TRUE;
						break;
					}
				}
				// OK
				case ReturnK: {
					if (!currentFunction) break;

					BucketList symbol = symbolTableLookup(currentFunction);
					if (!symbol) break;

					if (symbol->type != Void && !node->child[0]) {
						pce("Semantic error at line %d: Missing return value in function returning "
							"non-void\n",
							node->lineno);
						Error = TRUE;
						break;
					}
				}
				// OK
				case CompoundK: {
					if (compoundScopeFromFunctionDeclaration ||
						strcmp(currentScope->name, "global") == 0) {
						compoundScopeFromFunctionDeclaration = FALSE;
						break;
						}
					numberOfCompoundScopes++;
					char scopeName[25];
					sprintf(scopeName, "compound%d", numberOfCompoundScopes);
					enterScope(scopeName);
					node->attr.name = strdup(scopeName);
					break;
				}
				default:
					break;
			}
			break;
		}

		case ExpK: {
			switch (node->kind.exp) {
				case IdK:
				case CallK: {
					if (!symbolTableLookup(node->attr.name)) {
						pce("Semantic error at line %d: '%s' was not declared in this scope\n",
							node->lineno, node->attr.name);
						Error = TRUE;
						break;
					}
					symbolTableAddLineNumberToSymbol(node->attr.name, node->lineno);
					break;
				}
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
}

/**
 * @brief Reports a type error.
 *
 * @param node The syntax tree node where the error occurred.
 * @param message The error message.
 */
static void typeError(const TreeNode* node, const char* message) {
	pce("Type error at line %d: %s\n", node->lineno, message);
	Error = TRUE;
}

/**
 * @brief Checks the types of nodes in the syntax tree.
 *
 * @param node The syntax tree node to check.
 */
static void checkNode(TreeNode* node) {
	switch (node->nodekind) {
		case ExpK: {
			switch (node->kind.exp) {
				case IdK: {
					BucketList symbol = symbolTableLookup(node->attr.name);
					if (symbol) {
						node->type = symbol->type;
						break;
					}
					node->type = Integer;
					break;
				}
				case OpK: {
					if (node->child[0]->type != Integer || node->child[1]->type != Integer) {
						typeError(node, "operands must be of type integer");
						break;
					}
					if (node->attr.op == EQ || node->attr.op == NEQ || node->attr.op == LT ||
						node->attr.op == LEQ || node->attr.op == GT || node->attr.op == GEQ) {
						node->type = Boolean;
						break;
						}
					node->type = Integer;
					break;
				}
				case ConstK: {
					node->type = Integer;
					break;
				}
				case AssignK: {
					if (!node->child[1]) {
						pce("Semantic error at line %d: assign operation needs two operands", node->lineno);
						break;
					}
					if (node->child[1]->nodekind && node->child[1]->nodekind != ExpK) {
						pce("Assign operation needs an expression\n");
						break;
					}
					if (node->child[1]->kind.exp == CallK) {
						const BucketList symbol = node->attr.name ? symbolTableLookup(node->child[1]->attr.name) : NULL;
						if (!symbol) break;
						if (symbol->type != Integer) {
							pce("Semantic error at line %d: invalid use of void expression", node->lineno);
							break;
						}
					}
					if (node->child[1]->type != Integer) {
						pce("Semantic error at line %d: invalid use of void expression", node->lineno);
						break;
					}
					if (node->child[1]->kind.exp == IdK) {
						if (!symbolTableLookup(node->child[1]->attr.name)) {
							pce("Variable %s not declared\n", node->child[1]->attr.name);
							break;
						}
					}
					break;
				}
				case CallK: {
					const BucketList symbol =
						node->attr.name ? symbolTableLookup(node->attr.name) : NULL;
					if (symbol) {
						node->type = symbol->type;
						break;
					}
					node->type = Void;
					break;
				}
				default:
					break;
			}
			break;
		}
		case StmtK: {
			switch (node->kind.stmt) {
				case ReturnK: {
					if (!currentFunction) break;

					BucketList symbol = symbolTableLookup(currentFunction);
					if (!symbol) break;

					if (symbol->type != Void && !node->child[0]) {
						pce("Semantic error at line %d: Missing return value in function returning "
							"non-void\n",
							node->lineno);
						Error = TRUE;
					}
					break;
				}
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
}

/**
 * @brief Performs type checking on the syntax tree.
 *
 * @param syntaxTree The root of the syntax tree.
 */
void typeCheck(TreeNode* syntaxTree) {
	if (!declaredMainFunction) {
		pce("Semantic error: undefined reference to 'main'\n");
		Error = TRUE;
	}
	traverse(syntaxTree, nullProc, checkNode);
}

/**
 * @brief Builds the symbol table by traversing the syntax tree.
 *
 * @param syntaxTree The root of the syntax tree.
 */
void buildSymbolTable(TreeNode* syntaxTree) {
	enterScope("global");
	symbolTableInsert("input", -1, location++, Integer, FuncK, FALSE, "global");
	symbolTableInsert("output", -1, location++, Void, FuncK, FALSE, "global");

	traverse(syntaxTree, insertNode, exitScope);
	exitScope(syntaxTree);
	if (TraceAnalyze) {
		pc("\nSymbol table:\n\n");
		printSymbolTable();
	}
}