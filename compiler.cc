#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "Stack.h"
#include "compiler.h"

#define DEBUG 1     // 1 => Turn ON debugging, 0 => Turn OFF debugging

void debug(const char * format, ...)
{
    va_list args;
    if (DEBUG)
    {
        va_start (args, format);
        vfprintf (stdout, format, args);
        va_end (args);
    }
}

//---------------------------------------------------------
// Lexer

char token[MAX_TOKEN_LENGTH];      // token string
int  ttype;                        // token type
int  activeToken = FALSE;
int  tokenLength;
int  line_no = 1;

char *reserved[] =
{
    "",
    "VAR",
    "IF",
    "WHILE",
    "SWITCH",
    "CASE",
    "DEFAULT",
    "print",
    "ARRAY",
    "+",
    "-",
    "/",
    "*",
    "=",
    ":",
    ",",
    ";",
    "[",
    "]",
    "(",
    ")",
    "{",
    "}",
    "<>",
    ">",
    "<",
    "ID",
    "NUM",
    "ERROR"
};

void skipSpace()
{
    char c;

    c = getchar();
    line_no += (c == '\n');
    while (!feof(stdin) && isspace(c))
    {
        c = getchar();
        line_no += (c == '\n');
    }

    // return character to input buffer if eof is not reached
    if (!feof(stdin))
        ungetc(c, stdin);
}

int isKeyword(char *s)
{
    int i;

    for (i = 1; i <= KEYWORDS; i++)
        if (strcmp(reserved[i], s) == 0)
            return i;
    return FALSE;
}

int scan_number()
{
    char c;

    c = getchar();
    if (isdigit(c))
    {
        if (c == '0')
        {
            token[tokenLength] = c;
            tokenLength++;
        }
        else
        {
            while (isdigit(c))
            {
                token[tokenLength] = c;
                tokenLength++;;
                c = getchar();
            }
            ungetc(c, stdin);
        }
        token[tokenLength] = '\0';
        return NUM;
    }
    else
        return ERROR;
}


int scan_id_or_keyword()
{
    int ttype;
    char c;

    c = getchar();
    if (isalpha(c))
    {
        while (isalnum(c))
        {
            token[tokenLength] = c;
            tokenLength++;
            c = getchar();
        }
        if (!feof(stdin))
			if(c == '('){
				// this is a function call
				ungetc(c, stdin);
				ttype = FUNCTION;
				token[tokenLength] = '\0';
				return ttype;
			}
			else
			 ungetc(c, stdin);

        token[tokenLength] = '\0';
        ttype = isKeyword(token);
        if (ttype == 0)
            ttype = ID;
        return ttype;
    }
    else
        return ERROR;
}

void ungetToken()
{
    activeToken = TRUE;
}

int getToken()
{
    char c;

    if (activeToken)
    {
        activeToken = FALSE;
        return ttype;
    }

    skipSpace();
    tokenLength = 0;
    c = getchar();

    switch (c)
    {
        case '+':   ttype = PLUS;       break;
        case '-':   ttype = MINUS;      break;
        case '/':   ttype = DIV;        break;
        case '*':   ttype = MULT;       break;
        case '=':   ttype = EQUAL;      break;
        case ':':   ttype = COLON;      break;
        case ',':   ttype = COMMA;      break;
        case ';':   ttype = SEMICOLON;  break;
        case '[':   ttype = LBRAC;      break;
        case ']':   ttype = RBRAC;      break;
        case '(':   ttype = LPAREN;     break;
        case ')':   ttype = RPAREN;     break;
        case '{':   ttype = LBRACE;     break;
        case '}':   ttype = RBRACE;     break;
        case '>':   ttype = GREATER;    break;
        case '<':
            c = getchar();
            if (c == '>')
                ttype = NOTEQUAL;
            else
            {
                ungetc(c, stdin);
                ttype = LESS;
            }
            break;
        default :
            if (isdigit(c))
            {
                ungetc(c, stdin);
                ttype = scan_number();
            }
            else if (isalpha(c))
            {
                // token is either a keyword or ID or function call
                ungetc(c, stdin);
                ttype = scan_id_or_keyword();
            }
            else if (c == EOF)
                ttype = EOF;
            else
                ttype = ERROR;
            break;
    } // End Switch
    return ttype;
}

//---------------------------------------------------------
// Execute
ValueNode* accessValueNode(VarAccess* var){
	if(var->isArrayAccess){
		int index = evaluateExpr(var->expr);
		return var->varArray->table[index];
	}
	else{
		return var->var;
	}
}

int evaluateVar(VarAccess* var){
	if(var->isArrayAccess){
		int index = evaluateExpr(var->expr);
		if(index >= var->varArray->tableSize){
			// index out of bounds error
			debug("index out of bounds");
			exit(1);
		}
		return *var->varArray->table[index]->value;
	}
	else{
		return *var->var->value;
	}
}

int evaluateFactor(Factor* factor){
	switch(factor->type){
		case NUM:
			return factor->num;
			break;
		case VAR:
			return evaluateVar(factor->var);
			break;
		case 0:
			return evaluateExpr(factor->expr);
			break;
	}
}

int evaluateTerm(Term* term){
	switch(term->op){
		case 0:
			return  evaluateFactor(term->LHS);
			break;
		case MULT:
			return evaluateFactor(term->LHS) * evaluateTerm(term->RHS);
			break;
		case DIV:
			return evaluateFactor(term->LHS) / evaluateTerm(term->RHS);
			break;
		default:
			debug("Missing operand");
			exit(1);
			break;
	}
}

int evaluateExpr(Expr* expression){
	switch(expression->op){
		case 0:
			return evaluateTerm(expression->LHS);
			break;
		case PLUS:
			return evaluateTerm(expression->LHS) + evaluateExpr(expression->RHS);
			break;
		case MINUS:
			return evaluateTerm(expression->LHS) - evaluateExpr(expression->RHS);
			break;
	}
}

void execute_program(struct StatementNode* program)
{
    struct StatementNode* pc = program;
    int op1, op2, result;
	ValueNode* test = NULL;
	Stack* callStack = new Stack();

    while (pc != NULL)
    {
        switch (pc->type)
        {
			case FUNCTION_CALL:
				if(pc->function_call == NULL){
					debug("Error: pc points to a function call but pc->function_call is null.\n");
					exit(1);
				}
				if(pc->function_call->node == NULL){
					debug("Error: function call has a null function");
					exit(1);
				}
				for(int i = 0; i < pc->function_call->node->argumentSize; i++){
					// link up function variables now, we reassign the pointers to the new 
					pc->function_call->node->varTable[i]->name = pc->function_call->arguments[i]->name;
					pc->function_call->node->varTable[i]->value = pc->function_call->arguments[i]->value;
				}
				// link up the links for function
				pc->function_call->node->link->type = pc->function_call->link->type;
				switch (pc->function_call->link->type)
				{
				case FUNCTION_CALL:
					pc->function_call->node->link->function_call = pc->function_call->link->function_call;
					break;
				case NOOP_STMT:
					pc->function_call->node->link->next = NULL;
					break;
				case PRINT_STMT:
					pc->function_call->node->link->print_stmt = pc->function_call->link->print_stmt;
					break;
				case ASSIGN_STMT:
					pc->function_call->node->link->assign_stmt = pc->function_call->link->assign_stmt;
					break;
				case IF_STMT:
					pc->function_call->node->link->if_stmt = pc->function_call->link->if_stmt;
					break;
				case GOTO_STMT:
					pc->function_call->node->link->goto_stmt = pc->function_call->link->goto_stmt;
					break;
				default:
					break;
				}
				pc = pc->function_call->node->body;
				break;
            case NOOP_STMT:
				//debug("NOOP\n");
				if(pc->next != NULL){
					// this may be cheating... but this is the expected behavior
					pc->next = NULL;
				}
                pc = pc->next;
                break;
            case PRINT_STMT:
				//debug("PRINT\n");
                if (pc->print_stmt == NULL)
                {
                    debug("Error: pc points to a print statement but pc->print_stmt is null.\n");
                    exit(1);
                }
                if (pc->print_stmt->var == NULL)
                {
                    debug("Error: print_stmt-> is null.\n");
                    exit(1);
                }
				printf("%d\n", evaluateVar(pc->print_stmt->var));
                pc = pc->next;
                break;

            case ASSIGN_STMT:
				//debug("ASSIGN_STMT\n");
                if (pc->assign_stmt == NULL)
                {
                    debug("Error: pc points to an assignment statement but pc->assign_stmt is null.\n");
                    exit(1);
                }
				if (pc->assign_stmt->expr == NULL)
                {
                    debug("Error: assign_stmt->expr is null.\n");
                    exit(1);
                }
				if (pc->assign_stmt->var == NULL)
                {
					debug("Error: assign_stmt->var is null.\n");
					exit(1);
                }
				result = evaluateExpr(pc->assign_stmt->expr);
				test = accessValueNode(pc->assign_stmt->var);
				*accessValueNode(pc->assign_stmt->var)->value = result;
                pc = pc->next;
                break;

            case IF_STMT:
				//debug("IF\n");
                if (pc->if_stmt == NULL)
                {
                    debug("Error: pc points to an if statement but pc->if_stmt is null.\n");
                    exit(1);
                }
                if (pc->if_stmt->true_branch == NULL)
                {
                    debug("Error: if_stmt->true_branch is null.\n");
                    exit(1);
                }
                if (pc->if_stmt->false_branch == NULL)
                {
                    debug("Error: if_stmt->false_branch is null.\n");
                    exit(1);
                }
                if (pc->if_stmt->condition_operand1 == NULL)
                {
                    debug("Error: if_stmt->condition_operand1 is null.\n");
                    exit(1);
                }
                if (pc->if_stmt->condition_operand2 == NULL)
                {
                    debug("Error: if_stmt->condition_operand2 is null.\n");
                    exit(1);
                }
				op1 = evaluateExpr(pc->if_stmt->condition_operand1);
                op2 = evaluateExpr(pc->if_stmt->condition_operand2);
                switch (pc->if_stmt->condition_op)
                {
                    case GREATER:
                        if (op1 > op2)
                            pc = pc->if_stmt->true_branch;
                        else
                            pc = pc->if_stmt->false_branch;
                        break;
                    case LESS:
                        if (op1 < op2)
                            pc = pc->if_stmt->true_branch;
                        else
                            pc = pc->if_stmt->false_branch;
                        break;
                    case NOTEQUAL:
                        if (op1 != op2)
                            pc = pc->if_stmt->true_branch;
                        else
                            pc = pc->if_stmt->false_branch;
                        break;
                    default:
                        debug("Error: invalid value for if_stmt->condition_op (%d).\n", pc->if_stmt->condition_op);
                        exit(1);
                        break;
                }
                break;

            case GOTO_STMT:
				//debug("GOTO\n");
                if (pc->goto_stmt == NULL)
                {
                    debug("Error: pc points to a goto statement but pc->goto_stmt is null.\n");
                    exit(1);
                }
                if (pc->goto_stmt->target == NULL)
                {
                    debug("Error: goto_stmt->target is null.\n");
                    exit(1);
                }
                pc = pc->goto_stmt->target;
                break;

            default:
                debug("Error: invalid value for pc->type (%d).\n", pc->type);
                exit(1);
                break;
        }
    }
}

int main()
{
    struct StatementNode * program;
    program = parse_generate_intermediate_representation();
    execute_program(program);
    return 0;
}