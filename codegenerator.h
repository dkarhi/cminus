// By: David Karhi
//
//   This is a code generator for a C- compiler
//
#ifndef CODE_GEN_H
#define CODE_GEN_H
#include <fstream>
#include "parsenode.h"

typedef enum {lw,sw,add,sub,jr,jl,li,scall,move} Opcode;    

#define NUM_V_REGS 2
#define NUM_A_REGS 4
#define NUM_T_REGS 10
#define NUM_S_REGS 8
#define NUM_REGS 28

using namespace std;

class CodeGenerator
{
   private:
      int numV;
      int numA;
      int numT; 
      int tPointer;
      int numS;      
      // used is true if the register is being used. The index
      // corresponds to the enumerated type Reg.
      bool used[NUM_REGS];
      ofstream outputFile;
      char label[STRINGSIZE];
      char exitLabel[STRINGSIZE];
      char exitJumpString[STRINGSIZE];
      char *whilelabel;
      char *endlabel;
      char *elselabel;
      char *endiflabel;
      int numParameters;
      int numMemLocations;
      int loopNum;
   public:
      CodeGenerator();
      char *generateLabel(char *s);
      char *generateExitLabel(char *s);
      char *generateJumpLabel(char *s);
      char *functionLabel(char *s);
      void generateFunctionCode(ParseNode *);
      void generateCode(ParseNode *root,int numErrors);
      void generateSpim(ParseNode *treeNode);
      void writeLabel(char *label);
      void writeReturn(ParseNode *node);
      void printReg(int r);
      Reg getRegister(void);
      char* itoa (int);
 
};

CodeGenerator::CodeGenerator()
{
   numV = 0;
   numA = 0;
   numT = 0;
   numS = 0;
   tPointer = 0;
   numParameters = 0;
   numMemLocations = 0;
   loopNum = 0;

   for (unsigned int i = 0; i < NUM_REGS; ++i)
      used[i] = false;
}

char* CodeGenerator::itoa (int n){
   int i=0,j;
   char* s;
   char* u;

   s= (char*) malloc(17);
   u= (char*) malloc(17);
  
   do
   {
      s[i++]=(char)( n%10+48 );
      n-=n%10;
   }
   while((n/=10)>0);
   for (j=0;j<i;j++)
      u[i-1-j]=s[j];

   u[j]='\0';
   return u;
}


void CodeGenerator::generateCode(ParseNode *root,int numErrors)
{
   if (numErrors > 0)
   {
      cerr << "Can Not Continue With Errors." << endl;
      cerr << "Exiting..." << endl;
      exit (EXIT_FAILURE);
   }   
   outputFile.open("output.asm");
   if(outputFile.fail())
   {
      cerr << "Error opening output file!!" << endl << endl;
      exit(EXIT_FAILURE);
   }

   generateSpim(root);
   
}

void CodeGenerator::generateSpim(ParseNode *treeNode)
{
   int offset = 0; 
   int paramLocation;

   if(!treeNode) return;
   switch(treeNode->getNodeKind())
   {
      case DeclKind:
         if (strcmp(treeNode->getString(),"input") == 0 ||
             strcmp(treeNode->getString(),"output") == 0)
            break;

         if(treeNode->isFuncBegin())
         {
            outputFile << ".text" << endl;
            strcpy(label, generateLabel(treeNode->getString()));
            strcpy(exitLabel, generateExitLabel(treeNode->getString()));
            strcpy(exitJumpString, generateJumpLabel(treeNode->getString()));
            writeLabel(label);
            generateFunctionCode(treeNode);
         }
         else if(treeNode->getScope() == 0)
         {
            outputFile << ".data" << endl;
            strcpy(label,generateLabel(treeNode->getString()));
            writeLabel(label);
            int size;
            if (treeNode->getChild(0))
               size = treeNode->getChild(0)->getNum();
            else
               size = 1;

            size *= 4;
            
            outputFile << "   .space " << size << endl;
         }
         break;
      case StmtKind:
         switch(treeNode->getStmt())
         {
            case ReturnStmt:
               writeReturn(treeNode);
               break;
            case WhileStmt:
               whilelabel=generateLabel("L");
               writeLabel(whilelabel);
               if (treeNode->getChild(0)->isNum())
               {  
                  outputFile << "   add $t5, $zero, ";
                  outputFile << treeNode->getChild(0)->getNum();
                  outputFile << "   beqz $t5, L_END" << loopNum<< endl;
               } 
               else if (treeNode->getChild(0)->isVar())
               {
                  if (treeNode->getChild(0)->getParamNum() == MAXPARAMS)
                  {
                     paramLocation = treeNode->getChild(0)->getMem();
                     if (treeNode->getChild(0)->getChild(0))
                     {
                        paramLocation += treeNode->getChild(0)->getChild(0)->getNum();
                     }
                     paramLocation *= 4;
                     outputFile << "   lw $t5, " << -paramLocation << "($fp)" << endl;

                  }
                  else
                  {
                     paramLocation = treeNode->getChild(0)->getParamNum();
                     if (treeNode->getChild(0)->getChild(0))
                        paramLocation -= treeNode->getChild(0)->getChild(0)->getNum();
                     paramLocation *= 4;
                     paramLocation =  ((numParameters - treeNode->getChild(0)->getParamNum()) * 4) - paramLocation;
                     outputFile << "   lw $t5, " << paramLocation << "($fp)" << endl;
                  }
                  outputFile << "   beqz $t5, L_END" << loopNum << endl;
               }
               else if (treeNode->getChild(0)->isCall())
               {
                  generateSpim(treeNode->getChild(0));
                  outputFile << "   move $t5, $a3" << endl;
                  outputFile << "   beqz $t5, L_END" << loopNum << endl;
               }
               else if (treeNode->getChild(0)->isRelOp())
               {
                  generateSpim(treeNode->getChild(0)->getChild(0));
                  if (treeNode->getChild(0)->getChild(0)->isCall())
                  {
                     outputFile << "   move $t4, $a3" << endl;
                  }
                  else
                     outputFile << "   move $t4, $t5" << endl;
                  
                  generateSpim(treeNode->getChild(0)->getChild(1));
                  if (treeNode->getChild(0)->getChild(1)->isCall())
                     outputFile << "   move $t5, $a3" << endl;
 
                  switch (treeNode->getChild(0)->getTokenType())
                  {
                     case LEQ:
                        outputFile << "   bgt $t4, $t5, " ;
                        outputFile << "L_END" << loopNum << endl;
                        break;
                     case LT:
                        outputFile << "   bge $t4, $t5, " ;
                        outputFile << "L_END" << loopNum << endl;
                        break;
                     case GT:
                        outputFile << "   ble $t4, $t5, " ;
                        outputFile << "L_END" << loopNum << endl;
                        break;
                     case GEQ:
                        outputFile << "   blt $t4, $t5, " ;
                        outputFile << "L_END" << loopNum << endl;
                        break;
                     case EQ:
                        outputFile << "   bne $t4, $t5, " ;
                        outputFile << "L_END" << loopNum << endl;
                        break;
                     case NOTEQ:
                        outputFile << "   beq $t4, $t5, " ;
                        outputFile << "L_END" << loopNum << endl;
                        break;
                     default:
                        break;
                  }
               }
 
               generateSpim(treeNode->getChild(1));
               outputFile << "   j L" << loopNum << endl;
               endlabel=generateLabel("L_END");
               writeLabel(endlabel);
               break;
            case IfStmt:
               if (treeNode->getChild(0)->isNum())
               {
                  outputFile << "   add $t5, $zero, ";
                  outputFile << treeNode->getChild(0)->getNum();
                  outputFile << "   beqz $t5, ELSE" << loopNum<< endl;
               }
               else if (treeNode->getChild(0)->isVar())
               {
                  if (treeNode->getChild(0)->getParamNum() == MAXPARAMS)
                  {
                     paramLocation = treeNode->getChild(0)->getMem();
                     if (treeNode->getChild(0)->getChild(0))
                     {
                        paramLocation += treeNode->getChild(0)->getChild(0)->getNum();
                     }
                     paramLocation *= 4;
                     outputFile << "   lw $t5, " << -paramLocation << "($fp)" << endl;

                  }
                  else
                  {
                     paramLocation = treeNode->getChild(0)->getParamNum();
                     if (treeNode->getChild(0)->getChild(0))
                        paramLocation -= treeNode->getChild(0)->getChild(0)->getNum();
                     paramLocation *= 4;
                     paramLocation =  ((numParameters - treeNode->getChild(0)->getParamNum()) * 4) - paramLocation;
                     outputFile << "   lw $t5, " << paramLocation << "($fp)" << endl;
                  }
                  outputFile << "   beqz $t5, ELSE" << loopNum << endl;
               }
               else if (treeNode->getChild(0)->isCall())
               {
                  generateSpim(treeNode->getChild(0));
                  outputFile << "   move $t5, $a3" << endl;
                  outputFile << "   beqz $t5, ELSE" << loopNum << endl;
               }
               else if (treeNode->getChild(0)->isRelOp())
               {
                  generateSpim(treeNode->getChild(0)->getChild(0));
                  if (treeNode->getChild(0)->getChild(0)->isCall())
                  {
                     outputFile << "   move $t4, $a3" << endl;
                  }
                  else
                     outputFile << "   move $t4, $t5" << endl;

                  generateSpim(treeNode->getChild(0)->getChild(1));
                  if (treeNode->getChild(0)->getChild(1)->isCall())
                     outputFile << "   move $t5, $a3" << endl;
                
                  switch (treeNode->getChild(0)->getTokenType())
                  {
                     case LEQ:
                        outputFile << "   bgt $t4, $t5, " ;
                        outputFile << "ELSE" << loopNum << endl;
                        break;
                     case LT:
                        outputFile << "   bge $t4, $t5, " ;
                        outputFile << "ELSE" << loopNum << endl;
                        break;
                     case GT:
                        outputFile << "   ble $t4, $t5, " ;
                        outputFile << "ELSE" << loopNum << endl;
                        break;
                     case GEQ:
                        outputFile << "   blt $t4, $t5, " ;
                        outputFile << "ELSE" << loopNum << endl;
                        break;
                     case EQ:
                        outputFile << "   bne $t4, $t5, " ;
                        outputFile << "ELSE" << loopNum << endl;
                        break;
                     case NOTEQ:
                        outputFile << "   beq $t4, $t5, " ;
                        outputFile << "ELSE" << loopNum << endl;
                        break;
                     default:
                        break;
                  }
               }

               generateSpim(treeNode->getChild(1));
               outputFile << "   j END_IF" << loopNum << endl;
               elselabel=generateLabel("ELSE");
               writeLabel(elselabel);
               generateSpim(treeNode->getChild(2));
               endiflabel=generateLabel("END_IF");
               writeLabel(endiflabel);
               break;
            case CmpStmt:
               generateSpim(treeNode->getChild(0));
               break;
            case ExpStmt:
               generateSpim(treeNode->getChild(0));
               break;
            case FuncStmt:
               generateSpim(treeNode->getChild(0));
            default: break;
	 }
         break;
      case ExpKind:
         switch(treeNode->getExp()) 
         {
            case CallExp:
               if (strcmp(treeNode->getString(),"input") == 0)
               {
                  outputFile << "   li $v0, 5" << endl;
                  outputFile << "   syscall" << endl;
               } 
               else if (strcmp(treeNode->getString(),"output") == 0)
               {
                  // load the parameter into a0
                  if (treeNode->getChild(0)->getParamNum() == MAXPARAMS)
                  {
                     paramLocation = treeNode->getChild(0)->getMem();
                     if (treeNode->getChild(0)->getChild(0))
                     {
                        paramLocation += treeNode->getChild(0)->getChild(0)->getNum();
                     }
                     paramLocation *= 4;
                     outputFile << "   lw $a0, " << -paramLocation << "($fp)" << endl;
                     outputFile << "   li $v0, 1" << endl;
                     outputFile << "   syscall" << endl;
  
                  }
                  else
                  {
                     paramLocation = treeNode->getChild(0)->getParamNum();
                     if (treeNode->getChild(0)->getChild(0))
                        paramLocation -= treeNode->getChild(0)->getChild(0)->getNum();
                     paramLocation *= 4;
                     outputFile << "   lw $a0, " << paramLocation << "($fp)" << endl;
                     outputFile << "   li $v0, 1" << endl;
                     outputFile << "   syscall" << endl;
                  }
               }
               else
               {
                  int argOffset = 0;
                  int numArguments = 0;
                  int argStackSize;
                  int pointer;
                  ParseNode *tmp;

                  tmp = treeNode->getChild(0);
                  while (tmp)
                  {
                    ++numArguments;
                    tmp = tmp->getSibling();
                  }

                  // Allocate enough space on the stack for the arguments
                  argStackSize = numArguments * 4;
                  outputFile << "   sub $sp, $sp, " << argStackSize << endl;

                  tmp = treeNode->getChild(0);
                  while(tmp)
                  {
                     if (tmp->isNum())
                     { 
                        if (numT >= NUM_T_REGS)
                           tPointer=0;
    
                        if (numA == NUM_A_REGS)
                           pointer = tPointer+7;
                        else
                        {  
                           pointer = (argOffset/4)+3;
                           ++numA;
                        }

                        outputFile << "   add $";
                        printReg(pointer); 
                        outputFile << ", $zero, ";
                        outputFile << tmp->getNum() << endl; 
                        outputFile << "   sw $"; 
                        printReg(pointer); 
                        outputFile << ", ";
                        outputFile << argOffset << "($sp)" << endl;
                        argOffset += 4;     
                     }
                     else 
                     {
                        if (tmp->getChild(0))
                        {
                           offset = tmp->getChild(0)->getNum();
                           offset *= 4;
                        }
                        else 
                           offset = 0;
                        
                        if (numT >= NUM_T_REGS)
                           tPointer=0;

                        if (numA == NUM_A_REGS)
                           pointer = tPointer+7;
                        else
                        {
                           pointer = (argOffset/4)+3;
                           ++numA;
                        }

                        if (tmp->getParamNum() == MAXPARAMS)
                        {
                           offset += (tmp->getMem() * 4);
                           offset += 8;
                           
                           outputFile << "   lw $";
                           printReg(pointer);
                           outputFile << -offset;
                           outputFile << "($fp)" << endl;
                           outputFile << "   sw $"; 
                           printReg(pointer);
                           outputFile << argOffset << "($sp)" << endl; 
                           argOffset += 4;  
                        }      
                        else
                        {
                           offset =  ((numArguments - tmp->getParamNum()) * 4) - offset;
                           outputFile << "   lw $";
                           printReg(pointer);
                           outputFile << offset;
                           outputFile << "($fp)" << endl;
                           outputFile << "   sw $";
                           printReg(pointer);
                           outputFile << argOffset << "($sp)" << endl;
                           argOffset += 4;
                        }
                     }
                     tmp = tmp->getSibling();
                  }
                  outputFile << "   jal " << treeNode->getString() << endl; 
                  outputFile << "   add $sp, $sp, " << argStackSize << endl;
               }
               break;
            case OpExp:
               if (treeNode->getChild(0)->isCall())
               {
                  generateSpim(treeNode->getChild(0));
                  outputFile << "   move $s1, $v0" << endl;
               }
               if (treeNode->getChild(1)->isCall())
               {
                  generateSpim(treeNode->getChild(1));
                  outputFile << "   move $s2, $v0" << endl;
               }  

               if (treeNode->getChild(0)->isNum())
               {
                  outputFile << "   add $s1, $zero, ";
                  outputFile << treeNode->getChild(0)->getNum() << endl;
               }
               if (treeNode->getChild(1)->isNum())
               {
                  outputFile << "   add $s2, $zero, ";
                  outputFile << treeNode->getChild(0)->getNum() << endl;
               }
               if (treeNode->getChild(0)->isVar())
               {
                  if (treeNode->getChild(0)->getChild(0))
                  {
                     offset = treeNode->getChild(0)->getChild(0)->getNum();
                     offset *= 4;
                  }
                  else 
                     offset = 0;

                  if (treeNode->getChild(0)->getParamNum() == MAXPARAMS)
                  {
                     offset += (treeNode->getChild(0)->getMem() * 4);
                     offset += 8;
                     
                     outputFile << "   lw $s1, " << -offset;
                     outputFile <<"($fp)" << endl;
                  }
                  else
                  {
                     offset =  ((numParameters - treeNode->getChild(0)->getParamNum()) * 4) - offset;
                     
                     outputFile << "   lw $s1, " << offset;
                     outputFile <<"($fp)" << endl;
                  }
               }
               if (treeNode->getChild(1)->isVar())
               {
                  if (treeNode->getChild(1)->getChild(0))
                  {
                     offset = treeNode->getChild(1)->getChild(0)->getNum();
                     offset *= 4;
                  }
                  else
                     offset = 0;

                  if (treeNode->getChild(1)->getParamNum() == MAXPARAMS)
                  {
                     offset += (treeNode->getChild(1)->getMem() * 4);
                     offset += 8;

                     outputFile << "   lw $s2, " << -offset;
                     outputFile <<"($fp)" << endl;
                  }
                  else
                  {
                     offset =  ((numParameters - treeNode->getChild(1)->getParamNum()) * 4) - offset;

                     outputFile << "   lw $s2, " << offset;
                     outputFile <<"($fp)" << endl;
                  }
               }
               if (treeNode->getChild(0)->isMathOperator())
               {
                  generateSpim(treeNode->getChild(0));
                  
                  outputFile << "   move $s1, $s3" << endl; 
               }
               if (treeNode->getChild(1)->isMathOperator())
               {
                  generateSpim(treeNode->getChild(0));

                  outputFile << "   move $s2, $s3" << endl;
               }
               
               switch (treeNode->getTokenType()) 
               {
                  case PLUS:
                     outputFile << "   add $s3, $s1, $s2" << endl;
                     break;
                  case MINUS:
                     outputFile << "   sub $s3, $s1, $s2" << endl;
                     break;
                  case STAR:
                     outputFile << "   mul $s3, $s1, $s2" << endl;
                     break;
                  case DIV:
                     outputFile << "   div $s3, $s1, $s2" << endl;
                     break;
                  default:
                     break;
               }
            case AssignExp:
               if (treeNode->getChild(1)->isCall())
               {
                  generateSpim(treeNode->getChild(1));
                  outputFile << "   add $s0, $zero, $v0" << endl;
               }
               else if (treeNode->getChild(1)->isNum())
               {
                  outputFile << "   add $s0, $zero, ";
                  outputFile << treeNode->getChild(1)->getNum() << endl;
               }   
               else if (treeNode->getChild(1)->isVar())
               {
                  if(treeNode->getChild(1)->getChild(0))
                  {
                     offset = treeNode->getChild(1)->getChild(0)->getNum();
                     offset *= 4; 
                  }
                  else 
                     offset = 0;
                  
                  if (treeNode->getChild(1)->getParamNum() == MAXPARAMS)
                  {
                     offset += (treeNode->getChild(1)->getMem() * 4);
                     offset += 8;

                     outputFile << "   lw $s0, " << -offset;
                     outputFile << "($fp)" << endl;
                  }
                  else 
                  {
                     offset =  ((numParameters - treeNode->getParamNum()) * 4) - offset;
                     outputFile << "   lw $s0, " << offset;
                     outputFile << "($fp)" << endl;
                  }   
               }
               else if (treeNode->getChild(1)->isMathOperator())
               {
                  generateSpim(treeNode->getChild(1));
                  outputFile << "   move $s0, $s3" << endl;
               }
               
               if (treeNode->getChild(0)->getChild(0))
               {
                  offset = treeNode->getChild(0)->getChild(0)->getNum();
                  offset *= 4;
               }
               else
                  offset = 0;

               if (treeNode->getChild(0)->getParamNum() == MAXPARAMS)
               {
                  offset += (treeNode->getChild(0)->getMem() * 4);
                  offset += 8;

                  outputFile << "   sw $s0, " << -offset;
                  outputFile << "($fp)" << endl;
               }
               else
               {
                  offset = ((numParameters - treeNode->getParamNum()) * 4) - offset;
                  outputFile << "   sw $s0, " << offset;
                  outputFile << "($fp)" << endl;
               }
               break;
            case NumExp: 
               outputFile << "   add $t5, $zero, ";
               outputFile << treeNode->getNum() << endl;
               break;
            case VarExp: 
               if (treeNode->getChild(0))
               {
                  offset = treeNode->getChild(0)->getNum();
                  offset *= 4;
               }
               else
                  offset = 0;

               if (treeNode->getParamNum() == MAXPARAMS)
               {
                  offset += (treeNode->getMem() * 4);
                  outputFile << "   lw $t5, " << -offset << "($fp)" << endl;

               }
               else
               {
                  offset = ((numParameters - treeNode->getParamNum()) * 4) - offset;
                  outputFile << "   lw $t5, " << offset << "($fp)" << endl;
               }
               break;
            default: 
               break;
         }
      }
   
   generateSpim(treeNode->getSibling());
   return;
}

char *CodeGenerator::generateLabel( char* s)
{
   char *returnString = label;

   strcpy(returnString,s);
   
   if (strcmp(s, "L") == 0 || strcmp(s, "ELSE") == 0)
      strcat(returnString, itoa(loopNum));
   if ( strcmp(s, "L_END") == 0 || strcmp(s, "END_IF") == 0)
   {
      strcat(returnString, itoa(loopNum));
      ++loopNum;
   }
   strcat(returnString,":");

   return returnString;
}

char *CodeGenerator::generateExitLabel( char* s)
{
   char *returnString = exitLabel;

   strcpy (returnString,s);
   strcat (returnString,"_exit:");

   return returnString;
}

char *CodeGenerator::generateJumpLabel( char* s)
{
   char *returnString = exitJumpString;

   strcpy (returnString,s);
   strcat (returnString,"_exit");

   return returnString;
}

void CodeGenerator::writeLabel(char *label)
{
   outputFile << label << endl;
}

void CodeGenerator::generateFunctionCode(ParseNode* node)
{
   // input and output functions are ignored
   if (strcmp(node->getString(),"input") == 0 || 
       strcmp(node->getString(),"output") == 0)
      return;

   numMemLocations=0;
   ParseNode* tmp;
   int saveS = numS;
   
   numParameters = 0;
   // this is a function declaration, so first we need to figure out
   // how many variables and parameters are declared so that we can 
   // allocate the stack.
   tmp = node->getChild(0);
   while (tmp)
   {
      ++numParameters;
      tmp = tmp->getSibling();
   }
  
   tmp = node->getChild(1); 
   if(tmp)
      tmp = node->getChild(0);
  
   while (tmp)
   {  
      // if this is a variable declaration, allocate space for the variable
      if (tmp->isVarDecl()) {
         // if this is an array, add the index so we can allocate enough space
         if (tmp->getChild(0))
            numMemLocations += tmp->getChild(0)->getNum();
         else
            ++numMemLocations;
      }
 
      tmp = tmp->getSibling();
   }
   
   // check how many S registers are used so we can save them
   if (numS > 0)
      numMemLocations+=numS;

   // finally, add 2 to the number of memory locations to save the 
   // return address and frame pointer. Multiply the whole thing by
   // 4 to get the actual number of bytes needed.      
   numMemLocations += 2;
   numMemLocations *= 4;
 
   // write the memory allocation
   outputFile << "   sub $sp, $sp, " << numMemLocations << endl;

   // save the frame pointer and the return address
   outputFile << "   sw $fp, " << numMemLocations - 4 << "($sp)" << endl; 
   outputFile << "   sw $ra, " << numMemLocations - 8 << "($sp)" << endl;
   
   // now the new frame pointer is assigned
   outputFile << "   add $fp, $sp, " << numMemLocations << endl;

   // save and clear the S registers
   int savedLocation = numMemLocations - 8;
   if (numS > 0)
   {
      for ( int i = 17; i < 25; ++i)
      {
         if (used[i])
         {
            outputFile << "   sw $";
            printReg(i);
            outputFile << ", ";
            outputFile << savedLocation << "$(sp)" << endl;

            savedLocation += 4;
            used[i] = false;
         } 
      } 
      numS = 0;
   }
   
   strcpy(exitLabel, generateExitLabel(node->getString()));
   generateSpim(node->getChild(1));
   writeLabel(exitLabel);

   int index;
   // restore the S registers
   if (saveS > 0)
   {
      numS = saveS;
      saveS *= 4;
      savedLocation += 4;
      while (saveS > 0)
      {
         // figure out which register we need to restore
         index = (17+(saveS/4));
         saveS -= 4;
         outputFile << "   lw ";
         printReg(index);
         outputFile << ", " << savedLocation << "($sp)" << endl; 
         used[index] = true;
         savedLocation -= 4;
      } 
   }

   // restore the return address and frame pointer, then restore the 
   // stack    
   outputFile << "   lw $fp, " << numMemLocations - 4 << "($sp)" << endl;
   outputFile << "   lw $ra, " << numMemLocations - 8 << "($sp)" << endl;

   outputFile << "   add $sp, $sp, " << numMemLocations << endl;

   // return
   outputFile << "   jr $ra" << endl;
   
}  

void CodeGenerator::writeReturn(ParseNode *node)
{
   int offset = 0;
   int registerVal;
   int variableLocation;

   if (node->getChild(0))
   {
      if (node->getChild(0)->isNum())
      {
         outputFile << "   add $v0, $zero, " ;
         outputFile << node->getChild(0)->getNum() << endl;
      }   
      else if (node->getChild(0)->isVar())
      {
         // check if this is an array value
         if (node->getChild(0)->getChild(0))
            offset = node->getChild(0)->getChild(0)->getNum();
         offset *= 4;

         if (node->getChild(0)->getScope() == 0)
         {
            // find an available register. If the registers are full,
            // replace the first value we inserted.
            if (numT >= NUM_T_REGS)
               tPointer = 0;

            registerVal = 7 + tPointer;
            outputFile << "   add $";
            printReg(registerVal); 
            outputFile << ", $zero, ";
            outputFile << offset << endl;
            outputFile << "   lw $v0, " << node->getChild(0)->getString();
            outputFile << "($";
            printReg(registerVal);
            outputFile << ")" << endl;   
         }
         else
         {
            if (node->getChild(0)->getParamNum() == MAXPARAMS)
            {
               variableLocation = node->getMem();
               // offset for the return address and the saved frame pointer
               // since we are basing this off of the frame pointer
               variableLocation += 3;
               variableLocation *= 4;
               variableLocation += offset;
               
               outputFile << "   lw $v0, " << -variableLocation; 
               outputFile << "($fp)" << endl; 
            }
            // if we get here it is a parameter that we are looking for
            else
            {
               variableLocation = node->getParamNum();
               variableLocation = numParameters - variableLocation;

               variableLocation *= 4;
               variableLocation -= offset;

               outputFile << "   lw $v0, " << variableLocation;
               outputFile << "($fp)" << endl;
            }
         }
      }
      else if (node->getChild(0)->isCall() ||
               node->getChild(0)->isMathOperator())
      {
	 // this will put the result in v0
         generateSpim(node->getChild(0));
      }
   }
   outputFile << "   j " << exitJumpString  <<  endl;
}

void CodeGenerator::printReg(int r)
{
   switch (r)
   {
      case 0:
         outputFile << "zero";
         break;
      case 1:
         outputFile << "v0";
         break;
      case 2:
         outputFile << "v1";
         break;
      case 3:
         outputFile << "a0";
         break;
      case 4:
         outputFile << "a1";
         break;
      case 5:
         outputFile << "a2";
         break;
      case 6:
         outputFile << "a3";
         break;
      case 7:
         outputFile << "t0";
         break;
      case 8:
         outputFile << "t1";
         break;
      case 9:
         outputFile << "t2";
         break;
      case 10:
         outputFile << "t3";
         break;
      case 11:
         outputFile << "t4";
         break;
      case 12:
         outputFile << "t5";
         break;
      case 13:
         outputFile << "t6";
         break;
      case 14:
         outputFile << "t7";
         break;
      case 15:
         outputFile << "t8";
         break;
      case 16:
         outputFile << "t9";
         break;
      case 17:
         outputFile << "s0";
         break;
      case 18:
         outputFile << "s1";
         break;
      case 19:
         outputFile << "s2";
         break;
      case 20:
         outputFile << "s3";
         break;
      case 21:
         outputFile << "s4";
         break;
      case 22:
         outputFile << "s5";
         break;
      case 23:
         outputFile << "s6";
         break;
      case 24:
         outputFile << "s7";
         break;
      case 25:
         outputFile << "sp";
         break;
      case 26:
         outputFile << "fp";
         break;
      case 27:
         outputFile << "ra";
         break;
      default:
         break;

   }
}
#endif
