#include "cgen.h"
#include "code.h"
#include "hash.h"
#include "symtab.h"

/**
 * Default memory location for the main function.
 */
#define DEFAULT_MAIN_LOCATION 3

/**
 * Temporary offset for memory locations.
 */
static int tmpOffset = -2;

/**
 * Counter for the number of compound scopes.
 */
static int numberOfCompoundScopes = 0;

/**
 * Name of the current scope.
 */
static char* currentScopeName = "global";

/**
 * Flag indicating if parameters are from a function call.
 */
static bool areParametersFromFunctionCall = FALSE;

/**
 * Flag indicating if the first declared function is being processed.
 */
static bool isFirstDeclaredFunction = TRUE;

/**
 * Memory location of the main function.
 */
static int mainFunctionMemoryLocation = 3;

static void cGen(TreeNode* tree);

static void generateStatementCode(TreeNode* node) {
	int savedLocation1, savedLocation2, savedLocation3;

	switch (node->kind.stmt) {
		case FuncK: {
			char comment[50];
			sprintf(comment, "-> Init Function (%s)", node->attr.name);
			emitComment(comment);

			const int initialLocation = emitSkip(0);

			if (isFirstDeclaredFunction) {
				mainFunctionMemoryLocation = emitSkip(1);
				isFirstDeclaredFunction    = FALSE;
				insert(node->attr.name, mainFunctionMemoryLocation + 1);
			} else {
				insert(node->attr.name, initialLocation);
			}

			currentScopeName = node->attr.name;
			tmpOffset        = -2;

			if (strcmp(node->attr.name, "main") == 0) {
				savedLocation1 = emitSkip(0);
				emitBackup(mainFunctionMemoryLocation);
				if (savedLocation1 == DEFAULT_MAIN_LOCATION)
					emitRM_Abs("LDA", PROGRAM_COUNTER, savedLocation1 + 1, "jump to main");
				else
					emitRM_Abs("LDA", PROGRAM_COUNTER, savedLocation1, "jump to main");

				emitRestore();

				if (node->child[0]) cGen(node->child[0]);

				if (node->child[1]) cGen(node->child[1]);

				emitComment("<- End Function");
				break;
			}

			emitRM("ST", ACCUMULATOR, -1, FRAME_POINTER, "store return address");

			if (node->child[0]) cGen(node->child[0]);

			if (node->child[1]) cGen(node->child[1]);

			if (node->type == Void) {
				emitRM("LDA", ACCUMULATOR_1, 0, FRAME_POINTER, "save current fp into ac1");
				emitRM("LD", FRAME_POINTER, 0, FRAME_POINTER, "make fp = ofp");
				emitRM("LD", PROGRAM_COUNTER, -1, ACCUMULATOR_1, "return to caller");
			}

			emitComment("<- End Function");
			break;
		}
		case ParamK: {
			if (node->isArray) {
				emitComment("-> Param vector");
				tmpOffset--;
				emitComment("<- Param vector");
				break;
			}
			emitComment("-> Param");
			tmpOffset--;
			emitComment("<- Param");
			break;
		}
		case VarK: {
			if (node->isArray) {
				emitComment("-> declare vector");
				if (strcmp(node->scope->name, "global") == 0) {
					BucketList symbol = symbolTableLookupFromScope(node->attr.name, node->scope);
					const int  memoryLocation = symbol->memoryLocation;
					emitRM("LDC", ACCUMULATOR, memoryLocation, 0, "load global position to ac");
					emitRM("LDC", GLOBAL_POINTER, 0, 0, "load 0");
					emitRM("ST", ACCUMULATOR, memoryLocation, GLOBAL_POINTER,
					       "store global position");
					emitComment("<- declare vector");
					break;
				}
				emitRM("LDA", ACCUMULATOR, tmpOffset, FRAME_POINTER, "guard vector address");
				emitRM("ST", ACCUMULATOR, tmpOffset, FRAME_POINTER, "store vector address");
				tmpOffset -= node->child[0]->attr.val + 1;
				emitComment("<- declare vector");
				break;
			}
			emitComment("-> declare var");
			tmpOffset--;
			emitComment("<- declare var");
			break;
		}
		case ReturnK: {
			emitComment("-> return");

			if (node->child[0]) {
				cGen(node->child[0]);
			}

			emitRM("LDA", ACCUMULATOR_1, 0, FRAME_POINTER, "load return address");
			emitRM("LD", FRAME_POINTER, 0, FRAME_POINTER, "make fp = ofp");
			emitRM("LD", PROGRAM_COUNTER, -1, ACCUMULATOR_1, "return to caller");

			emitComment("<- return");
			break;
		}
		case CompoundK: {
			sprintf(currentScopeName, "compound%d", numberOfCompoundScopes);

			// Local declarations
			if (node->child[0]) {
				cGen(node->child[0]);
			}

			// Statements list
			if (node->child[1]) {
				cGen(node->child[1]);
			}
			break;
		}
		case IfK: {
			emitComment("-> if");

			// Condition
			if (node->child[0]) {
				cGen(node->child[0]);
			}
			savedLocation1 = emitSkip(1);
			emitComment("if: jump to else belongs here");

			// If body
			if (node->child[1]) {
				cGen(node->child[1]);
			}
			savedLocation2 = emitSkip(1);
			emitComment("if: jump to end belongs here");

			emitBackup(savedLocation1);
			emitRM_Abs("JEQ", ACCUMULATOR, savedLocation2 + 1, "if: jmp to else");
			emitRestore();

			// Else body
			if (node->child[2]) {
				cGen(node->child[2]);
			}
			savedLocation3 = emitSkip(0);
			emitBackup(savedLocation2);
			emitRM_Abs("LDA", PROGRAM_COUNTER, savedLocation3, "jmp to end");
			emitRestore();

			emitComment("<- if");
			break;
		}
		case WhileK: {
			emitComment("-> while");
			emitComment("repeat: jump after body comes back here");

			// Condition
			savedLocation1 = emitSkip(0);
			if (node->child[0]) {
				cGen(node->child[0]);
			}

			// Body
			savedLocation2 = emitSkip(1);
			if (node->child[1]) {
				cGen(node->child[1]);
			}

			emitRM_Abs("LDA", PROGRAM_COUNTER, savedLocation1, "jump back to body");
			savedLocation1 = emitSkip(0);
			emitBackup(savedLocation2);
			emitRM_Abs("JEQ", ACCUMULATOR, savedLocation1, "repeat: jmp to end");

			emitRestore();
			emitComment("<- while");
			break;
		}
		default:
			break;
	}
}

static void generateExpressionCode(TreeNode* node) {
	BucketList symbol;

	switch (node->kind.exp) {
		case OpK: {
			emitComment("-> Op");

			if (node->child[0]) {
				cGen(node->child[0]);
			}
			emitRM("ST", ACCUMULATOR, tmpOffset--, FRAME_POINTER, "op: push left");

			if (node->child[1]) {
				cGen(node->child[1]);
			}
			emitRM("LD", ACCUMULATOR_1, ++tmpOffset, FRAME_POINTER, "op: load left");

			switch (node->attr.op) {
				case PLUS: {
					emitRO("ADD", ACCUMULATOR, ACCUMULATOR_1, ACCUMULATOR, "op +");
					break;
				}
				case MINUS: {
					emitRO("SUB", ACCUMULATOR, ACCUMULATOR_1, ACCUMULATOR, "op -");
					break;
				}
				case TIMES: {
					emitRO("MUL", ACCUMULATOR, ACCUMULATOR_1, ACCUMULATOR, "op *");
					break;
				}
				case OVER: {
					emitRO("DIV", ACCUMULATOR, ACCUMULATOR_1, ACCUMULATOR, "op /");
					break;
				}
				case LT: {
					emitRO("SUB", ACCUMULATOR, ACCUMULATOR_1, ACCUMULATOR, "op <");
					emitRM("JLT", ACCUMULATOR, 2, PROGRAM_COUNTER, "br if true");
					emitRM("LDC", ACCUMULATOR, 0, ACCUMULATOR, "false case");
					emitRM("LDA", PROGRAM_COUNTER, 1, PROGRAM_COUNTER, "unconditional jmp");
					emitRM("LDC", ACCUMULATOR, 1, ACCUMULATOR, "true case");
					break;
				}
				case LEQ: {
					emitRO("SUB", ACCUMULATOR, ACCUMULATOR_1, ACCUMULATOR, "op <=");
					emitRM("JLE", ACCUMULATOR, 2, PROGRAM_COUNTER, "br if true");
					emitRM("LDC", ACCUMULATOR, 0, ACCUMULATOR, "false case");
					emitRM("LDA", PROGRAM_COUNTER, 1, PROGRAM_COUNTER, "unconditional jmp");
					emitRM("LDC", ACCUMULATOR, 1, ACCUMULATOR, "true case");
					break;
				}
				case GT: {
					emitRO("SUB", ACCUMULATOR, ACCUMULATOR_1, ACCUMULATOR, "op >");
					emitRM("JGT", ACCUMULATOR, 2, PROGRAM_COUNTER, "br if true");
					emitRM("LDC", ACCUMULATOR, 0, ACCUMULATOR, "false case");
					emitRM("LDA", PROGRAM_COUNTER, 1, PROGRAM_COUNTER, "unconditional jmp");
					emitRM("LDC", ACCUMULATOR, 1, ACCUMULATOR, "true case");
					break;
				}
				case GEQ: {
					emitRO("SUB", ACCUMULATOR, ACCUMULATOR_1, ACCUMULATOR, "op >=");
					emitRM("JGE", ACCUMULATOR, 2, PROGRAM_COUNTER, "br if true");
					emitRM("LDC", ACCUMULATOR, 0, ACCUMULATOR, "false case");
					emitRM("LDA", PROGRAM_COUNTER, 1, PROGRAM_COUNTER, "unconditional jmp");
					emitRM("LDC", ACCUMULATOR, 1, ACCUMULATOR, "true case");
					break;
				}
				case EQ: {
					emitRO("SUB", ACCUMULATOR, ACCUMULATOR_1, ACCUMULATOR, "op ==");
					emitRM("JEQ", ACCUMULATOR, 2, PROGRAM_COUNTER, "br if true");
					emitRM("LDC", ACCUMULATOR, 0, ACCUMULATOR, "false case");
					emitRM("LDA", PROGRAM_COUNTER, 1, PROGRAM_COUNTER, "unconditional jmp");
					emitRM("LDC", ACCUMULATOR, 1, ACCUMULATOR, "true case");
					break;
				}
				case NEQ: {
					emitRO("SUB", ACCUMULATOR, ACCUMULATOR_1, ACCUMULATOR, "op !=");
					emitRM("JNE", ACCUMULATOR, 2, PROGRAM_COUNTER, "br if true");
					emitRM("LDC", ACCUMULATOR, 0, ACCUMULATOR, "false case");
					emitRM("LDA", PROGRAM_COUNTER, 1, PROGRAM_COUNTER, "unconditional jmp");
					emitRM("LDC", ACCUMULATOR, 1, ACCUMULATOR, "true case");
					break;
				}
				default: {
					emitComment("Unkwown operator");
					break;
				}
			}

			emitComment("<- Op");
			break;
		}
		case IdK: {
			emitComment("-> Id");

			symbol = symbolTableLookupFromScope(node->attr.name, node->scope);
			if (node->isArray) {
				emitComment("-> Vector");
				// Global array
				if (strcmp(symbol->scope, "global") == 0) {
					emitRM("LDC", GLOBAL_POINTER, 0, 0, "load 0");
					emitRM("LD", ACCUMULATOR, symbol->memoryLocation, GLOBAL_POINTER,
					       "get the address of the vector");
				} else { // Local array
					emitRM("LD", ACCUMULATOR, symbol->memoryLocation - MAX_MEMORY, FRAME_POINTER,
					       "get the address of the vector");
				}

				// Array indexing
				int arrayIndex;
				if (node->child[0]->nodekind == ExpK && node->child[0]->kind.exp == ConstK) {
					arrayIndex = node->child[0]->attr.val;
					emitRM("LDC", INDEX_POINTER, arrayIndex, 0, "get the value of the index");
				} else {
					symbol     = symbolTableLookupFromScope(node->child[0]->attr.name,
					                                        node->child[0]->scope);
					arrayIndex = symbol->memoryLocation - MAX_MEMORY;
					emitRM("LD", INDEX_POINTER, arrayIndex, FRAME_POINTER,
					       "get the value of the index");
				}

				emitRM("LDC", ACCUMULATOR_2, 1, 0, "load 1");
				emitRO("ADD", INDEX_POINTER, INDEX_POINTER, ACCUMULATOR_2, "sub 3 by 1");
				emitRO("SUB", ACCUMULATOR, ACCUMULATOR, INDEX_POINTER, "get the address");
				emitRM("LD", ACCUMULATOR, 0, ACCUMULATOR, "get the value of the vector");

				emitComment("<- Vector");
				break;
			}

			if (strcmp(symbol->scope, "global") == 0) {
				emitRM("LDC", GLOBAL_POINTER, 0, 0, "load 0");
				emitRM("LD", ACCUMULATOR, symbol->memoryLocation, GLOBAL_POINTER, "load id value");
			} else {
				emitRM("LD", ACCUMULATOR, symbol->memoryLocation - MAX_MEMORY, FRAME_POINTER,
				       "load id value");
			}

			emitComment("<- Id");
			break;
		}
		case CallK: {
			char comment[50];
			sprintf(comment, "-> Function call (%s)", node->attr.name);
			emitComment(comment);

			if (strcmp(node->attr.name, "input") == 0) {
				emitRO("IN", ACCUMULATOR, 0, 0, "read input");
			} else if (strcmp(node->attr.name, "output") == 0) {
				if (node->child[0]) cGen(node->child[0]);
				emitRO("OUT", ACCUMULATOR, 0, 0, "print value");
			} else {
				const int auxiliar = tmpOffset;
				emitRM("ST", FRAME_POINTER, tmpOffset, FRAME_POINTER, "guard fp");
				tmpOffset -= 2;

				areParametersFromFunctionCall = TRUE;
				TreeNode* paramPointer        = node->child[0];
				while (paramPointer) {
					cGen(paramPointer);
					emitRM("ST", ACCUMULATOR, tmpOffset--, FRAME_POINTER,
					       "Store value of func argument");
					paramPointer = paramPointer->sibling;
				}
				areParametersFromFunctionCall = FALSE;
				tmpOffset                     = auxiliar;

				emitRM("LDA", FRAME_POINTER, tmpOffset, FRAME_POINTER, "change fp");
				int savedLocation = emitSkip(0);
				emitRM("LDC", ACCUMULATOR, savedLocation + 2, 0, "load return address");
				emitRM_Abs("LDA", PROGRAM_COUNTER, lookup(node->attr.name), "jump to function");

				emitComment("<- Function Call");
			}
			break;
		}
		case AssignK: {
			if (node->child[0]->isArray) {
				emitComment("-> assign vector");
				emitComment("-> vector");

				if (node->child[1]) {
					cGen(node->child[1]);
				}

				symbol =
				    symbolTableLookupFromScope(node->child[0]->attr.name, node->child[0]->scope);
				if (strcmp(symbol->scope, "global") == 0) {
					emitRM("LDC", GLOBAL_POINTER, 0, 0, "load 0");
					emitRM("LD", ACCUMULATOR_1, symbol->memoryLocation, GLOBAL_POINTER,
					       "get the address of the vector");
				} else {
					emitRM("LD", ACCUMULATOR_1, symbol->memoryLocation - MAX_MEMORY, FRAME_POINTER,
					       "get the address of the vector");
				}

				// Array indexing
				const TreeNode* tmp = node->child[0]->child[0];
				if (tmp->nodekind == ExpK && tmp->kind.exp == ConstK) {
					emitRM("LDC", INDEX_POINTER, tmp->attr.val, 0, "load array index");
				} else {
					symbol = symbolTableLookupFromScope(tmp->attr.name, tmp->scope);
					emitRM("LD", INDEX_POINTER, symbol->memoryLocation - MAX_MEMORY, FRAME_POINTER,
					       "load array index");
				}

				emitRM("LDC", ACCUMULATOR_2, 1, 0, "load 1");
				emitRO("ADD", INDEX_POINTER, INDEX_POINTER, ACCUMULATOR_2, "sub 3 by 1");
				emitRO("SUB", ACCUMULATOR_1, ACCUMULATOR_1, INDEX_POINTER, "get the address");
				emitRM("ST", ACCUMULATOR, 0, ACCUMULATOR_1, "get the value of the vector");

				emitComment("<- vector");
				emitComment("<- assign vector");
				break;
			}

			emitComment("-> assign");

			if (node->child[1]) cGen(node->child[1]);

			symbol = symbolTableLookupFromScope(node->child[0]->attr.name, node->child[0]->scope);
			emitRM("ST", ACCUMULATOR, symbol->memoryLocation - MAX_MEMORY, FRAME_POINTER,
			       "store value");

			emitComment("<- assign");
			break;
		}
		case ConstK: {
			emitComment("-> Const");
			emitRM("LDC", ACCUMULATOR, node->attr.val, 0, "load const");
			emitComment("<- Const");
			break;
		}
		case UnaryK: {
			emitComment("-> Unary");

			if (node->child[0]) cGen(node->child[0]);

			if (node->attr.op == MINUS) {
				emitRM("LDC", ACCUMULATOR_1, 0, 0, "load constant 0");
				emitRO("SUB", ACCUMULATOR, ACCUMULATOR_1, ACCUMULATOR, "unary -");
			}

			emitComment("<- Unary");
			break;
		}
		default:
			break;
	}
}

static void cGen(TreeNode* tree) {
	if (tree) {
		switch (tree->nodekind) {
			case StmtK:
				generateStatementCode(tree);
				break;
			case ExpK:
				generateExpressionCode(tree);
				break;
			default:
				break;
		}
		if (!areParametersFromFunctionCall) cGen(tree->sibling);
	}
}

void generateCode(TreeNode* syntaxTree) {
	emitComment("TINY Compilation to TM Code");

	emitComment("Standard prelude:");
	emitRM("LD", MEMORY_POINTER, 0, 0, "load maxaddress from location 0");
	emitRM("LD", FRAME_POINTER, 0, 0, "load maxaddress from location 0");
	emitRM("ST", ACCUMULATOR, 0, 0, "clear location 0");
	emitComment("End of standard prelude.");

	currentScopeName = "global";
	cGen(syntaxTree);

	emitComment("End of execution.");
	emitRO("HALT", 0, 0, 0, "");
}
