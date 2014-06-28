// David Karhi
//
//   This is the header file for the ParseNode class. This class implements
//   a node that is used in a parse tree as part of a compiler. 

#ifndef PARSENODE_H
#define PARSENODE_H
#include "token.h"

typedef enum {zero,v0,v1,a0,a1,a2,a3,t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,
              s0,s1,s2,s3,s4,s5,s6,s7,sp,fp,ra} Reg;

typedef enum {DeclKind, StmtKind, ExpKind} NodeKind;
typedef enum {FuncDecl, VarDecl, ParamDecl } DeclNode;
typedef enum {IfStmt, WhileStmt, ReturnStmt, ExpStmt, CmpStmt, FuncStmt,EndFunc} StmtNode;
typedef enum {NumExp, VarExp, AssignExp, OpExp, CallExp,RelExp} ExpNode;
typedef enum {Void, Integer, Array} Type;

#define MAXCHILDREN 3
#define MAXPARAMS 8

class ParseNode
{
   private:
      NodeKind nodekind;
      DeclNode decl;
      StmtNode stmt;
      ExpNode exp;
      Type type;
      TokenType tt;
      char cString[STRINGSIZE];
      int paramNumber;
      int numVal;
      int lineNo;
      int linePos;
      int memory;
      unsigned int scope;
      bool leaf;
      ParseNode *children[MAXCHILDREN];
      ParseNode *sibling;
      Reg reg;
   public:
      // constructor for a generic ParseNode
      ParseNode(NodeKind nk, Type tp, int ln, int pos, unsigned int scopeParam );
      // constructor for a declaration node
      ParseNode(NodeKind nk, DeclNode dn, Type tp, char *string, int ln, int pos, unsigned int scopeParam);
      // constructor for a statement node
      ParseNode(StmtNode st,int ln, int pos, unsigned int scopeParam);
      // constructor for an expression node
      ParseNode(ExpNode expn,Token tk, unsigned int scopeParam);	
      // NUM constructor 
      ParseNode (int val);
      // display a node
      void displayNode(void);
      // set bool leaf to isLeaf
      void setLeaf(bool isLeaf);
      // returns leaf
      bool getLeaf(){return leaf;}
      // sets the cString value
      void setString(char *string){strcpy(cString,string);}
      // sets the DeclType
      void setDeclType(DeclNode dn){decl = dn;}
      // set the Type
      void setType(Type tp){type = tp;}
      // set a child
      void setChild(int idx, ParseNode* node){children[idx] = node;}
      // set a sibling
      void setSibling(ParseNode *node){sibling = node;}
      // returns child
      ParseNode* getChild(int idx){return children[idx];}
      // returns sibling
      ParseNode* getSibling(void){return sibling;}
      // function returns DeclType
      DeclNode getDeclType(void){return decl;}
      // returns node type
      Type getType(void){return type;}
      // returns stmt
      StmtNode getStmt(void) {return stmt;}
      // function returns node kind
      NodeKind getNodeKind(void){return nodekind;}
      // function sets scope
      void setScope(unsigned int s) {scope = s;}
      // function returns scope
      unsigned int getScope(void){return scope;}
      // returns true if this node is a +,-,/,or * operator
      bool isMathOperator(void);
      // returns the number value of the string
      int numberValue(void);
      // returns true if this node is a NUM
      bool isNum(void);
      // returns numVal
      int getNum(void) {return numVal;}
      // returns tt
      TokenType getTokenType(void){return tt;}
      // sets exp
      void setExp(ExpNode ex){exp=ex;}
      // gets exp
      ExpNode getExp(void){return exp;}
      // sets numVal
      void setNum(int number){numVal=number;}
      // returns cString
      char *getString(){return cString;}
      // returns line number
      int getLineNo(void) { return lineNo;}
      // retuns line position
      int getPosition(void) {return linePos;}
      // returns true if this is a variable or a call
      bool isVar(void);
      // returns true if this node is the beginning of a function
      bool isFuncBegin(void);
      // returns true if this node is the end of a function
      bool isFuncEnd(void);
      // returns true if this noe is an assignment node
      bool isAssignment(void);
      // returns true if this node is a declaration
      bool isDecl(void);
      // returns true if this node is the declaration of main
      bool isMainDecl(void);
      // true if this is a variable declaration
      bool isVarDecl(void);
      // returns true if this node is a relational operator
      bool isRelOp(void);
      // returns true if this is a function call
      bool isCall(void);
      // this function returns true if this node is a return statement
      bool isReturnStmt(void);
      // this function sets the memory location
      void setMem(int mem) {memory = mem;}
      // this function gets the memory location 
      int getMem(void) {return memory;}
      // this functin sets the register used
      void setReg(Reg r) {reg = r;}
      // this function gets the register used
      Reg getReg(void) {return reg;}
      // this function sets the parameter number. if the parameter
      // number is set to MAXPARAMS then it is not a parameter
      void setParamNum(int p) {paramNumber = p;}
      // this function returns the parameter number.
      int getParamNum(void) {return paramNumber;}
      // these should be public so other parts of the program can 
      // use them for output.
      const static char NODE_TYPE[3][STRINGSIZE];
      const static char DECL_TYPE[3][STRINGSIZE];
      const static char STMT_TYPE[7][STRINGSIZE];
      const static char EXPR_TYPE[6][STRINGSIZE];
      const static char TYPE_TYPE[3][STRINGSIZE];
};

// these are initialized out here because they are static
const char ParseNode::NODE_TYPE[][STRINGSIZE] = {"Declaration","Statement","Expression"};
const char ParseNode::DECL_TYPE[][STRINGSIZE] = {"Function", "Variable", "Parameter"};
const char ParseNode::STMT_TYPE[][STRINGSIZE] = {"If","While", "Return", "Expression","Compound", "Function", "Function End"};
const char ParseNode::EXPR_TYPE[][STRINGSIZE] = {"Number","Variable", "Assignment","Operator","Call", "Operator"};
const char ParseNode::TYPE_TYPE[][STRINGSIZE] = {"Void","Integer","Address"};

ParseNode::ParseNode(NodeKind nk, Type tp, int ln, int pos, unsigned int scopeParam)
{
   nodekind = nk;
   type = tp;
   lineNo = ln;
   linePos = pos;
   cString[0] = '\0';
   for (int i=0; i<MAXCHILDREN; ++i)
      children[i] = NULL;
   sibling = NULL;
   scope = scopeParam;
   paramNumber = MAXPARAMS;
}

ParseNode::ParseNode(StmtNode st, int ln, int pos, unsigned int scopeParam)
{
   nodekind=StmtKind;
   stmt = st;
   lineNo = ln;
   linePos = pos;
   cString[0] = '\0';
   for (int i=0; i<MAXCHILDREN; ++i)
      children[i] = NULL;
   sibling = NULL;
   scope = scopeParam;
   paramNumber = MAXPARAMS;

}

ParseNode::ParseNode(ExpNode expn, Token tk, unsigned int scopeParam)
{
   nodekind=ExpKind;
   exp = expn;
   lineNo = tk.getLineNo();
   linePos = tk.getPosition();
   strcpy(cString, tk.getString());
   tt = tk.getType();
   for (int i=0; i<MAXCHILDREN; ++i)
      children[i] = NULL;
   sibling = NULL;
   scope = scopeParam;
   if (isdigit(cString[0]))
   {
      type = Integer;
      numVal = numberValue();
   }
   paramNumber = MAXPARAMS;
}

ParseNode::ParseNode(NodeKind nk, DeclNode dn, Type tp, char *string, int ln, int pos, unsigned int scopeParam)
{
   nodekind = nk;
   decl = dn;
   strcpy(cString, string);
   type = tp;
   for (int i=0; i<MAXCHILDREN; ++i)
      children[i] = NULL;
   sibling = NULL;
   scope = scopeParam;
   lineNo = ln;
   linePos = pos;
   paramNumber = MAXPARAMS;

}

ParseNode::ParseNode(int val)
{
   nodekind = ExpKind;
   exp = NumExp;
   for (int i=0; i<MAXCHILDREN; ++i)
      children[i] = NULL;
   sibling = NULL; 
   numVal = val;
   tt = NUM;
   type = Integer;
   paramNumber = MAXPARAMS;
}

void ParseNode::displayNode(void)
{
   if (nodekind==DeclKind)
   {
      cout << NODE_TYPE[nodekind] << " ";
      cout << cString << " ";
      cout << DECL_TYPE[decl] << " ";
      cout << TYPE_TYPE[type] << endl;
   }
   else if (nodekind==StmtKind)
   {
      if (stmt == FuncStmt)
         cout << STMT_TYPE[stmt] << " " << NODE_TYPE[nodekind] << endl;
      else
         cout << NODE_TYPE[nodekind] << " " << STMT_TYPE[stmt] << endl; 
   }
   else if (nodekind==ExpKind)
   {
      cout << NODE_TYPE[nodekind] << " ";
      if (exp == AssignExp) 
         cout << EXPR_TYPE[exp] << endl;
      else if (exp == NumExp || exp == VarExp || exp == CallExp)
      {
         cout << EXPR_TYPE[exp] << " ";
         if (exp == NumExp)
            cout << numVal << endl;
         else
            cout << cString << endl;
      }
      else if (exp == OpExp || exp == RelExp)
      {
         cout << EXPR_TYPE[exp] << " ";
         cout << Token::TYPE_STRINGS[tt]; 
         cout << endl;
      }
   } 
}

void ParseNode::setLeaf(bool isLeaf)
{
   if (isLeaf)
      leaf = true;
   else
      leaf = false;
}

bool ParseNode::isMathOperator()
{
   if ( nodekind == ExpKind && exp == OpExp)
   {
      if (tt == PLUS || tt == MINUS || tt == STAR || tt == DIV )
         return true;
   }
   
   return false;
}

int ParseNode::numberValue()
{
   type = Integer;
   return atoi(cString);
}

bool ParseNode::isNum()
{
   if (exp == NumExp)
      return true;
   else 
      return false;
}

bool ParseNode::isVar()
{
   if (exp == VarExp)
      return true;
   else 
      return false;
}

bool ParseNode::isFuncBegin()
{
   if (nodekind == DeclKind && decl == FuncDecl)
      return true;
   else 
      return false;
}

bool ParseNode::isFuncEnd()
{
   if (nodekind == StmtKind && stmt == EndFunc)
      return true;
   else 
      return false;
}

bool ParseNode::isAssignment()
{
   if (nodekind == ExpKind && exp == AssignExp)
      return true;
   else
      return false;
}
bool ParseNode::isDecl(void) 
{
   if (nodekind == DeclKind) 
      return true; 
   else 
      return false;
}

bool ParseNode::isMainDecl(void)
{
   if (nodekind == DeclKind && decl == FuncDecl)
   {  
      // main has to be all lowercase
      if (strcmp(cString,"main")==0) 
         return true;
      else
         return false;
   }
   else
      return false;
}

bool ParseNode::isVarDecl(void)
{
   if (nodekind == DeclKind && decl == VarDecl)
      return true;
   else 
      return false;
}

bool ParseNode::isRelOp()
{
  if (nodekind == ExpKind && exp == RelExp)
     return true;
  else
     return false; 
}

bool ParseNode::isCall()
{
  if (nodekind == ExpKind && exp == CallExp)
     return true;
  else
     return false;

}

bool ParseNode::isReturnStmt()
{
   if (nodekind == StmtKind && stmt == ReturnStmt)
      return true;
   else 
      return false;
}
#endif

