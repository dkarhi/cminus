// David Karhi
//
//   This is the header file for the Parse class. This class implements
//   a parser that uses ParseNodes to create a parse table from a given
//   source code file.
//

#ifndef PARSE_H
#define PARSE_H

#include "parsenode.h"
#include "symboltable.h"
#include "codegenerator.h"

class Parse
{
   private:
      CodeGenerator codeGenerator;
      SymbolTable table;
      Tokenizer tk;
      ParseNode *root;
      int scope;
      bool mainDeclared;
      int numErrors;
   public:
      // default constructor
      Parse(){ root = NULL;}
      // constructor using a filename
      Parse(char *file, bool dbug);
      // deconstructor
      ~Parse(void);
      // return the root of the parse tree
      ParseNode *getParseTree(void){return root;}
      // output the tree to the screen
      void displayTree(void);
      // internal function for displayTree
      void display(ParseNode*,int);
      // this function prints the entire symbol table
      void displayTable(void);
      // this function helps with the deconstructor
      void deleteTree(ParseNode *headPtr);
      // this function performs post traversal on the tree
      void postTraversal(bool debug);
      // this is a helper function for postTraverse
      void traverse(ParseNode *tree,bool debug);
      // this function parses declarations
      ParseNode *parseDeclarations(void);
      // this function parses terms
      ParseNode *parseTerm(void);
      // this function parses a declaration
      ParseNode *parseDeclaration(void);
      // this function parses params
      ParseNode *parseParams(void);
      // this function parses a param list
      ParseNode *parseParamList(void);
      // this function parses a param
      ParseNode *parseParam(void);
      // this function parses a factor
      ParseNode *parseFactor(void);
      // this function parses a function statement
      ParseNode *parseFunctionStatement(void);
      // this function parses an expression
      ParseNode *parseExpression(void);
      // this function parses an additive expression
      ParseNode *parseAdditiveExpression(void);
      // this function parses a relational operator
      ParseNode *parseRelop(void);
      // this function parses a simple expression
      ParseNode *parseSimpleExpression(void);
      // this function parses a var
      ParseNode *parseVar(void);
      // this function parses a variable declaration
      ParseNode *parseVarDeclaration(void);
      // this function parses args
      ParseNode *parseArgs(void);
      // this function parses local declarations
      ParseNode *parseLocalDeclarations(void);
      // this function parses statement lists
      ParseNode *parseStatementList(void);
      // this function parses a selection statement
      ParseNode *parseSelectionStatement(void);
      // this function parses an iteration statement
      ParseNode *parseIterationStatement(void);
      // this function parses a return statement
      ParseNode *parseReturnStatement(void);
      // this function parses a compound statement
      ParseNode *parseCompoundStatement(void);
      // this function parses an expression statement
      ParseNode *parseExpressionStatement(void);
      // this function parses a statement
      ParseNode *parseStatement(void);
      // this function folds a math operator into a number
      bool fold (ParseNode *op,bool &flag);
      // this function calls fold
      void folding(void);
      // this function handles the return statements
      void checkReturn(ParseNode *,Type);


};

Parse::Parse(char *file, bool dbug)
{
   scope = 0;
   numErrors = 0;
   tk.setInput(file);
   tk.setDebug(dbug);
   root=parseDeclarations();
   numErrors += tk.getNumErrors();
   postTraversal(dbug);
   if (dbug)
      displayTree();
}

Parse::~Parse(void)
{
   deleteTree(root);      
}

void Parse::deleteTree(ParseNode *headPtr)
{
   if (headPtr != NULL)
   {
      for (int i=0; i<MAXCHILDREN; ++i)
         deleteTree(headPtr->getChild(i));
      deleteTree(headPtr->getSibling());
      delete headPtr;
   }
}

// we're basically nesting a display function in the display function
// for recursive reasons. This makes it so that the programmer using this 
// class does not have to know to pass an integer to this function
void Parse::displayTree()
{
   int spaces = 0;
   cout << endl;
   display(root, spaces);
   cout << endl;
}

void Parse::display(ParseNode* currentRoot, int spaces)
{
   if (currentRoot == NULL)
      return;

   for (int i=0; i<spaces; ++i)
      cout << " ";

   currentRoot->displayNode();
   for(int j=0; j<MAXCHILDREN; ++j)
      display(currentRoot->getChild(j),spaces+3);
   display(currentRoot->getSibling(),spaces); 
}   

void Parse::displayTable()
{
   table.displayTable();   
}

void Parse::postTraversal(bool debug)
{
   mainDeclared = false;
   // call the function to do folding
   folding();
   traverse(root, debug);
   if (!mainDeclared)
   {
      cerr << "ERROR: Main Function Not Declared" << endl;
      ++numErrors;
   }
   numErrors += table.getErrors();
   codeGenerator.generateCode(root,numErrors);
}

void Parse::traverse (ParseNode *tree,bool debug)
{  
   Type treeType;

   if (tree == NULL)
      return;
   // this puts everything we need in the symbol table
   else if (tree == root)
   {
      table.startScope(tree);
      if (debug)
         displayTable(); 
   }
   else if (tree->isFuncBegin())
   { 
      // the output function has aldready been inserted, so we do not
      // want to insert it again
      if (strcmp(tree->getString(),"output") != 0)
      {
         if (mainDeclared)
         {
            cerr << "ERROR: Function Declared After Main: ";
            tree->displayNode();
            ++numErrors;
         }
      
         if (table.lookup(tree))
         {
            // means the function has already been declared
            cerr << "ERROR: Function Previously Declared: ";
            tree->displayNode();
            ++numErrors;
         }
         if (tree->isMainDecl())
         {
            if (tree->getType() != Void)
            {
               cerr << "ERROR: Main Not Declared As Void Function" << endl;
               ++numErrors;
            }
            mainDeclared = true;
         }
      
         table.startScope(tree);
         if (debug)
            displayTable();
         
         treeType = tree->getType();
         checkReturn(tree,treeType);
      }
   }
   // this removes all the things we don't need from the symbol table
   else if (tree->isFuncEnd())
      table.endScope(); 
   else if (tree->isMathOperator())
   {
      tree->setType(Integer);
      if (tree->getChild(0)->isRelOp() || tree->getChild(1)->isRelOp())
      {
          cerr << "ERROR: Invalid Use of Relational Operator: ";
          cerr << tree->getChild(0)->getLineNo() << endl;
          ++numErrors;
      }
      if (tree->getChild(0) && tree->getChild(1))
      {   
         // first we want to do a lookup to see if these vars 
         // have been declared and what type they have been
         // declared as since lookup adds extra type information 
         // to the node
         if (tree->getChild(0)->isVar()) 
         {
            if (!table.lookup(tree->getChild(0)))
            {
               cerr << "ERROR: Undeclared Variable: ";
               cerr << tree->getChild(0)->getString() << " ";
               cerr << tree->getChild(0)->getLineNo() << endl;
               ++numErrors;
            }
         }
         if (tree->getChild(1)->isVar())
         {
            if (!table.lookup(tree->getChild(1)))
            {
               cerr << "ERROR: Undeclared Variable: ";
               cerr << tree->getChild(1)->getString() << " ";
               cerr << tree->getChild(1)->getLineNo() << endl;
               ++numErrors;
            }
         }

         if (tree->getChild(0)->isCall())
            table.lookup(tree->getChild(0));
         if (tree->getChild(1)->isCall())
            table.lookup(tree->getChild(1));

         if (tree->getChild(0)->isMathOperator())
            tree->getChild(0)->setType(Integer);
         if (tree->getChild(1)->isMathOperator())
            tree->getChild(1)->setType(Integer);

         Type child0Type,child1Type;
         child0Type = tree->getChild(0)->getType();
         child1Type = tree->getChild(1)->getType();
         
         if (child0Type == Void)
         {
            cerr << "ERROR: Invalid Use of Void Type: ";
            cerr << tree->getChild(0)->getString() << " ";
            cerr << tree->getChild(0)->getLineNo() << endl;
            ++numErrors;
         }
         if (child1Type == Void)
         {
            cerr << "ERROR: Invalid Use of Void Type: ";
            cerr << tree->getChild(1)->getString() << " ";
            cerr << tree->getChild(1)->getLineNo() << endl;
            ++numErrors;
         }

         // we have already sent an error if an array was not 
         // declared as an int, so we can just do this quick
         // conversion. If it is an array, we can be sure it is 
         // either declared as an int or an error has already
         // been displayed
         if (child0Type == Array)
            child0Type = Integer;
         if (child1Type == Array)
            child1Type = Integer;

         if (child0Type != child1Type)
         {
            cerr << "ERROR: Type Mismatch on Line ";
            cerr << tree->getLineNo() << endl;
            ++numErrors;
         }   
      }
   }
   else if (tree->isRelOp())
   {
      if (!table.lookup(tree->getChild(0)) )
      {
         if (tree->getChild(0)->isVar())
         { 
            cerr << "ERROR: Undeclared Variable: ";
            cerr << tree->getChild(0)->getString() << " ";
            cerr << tree->getChild(0)->getLineNo() << endl;
            ++numErrors;
         }
      }
      if (!table.lookup(tree->getChild(1)) )
      { 
         if (tree->getChild(1)->isVar())
         { 
            cerr << "ERROR: Undeclared Variable: ";
            cerr << tree->getChild(1)->getString() << " ";
            cerr << tree->getChild(1)->getLineNo() << endl;
            ++numErrors;
         }
      }
      
      if (tree->getChild(0)->isCall() &&
          tree->getChild(0)->getType() == Void)
      {
         cerr << "ERROR: Invalid Use of Void Type: ";
         cerr << tree->getChild(0)->getString() << " ";
         cerr << tree->getChild(0)->getLineNo() << endl;
         ++numErrors;
      }
      if (tree->getChild(1)->isCall() &&
          tree->getChild(1)->getType() == Void)
      {
         cerr << "ERROR: Invalid Use of Void Type: ";
         cerr << tree->getChild(1)->getString() << " ";
         cerr << tree->getChild(1)->getLineNo() << endl;
         ++numErrors;
      }

   }
   else if (tree->isCall())
   {
      if (!table.lookup(tree))
      {
         cerr << "ERROR: Undeclared Function: ";
         cerr << tree->getString() << " " << tree->getLineNo() << endl;
         ++numErrors;
      }
   }
   else if (tree->isAssignment())
   {
      if (tree->getChild(0)->isVar())
      {
         if (!table.lookup(tree->getChild(0)))
         {
            cerr << "ERROR: Undeclared Variable: ";
            cerr << tree->getChild(0)->getString() << " ";
            cerr << tree->getChild(0)->getLineNo() << endl;
            ++numErrors;
         }
      
      
         if (tree->getChild(0)->getType() == Array)
         {  
            cerr << "ERROR: Invalid Assignment: ";
            cerr << tree->getChild(0)->getString() << " ";
            cerr << tree->getChild(0)->getLineNo() << endl;
            ++numErrors;
         }
      }

      if (tree->getChild(1)->isRelOp())
      {
         cerr << "ERROR: Can Not Assign Relational Operator: ";
         cerr << tree->getChild(1)->getLineNo() << endl;
         ++numErrors;
      }

      if (tree->getChild(1)->isVar())
      {
         if(!table.lookup(tree->getChild(1)))
         {
            cerr << "ERROR: Undeclared Variable: ";
            cerr << tree->getChild(1)->getString() << " ";
            cerr << tree->getChild(1)->getLineNo() <<endl;
            ++numErrors;
         }
      }
     
      if (tree->getChild(0)->isCall())
      {
         if (table.lookup(tree->getChild(0)) &&
            tree->getChild(0)->getType() == Void)
         {
            cerr << "ERROR: Invalid Use of Void Type: ";
            cerr << tree->getChild(0)->getString() << " ";
            cerr << tree->getChild(0)->getLineNo() << endl; 
            ++numErrors;
         }
      }
      if (tree->getChild(1)->isCall())
      {
         if (table.lookup(tree->getChild(1)) && 
             tree->getChild(1)->getType() == Void)
         {
            cerr << "ERROR: Invalid Use of Void Type: ";
            cerr << tree->getChild(1)->getString() << " ";
            cerr << tree->getChild(1)->getLineNo() << endl;
            ++numErrors;
         }
      }
   }
   else if (tree->isVarDecl() && tree->getScope() == 0)
   {
      table.insert(tree);
   }
  

   for (unsigned int i=0; i<MAXCHILDREN; ++i)
      traverse(tree->getChild(i),debug);
   traverse(tree->getSibling(),debug);
   
}

ParseNode *Parse::parseDeclarations(void)
{       
   // first we create the input and output function nodes
   root = new ParseNode (DeclKind,FuncDecl, Integer,"input", 0, 0,scope);
   root->setSibling(new ParseNode (DeclKind, FuncDecl, Void, "output",0,0,scope));
   ParseNode* sibling = root->getSibling();
   sibling->setChild(0, new ParseNode (DeclKind, ParamDecl, Integer, "x",0,0,scope));

   sibling->setSibling(parseDeclaration()); 
   sibling = sibling->getSibling(); 
   
   if(sibling)
   {
      ParseNode *tmp = sibling;
      while(tmp && (tk.isMatch(VOID) || tk.isMatch(INT)))
      {
         tmp->setSibling( parseDeclaration());
         tmp=tmp->getSibling();
      }
   }	
   // if we get here, it should be the end of file
   tk.match(END);
   return root;

}

ParseNode *Parse::parseTerm(void)
{
   ParseNode *tmp;
   ParseNode *node = parseFactor();
   tmp = node;

   while(tmp && (tk.isMatch(STAR) || tk.isMatch(DIV))) 
   {
      node = new ParseNode(OpExp, tk.getToken(), scope);
      tk.match(tk.getToken().getType());
      node->setChild(0,tmp);
      node->setChild(1, parseFactor());
      tmp = node;
   }
   return node; 
}


ParseNode *Parse::parseDeclaration(void)
{
   ParseNode *node = NULL;
   if(tk.isMatch(INT))
   {
      node = new ParseNode(DeclKind,VarDecl, Integer,"", tk.getLineNo(), tk.getPosition(), scope);
      tk.match(INT);
   }
   else if (tk.isMatch(VOID))
   { 
      node = new ParseNode(DeclKind,VarDecl, Void,"", tk.getLineNo(),tk.getPosition(),scope);
      tk.match(VOID);
   }
   node->setString(tk.getString());
   tk.match(ID);
   if(tk.isMatch(LPAR))
   {
      node->setDeclType(FuncDecl);
      tk.match(LPAR);
      scope = 1;
      node->setChild(0,parseParams());
      tk.match(RPAR);
      node->setChild(1,parseFunctionStatement());
      scope=0;
   }
   else if(tk.isMatch(LSQ))
   {
      if (node->getType() == Void)
      {
         cerr << "ERROR: Variable Declared With Void Type: ";
         cerr << node->getString() << " " << node->getLineNo();
         cerr << " " << node->getPosition() << endl;
         ++numErrors;
      }
      node->setType(Array);
      tk.match(LSQ);
      node->setChild(0,new ParseNode(NumExp,tk.getToken(),scope));
      tk.match(NUM);
      tk.match(RSQ);
      tk.match(SEMIC);
   }
   else if (node->getDeclType() == VarDecl && node->getType() == Void)
   {
      cerr << "ERROR: Variable Declared With Void Type: ";
      cerr << node->getString() << " " << node->getLineNo();
      cerr << " " << node->getPosition() << endl;
      tk.match(SEMIC);
      ++numErrors;
   }
   else
      tk.match(SEMIC);

   return node;
}

ParseNode *Parse::parseParams(void)
{
   ParseNode *node = NULL;

   if(!tk.isMatch(VOID))
   {
      node = parseParamList();
   }
   else 
      tk.match(VOID);
   return node;
}

ParseNode *Parse::parseParamList(void)
{
   ParseNode *node = parseParam();
   
   if(node)
   {
      ParseNode *tmp = node; 
      while(tmp && tk.isMatch(COMMA))
      {
         tk.match(COMMA);
         tmp->setSibling( parseParam());
         tmp = tmp->getSibling();
      }
   }
   return node;
}

ParseNode *Parse::parseParam(void)
{
   ParseNode *node;
   
   node = new ParseNode(DeclKind,ParamDecl,Void,"",tk.getLineNo(),tk.getPosition(),scope);
   if (tk.isMatch(INT))
      node->setType(Integer);
   
   tk.match(tk.getToken().getType());
   node->setString(tk.getToken().getString());
   tk.match(ID);
   if (tk.isMatch(LSQ))
   {
      tk.match(LSQ);
      tk.match(RSQ);
      node->setType(Array);
   }

   return node;
}

ParseNode *Parse::parseFactor(void)
{
   ParseNode* node = NULL;
   
   if (tk.isMatch(LPAR))
   {
      tk.match(LPAR);
      node = parseExpression();
      tk.match(RPAR);
   }
   else if (tk.isMatch(ID))
   {
      if (tk.isPeekMatch(LPAR))
      { 
         node = new ParseNode (CallExp, tk.getToken(),scope);
         tk.match(ID);
         tk.match(LPAR);
         node->setChild(0, parseArgs());
         tk.match(RPAR);
      }
      else 
      {
         node = new ParseNode (VarExp, tk.getToken(),scope);
         tk.match(ID);
         if (tk.isMatch(LSQ))
         {
            tk.match(LSQ);
            node->setChild(0,parseAdditiveExpression());
            tk.match(RSQ); 
         }
      }
   }
   else if (tk.isMatch(NUM))
   {
      node = new ParseNode (NumExp, tk.getToken(),scope);
      tk.match(NUM);
   }

   return node;
}

ParseNode *Parse::parseFunctionStatement(void)
{
   ParseNode *node;
   tk.match(LCURL);
   node = new ParseNode (FuncStmt, tk.getLineNo(),tk.getPosition(),scope);
   node->setChild(0,parseLocalDeclarations());
   if (node->getChild(0))
      node->setChild(1,parseStatementList());
   else 
      node->setChild(0,parseStatementList());
   node->setSibling(new ParseNode (EndFunc,tk.getLineNo(), tk.getPosition(),scope));
   tk.match(RCURL);
   return node;

}

ParseNode *Parse::parseExpression(void)
{
   ParseNode *node=NULL, *tmp=NULL,*relop=NULL;
  
   if (tk.isMatch(ID))
   {
      if (tk.isPeekMatch(LSQ) || tk.isPeekMatch(ASSIGN))
         node = parseVar();

      if (tk.isMatch(ASSIGN))
      {
         tmp = node;
         node = new ParseNode (AssignExp,tk.getToken(),scope);
         tk.match(ASSIGN);
         node->setChild(0,tmp);
         node->setChild(1,parseSimpleExpression());
      }
      else if (tk.isMatch(LT) || tk.isMatch(GT) || tk.isMatch(LEQ) ||
               tk.isMatch(GEQ) || tk.isMatch(EQ) || tk.isMatch(NOTEQ))
      {
         // it's hard to figure out whether the variable is the left
         // hand part of an assignment or a var that is part of a 
         // simple expression or term or factor. So this is a bad way of 
         // doing this but we are basically parsing a simple expression 
         // inside of this function
         relop = parseRelop();
         
         while (relop)
         {
            tmp = node;
            node = new ParseNode (RelExp, tk.getToken(),scope);
            node->setChild(0,tmp);
            node->setChild(1,parseAdditiveExpression());
            relop = parseRelop();
         }
      }
      else if(tk.isMatch(PLUS) || tk.isMatch(MINUS))
      {
         while(tk.isMatch(PLUS) || tk.isMatch(MINUS))
         {
            tmp = node;
            node = new ParseNode(OpExp, tk.getToken(),scope);
            tk.match(tk.getToken().getType());
            node->setChild(0, tmp);
            node->setChild(1, parseTerm());
            
         }
      }
      else if(tk.isMatch(STAR) || tk.isMatch(DIV))
      {
         while(tk.isMatch(STAR) || tk.isMatch(DIV))
         {
            tmp = node;
            node = new ParseNode(OpExp, tk.getToken(),scope);
            tk.match(tk.getToken().getType());
            node->setChild(0, tmp);
            node->setChild(1, parseFactor());

         }
      }
   
      if (!node)
         node=parseSimpleExpression();
   }
   else
      node=parseSimpleExpression();
 
   return node;
}

ParseNode *Parse::parseSimpleExpression(void)
{
   ParseNode *node=NULL, *tmp=NULL, *relop=NULL;
   
   node = parseAdditiveExpression();
   relop = parseRelop();

   if (relop)
   {
      tmp = node;

      while (tmp && relop)
      {  
         node = relop;
         node->setChild(0,tmp);
         node->setChild(1,parseAdditiveExpression()); 
         tmp = node;
         relop = parseRelop();
      }
   }

   return node;
      
}

ParseNode *Parse::parseRelop(void)
{
   ParseNode *node = NULL;

   switch(tk.getToken().getType())
   {
      case (LT):
      {
         node = new ParseNode (RelExp, tk.getToken(),scope);
         tk.match(LT);
         break;
      }
      case (GT):
      {
         node = new ParseNode(RelExp, tk.getToken(),scope);
         tk.match(GT);
         break;
      }
      case (LEQ):
      {
         node = new ParseNode(RelExp, tk.getToken(),scope);
         tk.match(LEQ);
         break;
      }
      case (GEQ):
      {
         node = new ParseNode(RelExp, tk.getToken(),scope);
         tk.match(GEQ);
         break;
      }
      case (NOTEQ):
      {
         node = new ParseNode (RelExp, tk.getToken(),scope);
         tk.match(NOTEQ);
         break;
      }
      case (EQ):
      {
         node = new ParseNode (RelExp,tk.getToken(),scope);
         tk.match(EQ);
         break;
      }
      default:
         break;
   }
   return node;
}

ParseNode *Parse::parseAdditiveExpression(void)
{
   ParseNode *tmp;
   ParseNode *node = parseTerm();
   tmp = node;

   while(tmp && tk.isMatch(PLUS) || tk.isMatch(MINUS))
   {
      
      node = new ParseNode(OpExp, tk.getToken(),scope);
      tk.match(tk.getToken().getType());
      node->setChild(0, tmp);
      node->setChild(1, parseTerm());
      tmp = node;
      
   }
   return node;
}

ParseNode *Parse::parseArgs(void)
{
   ParseNode *node = NULL;
   ParseNode *tmp = NULL;

   // if the next token is a ) character then there are no args
   if (tk.isMatch(RPAR))
      return node;

   node = parseExpression();
   tmp = node;
 
   while (tk.isMatch(COMMA))
   {
      tk.match(COMMA);
      tmp->setSibling(parseExpression());
      tmp = tmp->getSibling();
   }

   return node;
}

ParseNode *Parse::parseVar(void)
{
   ParseNode *node;

   node = new ParseNode (VarExp, tk.getToken(),scope);
   
   tk.match(ID);
   if (tk.isMatch(LSQ))
   {  
      tk.match(LSQ);
      node->setChild(0,parseAdditiveExpression());
      tk.match(RSQ);
   }
   return node;
}

ParseNode *Parse::parseLocalDeclarations(void)
{
   ParseNode *node=NULL;
   ParseNode *tmp=NULL;

   if (tk.isMatch(INT) || tk.isMatch(VOID)) 
   {
      node = parseVarDeclaration();
      tmp = node;  
   }
 
   while (tk.isMatch(INT) || tk.isMatch(VOID))
   {
      tmp->setSibling(parseVarDeclaration());
      tmp=tmp->getSibling();
   }  
   
   return node;    
}

ParseNode *Parse::parseStatementList(void)
{
   ParseNode *node=NULL;
   ParseNode *tmp=NULL;
      
   if (tk.isMatch(RCURL))
      return node;
   else 
   {
      node = parseStatement();
      tmp = node;   

      while (tmp && !tk.isMatch(RCURL))
      {
         tmp->setSibling(parseStatement());
         tmp=tmp->getSibling();         
      }
   }       
   return node;
}

ParseNode *Parse::parseSelectionStatement(void)
{
   ParseNode *node;

   node = new ParseNode (IfStmt, tk.getLineNo(),tk.getPosition(),scope);
   tk.match(IF);
   tk.match(LPAR);
   node->setChild(0,parseExpression());
   tk.match(RPAR);
   node->setChild(1,parseStatement());
 
   if (tk.isMatch(ELSE))
   {
      tk.match(ELSE);
      node -> setChild(2,parseStatement());
   }

   return node;
}        

ParseNode *Parse::parseIterationStatement(void)
{
   ParseNode *node;

   node = new ParseNode (WhileStmt,tk.getLineNo(), tk.getPosition(),scope);
   tk.match(WHILE);
   tk.match(LPAR);
   node->setChild(0,parseExpression());
   tk.match(RPAR);
   node->setChild(1,parseStatement());

   return node;
}
   
ParseNode *Parse::parseReturnStatement(void)
{
   ParseNode *node;

   node = new ParseNode (ReturnStmt, tk.getLineNo(), tk.getPosition(),scope);
   tk.match(RETURN);
   
   if (!tk.isMatch(SEMIC))
      node->setChild(0,parseSimpleExpression());

   tk.match(SEMIC);
   return node;
}

ParseNode *Parse::parseCompoundStatement(void)
{
   ParseNode *node;

   tk.match(LCURL); 
   node = new ParseNode (CmpStmt,tk.getLineNo(),tk.getPosition(),scope);
   node->setChild(0,parseStatementList());
   tk.match(RCURL);
   
   return node;
}

ParseNode *Parse::parseExpressionStatement(void)
{
   ParseNode *node=NULL;
 
   if (!tk.isMatch(SEMIC))
      node = parseExpression();
 
   tk.match(SEMIC);
   return node;
}

ParseNode *Parse::parseStatement(void)
{
   ParseNode *node=NULL;

   if (!tk.isMatch(RCURL))
   {  
      if (tk.isMatch(IF))
         node=parseSelectionStatement();
      else if (tk.isMatch(WHILE))
         node= parseIterationStatement();
      else if (tk.isMatch(RETURN))
         node=parseReturnStatement();
      else if (tk.isMatch(LCURL))
         node = parseCompoundStatement();
      else
         node = parseExpressionStatement();

   }

   return node;

}

ParseNode *Parse::parseVarDeclaration(void)
{
   ParseNode *node = NULL;
   if(tk.isMatch(INT))
   {
      node = new ParseNode(DeclKind,VarDecl, Integer,"", tk.getLineNo(), tk.getPosition(),scope);
      tk.match(INT);
   }
   else if (tk.isMatch(VOID))
   {
      node = new ParseNode(DeclKind,VarDecl, Void,"", tk.getLineNo(),tk.getPosition(),scope);
      tk.match(VOID);
   }
   node->setString(tk.getString());
   tk.match(ID);
   
   if(tk.isMatch(LSQ))
   {
      if (node->getType() == Void)
      {
         cerr << "ERROR: Variable Declared With Void Type: ";
         cerr << node->getString() << " " << node->getLineNo();
         cerr << " " << node->getPosition() << endl;
         ++numErrors;
      }
      node->setType(Array);
      tk.match(LSQ);
      node->setChild(0,new ParseNode(NumExp,tk.getToken(),scope));
      tk.match(NUM);
      tk.match(RSQ);
      tk.match(SEMIC);
   }
   else if (node->getDeclType() == VarDecl && node->getType() == Void)
   {
      cerr << "ERROR: Variable Declared With Void Type: ";
      cerr << node->getString() << " " << node->getLineNo();
      cerr << " " << node->getPosition() << endl;
      tk.match(SEMIC);
      ++numErrors;
   }
   else
      tk.match(SEMIC);

   return node;
}

void Parse::folding()
{
   bool condition = false;
   
   // if we succeed in folding two nodes we want to check if the new number
   // node could be folded again.
   while(fold(root,condition))
      condition=false;
   
}

bool Parse::fold(ParseNode *op, bool &flag)
{
   ParseNode *child0,*child1;
   int number = 0;
 
   child0=op->getChild(0);
   child1=op->getChild(1);

   if (op->isMathOperator())
   {
      if (child0->isNum() && child1->isNum())
      {
         if (op->getTokenType() == PLUS)
         {
            number = child0->getNum() + child1->getNum();
            flag = true;
         }
         else if (op->getTokenType() == MINUS)
         {
            number = child0->getNum() - child1->getNum();
            flag = true;
         }
         else if (op->getTokenType() == STAR)
         {
            number = child0->getNum() * child1->getNum();
            flag = true;
         }
         else if (op->getTokenType() == DIV)
         {
            number = child0->getNum() / child1->getNum();
            flag = true;
         }

         deleteTree(child0);
         deleteTree(child1);
         op->setChild(0,NULL);
         op->setChild(1,NULL);
 
         op->setExp(NumExp);
         op->setNum(number);
      }
   }
   
   if (child0)
      fold(child0,flag);
   if (child1)
      fold(child1,flag);
   if (op->getSibling())
      fold(op->getSibling(),flag);

   if (flag)
      return true;
   else
      return false;
}

void Parse::checkReturn(ParseNode* tree, Type treeType)
{

   if (tree == NULL)
      return;
   else if (tree->isFuncEnd())
      return;
   else if (tree->isReturnStmt())
   {
      if (treeType == Void)
      {
         if (tree->getChild(0) != NULL)
         {
            cerr << "ERROR: Void Function Returns A Value: ";
            cerr << tree->getLineNo() << " " << tree->getPosition() << endl;
            ++numErrors;
         }
      }
      else if (treeType == Integer)
      {
         if (tree->getChild(0) == NULL)
         {
            cerr << "ERROR: Integer Function Returns No Value: ";
            cerr << tree->getLineNo() << " " << tree->getPosition() << endl;
            ++numErrors;
         }
         else 
         {
            table.lookup(tree->getChild(0));
            if (  tree->getChild(0)->getType() != Integer)
            {
               if (!tree->getChild(0)->isMathOperator())
               {
                  cerr << "ERROR: Integer Function Does Not Return Integer: ";
                  cerr << tree->getLineNo() << " " << tree->getPosition() << endl;
                  ++numErrors;
               }
            }
         }
      }
   }
   for (unsigned int i=0; i<MAXCHILDREN; ++i)
      checkReturn(tree->getChild(i),treeType);
   // a new declaration can't be a child, but a sibling might be a 
   // new function declaration that we don't want to look at yet
   if (tree->getSibling())
   {
      if (tree->getSibling()->isFuncBegin())
         return;
      else
         checkReturn(tree->getSibling(),treeType);    
   }
}

#endif
