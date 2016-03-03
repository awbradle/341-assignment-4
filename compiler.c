/*
Andrew Bradley
COSC 341 Winter 2016
Compiler for XMicro language

XMicro is an extention of the Micro language
This compiler uses and modifies a compiler for Micro
originally written by Prof. Suchindran Maniccam at 
Eastern Michigan University

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*constants for true and false*/
#define FALSE 0
#define TRUE 1

/*enumerated types for token types*/
typedef enum
{
	ID, INTLITERAL, MAIN, READ, WRITE, IF, ELSE, WHILE,
	PLUSOP, MINUSOP, ASSIGNOP, LPAREN, RPAREN, MULTOP, DIVOP,
    COMMA, SEMICOLON, SCANEOF, GREATER, GREATEQ, LESS,
	LESSEQ, EQUALS, NOTEQ, LCURL, RCURL
}token;

/*string representations of the enum types*/
char *tokens[] = {"ID","INTLITERAL","MAIN","READ","WRITE", "IF", "ELSE",
				  "WHILE","PLUSOP","MINUSOP","ASSIGNOP","LPAREN","RPAREN",
				  "MULTOP","DIVOP","COMMA","SEMICOLON","SCANEOF",
				  "GREATER", "GREATEQ", "LESS","LESSEQ", "EQUALS", "NOTEQ",
				  "LCURL","RCURL"};

/*functions delcarations related to scanner*/
token scanner();
void clear_buffer();
void buffer_char(char c);
token check_reserved();
void lexical_error();

/*function declarations related to parser*/
void parser();
void program();
void statement_list();
void statement();
void id_list();
void expression_list();
void expression();
void term();
void bool();
void relop();
void add_op();
void mult_op();
void match(token tok);
void syntax_error();

/*global variables*/
FILE *fin;				//source file
FILE *fout;				//source file
token next_token;		//next token in source file
char token_buffer[100];	//token buffer
int token_ptr;			//buffer pointer
int line_num = 1;		//line number in source file
int error = FALSE;		//flag to indicate error

/*****************************************************************************/

/*returns next token from source file*/
token scanner()
{
	char c;
	
	clear_buffer();
	
	while(TRUE)
	{
		c = getc(fin);
		
		if(c == EOF)
			return SCANEOF;
		
		else if(isspace(c))
		{
			if (c == '\n')
				line_num = line_num + 1;
		}
		
		else if (isalpha(c))
		{
			buffer_char (c);
			c = getc (fin);
			while (isalnum(c) || c == '_')
			{
				buffer_char(c);
				c = getc(fin);
			}
			ungetc(c, fin);
			return check_reserved();
		}
		
		else if (isdigit(c))
		{
			buffer_char(c);
			c = getc(fin);
			while(isdigit(c))
			{
				buffer_char(c);
				c = getc(fin);
			}
			ungetc(c,fin);
			return INTLITERAL;
		}
		
		else if (c == '(')
			return LPAREN;
		
		else if (c == ')')
			return RPAREN;
		else if (c == '{')
			return LCURL;
		
		else if (c == '}')
			return RCURL;
		else if (c == ',')
			return COMMA;
			
		else if (c == ';')
			return SEMICOLON;
			
		else if (c == '+')
			return PLUSOP;
		else if (c == '-')
			return MINUSOP;
		else if (c == '*')
			return MULTOP;	
		else if (c == '/')
			{
				c = getc(fin);
				if(c == '/')
				{
					do
						c = getc(fin);
					while (c != '\n');
					line_num = line_num + 1;
				}
				else
				{
					ungetc(c, fin);
					return DIVOP;
				}
			}
		else if (c == '=')
			{
				c = getc(fin);
				if(c == '=')
				{
					return EQUALS;
				}
				else
				{
					ungetc(c, fin);
					lexical_error();
				}
			}
		else if (c == '!')
			{
				c = getc(fin);
				if(c == '=')
				{
					return NOTEQ;
				}
				else
				{
					ungetc(c, fin);
					lexical_error();
				}
			}
		else if (c == '<')
			{
				c = getc(fin);
				if(c == '=')
				{
					return LESSEQ;
				}
				else
				{
					ungetc(c, fin);
					return LESS;
				}
			}
		else if (c == '>')
			{
				c = getc(fin);
				if(c == '=')
				{
					return GREATEQ;
				}
				else
				{
					ungetc(c, fin);
					return GREATER;
				}
			}
		else if (c == ':')
		{
			c = getc(fin);
			if(c == '=')
				return ASSIGNOP;
			else
			{
				ungetc(c, fin);
				lexical_error();
			}
		}
		
		else
			lexical_error();
	}
}

/*****************************************************************************/

/*clear the buffer*/
void clear_buffer()
{
	token_ptr = 0;
	token_buffer[token_ptr] = '\0';
}

/*****************************************************************************/

/*appends the character to buffer*/
void buffer_char(char c)
{
	token_buffer[token_ptr] = c;
	token_ptr = token_ptr + 1;
	token_buffer[token_ptr] = '\0';
}

/*****************************************************************************/

/*checks whether buffer is a reserved word or identifier*/
token check_reserved()
{
	if(strcmp(token_buffer, "main") == 0)
		return MAIN;
	else if(strcmp(token_buffer, "if") == 0)
		return IF;
	else if(strcmp(token_buffer, "else") == 0)
		return ELSE;
	else if(strcmp(token_buffer, "while") == 0)
		return WHILE;
	else if(strcmp(token_buffer, "read") == 0)
		return READ;
	else if(strcmp(token_buffer, "write") == 0)
		return WRITE;
	else
		return ID;
}

/*****************************************************************************/

/*reports lexical error and sets error flag*/
void lexical_error()
{
	printf("lexical error in line %d/n", line_num);
	error = TRUE;
}

/*****************************************************************************/

/*parses source file*/
void parser()
{
	next_token = scanner();
	program();
	match(SCANEOF);
}

/*****************************************************************************/

/*parses a program*/
/* <program> --> begin<stmtlist>end*/
void program()
{
	match(MAIN);
	match(LCURL);
	statement_list();
	match(RCURL);
}

/*****************************************************************************/

/*parses list of statements*/
/* <stmtlist> --> <stmt>{<stmt>}*/
void statement_list()
{
	statement();
	while(TRUE)
	{
		if(next_token == ID || next_token == READ || next_token == WRITE
			|| next_token == IF || next_token == ELSE || next_token == WHILE)
			statement();
		else
			break;
	}
}

/*****************************************************************************/

/*parses one statement*/
/* 	<stmt> -->	id:=<expr>;
	<stmt> -->	read(<idlist>);
	<stmt> -->	write(<idlist>); 
	<stmt> -->	if(<idlist>){<stmtlist>}{else(<idlist>){<stmtlist>}};  
	<stmt> -->	while(<idlist>){<stmtlist>}; 
*/
void statement()
{
	if(next_token == ID)
	{
		match(ID);
		match(ASSIGNOP);
		expression();
		match(SEMICOLON);	
	}
	else if(next_token == READ)
	{
		match(READ);
		match(LPAREN);
		id_list();
		match(RPAREN);
		match(SEMICOLON);
	}
	else if(next_token == WRITE)
	{
		match(WRITE);
		match(LPAREN);
		id_list();
		match(RPAREN);
		match(SEMICOLON);
		
	}
	else if(next_token == IF)
	{
		match(IF);
		match(LPAREN);
		bool();
		match(RPAREN);
		match(LCURL);
		statement_list();
		match(RCURL);
		if(next_token == ELSE)
		{
	 		match(ELSE);
			match(LCURL);
			statement_list();
			match(RCURL);
		}
		
	}
	else
		syntax_error();
}

/*****************************************************************************/

/*parses list of identifiers*/
/* <idlist> --> id{, id}*/
void id_list()
{
	match(ID);
	while(next_token == COMMA)
	{
		match(COMMA);
		match(ID);
	}
}

/*****************************************************************************/

/*parses a boolean expression*/
/* <bool> --> [id{, id}*/
void bool()
{
	if (next_token == ID)
		match(ID);
	else
		match(INTLITERAL);
	relop();
	if (next_token == ID)
		match(ID);
	else
		match(INTLITERAL);
}
/*****************************************************************************/

/*parses list of expressions*/
/* <explist> --> <exp>,{,<exp>}*/
void expression_list()
{
	expression();
	while(next_token == COMMA)
	{
		match(COMMA);
		expression();
	}
}

/*****************************************************************************/

/*parses one expression*/
/* <exp> --> <term>{<adop><term>}*/
void expression()
{
	term();
	while(next_token == PLUSOP || next_token == MINUSOP)
	{
		add_op();
		term();
	}
}

/*****************************************************************************/

/*parses one term*/
/*	<term> --> id
	<term> --> integer
	<term> --> <term> 
	<term> --> (<expr>)
	<term> --> <term> <multop> <term>*/
void term()
{
	if(next_token == ID)
	{
		match(ID);
		if (next_token == MULTOP || next_token == DIVOP)
		{
			mult_op();
			term();
		}
	}
	else if(next_token == INTLITERAL)
	{
		match(INTLITERAL);
		if (next_token == MULTOP || next_token == DIVOP)
		{
			mult_op();
			term();
		}
	}
	else if(next_token == LPAREN)
	{
		match(LPAREN);
		expression();
		match(RPAREN);
		if (next_token == MULTOP || next_token == DIVOP)
		{
			mult_op();
			term();
		}
	}
	else
		syntax_error();
}

/*****************************************************************************/

/*parses plus or minus operator*/
/* <adop> --> +|-
*/
void add_op()
{
	if(next_token == PLUSOP || next_token == MINUSOP)
		match(next_token);
	else
		syntax_error();
}

/*****************************************************************************/

/*parses multiplication or division operator*/
/* <multop> --> *|/
*/
void mult_op()
{
	if(next_token == MULTOP || next_token == DIVOP)
		match(next_token);
	else
		syntax_error();
}

/*****************************************************************************/

/*parses boolean operators*/
/* <bool> --> <|<=|>|>=|==|!=
*/
void relop()
{
	if(next_token == GREATER || next_token == GREATEQ || next_token == LESS
		|| next_token == LESSEQ || next_token == EQUALS || next_token == NOTEQ)
		match(next_token);
	else
		syntax_error();
}

/*****************************************************************************/

/*checks whether the expected token and the actual token match,
and also reads the next token from source file
*/
void match(token tok)
{
	if(tok == next_token)
		;
	else
		syntax_error();
	next_token = scanner();
}

/*****************************************************************************/

/*reports syntax error*/
void syntax_error()
{
	printf("syntax error in line %d error with token%s\n", line_num,tokens[next_token]);
	error = TRUE;
}

/*****************************************************************************/

int main()
{
	token t;
	int linecounter = 1;
	int selection;
	
	printf("Please Select an Option\n");
	printf("1-Scan and Output Tokens to a File\n");
	printf("2-Parse a File\n");
	printf("3-Quit\n");
	char input_file[50];
	char output_file[50];
	scanf("%d",&selection);
	
	switch(selection)
	{
		case 1: //Scan File and Output Tokens
		{
			printf("Enter the input file name: ");
			scanf("%s",input_file);
			fin = fopen(input_file, "r");
			printf("Enter the output file name: ");
			scanf("%s",output_file);
			fout = fopen(output_file, "w");
			
			do 
			{
				t = scanner();
				if (linecounter < line_num)
				{
					fprintf(fout,"\n");
					linecounter = line_num;
				}
				fprintf(fout,"%s ",tokens[t]);
			}while(t != SCANEOF);
			fclose(fin); //close file
			fclose(fout); //close file
			printf("File successfully scanned\n");
			break;
		}
		case 2: //Parse the file
		{
			printf("Enter the input file name: ");
			scanf("%s",input_file);
			fin = fopen(input_file, "r");
			parser();
			fclose(fin); //close file
			
			//Inform user file had no errors
			if(error == FALSE)
				printf("\nNo errors in parsing\n");
			break;
		}
		default: //Quit
		{
			return 0;
		}
	}
	return 0;
}

/*****************************************************************************/