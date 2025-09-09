/* This module contains the instruction and operand structs responsible for representing assembly instructions, as well as related utilities */

#ifndef _MMN14_INSTRUCTIONS_H_
#define _MMN14_INSTRUCTIONS_H_

#include "bool.h"
#include "utils.h"

/* The type of the instruction. */
typedef enum
{
    INSTRUCTION_MOV,
    INSTRUCTION_CMP,
    INSTRUCTION_ADD,
    INSTRUCTION_SUB,
    INSTRUCTION_LEA,
    INSTRUCTION_CLR,
    INSTRUCTION_NOT,
    INSTRUCTION_INC,
    INSTRUCTION_DEC,
    INSTRUCTION_JMP,
    INSTRUCTION_BNE,
    INSTRUCTION_JSR,
    INSTRUCTION_RED,
    INSTRUCTION_PRN,
    INSTRUCTION_RTS,
    INSTRUCTION_STOP
} InstructionType;

/* The type of operand in an instruction */
typedef enum
{
    /* Immediate operands start with a # and then an signed integer */
    OPERAND_IMMEDIATE,
    /* symbol operands are labels/symbols which are defined somewhere in the program */
    OPERAND_SYMBOL,
    /* address operands are labels/symbols which are defined somewhere in the program and used in jmp operations */
    OPERAND_ADDRESS,
    /* registers r0-r7 */
    OPERAND_REGISTER
} OperandType;

/* An operand of an instruction */
typedef struct
{
    /* the type of the operand. */
    OperandType type;
    union
    {
        /* has valid value when type is OPERAND_IMMEDIATE */
        int immediate;
        /* has valid value when type is OPERAND_REGISTER*/
        uint8 register_num;
        /* has valid value when type is OPERAND_SYMBOL or OPERAND_ADDRESS */
        char symbol_name[MAX_LABEL_SIZE + 1];
    } value;
} Operand;

/* An instruction */
typedef struct
{
    /* the type of the instruction */
    InstructionType type;
    /* amount of operands found in the instruction (1 or 2)*/
    uint32 operand_amount;
    /* the first operand of the instruction. This field is valid only if operand_amount is greater or equal to 1. */
    Operand operand1;
    /* the second operand of the instruction. This field is only valid if operand_amount is 2.*/
    Operand operand2;
} Instruction;

/**
 * @brief Encode an instruction's first word. Note: an instruction may contain up to 3 words, this only gives the first one.
 * @param instruction the instruction to encode
 * @return the encoded word
 */
uint32 encode_instruction(Instruction *instruction);

/**
 * @brief Attempts to parse a string as an instruction. Note: this function will only attemps to parse as much as necessary,
 * so for example a string like "add ignore" would get flagged as the add instruction.
 * @param str the string to parse
 * @param type out parameter. If the string is an instruction, this will be set to the corresponding InstructionType
 * @return TRUE if the given str starts with the name of an instruction, FALSE otherwise.
 */
bool str_to_instruction_type(const char *str, InstructionType *type);

/**
 * @brief Check if a string exactly matches an instruction's name. Note: this does exact matching,
 * so for example someting like "add ignore" would get flagged as false, however "add" would be flagged as true.
 * @param str the string to check
 * @return TRUE if the string matches an instruction's name, FALSE otherwise.
 */
bool is_an_instruction(const char *str);

/**
 * @brief Get an instruction's name length
 * @param type the instruction of which to get the name length
 * @return the length of the name of the instruction
 */
uint32 instruction_name_len(InstructionType type);

/**
 * @brief Get an instruction's opcode
 * @param type the instruction of which to get the opcode
 * @return the opcode of the instruction
 */
uint32 instruction_opcode(InstructionType type);

/**
 * @brief Get an instruction's funct
 * @param type the instruction of which to get the funct
 * @return the funct of the instruction
 */
uint32 instruction_funct(InstructionType type);

/**
 * @brief Get an instruction's operand amount
 * @param type the instruction of which to get the operand_amount
 * @return the operand amount of the instruction
 */
uint32 instruction_operand_amount(InstructionType type);

/**
 * @brief Get the amount of words necessary to encode an instruction
 * @param type the instruction of which to get the word count
 * @return the amount of words necessary to encode the instruction
 */
uint32 instruction_encoding_word_count(Instruction *instruction);

/**
 * @brief Gets an list of acceptable dest operands of an instruction. Note that the dest operand is the first operand of an instruction with 1 operand,
   and the second operand of an instruction with 2 operands.
 * @param type the specified instruction
 * @param array_size out parameter. The size of the array which is returned
 * @return an array which contains each OperandType this instruction accepts as dest operand.
 * Note: this returns a static array. YOU SHOULD NOT MODIFY IT!
 */
const OperandType *acceptable_dest_operands(InstructionType type, int *array_size);

/**
 * @brief Gets an list of acceptable src operands of an instruction with 2 operands. Note that the src operand is the first operand.
 * @param type the specified instruction
 * @param array_size out parameter. The size of the array which is returned
 * @return an array which contains each OperandType this instruction accepts for an src operand.
 * Note: this is a static array. YOU SHOULD NOT MODIFY IT!
 */
const OperandType *acceptable_src_operands(InstructionType type, int *array_size);

/* An upper bound to the length of a the name of an operand type */
#define MAX_OPERAND_TYPE_STR_LENGTH (20)

/**
 * @brief Get the name of an operand type
 * @param op_type the operand type to get the name of
 * @return the name of the operand type. Note: this is a constant string. YOU SHOULD NOT MODIFY IT!
 */
const char *operand_type_name(OperandType op_type);

#endif