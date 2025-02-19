#include "util.h"
#include "globals.h"

/**
 * @brief Prints a token and its corresponding string.
 *
 * @param token The token type.
 * @param tokenString The string representation of the token.
 */
void printToken(const TokenType token, const char* tokenString) {
	switch (token) {
		case IF:
		case ELSE:
		case INT:
		case RETURN:
		case VOID:
		case WHILE:
			pc("reserved word: %s\n", tokenString);
			break;
		case ASSIGN:
			pc("=\n");
			break;
		case EQ:
			pc("==\n");
			break;
		case NEQ:
			pc("!=\n");
			break;
		case LT:
			pc("<\n");
			break;
		case LEQ:
			pc("<=\n");
			break;
		case GT:
			pc(">\n");
			break;
		case GEQ:
			pc(">=\n");
			break;
		case LPAREN:
			pc("(\n");
			break;
		case RPAREN:
			pc(")\n");
			break;
		case LBRACKET:
			pc("[\n");
			break;
		case RBRACKET:
			pc("]\n");
			break;
		case LBRACE:
			pc("{\n");
			break;
		case RBRACE:
			pc("}\n");
			break;
		case SEMI:
			pc(";\n");
			break;
		case COMMA:
			pc(",\n");
			break;
		case PLUS:
			pc("+\n");
			break;
		case MINUS:
			pc("-\n");
			break;
		case TIMES:
			pc("*\n");
			break;
		case OVER:
			pc("/\n");
			break;
		case ENDFILE:
			pc("EOF\n");
			break;
		case NUM:
			pc("NUM, val= %s\n", tokenString);
			break;
		case ID:
			pc("ID, name= %s\n", tokenString);
			break;
		case ERROR:
			pce("ERROR: %s\n", tokenString);
			break;
		default:
			pce("Unknown token: %d\n", token);
	}
}

/**
 * @brief Creates a new statement node.
 *
 * @param kind The kind of statement.
 * @return A pointer to the new statement node.
 */
TreeNode* newStmtNode(const StmtKind kind) {
	TreeNode* t = malloc(sizeof(TreeNode));
	if (t == NULL)
		pce("Out of memory error at line %d\n", lineno);
	else {
		for (int i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling   = NULL;
		t->nodekind  = StmtK;
		t->kind.stmt = kind;
		t->lineno    = lineno;
	}
	return t;
}

/**
 * @brief Creates a new expression node.
 *
 * @param kind The kind of expression.
 * @return A pointer to the new expression node.
 */
TreeNode* newExpNode(const ExpKind kind) {
	TreeNode* t = malloc(sizeof(TreeNode));
	if (t == NULL)
		pce("Out of memory error at line %d\n", lineno);
	else {
		for (int i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling  = NULL;
		t->nodekind = ExpK;
		t->kind.exp = kind;
		t->lineno   = lineno;
		t->type     = Void;
	}
	return t;
}

/**
 * @brief Copies a string.
 *
 * @param s The string to copy.
 * @return A pointer to the copied string.
 */
char* copyString(const char* s) {
	if (s == NULL) return NULL;
	const int n = strlen(s) + 1;
	char*     t = malloc(n);
	if (t == NULL)
		pce("Out of memory error at line %d\n", lineno);
	else
		strcpy(t, s);
	return t;
}

static int indentno = 0;

#define INDENT indentno += 4
#define UNINDENT indentno -= 4

/**
 * @brief Prints spaces for indentation.
 */
static void printSpaces(void) {
	for (int i = 0; i < indentno; i++) pc(" ");
}

/**
 * @brief Prints a line from the redundant source file.
 */
void printLine() {
	static int currentLine = 0;
	static int firstCall   = 1;
	char       line[1024];

	if (firstCall) {
		rewind(redundant_source); // Restart reading from the beginning
		firstCall = 0;
	}

	const char* ret = fgets(line, sizeof(line), redundant_source);
	if (ret) {
		currentLine++;
		pc("%d: %s", currentLine, line);

		// Handle EOF condition
		if (feof(redundant_source)) {
			// If the line doesn't end with a newline, add one
			if (line[strlen(line) - 1] != '\n') {
				pc("\n");
			}
		}
	}
}

/**
 * @brief Converts an expression type to a string.
 *
 * @param type The expression type.
 * @return The string representation of the expression type.
 */
const char* ExpTypeToString(const ExpType type) {
	switch (type) {
		case Void:
			return "void";
		case Integer:
			return "int";
		default:
			return "unknown";
	}
}

/**
 * @brief Prints the syntax tree.
 *
 * @param tree The syntax tree to print.
 */
void printTree(const TreeNode* tree) {
	while (tree != NULL) {
		// Check if the node is a CompoundK. If so, don't print anything for it.
		if (tree->nodekind == StmtK && tree->kind.stmt == CompoundK) {
			for (int i = 0; i < MAXCHILDREN; i++) {
				printTree(tree->child[i]);
			}
		} else {
			printSpaces();
			if (tree->nodekind == StmtK) {
				switch (tree->kind.stmt) {
					case IfK:
						pc("Conditional selection\n");
						break;
					case WhileK:
						pc("Iteration (loop)\n");
						break;
					case ReturnK:
						pc("Return\n");
						break;
					case ParamK:
						pc("Function param (%s %s): %s\n", ExpTypeToString(tree->type),
						   tree->isArray ? "array" : "var", tree->attr.name);
						break;
					case VarK:
						pc("Declare %s %s: %s\n", ExpTypeToString(tree->type),
						   (tree->child[0] ? "array" : "var"), tree->attr.name);
						break;
					case FuncK:
						pc("Declare function (return type \"%s\"): %s\n",
						   ExpTypeToString(tree->type), tree->attr.name);
						break;
					// For CompoundK, do nothing.
					default:
						pce("Unknown Stmt kind\n");
						break;
				}
			} else if (tree->nodekind == ExpK) {
				switch (tree->kind.exp) {
					case OpK:
						pc("Op: ");
						printToken(tree->attr.op, "\0");
						break;
					case ConstK:
						pc("Const: %d\n", tree->attr.val);
						break;
					case IdK:
						pc("Id: %s\n", tree->attr.name);
						break;
					case CallK:
						pc("Function call: %s\n", tree->attr.name);
						break;
					case AssignK: {
						const TreeNode* varNode = tree->child[0];
						if (varNode->nodekind == ExpK && varNode->kind.exp == IdK) {
							if (varNode->child[0] != NULL) {
								pc("Assign to array: %s\n", varNode->attr.name);
								INDENT;
								if (varNode->child[0]->attr.val != 0) printTree(varNode->child[0]);
								UNINDENT;
							} else {
								pc("Assign to var: %s\n", varNode->attr.name);
							}
						} else {
							pc("Assign to: (unknown)\n");
						}
						break;
					}
					default:
						pce("Unknown ExpNode kind\n");
						break;
				}
			} else {
				pce("Unknown node kind\n");
			}

			for (int i = 0; i < MAXCHILDREN; i++) {
				if (tree->nodekind == ExpK && tree->kind.exp == AssignK && i == 0) continue;
				INDENT;
				printTree(tree->child[i]);
				UNINDENT;
			}
		}
		tree = tree->sibling;
	}
}
