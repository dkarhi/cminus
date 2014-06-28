// David Karhi
//
//   This is the driver for the C- compiler cm. This program performs
//   the initial stages of lexical analysis on a given input file and 
//   then parses the tokens into a parse tree. Finally, it performs 
//   semantic analysis and generates SPIM assembly code.  It optionally 
//   outputs debugging information to the screen.
//   
//
//  

#include <fstream>
#include <iostream>
#include <cstring>
#include "token.h"
#include "tokenizer.h"
#include "parser.h"


using namespace std;

int main(int argc, char *argv[])
{
   bool debug;

   // check if the program was run with the proper arguments
   if(argc == 2)
   {
      debug = false;
      // Instantiate the class
      Parse parser (argv[1], debug);
      
      return EXIT_SUCCESS;
   }
   else if (argc == 3)
   {
      // check if we are supposed to output debugging information
      if (strcmp(argv[2],"-d")==0)
      {  
         debug = true;
         Parse parser (argv[1], debug);
         return EXIT_SUCCESS;
      }
      else 
      cerr << "The compiler was not run with the proper arguments!!" << endl;
      cerr << "Please specify a single input file as follows:";
      cerr << endl << "cm <inputfile name>" << endl;
      cerr << "If you want to output debugging information, add" << endl;
      cerr << "the -d argument after the inputfile name."<< endl;
      return EXIT_FAILURE;

   }
   // if the number of arguments is not 2, then we give an error and die
   else
   {
      cerr << "The compiler was not run with the proper arguments!!" << endl;
      cerr << "Please specify a single input file as follows:";
      cerr << endl << "cm <inputfile name>" << endl;
      cerr << "If you want to output debugging information, add" << endl;
      cerr << "the -d argument after the inputfile name."<< endl;
      return EXIT_FAILURE;
   }
}


