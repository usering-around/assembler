#include <string.h>
#include "instructions.h"

/* The bit at which the opcode field starts in an instruction's encoding */
#define OPCODE_START_BIT 18
/* The bit at which the src operand field starts in an instruction's encoding */
#define SRC_OPERAND_START_BIT 16
/* The bit at which the src register field starts in an instruction's encoding*/
#define SRC_REGISTER_START_BIT 13
/* The bit at which the dest operand field starts in an instruction's encoding*/
#define DEST_OPERAND_START_BIT 11
/* The bit at which the dest register field starts in an instruction's encoding*/
#define DEST_REGISTER_START_BIT 8
/* The bit at which the funct field starts in an instruction's encoding*/
#define FUNCT_START_BIT 3

uint32 encode_instruction(Instruction *instruction)
{
    uint32 opcode = instruction_opcode(instruction->type);
    uint32 funct = instruction_funct(instruction->type);
    uint32 out = 0;
    out |= 0x4; /*instruction must turn on the 'A' of the "A,R,E" field*/
    out |= (funct << FUNCT_START_BIT);
    if (instruction->operand_amount == 1)
    {
        /* in instruction with 1 operand, its operand is the dest operand */
        /* encode the type of operand into the instruction */
        out |= (instruction->operand1.type << DEST_OPERAND_START_BIT);
        /* if the instruction is a register, encode the regsiter number*/
        if (instruction->operand1.type == OPERAND_REGISTER)
        {
            out |= (instruction->operand1.value.register_num << DEST_REGISTER_START_BIT);
        }
    }
    if (instruction->operand_amount == 2)
    {
        /* In an instruction with 2 operands, its first operand is the src operand and the second operand is the dest operand */
        out |= (instruction->operand2.type << DEST_OPERAND_START_BIT);
        if (instruction->operand2.type == OPERAND_REGISTER)
        {
            out |= (instruction->operand2.value.register_num << DEST_REGISTER_START_BIT);
        }
        out |= (instruction->operand1.type << SRC_OPERAND_START_BIT);
        if (instruction->operand1.type == OPERAND_REGISTER)
        {
            out |= (instruction->operand1.value.register_num << SRC_REGISTER_START_BIT);
        }
    }
    out |= (opcode << OPCODE_START_BIT);
    return out;
}

bool str_to_instruction_type(const char *str, InstructionType *type)
{
    if (strncmp(str, "add", 3) == 0)
    {
        *type = INSTRUCTION_ADD;
    }
    else if (strncmp(str, "cmp", 3) == 0)
    {
        *type = INSTRUCTION_CMP;
    }
    else if (strncmp(str, "mov", 3) == 0)
    {
        *type = INSTRUCTION_MOV;
    }
    else if (strncmp(str, "sub", 3) == 0)
    {
        *type = INSTRUCTION_SUB;
    }
    else if (strncmp(str, "lea", 3) == 0)
    {
        *type = INSTRUCTION_LEA;
    }
    else if (strncmp(str, "clr", 3) == 0)
    {
        *type = INSTRUCTION_CLR;
    }
    else if (strncmp(str, "not", 3) == 0)
    {
        *type = INSTRUCTION_NOT;
    }
    else if (strncmp(str, "inc", 3) == 0)
    {
        *type = INSTRUCTION_INC;
    }
    else if (strncmp(str, "dec", 3) == 0)
    {
        *type = INSTRUCTION_DEC;
    }
    else if (strncmp(str, "jmp", 3) == 0)
    {
        *type = INSTRUCTION_JMP;
    }
    else if (strncmp(str, "bne", 3) == 0)
    {
        *type = INSTRUCTION_BNE;
    }
    else if (strncmp(str, "jsr", 3) == 0)
    {
        *type = INSTRUCTION_JSR;
    }
    else if (strncmp(str, "red", 3) == 0)
    {
        *type = INSTRUCTION_RED;
    }
    else if (strncmp(str, "prn", 3) == 0)
    {
        *type = INSTRUCTION_PRN;
    }
    else if (strncmp(str, "rts", 3) == 0)
    {
        *type = INSTRUCTION_RTS;
    }
    else if (strncmp(str, "stop", 4) == 0)
    {
        *type = INSTRUCTION_STOP;
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

bool is_an_instruction(const char *str)
{
    InstructionType dummy;
    if (str_to_instruction_type(str, &dummy) && *(str + instruction_name_len(dummy)) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
uint32 instruction_name_len(InstructionType type)
{
    switch (type)
    {
    case INSTRUCTION_MOV:
    case INSTRUCTION_CMP:
    case INSTRUCTION_ADD:
    case INSTRUCTION_SUB:
    case INSTRUCTION_LEA:
    case INSTRUCTION_CLR:
    case INSTRUCTION_NOT:
    case INSTRUCTION_INC:
    case INSTRUCTION_DEC:
    case INSTRUCTION_JMP:
    case INSTRUCTION_BNE:
    case INSTRUCTION_JSR:
    case INSTRUCTION_RED:
    case INSTRUCTION_PRN:
    case INSTRUCTION_RTS:
    {
        return 3;
        break;
    }
    case INSTRUCTION_STOP:
    {
        return 4;
        break;
    }
    }
    return 0; /* should be unreachable */
}

uint32 instruction_opcode(InstructionType type)
{
    switch (type)
    {
    case INSTRUCTION_MOV:
    {
        return 0;
    }
    case INSTRUCTION_CMP:
    {
        return 1;
    }
    case INSTRUCTION_ADD:
    case INSTRUCTION_SUB:
    {
        return 2;
    }
    case INSTRUCTION_LEA:
    {
        return 4;
    }
    case INSTRUCTION_CLR:
    case INSTRUCTION_NOT:
    case INSTRUCTION_INC:
    case INSTRUCTION_DEC:
    {
        return 5;
    }
    case INSTRUCTION_JMP:
    case INSTRUCTION_BNE:
    case INSTRUCTION_JSR:
    {
        return 9;
    }

    case INSTRUCTION_RED:
    {
        return 12;
    }
    case INSTRUCTION_PRN:
    {
        return 13;
    }
    case INSTRUCTION_RTS:
    {
        return 14;
    }
    case INSTRUCTION_STOP:
    {
        return 15;
    }
    }
    return 0; /* should be unreachable */
}

uint32 instruction_funct(InstructionType type)
{
    switch (type)
    {
    case INSTRUCTION_ADD:
    case INSTRUCTION_CLR:
    case INSTRUCTION_JMP:
    {
        return 1;
    }

    case INSTRUCTION_SUB:
    case INSTRUCTION_NOT:
    case INSTRUCTION_BNE:
    {
        return 2;
    }

    case INSTRUCTION_INC:
    case INSTRUCTION_JSR:
    {
        return 3;
    }

    case INSTRUCTION_DEC:
    {
        return 4;
    }

    default:
    {
        return 0;
    }
    }
}

uint32 instruction_operand_amount(InstructionType type)
{
    switch (type)
    {
    case INSTRUCTION_MOV:
    case INSTRUCTION_CMP:
    case INSTRUCTION_ADD:
    case INSTRUCTION_SUB:
    case INSTRUCTION_LEA:
    {
        return 2;
        break;
    }
    case INSTRUCTION_CLR:
    case INSTRUCTION_NOT:
    case INSTRUCTION_INC:
    case INSTRUCTION_DEC:
    case INSTRUCTION_JMP:
    case INSTRUCTION_BNE:
    case INSTRUCTION_JSR:
    case INSTRUCTION_RED:
    case INSTRUCTION_PRN:
    {
        return 1;
        break;
    }

    case INSTRUCTION_RTS:
    case INSTRUCTION_STOP:
    {
        return 0;
        break;
    }
    }
    return -1; /* should be unreachable */
}

uint32 instruction_encoding_word_count(Instruction *instruction)
{
    uint32 words = 1; /* 1 word for the instruction*/
    /* we only need extra words if our operands are not registers */
    if (instruction->operand_amount >= 1 && instruction->operand1.type != OPERAND_REGISTER)
    {
        words += 1;
    }
    if (instruction->operand_amount >= 2 && instruction->operand2.type != OPERAND_REGISTER)
    {
        words += 1;
    }
    return words;
}

const OperandType *acceptable_dest_operands(InstructionType type, int *array_size)
{
    static OperandType first_kind[] = {OPERAND_SYMBOL, OPERAND_REGISTER};                     /* 1, 3 */
    static OperandType second_kind[] = {OPERAND_IMMEDIATE, OPERAND_SYMBOL, OPERAND_REGISTER}; /* 0, 1, 3 */
    static OperandType third_kind[] = {OPERAND_SYMBOL, OPERAND_ADDRESS};                      /* 1, 2 */
    switch (type)
    {
    case INSTRUCTION_MOV:
    case INSTRUCTION_ADD:
    case INSTRUCTION_SUB:
    case INSTRUCTION_LEA:
    case INSTRUCTION_CLR:
    case INSTRUCTION_NOT:
    case INSTRUCTION_INC:
    case INSTRUCTION_DEC:
    case INSTRUCTION_RED:
    {
        *array_size = 2;
        return first_kind;
    }

    case INSTRUCTION_CMP:
    case INSTRUCTION_PRN:
    {
        *array_size = 3;
        return second_kind;
    }

    case INSTRUCTION_JMP:
    case INSTRUCTION_BNE:
    case INSTRUCTION_JSR:
    {
        *array_size = 2;
        return third_kind;
    }

    default:
    {
        *array_size = 0;
        return NULL;
    }
    }
}

const OperandType *acceptable_src_operands(InstructionType type, int *array_size)
{
    static OperandType first_kind[] = {OPERAND_IMMEDIATE, OPERAND_SYMBOL, OPERAND_REGISTER}; /* 0, 1, 3 */
    static OperandType second_kind[] = {OPERAND_SYMBOL};                                     /* 1 */
    switch (type)
    {
    case INSTRUCTION_MOV:
    case INSTRUCTION_CMP:
    case INSTRUCTION_ADD:
    case INSTRUCTION_SUB:
    {
        *array_size = 3;
        return first_kind;
    }

    case INSTRUCTION_LEA:
    {
        *array_size = 1;
        return second_kind;
    }

    default:
    {
        *array_size = 0;
        return NULL;
    }
    }
}

const char *operand_type_name(OperandType op_type)
{
    switch (op_type)
    {
    case OPERAND_REGISTER:
    {
        return "register";
        break;
    }
    case OPERAND_IMMEDIATE:
    {
        return "immediate";
        break;
    }

    case OPERAND_SYMBOL:
    {
        return "symbol";
        break;
    }

    case OPERAND_ADDRESS:
    {
        return "address";
        break;
    }

    /* should be unreachable */
    default:
    {
        return "unknown??? bug";
        break;
    }
    }
}
