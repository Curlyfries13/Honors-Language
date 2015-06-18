#include "IntermediateCreator.h"
#include "compiler.h"

class Frame{
public:
	Frame* next();
};

class Stack{
public:
	void Push(ValueNode** arguments, FunctionNode* function);
	Frame* Pop();
};