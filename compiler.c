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
	ID, INTLITERAL, BEGIN, END, READ, WRITE,
	PLUSOP, MINUSOP, ASSIGNOP, LPAREN, RPAREN,
	COMMA, SEMICOLON, SCANEOF
}token;

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
void add_op();
void match(token tok);
void syntax_error();

/*global variables*/
FILE *fin;				//source file
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
		
		else if (c == ',')
			return COMMA;
			
		else if (c == ';')
			return SEMICOLON;
			
		else if (c == '+')
			return PLUSOP;	
			
		else if (c == '-')
			{
				c = getc(fin);
				if(c == '-')
				{
					do
						c = getc(fin);
					while (c != '\n');
					line_num = line_num + 1;
				}
				else
				{
					ungetc(c, fin);
					return MINUSOP;
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
	if(strcmp(token_buffer, "begin") == 0)
		return BEGIN;
	else if(strcmp(token_buffer, "end") == 0)
		return END;
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
/* <program> --> begin<stmtlist>end */
void program()
{
	match(BEGIN);
	statement_list();
	match(END);
}

/*****************************************************************************/

/*parses list of statements*/
/* <stmtlist> --> <stmt>{<stmt>} */
void statement_list()
{
	statement();
	while(TRUE)
	{
		if(next_token == ID || next_token == READ || next_token == WRITE)
			statement();
		else
			break;
	}
}

/*****************************************************************************/

/*parses one statement*/
/* 	<stmt> -->	id:=<expr>;
	<stmt> -->	read(<idlist>);
	<stmt> -->	write(<exprlist>); */
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

/*parses list of expressions*/
/* <explist> --> <exp>,{,<exp>} */
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
/* <exp> --> <term>{<adop><term>} */
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
	<term> --> (<expr>) */
void term()
{
	if(next_token == ID)
		match(ID);
	else if(next_token == INTLITERAL)
		match(INTLITERAL);
	else if(next_token == LPAREN)
	{
		match(LPAREN);
		expression();
		match(RPAREN);
	}
	else
		syntax_error();
}

/*****************************************************************************/

/*parses plus or minus operator*/
/* <adop> --> +|- */
void add_op()
{
	if(next_token == PLUSOP || next_token == MINUSOP)
		match(next_token);
	else
		syntax_error();
}

/*****************************************************************************/

/*checks whether the expected token and the actual token match,
and also reads the next token from source file*/
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
	printf("syntax error in line %d\n", line_num);
	error = TRUE;
}

/*****************************************************************************/

int main()
{
	return 0;
}

/*****************************************************************************/