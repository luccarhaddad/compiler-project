#include "code.h"
#include "globals.h"

/**
 * @brief TM location number for current instruction emission.
 */
static int emitLoc = 0;

/**
 * @brief Highest TM location emitted so far.
 *
 * This variable is used in conjunction with emitSkip, emitBackup, and emitRestore.
 */
static int highEmitLoc = 0;

void emitComment(char* comment) {
	if (TraceCode) pc("* %s\n", comment);
}

void emitRO(char* opcode, const int targetReg, const int srcReg1, const int srcReg2, char* comment) {
	pc("%3d:  %5s  %d,%d,%d ", emitLoc++, opcode, targetReg, srcReg1, srcReg2);
	if (TraceCode) pc("\t%s", comment);
	pc("\n");
	if (highEmitLoc < emitLoc) highEmitLoc = emitLoc;
}

void emitRM(char* opcode, const int targetReg, const int offset, const int baseReg, char* comment) {
	pc("%3d:  %5s  %d,%d(%d) ", emitLoc++, opcode, targetReg, offset, baseReg);
	if (TraceCode) pc("\t%s", comment);
	pc("\n");
	if (highEmitLoc < emitLoc) highEmitLoc = emitLoc;
}

void emitRM_Abs(char* opcode, const int targetReg, const int absLocation, char* comment) {
	pc("%3d:  %5s  %d,%d(%d) ", emitLoc, opcode, targetReg, absLocation - (emitLoc + 1), PROGRAM_COUNTER);
	++emitLoc;
	if (TraceCode) pc("\t%s", comment);
	pc("\n");
	if (highEmitLoc < emitLoc) highEmitLoc = emitLoc;
}

int emitSkip(const int howMany) {
	const int i = emitLoc;
	emitLoc += howMany;
	if (highEmitLoc < emitLoc) highEmitLoc = emitLoc;
	return i;
}

void emitBackup(const int location) {
	if (location > highEmitLoc) emitComment("BUG in emitBackup");
	emitLoc = location;
}

void emitRestore(void) {
	emitLoc = highEmitLoc;
}
