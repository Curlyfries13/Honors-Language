#include "IntermediateCreator.h"
#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <string>
#include <tuple>

#define DEFAULT_TABLE_SIZE 10
#define ARRAY_SZ_NAME "__ARRAY_SIZE__"


using namespace std;

ValueNode** varTable;
ArrayNode** arrayTable;
FunctionNode** functionTable;
char** arrayNames;
int functionTableSize = 0;
int functionTableEntries = 0;
int varTableSize = 0;
int varTableNames = 0;


void parse_id_list(){
	varTableSize = DEFAULT_TABLE_SIZE;
	varTableNames = 0;

	varTable = new ValueNode*[varTableSize];

	for( int i = 0; i < varTableSize; i++){
		// initialize to NULL pointers
		varTable[i] = NULL;
	}

	while(getToken() != SEMICOLON){
		// if we see a comma we skip to the next token
		if (ttype != COMMA){
			// we should be looking at an ID
			if(ttype == ID){
				//int test2 = nameHash(token);
				ValueNode* test = varTable[varTableNames];
				if (varTableNames >= varTableSize){
					varTable = expandVarTable(varTable, varTableSize);
					varTableSize = varTableSize*2;
				}
				varTable[varTableNames] = new ValueNode;
				varTable[varTableNames]->name = new char[MAX_TOKEN_LENGTH];
				//printf("HASHING\nname: %s\n hash: %d\n", token, varTableNames);
				strcpy(varTable[varTableNames]->name, token);
				varTable[varTableNames]->value = new int(0);
				varTableNames++;
			}
			else{
				// syntax error!!
			}
		}
	}
}

std::tuple<ValueNode**, int, int> parse_fuctnion_id_list(){
	int tableSize = DEFAULT_TABLE_SIZE;
	int aliases = 0;

	struct ValueNode** table = new ValueNode*[tableSize];

	for( int i = 0; i < tableSize; i++){
		// initialize to NULL pointers
		table[i] = NULL;
	}

	while(getToken() != RPAREN){
		// if we see a comma we skip to the next token
		if (ttype != COMMA){
			// we should be looking at an ID
			if(ttype == ID){
				//int test2 = nameHash(token);
				//ValueNode* test = varTable[aliases];
				if (table[aliases] >= NULL){
					table = expandVarTable(table, tableSize);
					tableSize = tableSize*2;
				}
				table[aliases] = new ValueNode;
				table[aliases]->name = new char[MAX_TOKEN_LENGTH];
				//printf("HASHING\nname: %s\n hash: %d\n", token, aliases);
				strcpy(table[aliases]->name, token);
				table[aliases]->value = 0;
				aliases++;
			}
			else{
				// syntax error!!
			}
		}
	}
	tuple<ValueNode**, int, int> returnTuple (table, tableSize, aliases);
	return	returnTuple;
}

void expandVarTable(){
	ValueNode** updatedTable = new ValueNode*[(varTableSize*2)];
	size_t hash;
	for (int i = 0; i < varTableSize*2; i++){
		updatedTable[i] = NULL;
	}
	for (int i = 0; i < varTableSize; i++){
		// rehash
		if(varTable[i] != NULL){
			updatedTable[i] = varTable[i];
		}
	}
	delete [] varTable;
	varTable = updatedTable;
	varTableSize = varTableSize*2;
}

ValueNode** expandVarTable(ValueNode** table, int tableSize){
	ValueNode** newTable = new ValueNode*[tableSize*2];
	size_t hash;
	for(int i = 0; i < tableSize*2; i++){
		newTable[i] = NULL;
	}
	for(int i = 0; i < varTableSize; i++){
		newTable[i] = table[i];
	}
	delete [] table;
	return newTable;
}

void expandFunctionTable(){
	FunctionNode** newTable = new FunctionNode*[functionTableSize*2];
	for (int i = 0; i < functionTableSize; i++){
		newTable[i]->argumentSize = functionTable[i]->argumentSize;
		newTable[i]->body = functionTable[i]->body;
		newTable[i]->link = functionTable[i]->link;
		strcpy(newTable[i]->name, functionTable[i]->name);
		newTable[i]->varTable = functionTable[i]->varTable;
	}
	functionTableSize = functionTableSize*2;
}

struct ValueNode* getValue(char* name, ValueNode** table, int tableSize){
	//printf("gettingVAL\n");
	for (int i = 0; i < tableSize; i ++){
		if(strcmp(table[i]->name, name) == 0){
			return table[i];
		}
	}
}

struct FunctionNode* find_function(char* name){
	for(int i = 0; i < functionTableEntries; i++){
		if (strcmp(functionTable[i]->name, name) == 0){
			return functionTable[i];
		}
	}
}
//struct ArrayNode* getArray(char* name){
//	for(int i = 0; i < arrayTableNames; i++){
//		if(strcmp(name, arrayNames[i]) == 0){
//			return arrayTable[i];
//		}
//	}
//	return NULL;
//}

struct StatementNode* parse_generate_intermediate_representation(){
	//printf("entry\nhash number: %d\n", hashNumber);
	StatementNode* program = new StatementNode;
	StatementNode* exitStmt = new StatementNode;
	functionTable = new FunctionNode*[DEFAULT_TABLE_SIZE];
	functionTableSize = DEFAULT_TABLE_SIZE;
	exitStmt->type = NOOP_STMT;
	//printf("parse ID's\n");
	parse_id_list();
	getToken();
	while(ttype == FUNCTION){
		parse_function_decl(token);
		getToken();
	}
	//printf("Status: tableSize = %d\n", varTableSize);
	//printf("parse body\n");
	program->type = FUNCTION_CALL;
	program->function_call = new FunctionStatement;
	program->function_call->arguments = varTable;
	program->function_call->node = find_function("main");
	program->function_call->link = exitStmt;
	exitStmt->next = NULL;
	//printf("exitStmt top: next loc: %d\n", exitStmt->next);
	program = trimLeaves(program, exitStmt);
	//printf("exitStmt after: next loc: %d\n", exitStmt->next);
	exitStmt->next = NULL;

	return program;
}

void parse_function_decl(char* name){
	FunctionNode* node = new FunctionNode;
	node->name = new char[MAX_TOKEN_LENGTH];
	strcpy(node->name, name);
	if(strcmp(name, "main") == 0){
		node->varTable = varTable;
		node->argumentSize = varTableNames;
		node->link = new StatementNode;
		node->link->type = NOOP_STMT;
		node->link->next = NULL;
		parse_fuctnion_id_list();
	}
	else{
		tuple<ValueNode**, int, int> holder = parse_fuctnion_id_list();
		node->varTable = get<0>(holder);
		node->argumentSize = get<2>(holder);
		node->link = new StatementNode;
		node->link->type = GOTO_STMT;
		node->link->goto_stmt = new GotoStatement;
	}
	node->body = new StatementNode;
	parse_body(node->body, node->link, node->varTable, node->argumentSize);
	functionTable[functionTableEntries] = node;
	functionTableEntries++;
	if(functionTableEntries >= functionTableSize){
		expandFunctionTable();
	}
}

void parse_body(StatementNode* node, StatementNode* outNext, ValueNode** table, int tableSize){
	if(getToken() == LBRACE){
		parse_stmtList(node, outNext, table, tableSize);
		if(getToken() != RBRACE){
			// syntax error
			exit(1);
		}
	}
	else{
		// syntax error
		exit(1);
	}
}

void parse_stmtList(StatementNode* node, StatementNode* outNext, ValueNode** table, int tableSize){
	StatementNode* link;
	getToken();
	while(ttype != RBRACE){	
		switch(ttype){
			case ID:
				//printf("ID\n");
				ungetToken();
				node->type = ASSIGN_STMT;
				node->assign_stmt = new AssignmentStatement;
				parse_assignStmt(node, table, tableSize);
				getToken();
				if(ttype != RBRACE){
					node->next = new StatementNode;
					//printf("make new node: %d\n", node->next);
					node = node->next;
				}
				else{
					node->next = outNext;
					node = node->next;
					ungetToken();
				}
				break;

			case FUNCTION:
				node->type = FUNCTION_CALL;
				node->function_call = new FunctionStatement;
				link = new StatementNode;
				link->type = GOTO_STMT;
				link->goto_stmt = new GotoStatement;
				link->goto_stmt->target = NULL;
				parse_function_call(token, node, link, table, tableSize);

				getToken();
				if(ttype != RBRACE){
					// this block isn't done yet
					link->goto_stmt->target = new StatementNode;
					node = link->goto_stmt->target;
				}
				else{
					// block is over, link to the outside
					link->goto_stmt->target = outNext;
					node->next = outNext;
					node = node->next;
					ungetToken();
				}
				break;

			case PRINT:
				//printf("PRINT\n");
				node->type = PRINT_STMT;
				parse_printStmt(node, table, tableSize);
				getToken();
				if(ttype != RBRACE){
					node->next = new StatementNode;
					node = node->next;
				}
				else{
					node->next = outNext;
					node = node->next;
					ungetToken();
				}	
				break;

			case WHILE:
				//printf("WHILE\n");
				link = new StatementNode;
				link->type = GOTO_STMT;
				link->goto_stmt = new GotoStatement;
				link->goto_stmt->target = NULL;
				node->type = IF_STMT;
				node->next = NULL;
				// get the while stmt incl the body
				parse_whileStmt(node, link, table, tableSize);

				getToken();
				if(ttype != RBRACE){
					// this block isn't done yet
					link->goto_stmt->target = new StatementNode;
					node = link->goto_stmt->target;
				}
				else{
					// block is over, link to the outside
					link->goto_stmt->target = outNext;
					node->next = outNext;
					node = node->next;
					ungetToken();
				}
				
				break;

			case IF:
				//printf("IF\n");
				node->type = IF_STMT;
				node->next = NULL;
				// link goes around the body to the outside body if needed
				link = new StatementNode;
				link->type = GOTO_STMT;
				link->goto_stmt = new GotoStatement;
				
				parse_ifStmt(node, link, table, tableSize);
				getToken();
				if(ttype != RBRACE){
					// the if statement is not the end of this body; we link the next
					// node up to the next statement which we will encounter
					
					link->goto_stmt->target = new StatementNode;
					node->if_stmt->false_branch = link;
					node = link->goto_stmt->target;
				}
				else{
					// we've gotten to the end of a body
					// here we need to link up the outer body's next statement to the
					// false statement so that we skip out to the right place.
					link->goto_stmt->target = outNext;
					ungetToken();
				}
				break;

			case SWITCH:
				node->type = IF_STMT;
				link = new StatementNode;
				link->type = GOTO_STMT;
				link->goto_stmt = new GotoStatement;
				parse_switchStmt(node, link, table, tableSize);
				getToken();
				if(ttype != RBRACE){
					link->goto_stmt->target = new StatementNode;
					node = link->goto_stmt->target;
				}
				else{
					link->goto_stmt->target = outNext;
					ungetToken();
				}
				break;

			case -1:
				// error here
				return;

			default:
				getToken();
				break;
		}
		
	}
	
}

void parse_assignStmt(StatementNode* node, ValueNode** table, int tableSize){
	//printf("in node %d\n", node);
	node->assign_stmt = new AssignmentStatement;
	if(getToken() == ID){
		char* name = token;
		VarAccess* varDest = new VarAccess;
		Expr* rhs = NULL;
		int op;
		ungetToken();
		varDest = parse_varAccess(varDest, table, tableSize);
		getToken();
		if(ttype == EQUAL){
			rhs = new Expr;
			rhs = parse_expr(rhs, table, tableSize);
			node->assign_stmt->var = varDest;
			node->assign_stmt->expr = rhs;
		}
		else{
			// syntax error
			exit(1);
		}
		if (getToken() != SEMICOLON){
			// syntax
			exit(1);
		}
	}
}

void parse_printStmt(StatementNode* node, ValueNode** table, int tableSize){
	node->print_stmt = new PrintStatement;
	node->print_stmt->var = new VarAccess;
	node->print_stmt->var = parse_varAccess(node->print_stmt->var, table, tableSize);
	if (getToken() != SEMICOLON){
		// syntax error
		exit(1);
	}
}

void parse_whileStmt(StatementNode* node, StatementNode* link, ValueNode** table, int tableSize){
	Expr* op1;
	Expr* op2;
	int op;
	node->if_stmt = new IfStatement;
	StatementNode* body;
	StatementNode* backLink;
	
	op1 = new Expr;
	op1 = parse_expr(op1, table, tableSize);
	getToken();
	if(ttype == GREATER || ttype == LESS || ttype == NOTEQUAL){
		op = ttype;
	}
	else{
		// syntax error
		exit(1);
	}
	op2 = new Expr;
	op2 = parse_expr(op2, table, tableSize);

	
	body = new StatementNode;
	backLink = new StatementNode;
	backLink->type = GOTO_STMT;
	backLink->goto_stmt = new GotoStatement;
	backLink->goto_stmt->target = node;
	// we link the body up to this node to get while functionality
	parse_body(body, backLink, table, tableSize);
	
	node->if_stmt->condition_op = op;
	node->if_stmt->condition_operand1 = op1;
	node->if_stmt->condition_operand2 = op2;

	node->if_stmt->true_branch = body;
	node->if_stmt->false_branch = link;
	

	//ValueNode* var1;
	//ValueNode* var2;
	//int op;
	//node->if_stmt = new IfStatement;
	//getToken();
	//switch(ttype){
	//	case ID:
	//		var1 = getValue(token);
	//		break;
	//	case NUM:
	//		var1 = new ValueNode;
	//		var1->name = "\0";
	//		var1->value = atoi(token);
	//		break;
	//	default:
	//		// syntax Error
	//		exit(1);
	//}
	//getToken();
	//switch(ttype){
	//	case GREATER:
	//		op = GREATER;
	//		break;
	//	case LESS:
	//		op = LESS;
	//		break;
	//	case NOTEQUAL:
	//		op = NOTEQUAL;
	//		break;
	//	default:
	//		// syntax error
	//		exit(1);
	//}
	//getToken();
	//switch(ttype){
	//	case ID:
	//		var2 = getValue(token);
	//		break;
	//	case NUM:
	//		var2 = new ValueNode;
	//		var2->name = "\0";
	//		var2->value = atoi(token);
	//		break;
	//	default:
	//		// syntax Error
	//		exit(1);
	//}
	//node->if_stmt->condition_operand1 = var1;
	//node->if_stmt->condition_operand2 = var2;
	//node->if_stmt->condition_op = op;
	//StatementNode* body = new StatementNode;
	//// the false branch links up to the outside here
	//node->if_stmt->false_branch = link;
	//StatementNode* backLink = new StatementNode;
	//backLink->type = GOTO_STMT;
	//backLink->goto_stmt = new GotoStatement;
	//backLink->goto_stmt->target = node;
	//// we link any inner bodies to this comparison node for while functionality
	//parse_body(body, backLink);
	//node->if_stmt->true_branch = body;
}

void parse_ifStmt(StatementNode* node, StatementNode* link, ValueNode** table, int tableSize){
	Expr* op1;
	Expr* op2;
	int op;
	node->if_stmt = new IfStatement;
	node->if_stmt->false_branch = link;

	op1 = new Expr;
	op1 = parse_expr(op1, table, tableSize);
	getToken();

	if(ttype == GREATER || ttype == LESS || ttype == NOTEQUAL){
		op = ttype;
	}
	op2 = new Expr;
	op2 = parse_expr(op2, table, tableSize);

	StatementNode* body = new StatementNode;
	parse_body(body, link, table, tableSize);
	node->if_stmt->true_branch = body; 
	node->if_stmt->condition_operand1 = op1;
	node->if_stmt->condition_operand2 = op2;
	node->if_stmt->condition_op = op;
	//ValueNode* var1;
	//ValueNode* var2;
	//int op;
	//node->if_stmt = new IfStatement;
	//getToken();
	//// discern the first op
	//switch(ttype){
	//	case ID:
	//		var1 = getValue(token);
	//		break;
	//	case NUM:
	//		var1 = new ValueNode;
	//		var1->name = "\0";
	//		var1->value = atoi(token);
	//		break;
	//	default:
	//		// syntax Error
	//		exit(1);
	//}
	//getToken();
	//// discern the operation type
	//switch(ttype){
	//	case GREATER:
	//		op = GREATER;
	//		break;
	//	case LESS:
	//		op = LESS;
	//		break;
	//	case NOTEQUAL:
	//		op = NOTEQUAL;
	//		break;
	//	default:
	//		// syntax error
	//		exit(1);
	//}
	//getToken();
	//// discern the second op
	//switch(ttype){
	//	case ID:
	//		var2 = getValue(token);
	//		break;
	//	case NUM:
	//		var2 = new ValueNode;
	//		var2->name = "\0";
	//		var2->value = atoi(token);
	//		break;
	//	default:
	//		// syntax Error
	//		exit(1);
	//}
	//node->if_stmt->condition_operand1 = var1;
	//node->if_stmt->condition_operand2 = var2;
	//node->if_stmt->condition_op = op;
	//node->if_stmt->false_branch = link;
	//StatementNode* body = new StatementNode;
	//parse_body(body, link);
	//node->if_stmt->true_branch = body;
}

void parse_switchStmt(StatementNode* node, StatementNode* link, ValueNode** table, int tableSize){
	VarAccess* var = new VarAccess;
	var = parse_varAccess(var, table, tableSize);
	if(getToken() == LBRACE){
		parse_caseList(node, link, var, table, tableSize);
	}
	else{
		// syntax error
		exit(1);
	}
}

void parse_caseList(StatementNode* node, StatementNode* link, VarAccess* switchValue, ValueNode** table, int tableSize){
	int caseCompare = 0;
	StatementNode* next;
	StatementNode* caseBody;
	while(ttype != RBRACE){
		getToken();
		if(ttype == CASE){
			node->type = IF_STMT;
			node->if_stmt = new IfStatement;
			node->if_stmt->condition_op = NOTEQUAL;
			node->if_stmt->condition_operand1 = new Expr;
			node->if_stmt->condition_operand1->op = 0;
			node->if_stmt->condition_operand1->LHS = new Term;
			node->if_stmt->condition_operand1->LHS->op = 0;
			node->if_stmt->condition_operand1->LHS->LHS = new Factor;
			node->if_stmt->condition_operand1->LHS->LHS->type = VAR;
			node->if_stmt->condition_operand1->LHS->LHS->var = switchValue;
			getToken();
			if(ttype == NUM){
				caseCompare = atoi(token);
				node->if_stmt->condition_operand2 = new Expr;
				node->if_stmt->condition_operand2->op = 0;
				node->if_stmt->condition_operand2->LHS = new Term;
				node->if_stmt->condition_operand2->LHS->op = 0;
				node->if_stmt->condition_operand2->LHS->LHS = new Factor;
				node->if_stmt->condition_operand2->LHS->LHS->type = NUM;
				node->if_stmt->condition_operand2->LHS->LHS->num = caseCompare;
				getToken();
				if(ttype == COLON){
					caseBody = new StatementNode;
					parse_body(caseBody, link, table, tableSize);
					node->if_stmt->false_branch = caseBody;
					//now we figure out what we do next
					getToken();
					next = new StatementNode;
					if(ttype == CASE){
						node->if_stmt->true_branch = next;
						node = next;
						ungetToken();
					}
					else if(ttype == DEFAULT){
						node->if_stmt->true_branch = next;
						node = next;
						ungetToken();
					}
					else if(ttype == RBRAC){
						next->type = GOTO_STMT;
						next->goto_stmt = new GotoStatement;
						next->goto_stmt->target = link;
						node->if_stmt->true_branch = next;
					}
				}
				else{
					//syntax error, missing colon
					exit(1);
				}
			}
			else{
				//syntax error
				exit(1);
			}
		}
		else if(ttype == DEFAULT){
			if(getToken() == COLON){
				parse_body(node, link, table, tableSize);
				if(getToken() == RBRACE){
					return;
				}
			}
			else{
				// syntax error
				exit(1);
			}
		}
	}
	ungetToken();
	//int caseVar = 0;
	//ValueNode* operand = NULL;
	//StatementNode* caseBody = NULL;
	//StatementNode* next = NULL;

	//while (ttype != RBRACE)
	//	getToken();
	//	if (ttype == CASE){
	//		if(getToken() == NUM){
	//			caseVar = atoi(token);
	//			operand = new ValueNode;
	//			operand->name = "/0";
	//			operand->value = caseVar;
	//			if (getToken() == COLON){
	//				node->type = IF_STMT;
	//				node->if_stmt = new IfStatement;
	//				node->if_stmt->condition_op = NOTEQUAL;
	//				node->if_stmt->condition_operand1 = switchValue;
	//				node->if_stmt->condition_operand2 = operand;
	//				caseBody = new StatementNode;
	//				next = new StatementNode;
	//				parse_body(caseBody, link);
	//				// since we are using NOTEQUAL the false branch is executed when the statements match
	//				node->if_stmt->false_branch = caseBody;
	//				// check the next bit to see what we need to do
	//				getToken();
	//				if (ttype == CASE){
	//					node->if_stmt->true_branch = next;
	//					//node->next = next;
	//					node = next;
	//					ungetToken();
	//				}
	//				else if(ttype == DEFAULT){
	//					node->if_stmt->true_branch = next;
	//					//node->next = next;
	//					node = next;
	//					ungetToken();
	//				}
	//				else if(ttype == RBRACE){
	//					// end of the list link up to the outside
	//					next->type = GOTO_STMT;
	//					next->goto_stmt = new GotoStatement;
	//					next->goto_stmt->target = link;
	//					node->if_stmt->true_branch = link;
	//					ungetToken();
	//				}
	//				else{
	//					// syntax error!
	//					exit(1);
	//				}
	//			}
	//			else{
	//				exit(1);
	//			}
	//		}
	//		else{
	//			exit(1);
	//		}
	//	}
	//	else  if (ttype == DEFAULT){
	//		if (getToken() == COLON){
	//			// we link this up to the outside because we expect this to be the end
	//			// of the switch statement (we'll check if it isn't later)
	//			parse_body(node, link);
	//			getToken();
	//			if(ttype != RBRACE){
	//				exit(1);
	//			} 
	//			ungetToken();
	//		}
	//	}
	//}
}

void parse_function_call(char* name, StatementNode* node, StatementNode* link, ValueNode** varTable, int tableSize){
	node->function_call->node = find_function(name);
	node->function_call->link = link;
	std::tuple<ValueNode**, int> parsedArgs = parse_function_call_arguments(varTable, tableSize);
	if(node->function_call->node->argumentSize != get<1>(parsedArgs)){
		// error! args don't match
		exit(2);
	}
	node->function_call->arguments = get<0>(parsedArgs);
	if (getToken() != SEMICOLON){
		//syntax error
		exit(1);
	}
}

std::tuple<ValueNode**, int> parse_function_call_arguments(ValueNode** table, int tableSize){
	ValueNode** argTable = new ValueNode*[DEFAULT_TABLE_SIZE];
	int argNumber = 0;
	int argTableSize = DEFAULT_TABLE_SIZE;
	if(getToken() == LPAREN){
		getToken();
		while(ttype != RPAREN){
			
			if(ttype == ID){
				if(argNumber >= tableSize){
					argTable = expandVarTable(argTable, argTableSize);
					argTableSize = argTableSize*2;
				}
				argTable[argNumber] = getValue(token, table, tableSize);
				argNumber++;
			}
			else if(ttype != COMMA){
				// syntax error
				exit(1);
			}
			getToken();
		}
	}
	else{
		//syntax error
		exit(1);
	}
	tuple<ValueNode**, int> returnTuple (argTable, argNumber);
	return returnTuple;
}

struct VarAccess* parse_varAccess(VarAccess* var, ValueNode** table, int tableSize){
	getToken();
	if (ttype == ID){
		char* name = new char[MAX_TOKEN_LENGTH];
		strcpy(name, token);
		getToken();
		if (ttype == LBRAC){
			//// array access
			//var->isArrayAccess = true;
			//var->varArray = new ArrayNode;
			//var->varArray = getArray(name);
			//var->expr = new Expr;
			//var->expr = parse_expr(var->expr);
			//getToken();
			//if(ttype == RBRAC){
			//	return var;
			//}
			//else{
			//	// syntax error
			//	exit(1);
			//}
		}
		else{
			// not an array access
			ungetToken();
			var->isArrayAccess = false;
			var->var = getValue(name, table, tableSize);
			return var;
		}
	}
}

struct Expr* parse_expr(Expr* node, ValueNode** table, int tableSize){
	node->LHS = new Term;
	node->LHS = parse_term(node->LHS, table, tableSize);
	getToken();
	if(ttype == PLUS || ttype == MINUS){
		node->op = ttype;
		node->RHS = new Expr;
		node->RHS = parse_expr(node->RHS, table, tableSize);
	}
	else{
		node->RHS = NULL;
		node->op = 0;
		ungetToken();
	}
	return node;
}

struct Term* parse_term(Term* term, ValueNode** table, int tableSize){
	term->LHS = new Factor;
	term->LHS = parse_factor(term->LHS, table, tableSize);
	getToken();
	if(ttype == MULT || ttype == DIV){
		term->op = ttype;
		term->RHS = new Term;
		term->RHS = parse_term(term->RHS, table, tableSize);
	}
	else{
		term->op = 0;
		term->RHS = NULL;
		ungetToken();
	}
	return term;
}

struct Factor* parse_factor(Factor* fac, ValueNode** table, int tableSize){
	getToken();
	if (ttype == LPAREN){
		fac->type = 0;
		fac->expr = new Expr;
		fac->expr = parse_expr(fac->expr, table, tableSize);
		getToken();
		if(ttype != RPAREN){
			// syntax error
			exit(1);
		}
	}
	else if(ttype == NUM){
		fac->type = NUM;
		fac->num = atoi(token);
	}
	else if(ttype == ID){
		fac->type = VAR;
		fac->var = new VarAccess;
		ungetToken();
		fac->var = parse_varAccess(fac->var, table, tableSize);
	}
	return fac;
}

// trim should be a noop statement
// gets rid of dangling GOTO statements and points them to the end
struct StatementNode* trimLeaves(StatementNode* node, StatementNode* trim){
	if(node != NULL){
		switch(node->type){
		case GOTO_STMT:
			if (node->goto_stmt != NULL && node->goto_stmt->target == NULL){
				node->goto_stmt->target = trim;
			}
			break;

		case IF_STMT:
			if (node->if_stmt != NULL){
				node->if_stmt->true_branch = trimLeaves(node->if_stmt->true_branch, trim);
				node->if_stmt->false_branch = trimLeaves(node->if_stmt->false_branch, trim);
			}
			else{
				exit(1);
			}
			break;

		case NULL:
			// found a null pointer; trim it up
			return trim;
			break;
		case NOOP_STMT:
			if(node != trim){
				node->next = trim;
			}

			break;
		case ASSIGN_STMT:
		case PRINT_STMT:
		
			// ASSIGN PRINT NOOP or SWITCH
			if(node->next == NULL){
				if(node == trim){
					// reached the end of a chain
					break;
				}
				node->next = trim;
			}
			else{
				node->next = trimLeaves(node->next, trim);
			}
			break;
		case FUNCTION_CALL:
			node->next = trimLeaves(node->function_call->node->body, node->function_call->link);
			if(node->function_call->link == NULL){
				node->function_call->link = trim;
			}
			break;
		default:
			node->type = NOOP_STMT;
			break;
		}
	}
	return node;
}
