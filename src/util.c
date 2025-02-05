#include "util.h"
#include "globals.h"

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken(TokenType token, const char* tokenString) {
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
		case LE:
			pc("<=\n");
			break;
		case GT:
			pc(">\n");
			break;
		case GE:
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

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode* newStmtNode(StmtKind kind) {
	TreeNode* t = (TreeNode*) malloc(sizeof(TreeNode));
	int       i;
	if (t == NULL)
		pce("Out of memory error at line %d\n", lineno);
	else {
		for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling   = NULL;
		t->nodekind  = StmtK;
		t->kind.stmt = kind;
		t->lineno    = lineno;
	}
	return t;
}

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode* newExpNode(ExpKind kind) {
	TreeNode* t = (TreeNode*) malloc(sizeof(TreeNode));
	int       i;
	if (t == NULL)
		pce("Out of memory error at line %d\n", lineno);
	else {
		for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling  = NULL;
		t->nodekind = ExpK;
		t->kind.exp = kind;
		t->lineno   = lineno;
		t->type     = Void;
	}
	return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char* copyString(char* s) {
	int   n;
	char* t;
	if (s == NULL) return NULL;
	n = strlen(s) + 1;
	t = malloc(n);
	if (t == NULL)
		pce("Out of memory error at line %d\n", lineno);
	else
		strcpy(t, s);
	return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno += 2
#define UNINDENT indentno -= 2

/* printSpaces indents by printing spaces */
static void printSpaces(void) {
	int i;
	for (i = 0; i < indentno; i++) pc(" ");
}
/* Procedure printLine prints a full line
 * of the source code, with its number
 * reduntand_source is ANOTHER instance
 * of file pointer opened with the source code.
 */
void printLine() {
	static int currentLine = 0;
	static int firstCall   = 1;
	char       line[1024];

	if (firstCall) {
		rewind(redundant_source); // Restart reading from the beginning
		firstCall = 0;
	}

	char* ret = fgets(line, sizeof(line), redundant_source);
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

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode* tree) {
	int i;
	INDENT;
	while (tree != NULL) {
		printSpaces();
		if (tree->nodekind == StmtK) {
			switch (tree->kind.stmt) {
				case IfK:
					pc("If\n");
					break;
				case WhileK:
					pc("While\n");
					break;
				case AssignK:
					pc("Assign to: %s\n", tree->attr.name);
					break;
				default:
					pce("Unknown ExpNode kind\n");
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
				default:
					pce("Unknown ExpNode kind\n");
					break;
			}
		} else
			pce("Unknown node kind\n");
		for (i = 0; i < MAXCHILDREN; i++) printTree(tree->child[i]);
		tree = tree->sibling;
	}
	UNINDENT;
}