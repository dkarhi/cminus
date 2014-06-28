// By: David Karhi
//
// This is a header file for the Entry class which is an 
// entry in a symbol table
//

#ifndef ENTRY_H
#define ENTRY_H
#include <string.h>

#define MAXPARAMS 8

class Entry
{
   public:
      Entry(ParseNode *node);
      bool nameMatch(char *s);
      unsigned int getScope(){return scope;}
      char *getString(void){return name;}
      Type getType(void){return type;}
      DeclNode getDeclType(void){return kindOfDecl;}
      ExpNode getExpType(void) {return exp;}
      NodeKind getNodeKind(void) {return nodeKind;}
      Type *getParams(void){return paramList;}
      void displayEntry(void);
      void setScope(unsigned int s) {scope = s;}
      unsigned int getParamSize(void) {return params;}
      Type getParam(int i){return paramList[i];}
      void setParamNum(int n){paramNum = n;}
      int getParamNum(void){return paramNum;} 
      void setMem(int m) {memLocation = m;}
      int getMem(void) {return memLocation;}
   private:
      void setParams(ParseNode *node);
      char name[STRINGSIZE];
      DeclNode kindOfDecl;
      NodeKind nodeKind;
      Type type;
      ExpNode exp;
      unsigned int scope;
      unsigned int params;
      Type paramList[MAXPARAMS];
      unsigned int arraySize;
      int paramNum;
      int memLocation;
};

Entry::Entry(ParseNode *node)
{
   strcpy(name,node->getString());
   nodeKind = node->getNodeKind();
   if (nodeKind == DeclKind)
      kindOfDecl = node->getDeclType();
   else if (nodeKind == ExpKind)
      exp = node->getExp();
   type = node->getType();
   scope = node->getScope();
   if (type == Array && kindOfDecl == VarDecl )
      arraySize = node->getChild(0)->getNum();
   else
      arraySize = 0;
   params = 0;
   setParams(node);
   paramNum = node->getParamNum();
   memLocation = node->getMem();
}

bool Entry::nameMatch(char *s)
{
   if (strcmp(name, s) == 0)
      return true;
   else
      return false;
}

void Entry::setParams(ParseNode *node)
{
   ParseNode *tmp=NULL;

   if (node->getNodeKind() == DeclKind && node->getDeclType() == FuncDecl)
   {
      tmp = node->getChild(0);
      while (tmp)
      {
         ++params;
         if (params > MAXPARAMS)
         {
            cerr << "Function Parameter Limit (" <<  MAXPARAMS;
            cerr << ") Exceeded in Declaration of: ";
            cerr << name << endl;
            exit(EXIT_FAILURE);
         }
         else
         {
            paramList[params-1] = tmp->getType();
            tmp = tmp->getSibling();
         }
      }
   }  
   else 
      return;
}

void Entry::displayEntry()
{
   cout << ParseNode::NODE_TYPE[nodeKind] << " ";
   if (nodeKind == DeclKind)
      cout << ParseNode::DECL_TYPE[kindOfDecl] << " ";
   else if (nodeKind == ExpKind)
      cout << ParseNode::EXPR_TYPE[exp] << " ";
   
   cout << ParseNode::TYPE_TYPE[type] << " " << name;
   cout << " Scope = " << scope << " ";

   if (params > 0)
   {
      cout << "Number of Parameters: " << params;
      cout << endl << "   ";
      cout << "Parameter Types: ";
      for (unsigned int i=0;i<params;++i)
         cout << ParseNode::TYPE_TYPE[paramList[i]] << " ";
   }
   
   if (type == Array && kindOfDecl == VarDecl )
      cout << "Array Size: " << arraySize;

   cout << endl;
}

#endif



