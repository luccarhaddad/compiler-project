#ifndef _CODE_H_
#define _CODE_H_

/* Program counter register */
#define PROGRAM_COUNTER 7

/* Memory pointer register, points to the top of memory for temporary storage */
#define MEMORY_POINTER 6

/* Global pointer register, points to the bottom of memory for global variable storage */
#define GLOBAL_POINTER 5

/* Frame pointer register, points to the current frame */
#define ACCUMULATOR_2 4

/* Index pointer, used for indexing operations */
#define INDEX_POINTER 3

/* Third accumulator register */
#define FRAME_POINTER 2

/* Second accumulator register */
#define ACCUMULATOR_1 1

/* Accumulator register */
#define ACCUMULATOR 0

/**
 * @brief Emits a comment line in the code file.
 *
 * @param comment The comment to be printed.
 */
void emitComment(char* comment);

/**
 * @brief Emits a register-only TM instruction.
 *
 * @param opcode The opcode of the instruction.
 * @param targetReg The target register.
 * @param srcReg1 The first source register.
 * @param srcReg2 The second source register.
*/
void emitRO(char* opcode, int targetReg, int srcReg1, int srcReg2, char* comment);

/**
 * @brief Emits a register-to-memory TM instruction.
 *
 * @param opcode The opcode of the instruction.
 * @param targetReg The target register.
 * @param offset The offset.
 * @param baseReg The base register.
 * @param comment A comment to be printed if TraceCode is TRUE.
 */
void emitRM(char* opcode, int targetReg, int offset, int baseReg, char* comment);

/**
 * @brief Emits a register-to-memory TM instruction with an absolute reference.
 *
 * @param opcode The opcode of the instruction.
 * @param targetReg The target register.
 * @param absLocation The absolute location in memory.
 * @param comment A comment to be printed if TraceCode is TRUE.
 */
void emitRM_Abs(char* opcode, int targetReg, int absLocation, char* comment);

/**
 * @brief Skips a number of code locations for later backpatching.
 *
 * @param howMany The number of locations to skip.
 * @return The current code position.
 */
int emitSkip(int howMany);

/**
 * @brief Backs up to a previously skipped location.
 *
 * @param location The location to back up to.
 */
void emitBackup(int location);

/**
 * @brief Restores the current code position to the highest previously unemitted position.
 */
void emitRestore(void);

#endif