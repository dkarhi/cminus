#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "entry.h"

#define HASHSIZE 211

// we're using chained hashing, so we need a linked list
struct linkedList
{
   Entry *entry;
   struct linkedList *next;
};

// this is an insert list used to keep track of what we 
// put into the hash table. 
struct insertList
{
   char string[STRINGSIZE];
   struct insertList *next;
};

class SymbolTable
{
   private:
      struct linkedList *table[HASHSIZE];
      struct insertList *list;
      struct insertList *tail;
      void setNode(struct linkedList*, ParseNode *);
      unsigned int hashFunction(char *s);
      int numErrors;
   public:
      // this is a constructor for the SymbolTable class
      SymbolTable();
      // this is the deconstructor
      ~SymbolTable();
      // this function displays the entire symbol table
      void displayTable(void);
      // insert returns true if it successfully inserts a node
      bool insert(ParseNode *);
      // lookup returns true if it finds the given node in the table
      bool lookup(ParseNode *);
      // remove returns true if it successfully removes an item
      bool remove(char *string);
      // endScope should be called when leaving a scope
      void endScope(void);
      // startScope should be called when entering a new scope
      void startScope(ParseNode *node);
      // push puts a string into the insertList
      void push(char* string);
      // pop removes a string from the insertList
       void pop(void);
      // this returns the top of the stack
      char *getTop(void);
      // this function checks function calls against function declarations
      void checkParams(struct linkedList*, ParseNode *node);
      // returns numErrors
      int getErrors(void){return numErrors;}
};

SymbolTable::SymbolTable()
{
   for (unsigned int i=0; i<HASHSIZE; ++i)
      table[i] = NULL;  

   list = NULL;
   tail = NULL;
   numErrors=0;
}

SymbolTable::~SymbolTable()
{ 
   struct linkedList *tmp,*remove;
   struct insertList *pointer,*rmv; 

   for (unsigned int i=0; i<HASHSIZE; ++i)
   {
      if (table[i])
      {
         tmp = table[i]->next;
         delete table[i];
         while (tmp)
         {
            remove = tmp;
            tmp = tmp->next;
            delete remove;
         }
      }
   }        
   
   if (list)
   {
      rmv = list;
      while (rmv)
      {
         pointer = rmv->next;
         delete rmv;
         rmv = pointer;
      }
   } 
}

void SymbolTable::displayTable()
{
   struct linkedList *list;
   
   cout << endl;
   for (unsigned int i=0; i<HASHSIZE; ++i)
   {
      list = table[i];
      while (list)
      {
         list->entry->displayEntry();
         list = list->next;
      }
   }   
}

bool SymbolTable::insert(ParseNode *n)
{
   struct linkedList *tmp;
   unsigned int key = hashFunction(n->getString());
   
   if(table[key]==NULL)
   {
      tmp = new linkedList;
      tmp->next=NULL;
      push(n->getString());
      tmp->entry = new Entry(n);
      table[key]=tmp;
   }
   else
   {
      if (n->getScope() > table[key]->entry->getScope())
      {
         tmp = new linkedList;
         tmp->next=table[key];
         push(n->getString());
         tmp->entry = new Entry(n);
         table[key] = tmp;   
      }
      else
      {
         tmp = table[key];
         while(tmp)
         {
            if(strcmp(tmp->entry->getString(),n->getString())==0)
            {
               if (table[key]->entry->getScope() == n->getScope())
               {
                  cerr << "ERROR: Variable Already Declared: ";
                  cerr << n->getString() << endl;
                  ++numErrors;
                  return false;
               }
            }
            tmp=tmp->next;
         }
         push(n->getString());
         tmp->entry = new Entry(n);
         tmp->next = table[key];
         table[key]=tmp;
      }
   }
   return true;
}

bool SymbolTable::lookup(ParseNode *n)
{
   struct linkedList *tmp;
   unsigned int key = hashFunction(n->getString());
   if(table[key]==NULL)
   {
      return false;
   }
   tmp = table[key];
   while(tmp)
   {
      if(strcmp(tmp->entry->getString(),n->getString())==0)
      {
         if (tmp->entry->getScope() <=  n->getScope())
         {
            setNode(tmp,n);
            return true;
         }
      }
      tmp=tmp->next;
   }

   return false;
}

bool SymbolTable::remove(char* string)
{
   struct linkedList *tmp, *remove;
   unsigned int key = hashFunction(string);
   
   if (table[key] == NULL)
      return false;
   else if (strcmp(table[key]->entry->getString(),string)==0)
   {
      tmp = table[key]->next;
      delete table[key];
      table[key] = tmp;
      return true;
   }
   else 
   {
      tmp = table[key];
      while (tmp->next && strcmp(tmp->next->entry->getString(),string)!=0)
         tmp = tmp->next;
      remove=tmp->next;
      tmp->next=remove->next;
      delete remove;
      return true;
   }  
}

unsigned int SymbolTable::hashFunction(char *s)
{
   unsigned int key = 0;

   for(unsigned i =0; i< strlen(s); ++i)
   {
      key=(key+(unsigned(s[i]))<<2) % HASHSIZE ;
   }
   return key;
}

void SymbolTable::startScope(ParseNode *node)
{
   ParseNode *tmp;
   int paramNumber = 0;
   int memLocation = 0;
   
   // we start a scope when we reach a function declaration,
   // so first we want to check if we need to insert that declaration
   if (node->isFuncBegin() )
   {
      // check if this is global scope
      if (strcmp(node->getString(),"input")==0)
      { 
         insert(node);
         if (node->getSibling())
         {
            tmp= node->getSibling();
            insert(tmp);
         }
         if (tmp && tmp->getSibling())
            tmp = tmp->getSibling();

         // mark the end of scope and return
         push("$"); 
         return;
      }

      // function declarations are global, so pop the 
      // scope separator and then push it after the function 
      // declaration is inserted
      pop();
      insert(node);
      push("$");
   
      // next, child 0 and its siblings are parameters that need
      // to be inserted
      if (node->getChild(0))
      { 
         if (node->getChild(0)->isDecl())
         {
            tmp = node->getChild(0);
            tmp->setParamNum(paramNumber);
            ++paramNumber;
            insert(tmp);
            while (tmp->getSibling() && tmp->getSibling()->isDecl())
            {
               tmp=tmp->getSibling();
               tmp->setParamNum(paramNumber);
               ++paramNumber;
               insert(tmp);
            }
         }
      }
   
      // finally, child 1 is the function statement that begins with 
      // declarations that need to be inserted.
      if (node->getChild(1))
      {  
         tmp = node->getChild(1);
         if (tmp->getChild(0))
         {
            // child 0 of child 1 is where the local declarations begin
            tmp = tmp->getChild(0);
            while (tmp && tmp->isVarDecl())
            {
               tmp->setMem(memLocation);
               // if this is an array, get the index so we can allocate
               // enough space
               if (tmp->getChild(0))
               {
                  memLocation += tmp->getChild(0)->getNum();
               }
               else
                  ++memLocation;
               insert(tmp);
               tmp = tmp->getSibling();
            }
         }  
      }
   }
   // we end the scope with a $ character
   push("$");
}

void SymbolTable::endScope()
{
   // first, pop out the $ char
   pop(); 
   while(strcmp(getTop(),"$") != 0)
   { 
      remove(getTop());
      pop();
   }                              // the $ character. So we push the $
}

void SymbolTable::push(char* string)
{
   // we want to stick a $ character at the beginning of the stack
   // so that we have a divider
   if (!list)
   {
      list = new struct insertList;
      strcpy(list->string,"$");
      list->next= new struct insertList;
      tail = list->next;
      strcpy(tail->string,string);
      tail->next = NULL;
   }
   else 
   {
      tail->next = new struct insertList;
      tail = tail->next;
      strcpy(tail->string,string);
      tail->next=NULL;
   }
}

void SymbolTable::pop()
{
   struct insertList *tmp,*remove;
    
   tmp = list;
   while (tmp->next && tmp->next != tail)
      tmp = tmp->next;

   remove = tail;
   tail = tmp;
   delete remove;
   tail->next = NULL;
}

void SymbolTable::setNode(struct linkedList *tmp,ParseNode * node)
{
   node->setType(tmp->entry->getType());
   // if the variable has a child, that means it's an array location
   // and the type should be set to Integer instead of Array.
   if (node->isVar())
   {
      if (node->getChild(0))
      {
         node->setType(Integer);
      }
   }
   // we're going to need the some information to
   // calculate the location on the stack later.
   node->setParamNum(tmp->entry->getParamNum());
   node->setMem(tmp->entry->getMem());  
   

   // check if this is a call
   if (node->getNodeKind() == ExpKind &&
       node->getExp() == CallExp)
   {
      checkParams(tmp,node);
   }
}

char *SymbolTable::getTop()
{
   if (tail && tail->string)
      return tail->string;
   else
      return NULL;
}

void SymbolTable::checkParams(struct linkedList *tmp, ParseNode *node)
{
   ParseNode *pointer;
   unsigned int callParamSize = 0;
   unsigned int declParamSize;
   Type type1,type2;

   declParamSize = tmp->entry->getParamSize();
   if (declParamSize > 0)
   {
      pointer = node->getChild(0);
      while (pointer)
      {
         ++callParamSize;
         pointer=pointer->getSibling();
      }

      if (declParamSize != callParamSize)
      {
         cerr << "ERROR: Call Does Not Match Declaration: ";
         cerr << node->getString() << " " << node->getLineNo() << endl;
         ++numErrors;
         return;
      }
   }

   pointer = node->getChild(0);
   for (unsigned short int i=0; i<tmp->entry->getParamSize(); ++i)
   {
      if(pointer)
      {
         lookup(pointer);
         if (pointer->isMathOperator())
            type1 = Integer;
         else
            type1 = pointer->getType();
         type2 = tmp->entry->getParam(i);

         if (type1 != type2)
         {
            cerr << "ERROR: Call Does Not Match Declaration: ";
            cerr << node->getString() << " ";
            cerr << node->getLineNo() << endl;
            ++numErrors;
            return;
         }
         pointer = pointer->getSibling();
      }
   }    
}

#endif 

