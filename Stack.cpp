#include "Stack.h"

class Frame{
	FunctionNode* m_function;
	ValueNode** m_arguments;
	ValueNode** m_localVariables;
	Frame* m_next;

public:
	Frame(FunctionNode* function, ValueNode** arguments, Frame* next){
		m_function = function;
		m_arguments = arguments;
		m_next = next;
	}
	
	Frame* next(){
		return m_next;
	}

	ValueNode** getLocalVars(){
		return m_localVariables;
	}
};

class Stack{
	Frame* m_currentFrame;
	int m_stackHeight;

public:

	Stack(){
		m_currentFrame = NULL;
		m_stackHeight = 0;
	}

	Frame* Pop(){
		 Frame* out = m_currentFrame;
		 m_currentFrame = m_currentFrame == NULL ? NULL : m_currentFrame->next() ;
		 return out;
		 m_stackHeight--;
	}
	
	void Push(ValueNode** arguments, FunctionNode* function){
		m_currentFrame = new Frame(function, arguments, m_currentFrame);
		m_stackHeight++;
	}
};