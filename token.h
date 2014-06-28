// David Karhi
//
//   This is the header file for the token class. This class defines
//   tokens that are later used by a parser as part of a compiler.
//
//   Version 2:
//               -Changed position intitialization from 1 to 0.
//               -Mapped the output string values for TokenType to a const
//                static character array. 
//               -The token output format is different. Tokens are now output
//                on a single line.
//
#ifndef TOKEN_H
#define TOKEN_H

#define STRINGSIZE 256
#define NUMTYPES 29

typedef enum {ERROR,COMMA,ID,NUM,LEQ,IF,WHILE,ELSE,INT,RETURN,
              VOID,PLUS,MINUS,STAR,DIV,LT,GT,GEQ,EQ,NOTEQ, 
              ASSIGN,LSQ,RSQ,LCURL,RCURL,LPAR,RPAR,SEMIC,END } TokenType;

using namespace std;

class Token
{
   private:
      TokenType type;
      char cString[STRINGSIZE];
      int lineNo;
      int position;

   public:
      // this is public so that other functions can use it for output
      // without having to pass tokens
      const static char TYPE_STRINGS[NUMTYPES][STRINGSIZE];
      // default constructor
      Token();
      // setToken sets all the values of the token at once
      void setToken(TokenType tt, char *cs, int ln, int pos);
      // displayToken outputs the token to the screen
      void displayToken();
      // isMatch returns true if two tokens have the same token type
      bool isMatch(TokenType tt){return tt == type;}
      // getType returns the token type
      TokenType getType(void) {return type;}
      // getLineNo returns the line number that the token is on
      int getLineNo(void) {return lineNo;}
      // getPosition returns the position of the token on this line
      int getPosition(void) {return position;}
      // getString returns a pointer to cString
      char *getString(void) {return cString;}
      // this is the overloaded assignment operator
      Token &operator=(Token &t);

};

// These are the string values for the corresponding TokenTypes. These
// strings are used when outputting tokens to the screen.
const char Token::TYPE_STRINGS[][STRINGSIZE] = {"ERROR","COMMA","ID","NUM",
            "LEQ","IF","WHILE","ELSE","INT","RETURN","VOID","PLUS",
            "MINUS","STAR","DIV","LT","GT","GEQ","EQ","NOTEQ","ASSIGN",
           "LSQ","RSQ","LCURL","RCURL","LPAR","RPAR","SEMIC","END"};


Token::Token()
{

   cString[0] = '\0';
   type = ERROR;
   lineNo = 1;
   position = 0; 
}

void Token::setToken(TokenType tt, char *cs, int ln, int pos)
{
   type = tt;
   strcpy (cString, cs);
   lineNo = ln;
   position = pos;
}

void Token::displayToken()
{
   cout << TYPE_STRINGS[type] << " ";
   cout << cString << " ";
   cout << lineNo << " ";
   cout << position << " " << endl; 
   
}

Token& Token::operator=(Token &t)
{
   // check for self-assignment
   if(this != &t)
   {
      type = t.getType();
      strcpy (cString, t.getString());
      lineNo = t.getLineNo();
      position = t.getPosition();
   }    

   return *this;
}

#endif

