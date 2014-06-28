// David Karhi
// 
//   This is a header file for the tokenizer class. This class defines
//   a tokenizer that is part of the lexical analysis phase of a compiler 
//   for the c- language.
//
//   Version 2:
//               -Removed the bool isReservedWord(char*) function because
//                it was redundant and not needed. Instead, the TokenType
//                getType(char*) function was improved. Instead of returning
//                an ERROR TokenType if the reserved word is not found, 
//                it returns ID since the only time we will check for a 
//                reserved word is if we either have a reserved word or an ID.
//               -NUM tokens were not displaying because of a missing function
//                call to displayToken(). This no longer matters since the 
//                parser program determines if we output tokens or not.
//               -The void match(TokenType) function was added as well as the
//                bool isMatch(char*) function.
//               -Added a peekToken.
//               -Added a ISSPACE state and changed the flow of the
//                nextToken() function to remove loops in loops.
//               -Removed the use of the '\t' character. It is now 
//                considered the same as a single whitespace.
//
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <cctype>
#include "token.h"

// define the number of errors acceptable before exiting
#define NUMERRORS 32

using namespace std;

class Tokenizer
{
   private:
      Token token;
      Token peekToken;
      ifstream inputfile;
      int line;
      int pos;
      int numErrors;
      bool debug;

   public:
       // default constuctor
      Tokenizer(void);
      // the constructor takes a filename to be scanned
      Tokenizer(char *filename, bool dbug);
      // get the next token and output it. Returns false if
      // the EOF is reached
      bool nextToken(void);
      // returns token
      Token getToken(void) {return token;}
      // returns the TokenType for a given string. If the string
      // can not be found in the list of reserved words, then
      // it returns ID.
      TokenType getType( char *string );
      // returns true if the TokenType matches this token
      bool isMatch(TokenType tt) {return token.isMatch(tt);}
      // checks that the given TokenType matches the current token.
      // If it does, it gets the next token. If it doesn't match, then
      // we output an error.
      void match(TokenType tt);
      // returns true if the peekToken matches tt
      bool isPeekMatch(TokenType tt) {return peekToken.isMatch(tt);}
      // returns lineNo
      int getLineNo(){return token.getLineNo();}
      // returns position
      int getPosition(){return token.getPosition();}
      // returns cString
      char *getString(){return token.getString();}
      // sets inputfile
      void setInput(char*file);
      // sets line
      void setLine(int ln){line=ln;}
      // sets position
      void setPos(int position){pos=position;}
      //sets numErrors
      void setNumErrors(int num){numErrors = num;}
      // returns the number of errors
      int getNumErrors(void) {return numErrors;}
      // sets the debug flag
      void setDebug(bool dbug){debug = dbug;}
      // get the peekToken
      Token getPeek(){ return peekToken;}

}; 

Tokenizer::Tokenizer(void)
{
   // initialize line, position, and numErrors
   line=1;
   pos=0;
   numErrors = 0;

}

Tokenizer::Tokenizer(char *filename,bool dbug)
{
   // try to open the given input file
   inputfile.open(filename);
   // if we can't open the file, report the error and die
   if(inputfile.fail())
   {
      cerr << "Error opening input file!!" << endl << endl;
      exit(EXIT_FAILURE);
   }

   // initialize line, position, and numErrors
   line=1;
   pos=0;
   numErrors = 0;
   debug = dbug;
   // load this class with tokens
   nextToken();
   token=peekToken;
   nextToken();

}

void Tokenizer::setInput(char*file)
{
   // try to open the given input file
   inputfile.open(file);
   // if we can't open the file, report the error and die
   if(inputfile.fail())
   {
      // call the error function
      cerr << "ERROR: Can Not Open Input File" << endl << endl;
      exit(EXIT_FAILURE);
   }
   // load this class with tokens
   nextToken();
   token=peekToken;
   nextToken();


}

void Tokenizer::match(TokenType tt)
{
   if (!isMatch(tt))
   {
      if (isMatch(ERROR) && strcmp("Buffer",token.getString())==0)
      {
         cout << "ERROR: Buffer Overflow: ";
         token.displayToken();
      }
      else if (isMatch(ERROR) && strcmp("EOF",token.getString())==0)
      {
         cout << "ERROR: Unexpected End of File: ";
         token.displayToken();
      }
      else
      {
         cout << "ERROR: Unexpected Symbol: ";
         token.displayToken();
         cout << "   Expected: " << Token::TYPE_STRINGS[tt] << endl;
      }
      ++numErrors;
   }

   if (numErrors >= NUMERRORS)
   {
      cout << "ERROR LIMIT EXCEEDED...EXITING" << endl;
      exit (EXIT_FAILURE);
   }
   if (debug)
      token.displayToken();
   token = peekToken;
   nextToken();
}


TokenType Tokenizer::getType( char *string )
{
   if (strcmp("if",string) == 0)
      return IF;
   if (strcmp("while",string) == 0)
      return WHILE;
   if (strcmp("else",string) == 0)
      return ELSE;
   if (strcmp("int",string) == 0)
      return INT;
   if (strcmp("return",string) == 0)
      return RETURN;
   if (strcmp("void",string) == 0)
      return VOID;
   
   return ID;
}

bool Tokenizer::nextToken(void)
{
   typedef enum {START, DONE, INID, INNUM, ISSPACE} States;
   int len = 0;
   int startPos=pos;
   States state = START;
   char ch;
   char inputString[STRINGSIZE];
   bool inComment;
   TokenType tokenType;

   while(state != DONE)
   {
      if (!inputfile.get(ch))
      {
         peekToken.setToken(END,"\0",line,pos);
         return false;
      }
      // determine what state we are in   
      if (isspace(ch)) 
      {
         if ( ch == '\n' )
         {
            line++;
            pos=0;
         }
         else
         {
            ++pos;
         }
         state=ISSPACE;
      } 
      else if (isalpha(ch))
      {
         state=INID;
         startPos=pos;
         ++pos;
      }
      else if (isdigit(ch))
      {
         state=INNUM;
         startPos=pos;
         ++pos;
      }
      else
         state=START;
 
      // process the state we are in
      switch(state)
      {
         case ISSPACE:
            break;
         case START:
            startPos = pos;
            ++pos;
            if(!isspace(ch))
            {
               switch(ch)
               {
                  case ',':
                     peekToken.setToken(COMMA,"\0",line,startPos);
                     state=DONE;
                     break;
                  case '+':
                     peekToken.setToken(PLUS,"\0",line,startPos);
                     state=DONE;
                     break;
                  case '-':
                     peekToken.setToken(MINUS,"\0",line,startPos);
                     state=DONE;
                     break;
                  case '*':
                     peekToken.setToken(STAR,"\0",line,startPos);
                     state=DONE;
                     break;
                  case '[':
                     peekToken.setToken(LSQ,"\0",line,startPos);
                     state=DONE;
                     break;
                  case ']':
                     peekToken.setToken(RSQ,"\0",line,startPos);
                     state=DONE;
                     break;
                  case '{':
                     peekToken.setToken(LCURL,"\0",line,startPos);
                     state=DONE;
                     break;
                  case '}':
                     peekToken.setToken(RCURL,"\0",line,startPos);
                     state=DONE;
                     break;
                  case '(':
                     peekToken.setToken(LPAR,"\0",line,startPos);
                     state=DONE;
                     break;
                  case ')':
                     peekToken.setToken(RPAR,"\0",line,startPos);
                     state=DONE;
                     break;
                  case ';':
                     peekToken.setToken(SEMIC,"\0",line,startPos);
                     state=DONE;
                     break;
                  case '<':
                     inputfile.get(ch);
                     ++pos;
                     if(ch == '=')
                     {
                        peekToken.setToken(LEQ,"\0",line,startPos);
                     }
                     else
                     {
                        --pos;
                        inputfile.putback(ch);
                        peekToken.setToken(LT,"\0",line,startPos);
                     }
                     state=DONE;
                     break;
                  case '>':
                     inputfile.get(ch);
                     ++pos;
                     if (ch == '=')
                     {
                        peekToken.setToken(GEQ,"\0",line,startPos);
                     }
                     else
                     {
                        --pos;
                        inputfile.putback(ch);
                        peekToken.setToken(GT,"\0",line,startPos);
                     }
                     state=DONE;
                     break;
                   case '=':
                     inputfile.get(ch);
                     ++pos;
                     if(ch == '=')
                     {
                        peekToken.setToken(EQ,"\0",line,startPos);
                     }
                     else
                     {
                        inputfile.putback(ch);
                        --pos;
                        peekToken.setToken(ASSIGN,"\0",line,startPos);
                     }
                     state=DONE;
                     break;
                   case '!':
                     inputfile.get(ch);
                     ++pos;
                     if(ch == '=')
                     {
                        peekToken.setToken(NOTEQ,"\0",line,startPos);
                     }
                     else
                     {
                        inputfile.putback(ch);
                        --pos;
                        peekToken.setToken(ERROR,"!",line,startPos);
                     }
                     state=DONE;
                     break;
                  case '/':
                     inputfile.get(ch);
                     ++pos;
                     if( ch == '*')
                     {
                        inputfile.get(ch);
                        ++pos; 
                        
                        inComment = true;
                        while ( inComment )
                        {
                           if ( ch == '\n' )
                           {
                              ++line;
                              pos=1;
                           }
                              
                           if ( inputfile.eof() )
                           {
                              peekToken.setToken(ERROR,"EOF",line,startPos);
                              return false;
                           }
                           if ( ch == '*')
                           {
                              inputfile.get(ch);
                              ++pos;
                              if ( ch == '/' )
                              {
                                 inComment = false;
                              }
                              else
                              {
                                 inputfile.putback(ch);
                                 --pos;
                              }
                           }
                           inputfile.get(ch);
                           ++pos;
                        }
                        state=START;    
                     }
                     else
                     {
                        inputfile.putback(ch);
                        --pos;
                        peekToken.setToken(DIV,"\0",line,startPos);
                        state=DONE;
                     }
                     break;   
                  default:
                     inputString[len++]=ch;

                     // check for buffer overflow
                     if (len > STRINGSIZE)
                     {
                        peekToken.setToken(ERROR,"Buffer",line,startPos);
                     }
                     else
                     {
                        inputString[len]='\0';
                        peekToken.setToken(ERROR,inputString,line,startPos);
                     }
                     
                     state=DONE;
                     break;
               }
            }
            break;
         case INID:
            while (isalpha(ch))
            {
               inputString[len++]=ch;
               
               // check for buffer overflow
               if (len > STRINGSIZE)
               {
                  peekToken.setToken(ERROR,"Buffer",line,startPos);
                  state=DONE;
                  return true;
               }

               inputfile.get(ch);
               ++pos;
            }
            
            state=DONE;
            inputString[len]='\0';
            tokenType = getType (inputString);
            
            if (tokenType == ID)
            {
               peekToken.setToken(tokenType,inputString,line,startPos);
            }
            else
            {
               peekToken.setToken(tokenType,"\0",line,startPos);
            }

            --pos;
            inputfile.putback(ch);
            break;
         case INNUM:
            while (isdigit(ch))
            {
               inputString[len++]=ch;
               // check for buffer overflow
               if (len > STRINGSIZE)
               {
                  peekToken.setToken(ERROR,"Buffer",line,startPos);
                  state=DONE;
                  return true;
               }

               inputfile.get(ch);
               ++pos;
            }

            state=DONE;
            inputString[len]='\0';
            peekToken.setToken(NUM,inputString,line,startPos);
            inputfile.putback(ch);
            --pos;
            break;
         case DONE:
            break;
         default:
            inputString[len++] = ch;
            
            // check for buffer overflow
            if (len > STRINGSIZE)
            {
               peekToken.setToken(ERROR,"Buffer",line,startPos);
               state=DONE;
               break;
            }
            else
            {
               inputString[len]='\0';
	       peekToken.setToken(ERROR,inputString,line,startPos);
               state=DONE;
               break;
            }
      }
   }
   return true;
}

#endif
