#include "compiler.h"
#include <tuple>

void expandVarTable();
struct ValueNode** expandVarTable(ValueNode** table, int tableSize);
void expandFunctionTable();
void parse_id_list();
void parse_function_decl(char* name);
std::tuple<ValueNode**, int, int> parse_fuctnion_id_list();
//struct StatementNode* parse_generate_intermediate_representation();
void parse_body(StatementNode* node, StatementNode* outNext, ValueNode** table, int tableSize);
void parse_printStmt(StatementNode* node, ValueNode** table, int tableSize);
void parse_whileStmt(StatementNode* node, StatementNode* link, ValueNode** table, int tableSize);
void parse_ifStmt(StatementNode* node, StatementNode* link, ValueNode** table, int tableSize);
void parse_switchStmt(StatementNode* node, StatementNode* link, ValueNode** table, int tableSize);
void parse_stmtList(StatementNode* node, StatementNode* outNext, ValueNode** table, int tableSize);
void parse_assignStmt(StatementNode* node, ValueNode** table, int tableSize);
void parse_caseList(StatementNode* node, StatementNode* link, VarAccess* switchValue, ValueNode** table, int tableSize);
void parse_function_call(char* name, StatementNode* node, StatementNode* link, ValueNode** table, int tableSize);
std::tuple<ValueNode**, int> parse_function_call_arguments(ValueNode** table, int tableSize);
struct VarAccess* parse_varAccess(VarAccess* var, ValueNode** table, int tableSize);
struct Expr* parse_expr(Expr* node, ValueNode** table, int tableSize);
struct ValueNode* getValue(char* name, ValueNode** table, int tableSize);
struct Term* parse_term(Term* term, ValueNode** table, int tableSize);
struct Factor* parse_factor(Factor* fac, ValueNode** table, int tableSize);
struct FunctionNode* find_function(char* name);
//int nameHash(char* name);
StatementNode* trimLeaves(StatementNode* node, StatementNode* trim);