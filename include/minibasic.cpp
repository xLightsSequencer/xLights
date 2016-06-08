/*
 * MiniBasic++
 * By Matt Brown
 * An embeddable BASIC interpreter
 *
 * Derived from the C implementation by Malcolm Mclean, Leeds University
 */

#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include <limits.h>
#include <iostream>
#include <fstream>


#define MINIBASIC_MAXFORS 16    /* maximum number of nested fors */
#define MINIBASIC_MAXTOKENS 128 /* maximum number of built-in functions and procedures */
#define MINIBASIC_MAXPRINT 128  /* maximum length of a line of print output */


class MiniBasicClass {

protected:

virtual void infunc(char*,char*,int) = 0;  /* prompt function */
virtual void outfunc(char*) = 0;           /* output function */
virtual void errfunc(const char*) = 0;     /* error function */

enum ErrorCodes {
  ERR_CLEAR = 0,
  ERR_SYNTAX,
  ERR_OUTOFMEMORY,
  ERR_IDTOOLONG,
  ERR_NOSUCHVARIABLE,
  ERR_BADSUBSCRIPT,
  ERR_TOOMANYDIMS,
  ERR_TOOMANYINITS,
  ERR_BADTYPE,
  ERR_TOOMANYFORS,
  ERR_NONEXT,
  ERR_NOFOR,
  ERR_DIVIDEBYZERO,
  ERR_NEGLOG,
  ERR_NEGSQRT,
  ERR_BADSINCOS,
  ERR_EOF,
  ERR_ILLEGALOFFSET,
  ERR_TYPEMISMATCH,
  ERR_INPUTTOOLONG,
  ERR_BADVALUE,
  ERR_NOTINT,
  ERR_IO
};

enum RelationOps {
  ROP_EQ,     /* equals */
  ROP_NEQ,    /* doesn't equal */
  ROP_LT,     /* less than */
  ROP_LTE,    /* less than or equals */
  ROP_GT,     /* greater than */
  ROP_GTE     /* greater than or equals */
};

enum toktypes {
  TOK_COMMAND,
  TOK_NFUNC,
  TOK_SFUNC,
  TOK_CHAR,
  TOK_OTHER
};

enum ReturnCodes {
  EXEC_PAUSE = -2,   // pause execution
  EXEC_HALT = -1,    // halt execution
  EXEC_NEXTLINE = 0  // execute next line in program
};

typedef struct {
  int no;           /* line number */
  const char *str;  /* points to start of line */
} LINE;

typedef struct
{
  char id[32];    /* id of variable */
  double dval;    /* its value if a real */
  char *sval;     /* its value if a string (malloced) */
} VARIABLE;

typedef struct {
  char id[32];    /* id of dimensioned variable */
  int type;       /* its type, STRID or FLTID */
  int ndims;      /* number of dimensions */
  int dim[5];     /* dimensions in x y order */
  char **str;     /* pointer to string data */
  double *dval;   /* pointer to real data */
} DIMVAR;

typedef struct {
  int type;       /* type of variable (STRID or FLTID or ERRORTOK) */
  char **sval;    /* pointer to string data */
  double *dval;   /* pointer to real data */
} LVALUE;

typedef struct {
  char id[32];    /* id of control variable */
  int nextline;   /* line below FOR to which control passes */
  double toval;   /* terminal value */
  double step;    /* step size */
} FORLOOP;

typedef int (MiniBasicClass::*CommandPtr)(void);
typedef double (MiniBasicClass::*NumericFuncPtr)(void);
typedef char* (MiniBasicClass::*StringFuncPtr)(void);

typedef struct {
  char name[20];
  int length;
  int tokennum;
  int tokentype;
  union {
    CommandPtr cfunc;
    NumericFuncPtr nfunc;
    StringFuncPtr sfunc;
  };
} TOKEN;

TOKEN tokentable[MINIBASIC_MAXTOKENS];/* table of built-in tokens - token # is the index */
int ntokens;                /* number of tokens in table */
int firstchartoken;         /* first token of type TOK_CHAR */
int AND, OR, THEN, STEP, TO, NEXT;
int EOS, EOL, DIV, MULT, PLUS, MINUS;
int OPAREN, CPAREN, SHRIEK, COMMA, QUOTE;
int MOD, EQUALS, GREATER, LESS, SEMICOLON;
int VALUE, STRID, FLTID, DIMFLTID, DIMSTRID, ERRORTOK;
double dvalue;              /* getvalue result */


FORLOOP forstack[MINIBASIC_MAXFORS];  /* stack for for loop conrol */
int nfors;                  /* number of fors on stack */

VARIABLE *variables;        /* the script's variables */
int nvariables;             /* number of variables */

DIMVAR *dimvariables;       /* dimensioned arrays */
int ndimvariables;          /* number of dimensioned arrays */

char *script;               /* script source */
char *scriptname;           /* script name */
LINE *lines;                /* list of line starts */
int nlines;                 /* number of BASIC lines in program */

const char *string;         /* string we are parsing */
TOKEN *token;               /* current token (lookahead) */
int errorflag;              /* set when error in input encountered */
char emptystring[1];


/*
  set the errorflag.
  Params: errorcode - the error.
  Notes: ignores error cascades
*/
void seterror(int errorcode)
{
  if(errorflag == 0 || errorcode == 0)
    errorflag = errorcode;
}

/*
  duplicate a string:
  Params: str - string to duplicate
  Returns: malloced duplicate.
*/
char *mystrdup(const char *str) {
  char *answer;

  answer = (char *)malloc(strlen(str) + 1);
  if(answer) strcpy(answer, str);

  return answer;
}

/*
  Count the instances of ch in str
  Params: str - string to check
          ch - character to count
  Returns: no time chs occurs in str.
*/
int mystrcount(const char *str, char ch) {
  int answer = 0;

  while(*str) {
    if(*str++ == ch)
      answer++;
  }
  return answer;
}

/*
  grab a literal from the parse string.
  Params: dest - destination string
          src - source string
  Notes: strings are in quotes, double quotes the escape
*/
void mystrgrablit(char *dest, const char *src) {
  assert(*src == '"');
  src++;

  while(*src) {
    if(*src == '"') {
      if(src[1] == '"') {
        *dest++ = *src;
        src++;
        src++;
      } else {
        break;
      }
    } else {
      *dest++ = *src++;
    }
  }

  *dest++ = 0;
}

/*
  find where a source string literal ends
  Params: src - string to check (must point to quote)
          quote - character to use for quotation
  Returns: pointer to quote which ends string
  Notes: quotes escape quotes
*/
char *mystrend(const char *str, char quote) {
  assert(*str == quote);
  str++;

  while (*str) {
    while (*str != quote) {
      if (*str == '\n' || *str == 0) return 0;
      str++;
    }
    if (str[1] == quote)
      str += 2;
    else
      break;
  }

  return (char *) (*str? str : 0);
}

void sendErrorMsg(const char *msg) {
  errfunc(msg);
}

/*
  Sets up all our globals, including the list of lines.
  Params: script - the script passed by the user
  Returns: true on success, false on failure
*/
bool setup() {
  int i;
  char msgbuf[100];
  char *s;

  s=script;
  nlines = mystrcount(s, '\n');
  lines = (LINE *)malloc(nlines * sizeof(LINE));
  if (!lines) {
    sendErrorMsg("Out of memory\n");
    return false;
  }

  for(i=0;i<nlines;i++) {
    if(isdigit(*s)) {
      lines[i].str = s;
      lines[i].no = strtol(s, 0, 10);
    } else {
      i--;
      nlines--;
    }
    s = strchr(s, '\n');
    s++;
  }

  if (!nlines) {
    sendErrorMsg("Can't read program, it needs line numbers\n");
    free(lines);
    return false;
  }

  for (i=1;i<nlines;i++) {
    if (lines[i].no <= lines[i-1].no) {
      sprintf(msgbuf, "program lines %d and %d not in order\n", lines[i-1].no, lines[i].no);
      sendErrorMsg(msgbuf);
      free(lines);
      return false;
    }
  }

  nfors = 0;
  nvariables = 0;
  variables = 0;
  dimvariables = 0;
  ndimvariables = 0;

  return true;
}

/*
  frees all the memory we have allocated
*/
void cleanup(void) {
  int i;
  int ii;
  int size;

  for(i=0;i<nvariables;i++)
    if (variables[i].sval)
      free(variables[i].sval);
  if (variables) free(variables);
  variables = 0;
  nvariables = 0;

  for (i=0; i<ndimvariables; i++) {
    if (dimvariables[i].type == STRID) {
      if (dimvariables[i].str) {
        size = 1;
        for (ii=0; ii<dimvariables[i].ndims; ii++)
          size *= dimvariables[i].dim[ii];
        for (ii=0; ii<size; ii++)
          if(dimvariables[i].str[ii])
            free(dimvariables[i].str[ii]);
        free(dimvariables[i].str);
      }
    } else {
      if (dimvariables[i].dval)
        free(dimvariables[i].dval);
    }
  }

  if (dimvariables) free(dimvariables);
  dimvariables = 0;
  ndimvariables = 0;
  if (lines) free(lines);
  lines = 0;
  nlines = 0;
  if (scriptname) free(scriptname);
  if (script) free(script);
}

/*
  error report function.
  for reporting errors in the user's script.
  checks the global errorflag.
  writes to fperr.
  Params: lineno - the line on which the error occurred
*/
void reporterror(int lineno)
{
  char msgbuf[100];

  switch(errorflag) {
    case ERR_CLEAR:
      assert(0);
      break;
    case ERR_SYNTAX:
      sprintf(msgbuf, "Syntax error line %d\n", lineno);
      break;
    case ERR_OUTOFMEMORY:
      sprintf(msgbuf, "Out of memory line %d\n", lineno);
      break;
    case ERR_IDTOOLONG:
      sprintf(msgbuf, "Identifier too long line %d\n", lineno);
      break;
    case ERR_NOSUCHVARIABLE:
      sprintf(msgbuf, "No such variable line %d\n", lineno);
      break;
    case ERR_BADSUBSCRIPT:
      sprintf(msgbuf, "Bad subscript line %d\n", lineno);
      break;
    case ERR_TOOMANYDIMS:
      sprintf(msgbuf, "Too many dimensions line %d\n", lineno);
      break;
    case ERR_TOOMANYINITS:
      sprintf(msgbuf, "Too many initialisers line %d\n", lineno);
      break;
    case ERR_BADTYPE:
      sprintf(msgbuf, "Illegal type line %d\n", lineno);
      break;
    case ERR_TOOMANYFORS:
      sprintf(msgbuf, "Too many nested fors line %d\n", lineno);
      break;
    case ERR_NONEXT:
      sprintf(msgbuf, "FOR without matching NEXT line %d\n", lineno);
      break;
    case ERR_NOFOR:
      sprintf(msgbuf, "Missing matching FOR line %d\n", lineno);
      break;
    case ERR_DIVIDEBYZERO:
      sprintf(msgbuf, "Divide by zero lne %d\n", lineno);
      break;
    case ERR_NEGLOG:
      sprintf(msgbuf, "Negative logarithm line %d\n", lineno);
      break;
    case ERR_NEGSQRT:
      sprintf(msgbuf, "Negative square root line %d\n", lineno);
      break;
    case ERR_BADSINCOS:
      sprintf(msgbuf, "Sine or cosine out of range line %d\n", lineno);
      break;
    case ERR_EOF:
      sprintf(msgbuf, "End of input file %d\n", lineno);
      break;
    case ERR_ILLEGALOFFSET:
      sprintf(msgbuf, "Illegal offset line %d\n", lineno);
      break;
    case ERR_TYPEMISMATCH:
      sprintf(msgbuf, "Type mismatch line %d\n", lineno);
      break;
    case ERR_INPUTTOOLONG:
      sprintf(msgbuf, "Input too long line %d\n", lineno);
      break;
    case ERR_BADVALUE:
      sprintf(msgbuf, "Bad value at line %d\n", lineno);
      break;
    case ERR_NOTINT:
      sprintf(msgbuf, "Not an integer at line %d\n", lineno);
      break;
    case ERR_IO:
      sprintf(msgbuf, "input/output error at line %d\n", lineno);
      break;
    default:
      sprintf(msgbuf, "ERROR line %d\n", lineno);
      break;
  }
  sendErrorMsg(msgbuf);
}

/*
  get a token from the string
  Params: str - string to read token from
  Notes: ignores white space between tokens
*/
TOKEN* gettoken(const char *str)
{
  while(isspace(*str))
    str++;

  if(isdigit(*str))
    return &tokentable[VALUE];

  int i=firstchartoken;
  while (tokentable[i].tokentype == TOK_CHAR) {
    if (*str == tokentable[i].name[0]) return &tokentable[i];
    i++;
  }

  int l;
  while (i < ntokens) {
    //printf("gettoken trying: %d %s\n",i,tokentable[i].name);
    l=tokentable[i].length;
    if(!strncmp(str, tokentable[i].name, l) && !isalnum(str[l])) return &tokentable[i];
    i++;
  }

  if (isalpha(*str)) {
    while(isalnum(*str))
      str++;
    switch(*str) {
      case '$':
        return &tokentable[str[1] == '(' ? DIMSTRID : STRID];
      case '(':
        return &tokentable[DIMFLTID];
      default:
        return &tokentable[FLTID];
    }
  }

  return &tokentable[ERRORTOK];
}

/*
  binary search for a line
  Params: no - line number to find
  Returns: index of the line, or -1 on fail.
*/
int findline(int no) {
  int high;
  int low;
  int mid;

  low = 0;
  high = nlines-1;
  while(high > low + 1) {
    mid = (high + low)/2;
    if(lines[mid].no == no)
      return mid;
    if(lines[mid].no > no)
      high = mid;
    else
      low = mid;
  }

  if(lines[low].no == no)
    mid = low;
  else if(lines[high].no == no)
    mid = high;
  else
    mid = -1;

  return mid;
}

/*
  get a numerical value from the parse string
  Params: str - the string to search
          len - return pinter for no chars read
  Retuns: the value of the string.
*/
void getvalue(const char *str) {
  char *end;

  dvalue = strtod(str, &end);
  assert(end != str);
  tokentable[VALUE].length = end - str;
}

/*
  getid - get an id from the parse string:
  Params: str - string to search
          out - id output [32 chars max ]
		  len - return pointer for id length
  Notes: triggers an error if id > 31 chars
         the id includes the $ and ( qualifiers.
*/
void getid(const char *str, char *out, int *len)
{
  int nread = 0;
  while(isspace(*str))
    str++;
  assert(isalpha(*str));
  while(isalnum(*str)) {
    if(nread < 31) {
      out[nread++] = *str++;
    } else {
      seterror(ERR_IDTOOLONG);
      break;
    }
  }
  if(*str == '$') {
    if(nread < 31)
      out[nread++] = *str++;
    else
     seterror(ERR_IDTOOLONG);
  }
  if(*str == '(') {
    if(nread < 31)
      out[nread++] = *str++;
    else
      seterror(ERR_IDTOOLONG);
  }
  out[nread] = 0;
  *len = nread;
}

/*
  get the next line number
  Params: str - pointer to parse string
  Returns: line no of next line, 0 if end
  Notes: goes to newline, then finds
         first line starting with a digit.
*/
int getnextline(const char *str) {
  while(*str) {
    while(*str && *str != '\n')
      str++;
    if(*str == 0) return 0;
    str++;
    if(isdigit(*str)) return atoi(str);
  }
  return 0;
}

/*
  check that we have a token of the passed type
  (if not set the errorflag)
  Move parser on to next token. Sets token and string.
*/
void match(int tok)
{
  int tnum=token->tokennum;
  if (tnum != tok) {
    seterror(ERR_SYNTAX);
    return;
  }

  while (isspace(*string))
    string++;

  /* get token length */
  int len = 0;
  char buff[32];
  if(tnum == VALUE) getvalue(string);
  if (tnum==DIMSTRID || tnum==DIMFLTID || tnum==STRID || tnum==FLTID)
    getid(string, buff, &len);
  else
    len = token->length;

  string += len;
  token = gettoken(string);
  if (token->tokennum == ERRORTOK) seterror(ERR_SYNTAX);
}

/*
  concatenate two strings
  Params: str - firsts string
          cat - second string
  Returns: malloced string.
*/
char *mystrconcat(const char *str, const char *cat) {
  int len;
  char *answer;

  len = strlen(str) + strlen(cat);
  answer = (char *)malloc(len + 1);
  if(answer) {
    strcpy(answer, str);
    strcat(answer, cat);
  }
  return answer;
}

/*
  compute x!
*/
double factorial(double x) {
  double answer = 1.0;
  double t;

  if( x > 1000.0)
    x = 1000.0;

  for(t=1;t<=x;t+=1.0)
    answer *= t;
  return answer;
}

/*
  cast a double to an integer, triggering errors if out of range
*/
int integer(double x)
{
  if( x < INT_MIN || x > INT_MAX )
    seterror( ERR_BADVALUE );
  if( x != floor(x) )
    seterror( ERR_NOTINT );
  return (int) x;
}

/*
  calculate the INSTR() function.
*/
double do_instr(void)
{
  char *str;
  char *substr;
  char *end;
  double answer = 0;
  int offset;

  match(OPAREN);
  str = stringexpr();
  match(COMMA);
  substr = stringexpr();
  match(COMMA);
  offset = integer( expr() );
  offset--;
  match(CPAREN);

  if(!str || ! substr) {
    if(str)
      free(str);
    if(substr)
      free(substr);
    return 0;
  }

  if(offset >= 0 && offset < (int) strlen(str)) {
    end = strstr(str + offset, substr);
    if (end) answer = end - str + 1.0;
  }

  free(str);
  free(substr);

  return answer;
}

/*
  find a scalar variable invariables list
  Params: id - id to get
  Returns: pointer to that entry, 0 on fail
*/
VARIABLE *findvariable(const char *id) {
  int i;

  for(i=0;i<nvariables;i++)
    if(!strcmp(variables[i].id, id))
      return &variables[i];
  return 0;
}

/*
  get the value of a scalar variable from string
  matches FLTID
*/
double variable(void) {
  VARIABLE *var;
  char id[32];
  int len;

  getid(string, id, &len);
  match(FLTID);
  var = findvariable(id);
  if(var) {
    return var->dval;
  } else {
    seterror(ERR_NOSUCHVARIABLE);
    return 0.0;
  }
}

/*
  get a dimensioned array by name
  Params: id (includes opening parenthesis)
  Returns: pointer to array entry or 0 on fail
*/
DIMVAR *finddimvar(const char *id)
{
  int i;

  for(i=0;i<ndimvariables;i++)
    if(!strcmp(dimvariables[i].id, id))
      return &dimvariables[i];
  return 0;
}

/*
  get value of a dimensioned variable from string.
  matches DIMFLTID
*/
double dimvariable(void) {
  DIMVAR *dimvar;
  char id[32];
  int len;
  int index[5];
  double *answer = NULL;

  getid(string, id, &len);
  match(DIMFLTID);
  dimvar = finddimvar(id);
  if(!dimvar) {
    seterror(ERR_NOSUCHVARIABLE);
    return 0.0;
  }

  if(dimvar) {
    switch(dimvar->ndims) {
      case 1:
        index[0] = integer( expr() );
        answer = (double *)getdimvar(dimvar, index[0]);
        break;
      case 2:
        index[0] = integer( expr() );
        match(COMMA);
        index[1] = integer( expr() );
        answer = (double *)getdimvar(dimvar, index[0], index[1]);
        break;
      case 3:
        index[0] = integer( expr() );
        match(COMMA);
        index[1] = integer( expr() );
        match(COMMA);
        index[2] = integer( expr() );
        answer = (double *)getdimvar(dimvar, index[0], index[1], index[2]);
        break;
      case 4:
        index[0] = integer( expr() );
        match(COMMA);
        index[1] = integer( expr() );
        match(COMMA);
        index[2] = integer( expr() );
        match(COMMA);
        index[3] = integer( expr() );
        answer = (double *)getdimvar(dimvar, index[0], index[1], index[2], index[3]);
        break;
      case 5:
        index[0] = integer( expr() );
        match(COMMA);
        index[1] = integer( expr() );
        match(COMMA);
        index[2] = integer( expr() );
        match(COMMA);
        index[3] = integer( expr() );
        match(COMMA);
        index[4] = integer( expr() );
        answer = (double *)getdimvar(dimvar, index[0], index[1], index[2], index[3], index[4]);
        break;

    }

    match(CPAREN);
  }

  if(answer)
    return *answer;

  return 0.0;
}

double do_e(void) {
  return exp(1.0);
}

double do_pi(void) {
  return acos(0.0) * 2.0;
}

// return # of days since 1/1/1900
// note that vbscript incorrectly treats the year 1900 as a leap year
// this function treats 1900 correctly - so it matches vbscript values starting with 3/1/1900
double dateserial(int y, int m, int d) {
  int jd=d+1461*(y+4800+(m-14)/12)/4+367*(m-2-(m-14)/12*12)/12-3*((y+4900+(m-14)/12)/100)/4 - 2447094;
  return (double)jd;
}

double timeserial(int h, int m, double s) {
  return (double)h/24.0 + (double)m/24.0/60.0 + s/24.0/60.0/60.0;
}

double do_dateserial(void) {
  match(OPAREN);
  int y = integer( expr() );
  match(COMMA);
  int m = integer( expr() );
  match(COMMA);
  int d = integer( expr() );
  match(CPAREN);
  return dateserial(y,m,d);
}

double do_timeserial(void) {
  match(OPAREN);
  int h = integer( expr() );
  match(COMMA);
  int m = integer( expr() );
  match(COMMA);
  double s = expr();
  match(CPAREN);
  return timeserial(h,m,s);
}

double do_now(void) {
  time_t rawtime;
  struct tm* timeinfo;
  rawtime=time(NULL);
  timeinfo = localtime ( &rawtime );
  return dateserial(1900+timeinfo->tm_year, 1+timeinfo->tm_mon, timeinfo->tm_mday) +
         timeserial(timeinfo->tm_hour, timeinfo->tm_min, (double)timeinfo->tm_sec);
}

double do_date(void) {
  time_t rawtime;
  struct tm* timeinfo;
  rawtime=time(NULL);
  timeinfo = localtime ( &rawtime );
  return dateserial(1900+timeinfo->tm_year, 1+timeinfo->tm_mon, timeinfo->tm_mday);
}

void dateparts(double datetime, int* year, int* month, int* day) {
  int L= floor(datetime)+68569+2415019;
  int N= 4*L/146097;
  L= L-(146097*N+3)/4;
  int I= 4000*(L+1)/1461001;
  L= L-1461*I/4+31;
  int J= 80*L/2447;
  *day= L-2447*J/80;
  L= J/11;
  *month= J+2-12*L;
  *year= 100*(N-49)+I+L;
}

double do_year(void) {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  int y,m,d;
  dateparts(answer,&y,&m,&d);
  return (double)y;
}

double do_month(void) {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  int y,m,d;
  dateparts(answer,&y,&m,&d);
  return (double)m;
}

double do_day(void) {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  int y,m,d;
  dateparts(answer,&y,&m,&d);
  return (double)d;
}

double do_hour(void) {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  double intpart, fraction;
  fraction=modf(answer, &intpart);
  return floor(fraction*24.0);
}

double do_minute(void) {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  double intpart, fraction;
  fraction=modf(answer, &intpart);
  int t=floor(fraction*24.0*60.0);
  return (double)(t % 60);
}

double do_second(void) {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  double intpart, fraction;
  fraction=modf(answer, &intpart);
  int t=floor(fraction*24.0*60.0*60.0);
  return (double)(t % 60);
}


double do_sin(void) {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  return sin(answer);
}

double do_cos(void) {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  return cos(answer);
}

double do_tan(void) {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  return tan(answer);
}

double do_ln() {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  if(answer > 0)
    answer = log(answer);
  else
    seterror(ERR_NEGLOG);
  return answer;
}

double do_pow() {
  match(OPAREN);
  double answer = expr();
  match(COMMA);
  answer = pow(answer, expr());
  match(CPAREN);
  return answer;
}

double do_sqrt() {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  if(answer >= 0.0)
    answer = sqrt(answer);
  else
    seterror(ERR_NEGSQRT);
  return answer;
}

double do_abs() {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  return fabs(answer);
}

double do_len() {
  double answer=0;
  match(OPAREN);
  char *str = stringexpr();
  match(CPAREN);
  if(str) {
    answer = strlen(str);
    free(str);
  }
  return answer;
}

double do_ascii() {
  double answer=0;
  match(OPAREN);
  char *str = stringexpr();
  match(CPAREN);
  if(str) {
    answer = *str;
    free(str);
  }
  return answer;
}

double do_asin() {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  if(answer >= -1 && answer <= 1)
    answer = asin(answer);
  else
    seterror(ERR_BADSINCOS);
  return answer;
}

double do_acos() {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  if(answer >= -1 && answer <= 1)
    answer = acos(answer);
  else
    seterror(ERR_BADSINCOS);
  return answer;
}

double do_atan() {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  return atan(answer);
}

double do_int() {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  return floor(answer);
}

double do_rnd() {
  match(OPAREN);
  double answer = expr();
  match(CPAREN);
  answer = integer(answer);
  if(answer > 1)
    answer = floor(rand()/(RAND_MAX + 1.0) * answer);
  else if(answer == 1)
    answer = rand()/(RAND_MAX + 1.0);
  else {
    if(answer < 0)
      srand( (unsigned) -answer);
    answer = 0;
  }
  return answer;
}

double do_val() {
  double answer=0;
  match(OPAREN);
  char *str = stringexpr();
  match(CPAREN);
  if(str) {
    answer = strtod(str, 0);
    free(str);
  }
  return answer;
}

double do_vallen() {
  double answer=0;
  char *end;
  match(OPAREN);
  char *str = stringexpr();
  match(CPAREN);
  if(str) {
    strtod(str, &end);
    answer = end - str;
    free(str);
  }
  return answer;
}

/*
  parses a factor
*/
double factor() {
  double answer = 0;
  int tnum=token->tokennum;
  TOKEN* lasttok=token;

  if (token->tokentype == TOK_NFUNC) {
    match(tnum);
    answer = (this->*lasttok->nfunc)();
  } else if (tnum == OPAREN) {
    match(OPAREN);
    answer = expr();
    match(CPAREN);
  } else if (tnum == VALUE) {
    match(VALUE);
    answer=dvalue;
  } else if (tnum == MINUS) {
    match(MINUS);
    answer = -factor();
  } else if (tnum == FLTID) {
    answer = variable();
  } else if (tnum == DIMFLTID) {
    answer = dimvariable();
  } else if(isstring(token)) {
    seterror(ERR_TYPEMISMATCH);
  } else {
    seterror(ERR_SYNTAX);
  }

  while(token->tokennum == SHRIEK) {
    match(SHRIEK);
    answer = factorial(answer);
  }

  return answer;
}

/*
  parses a term
*/
double term(void)
{
  double left;
  double right;

  left = factor();
  while(1) {
    int tnum=token->tokennum;
    if (tnum == MULT) {
      match(MULT);
      right = factor();
      left *= right;
    } else if (tnum == DIV) {
      match(DIV);
      right = factor();
      if(right != 0.0)
        left /= right;
      else
        seterror(ERR_DIVIDEBYZERO);
    } else if (tnum == MOD) {
      match(MOD);
      right = factor();
      left = fmod(left, right);
    } else {
      return left;
    }
  }
}

/*
  parses an expression
*/
double expr(void)
{
  double right;
  double left = term();
  while(1) {
    int tnum=token->tokennum;
    if (tnum == PLUS) {
      match(PLUS);
      right = term();
      left += right;
    } else if (tnum == MINUS) {
      match(MINUS);
      right = term();
      left -= right;
    } else {
      return left;
    }
  }
}

/*
  test if a token represents a string expression
  Params: token - token to test
  Returns: 1 if a string, else 0
*/
int isstring(TOKEN* token) {
  if(token->tokennum == STRID || token->tokennum == QUOTE || token->tokennum == DIMSTRID
    || token->tokentype == TOK_SFUNC)
  return 1;
  return 0;
}


/*
  parse the CHR$ token
*/
char *do_chr(void) {
  double x;
  char buff[6];
  char *answer;

  match(OPAREN);
  x = integer( expr() );
  match(CPAREN);

  buff[0] = (char) x;
  buff[1] = 0;
  answer = mystrdup(buff);

  if(!answer) seterror(ERR_OUTOFMEMORY);

  return answer;
}

/*
  parse the STR$ token
*/
char *do_str(void) {
  double x;
  char buff[64];
  char *answer;

  match(OPAREN);
  x = expr();
  match(CPAREN);

  sprintf(buff, "%g", x);
  answer = mystrdup(buff);
  if(!answer) seterror(ERR_OUTOFMEMORY);
  return answer;
}

/*
  parse the FORMATDATETIME$ token
*/
char *do_format(void) {
  char buff[100];
  char *answer;
  struct tm* timeinfo;

  match(OPAREN);
  double d = expr();
  match(COMMA);
  int fmtidx = integer( expr() );
  match(CPAREN);

  time_t utime = floor((d-25569.0)*86400.0 + 0.5);
  if (utime < 0) {
    seterror(ERR_BADVALUE);
    return emptystring;
  }
  timeinfo = gmtime(&utime);
  switch (fmtidx) {
    case 1:
    case 2:
      // format as date
      strftime(buff, 100, "%x", timeinfo);
      break;
    case 3:
      // format as long time
      strftime(buff, 100, "%X", timeinfo);
      break;
    case 4:
      // format as short time
      strftime(buff, 100, "%H:%M", timeinfo);
      break;
    default:
      // format as date/time
      strftime(buff, 100, "%c", timeinfo);
      break;
  }

  answer = mystrdup(buff);
  if(!answer) seterror(ERR_OUTOFMEMORY);
  return answer;
}

/*
  parse the LEFT$ token
*/
char *do_left(void) {
  char *str;
  int x;
  char *answer;

  match(OPAREN);
  str = stringexpr();
  if(!str) return 0;
  match(COMMA);
  x = integer( expr() );
  match(CPAREN);

  if(x > (int) strlen(str)) return str;
  if(x < 0) {
    seterror(ERR_ILLEGALOFFSET);
    return str;
  }
  str[x] = 0;
  answer = mystrdup(str);
  free(str);
  if(!answer) seterror(ERR_OUTOFMEMORY);
  return answer;
}

/*
  parse the RIGHT$ token
*/
char *do_right(void)
{
  int x;
  char *str;
  char *answer;

  match(OPAREN);
  str = stringexpr();
  if(!str) return 0;
  match(COMMA);
  x = integer( expr() );
  match(CPAREN);

  if( x > (int) strlen(str)) return str;

  if(x < 0) {
    seterror(ERR_ILLEGALOFFSET);
    return str;
  }

  answer = mystrdup( &str[strlen(str) - x] );
  free(str);
  if(!answer) seterror(ERR_OUTOFMEMORY);
  return answer;
}

/*
  parse the MID$ token
*/
char *do_mid(void)
{
  char *str;
  int x;
  int len;
  char *answer;
  char *temp;

  match(OPAREN);
  str = stringexpr();
  match(COMMA);
  x = integer( expr() );
  match(COMMA);
  len = integer( expr() );
  match(CPAREN);

  if(!str) return 0;

  if(len == -1) len = strlen(str) - x + 1;

  if( x > (int) strlen(str) || len < 1) {
    free(str);
    answer = mystrdup("");
    if(!answer) seterror(ERR_OUTOFMEMORY);
    return answer;
  }

  if(x < 1.0) {
    seterror(ERR_ILLEGALOFFSET);
    return str;
  }

  temp = &str[x-1];
  answer = (char *)malloc(len + 1);
  if(!answer) {
    seterror(ERR_OUTOFMEMORY);
    return str;
  }
  strncpy(answer, temp, len);
  answer[len] = 0;
  free(str);

  return answer;
}

/*
  parse the string$ token
*/
char *do_string(void)
{
  int x;
  char *str;
  char *answer;
  int len;
  int N;
  int i;

  match(OPAREN);
  x = integer( expr() );
  match(COMMA);
  str = stringexpr();
  match(CPAREN);

  if(!str) return 0;

  N = x;

  if(N < 1) {
    free(str);
    answer = mystrdup("");
    if(!answer) seterror(ERR_OUTOFMEMORY);
    return answer;
  }

  len = strlen(str);
  answer = (char *)malloc( N * len + 1 );
  if(!answer) {
    free(str);
    seterror(ERR_OUTOFMEMORY);
    return 0;
  }
  for(i=0; i < N; i++) {
    strcpy(answer + len * i, str);
  }
  free(str);

  return answer;
}

/*
  get the address of a dimensioned array element.
  works for both string and real arrays.
  Params: dv - the array's entry in variable list
          ... - integers telling which array element to get
  Returns: the address of that element, 0 on fail
*/
void *getdimvar(DIMVAR *dv, ...) {
  va_list vargs;
  int index[5];
  int i;
  void *answer = 0;

  va_start(vargs, dv);
  for(i=0;i<dv->ndims;i++) {
    index[i] = va_arg(vargs, int);
    index[i]--;
  }
  va_end(vargs);

  for(i=0;i<dv->ndims;i++) {
    if(index[i] >= dv->dim[i] || index[i] < 0) {
      seterror(ERR_BADSUBSCRIPT);
      return 0;
    }
  }

  if(dv->type == FLTID) {
    switch(dv->ndims) {
      case 1:
        answer = &dv->dval[ index[0] ];
        break;
      case 2:
        answer = &dv->dval[ index[1] * dv->dim[0]
          + index[0] ];
        break;
      case 3:
        answer = &dv->dval[ index[2] * (dv->dim[0] * dv->dim[1])
          + index[1] * dv->dim[0]
          + index[0] ];
        break;
      case 4:
        answer = &dv->dval[ index[3] * (dv->dim[0] + dv->dim[1] + dv->dim[2])
          + index[2] * (dv->dim[0] * dv->dim[1])
          + index[1] * dv->dim[0]
          + index[0] ];
      case 5:
        answer = &dv->dval[ index[4] * (dv->dim[0] + dv->dim[1] + dv->dim[2] + dv->dim[3])
          + index[3] * (dv->dim[0] + dv->dim[1] + dv->dim[2])
          + index[2] * (dv->dim[0] + dv->dim[1])
          + index[1] * dv->dim[0]
          + index[0] ];
        break;
    }
  } else if(dv->type == STRID) {
    switch(dv->ndims) {
      case 1:
        answer = &dv->str[ index[0] ];
        break;
      case 2:
        answer = &dv->str[ index[1] * dv->dim[0]
          + index[0] ];
        break;
      case 3:
        answer = &dv->str[ index[2] * (dv->dim[0] * dv->dim[1])
          + index[1] * dv->dim[0]
          + index[0] ];
        break;
      case 4:
        answer = &dv->str[ index[3] * (dv->dim[0] + dv->dim[1] + dv->dim[2])
          + index[2] * (dv->dim[0] * dv->dim[1])
          + index[1] * dv->dim[0]
          + index[0] ];
        break;
      case 5:
        answer = &dv->str[ index[4] * (dv->dim[0] + dv->dim[1] + dv->dim[2] + dv->dim[3])
          + index[3] * (dv->dim[0] + dv->dim[1] + dv->dim[2])
          + index[2] * (dv->dim[0] + dv->dim[1])
          + index[1] * dv->dim[0]
          + index[0] ];
        break;
    }
  }

  return answer;
}

/*
  read a dimensioned string variable from input.
  Returns: pointer to string (not malloced)
*/
char *stringdimvar(void) {
  char id[32];
  int len;
  DIMVAR *dimvar;
  char **answer = NULL;
  int index[5];

  getid(string, id, &len);
  match(DIMSTRID);
  dimvar = finddimvar(id);

  if(dimvar) {
    switch(dimvar->ndims) {
      case 1:
        index[0] = integer( expr() );
        answer = (char **)getdimvar(dimvar, index[0]);
        break;
      case 2:
        index[0] = integer( expr() );
        match(COMMA);
        index[1] = integer( expr() );
        answer = (char **)getdimvar(dimvar, index[0], index[1]);
        break;
      case 3:
        index[0] = integer( expr() );
        match(COMMA);
        index[1] = integer( expr() );
        match(COMMA);
        index[2] = integer( expr() );
        answer = (char **)getdimvar(dimvar, index[0], index[1], index[2]);
        break;
      case 4:
        index[0] = integer( expr() );
        match(COMMA);
        index[1] = integer( expr() );
        match(COMMA);
        index[2] = integer( expr() );
        match(COMMA);
        index[3] = integer( expr() );
        answer = (char **)getdimvar(dimvar, index[0], index[1], index[2], index[3]);
        break;
      case 5:
        index[0] = integer( expr() );
        match(COMMA);
        index[1] = integer( expr() );
        match(COMMA);
        index[2] = integer( expr() );
        match(COMMA);
        index[3] = integer( expr() );
        match(COMMA);
        index[4] = integer( expr() );
        answer = (char **)getdimvar(dimvar, index[0], index[1], index[2], index[3], index[4]);
        break;
    }

    match(CPAREN);
  } else {
    seterror(ERR_NOSUCHVARIABLE);
  }

  if(!errorflag)
    if(*answer)
      return *answer;

  return emptystring;
}

/*
  parse a string variable.
  Returns: pointer to string (not malloced)
*/
char *stringvar(void) {
  char id[32];
  int len;
  VARIABLE *var;

  getid(string, id, &len);
  match(STRID);
  var = findvariable(id);
  if(var) {
    if(var->sval) return var->sval;
    return emptystring;
  }
  seterror(ERR_NOSUCHVARIABLE);
  return emptystring;
}

/*
  parse a string literal
  Returns: malloced string literal
  Notes: newlines aren't allwed in literals, but blind
         concatenation across newlines is.
*/
char *stringliteral(void) {
  int len = 1;
  char *answer = 0;
  char *temp;
  char *substr;
  char *end;

  while(token->tokennum == QUOTE)   {
    while(isspace(*string))
      string++;

    end = mystrend(string, '"');
    if(end) {
      len = end - string;
      substr = (char *)malloc(len);
      if(!substr) {
        seterror(ERR_OUTOFMEMORY);
        return answer;
      }
      mystrgrablit(substr, string);
      if(answer) {
        temp = mystrconcat(answer, substr);
        free(substr);
        free(answer);
        answer = temp;
        if(!answer) {
          seterror(ERR_OUTOFMEMORY);
          return answer;
        }
      } else {
        answer = substr;
      }
      string = end;
    } else {
      seterror(ERR_SYNTAX);
      return answer;
    }

    match(QUOTE);
  }

  return answer;
}


/*
  high level string parsing function.
  Returns: a malloced pointer, or 0 on error condition.
  caller must free!
*/
char *stringexpr(void) {
  char *left;
  char *right;
  char *temp;
  int tnum=token->tokennum;
  TOKEN* lasttok=token;

  if (token->tokentype == TOK_SFUNC) {
    match(token->tokennum);
    left = (this->*lasttok->sfunc)();
  } else if (tnum == DIMSTRID) {
    left = mystrdup(stringdimvar());
  } else if (tnum == STRID) {
    left = mystrdup(stringvar());
  } else if (tnum == QUOTE) {
    left = stringliteral();
  } else {
    if(!isstring(token))
      seterror(ERR_TYPEMISMATCH);
    else
      seterror(ERR_SYNTAX);
    return mystrdup("");
  }

  if(!left) {
    seterror(ERR_OUTOFMEMORY);
    return 0;
  }

  if (token->tokennum == PLUS) {
    match(PLUS);
    right = stringexpr();
    if(right) {
      temp = mystrconcat(left, right);
      free(right);
      if(temp) {
        free(left);
        left = temp;
      } else {
        seterror(ERR_OUTOFMEMORY);
      }
    } else {
      seterror(ERR_OUTOFMEMORY);
    }
  }

  return left;
}




/*
  the PRINT statement
*/
int do_print(void) {
  char *str;
  char buff[MINIBASIC_MAXPRINT],numstr[30];
  int remlen=MINIBASIC_MAXPRINT;  // remaining length
  int l;
  double x;

  buff[0]='\0';
  remlen--;

  while(1) {
    if(isstring(token)) {
      str = stringexpr();
      if(str) {
        l=strlen(str);
        strncat(buff,str,remlen);
        remlen-=l;
        free(str);
      }
    } else {
      x = expr();
      sprintf(numstr, "%g", x);
      l=strlen(numstr);
      strncat(buff,numstr,remlen);
      remlen-=l;
    }
    if(token->tokennum == COMMA) {
      strncat(buff," ",remlen);
      remlen--;
      match(COMMA);
    } else {
      break;
    }
  }
  if(token->tokennum == SEMICOLON) {
    match(SEMICOLON);
  } else {
    strncat(buff,"\n",remlen);
  }
  outfunc(buff);
  return EXEC_NEXTLINE;
}

/*
  add a real varaible to our variable list
  Params: id - id of varaible to add.
  Returns: pointer to new entry in table
*/
VARIABLE *addfloat(const char *id) {
  VARIABLE *vars;

  vars = (VARIABLE *)realloc(variables, (nvariables + 1) * sizeof(VARIABLE));
  if(vars) {
    variables = vars;
    strcpy(variables[nvariables].id, id);
    variables[nvariables].dval = 0;
    variables[nvariables].sval = 0;
    nvariables++;
    return &variables[nvariables-1];
  } else {
    seterror(ERR_OUTOFMEMORY);
  }

  return 0;
}

/*
  add a string variable to table.
  Params: id - id of variable to get (including trailing $)
  Retruns: pointer to new entry in table, 0 on fail.
*/
VARIABLE *addstring(const char *id) {
  VARIABLE *vars;

  vars = (VARIABLE *)realloc(variables, (nvariables + 1) * sizeof(VARIABLE));
  if(vars) {
    variables = vars;
    strcpy(variables[nvariables].id, id);
    variables[nvariables].sval = 0;
    variables[nvariables].dval = 0;
    nvariables++;
    return &variables[nvariables-1];
  } else {
    seterror(ERR_OUTOFMEMORY);
  }

  return 0;
}

/*
  Get an lvalue from the environment
  Params: lv - structure to fill.
  Notes: missing variables (but not out of range subscripts)
         are added to the variable list.
*/
void lvalue(LVALUE *lv) {
  char name[32];
  int len;
  VARIABLE *var;
  DIMVAR *dimvar;
  int index[5];
  void *valptr = 0;
  int type;

  lv->type = ERRORTOK;
  lv->dval = 0;
  lv->sval = 0;

  if (token->tokennum == FLTID) {
    getid(string, name, &len);
    match(FLTID);
    var = findvariable(name);
    if(!var) var = addfloat(name);
    if(!var) {
      seterror(ERR_OUTOFMEMORY);
      return;
    }
    lv->type = FLTID;
    lv->dval = &var->dval;
    lv->sval = 0;
  } else if (token->tokennum == STRID) {
    getid(string, name, &len);
    match(STRID);
    var = findvariable(name);
    if(!var) var = addstring(name);
    if(!var) {
      seterror(ERR_OUTOFMEMORY);
      return;
    }
    lv->type = STRID;
    lv->sval = &var->sval;
    lv->dval = 0;
  } else if (token->tokennum == DIMFLTID || token->tokennum == DIMSTRID) {
    type = (token->tokennum == DIMFLTID) ? FLTID : STRID;
    getid(string, name, &len);
    match(token->tokennum);
    dimvar = finddimvar(name);
    if(dimvar) {
      switch(dimvar->ndims) {
        case 1:
          index[0] = integer( expr() );
          if(errorflag == 0)
            valptr = getdimvar(dimvar, index[0]);
          break;
        case 2:
          index[0] = integer( expr() );
          match(COMMA);
          index[1] = integer( expr() );
          if(errorflag == 0)
            valptr = getdimvar(dimvar, index[0], index[1]);
          break;
        case 3:
          index[0] = integer( expr() );
          match(COMMA);
          index[1] = integer( expr() );
          match(COMMA);
          index[2] = integer( expr() );
          if(errorflag == 0)
            valptr = getdimvar(dimvar, index[0], index[1], index[2]);
          break;
        case 4:
          index[0] = integer( expr() );
          match(COMMA);
          index[1] = integer( expr() );
          match(COMMA);
          index[2] = integer( expr() );
          match(COMMA);
          index[3] = integer( expr() );
          if(errorflag == 0)
            valptr = getdimvar(dimvar, index[0], index[1], index[2], index[3]);
          break;
        case 5:
          index[0] = integer( expr() );
          match(COMMA);
          index[1] = integer( expr() );
          match(COMMA);
          index[2] = integer( expr() );
          match(COMMA);
          index[3] = integer( expr() );
          match(COMMA);
          index[4] = integer( expr() );
          if(errorflag == 0)
            valptr = getdimvar(dimvar, index[0], index[1], index[2], index[3], index[4]);
          break;
      }
      match(CPAREN);
    } else {
      seterror(ERR_NOSUCHVARIABLE);
      return;
    }
    if(valptr) {
      lv->type = type;
      if(type == FLTID)
        lv->dval = (double *)valptr;
      else if(type == STRID)
        lv->sval = (char **)valptr;
      else
        assert(0);
    }
  } else {
    seterror(ERR_SYNTAX);
  }
}

/*
  the LET statement
*/
int do_let(void) {
  LVALUE lv;
  char *temp;

  lvalue(&lv);
  match(EQUALS);
  if (lv.type == FLTID) {
    *lv.dval = expr();
  } else if (lv.type == STRID) {
    temp = *lv.sval;
    *lv.sval = stringexpr();
    if(temp)
    free(temp);
  }
  return EXEC_NEXTLINE;
}

/*
  add a new array to our symbol table.
  Params: id - id of array (include leading ()
  Returns: pointer to new entry, 0 on fail.
*/
DIMVAR *adddimvar(const char *id) {
  DIMVAR *vars;

  vars = (DIMVAR *)realloc(dimvariables, (ndimvariables + 1) * sizeof(DIMVAR));
  if(vars) {
    dimvariables = vars;
    strcpy(dimvariables[ndimvariables].id, id);
    dimvariables[ndimvariables].dval = 0;
    dimvariables[ndimvariables].str = 0;
    dimvariables[ndimvariables].ndims = 0;
    dimvariables[ndimvariables].type = strchr(id, '$') ? STRID : FLTID;
    ndimvariables++;
    return &dimvariables[ndimvariables-1];
  } else {
    seterror(ERR_OUTOFMEMORY);
  }

  return 0;
}

/*
  dimension an array.
  Params: id - the id of the array (include leading ()
          ndims - number of dimension (1-5)
          ... - integers giving dimension size,
*/
DIMVAR *dimension(const char *id, int ndims, ...) {
  DIMVAR *dv;
  va_list vargs;
  int size = 1;
  int oldsize = 1;
  int i;
  int dimensions[5];
  double *dtemp;
  char **stemp;

  assert(ndims <= 5);
  if(ndims > 5)
    return 0;

  dv = finddimvar(id);
  if(!dv) dv = adddimvar(id);
  if(!dv) {
    seterror(ERR_OUTOFMEMORY);
    return 0;
  }

  if(dv->ndims) {
    for(i=0;i<dv->ndims;i++)
      oldsize *= dv->dim[i];
  } else {
    oldsize = 0;
  }

  va_start(vargs, ndims);
  for(i=0;i<ndims;i++) {
    dimensions[i] = va_arg(vargs, int);
    size *= dimensions[i];
  }
  va_end(vargs);

  if (dv->type == FLTID) {
    dtemp = (double *)realloc(dv->dval, size * sizeof(double));
    if(dtemp) {
      dv->dval = dtemp;
    } else {
      seterror(ERR_OUTOFMEMORY);
      return 0;
    }
  } else if (dv->type == STRID) {
    if(dv->str) {
      for(i=size;i<oldsize;i++)
        if(dv->str[i]) {
          free(dv->str[i]);
          dv->str[i] = 0;
        }
    }
    stemp = (char **)realloc(dv->str, size * sizeof(char *));
    if(stemp) {
      dv->str = stemp;
      for(i=oldsize;i<size;i++)
        dv->str[i] = 0;
    } else {
      for(i=0;i<oldsize;i++) {
        if(dv->str[i]) {
          free(dv->str[i]);
          dv->str[i] = 0;
        }
      }
      seterror(ERR_OUTOFMEMORY);
      return 0;
    }
  }

  for(i=0;i<5;i++)
    dv->dim[i] = dimensions[i];
  dv->ndims = ndims;

  return dv;
}

/*
  the DIM statement
*/
int do_dim(void) {
  int ndims = 0;
  double dims[6];
  char name[32];
  int len;
  DIMVAR *dimvar;
  int i;
  int size = 1;

  if (token->tokennum == DIMFLTID || token->tokennum == DIMSTRID) {
    getid(string, name, &len);
    match(token->tokennum);
    dims[ndims++] = expr();
    while(token->tokennum == COMMA) {
      match(COMMA);
      dims[ndims++] = expr();
      if(ndims > 5) {
        seterror(ERR_TOOMANYDIMS);
        return EXEC_HALT;
      }
    }

    match(CPAREN);

    for(i=0;i<ndims;i++) {
      if(dims[i] < 0 || dims[i] != (int) dims[i]) {
        seterror(ERR_BADSUBSCRIPT);
        return EXEC_HALT;
      }
    }
    switch(ndims) {
      case 1:
        dimvar = dimension(name, 1, (int) dims[0]);
        break;
      case 2:
        dimvar = dimension(name, 2, (int) dims[0], (int) dims[1]);
        break;
      case 3:
        dimvar = dimension(name, 3, (int) dims[0], (int) dims[1], (int) dims[2]);
        break;
      case 4:
        dimvar = dimension(name, 4, (int) dims[0], (int) dims[1], (int) dims[2], (int) dims[3]);
        break;
      case 5:
        dimvar = dimension(name, 5, (int) dims[0], (int) dims[1], (int) dims[2], (int) dims[3], (int) dims[4]);
        break;
    }
  } else {
    seterror(ERR_SYNTAX);
    return EXEC_HALT;
  }

  if(dimvar == 0) {
    /* out of memory */
    seterror(ERR_OUTOFMEMORY);
    return EXEC_HALT;
  }

  if(token->tokennum == EQUALS) {
    match(EQUALS);

    for(i=0;i<dimvar->ndims;i++)
      size *= dimvar->dim[i];

    if (dimvar->type == FLTID) {
      i = 0;
      dimvar->dval[i++] = expr();
      while(token->tokennum == COMMA && i < size) {
        match(COMMA);
        dimvar->dval[i++] = expr();
        if(errorflag)
        break;
      }
    } else if (dimvar->type == STRID) {
      i = 0;
      if(dimvar->str[i])
        free(dimvar->str[i]);
      dimvar->str[i++] = stringexpr();

      while(token->tokennum == COMMA && i < size) {
        match(COMMA);
        if(dimvar->str[i])
          free(dimvar->str[i]);
        dimvar->str[i++] = stringexpr();
        if(errorflag) break;
      }
    }

    if(token->tokennum == COMMA) {
      seterror(ERR_TOOMANYINITS);
      return EXEC_HALT;
    }
  }
  return EXEC_NEXTLINE;
}

/*
  parse a boolean expression
  consists of expressions or strings and relational operators,
  and parentheses
*/
int boolexpr(void)
{
  int right;
  int left = boolfactor();
  if (token->tokennum == AND) {
    match(AND);
    right = boolexpr();
    return (left && right) ? 1 : 0;
  } else if (token->tokennum == OR) {
    match(OR);
    right = boolexpr();
    return (left || right) ? 1 : 0;
  } else {
    return left;
  }
}

/*
  get a relational operator
  returns operator parsed or ERRORTOK
*/
int relop(void) {
  if (token->tokennum == EQUALS) {
    match(EQUALS);
    return ROP_EQ;
  } else if (token->tokennum == GREATER) {
    match(GREATER);
    if(token->tokennum == EQUALS) {
      match(EQUALS);
      return ROP_GTE;
    }
    return ROP_GT;
  } else if (token->tokennum == LESS) {
    match(LESS);
    if(token->tokennum == EQUALS) {
      match(EQUALS);
      return ROP_LTE;
    } else if(token->tokennum == GREATER) {
      match(GREATER);
      return ROP_NEQ;
    }
    return ROP_LT;
  } else {
    seterror(ERR_SYNTAX);
    return ERRORTOK;
  }
}

/*
  boolean factor, consists of expression relop expression
    or string relop string, or ( boolexpr() )
*/
int boolfactor(void) {
  int answer;
  double left;
  double right;
  int op;
  char *strleft;
  char *strright;
  int cmp;

  if (token->tokennum == OPAREN) {
    match(OPAREN);
    answer = boolexpr();
    match(CPAREN);
  } else if(isstring(token)) {
    strleft = stringexpr();
    op = relop();
    strright = stringexpr();
    if(!strleft || !strright) {
      if(strleft)
        free(strleft);
      if(strright)
        free(strright);
      return 0;
    }
    cmp = strcmp(strleft, strright);
    switch(op) {
      case ROP_EQ:
        answer = cmp == 0 ? 1 : 0;
        break;
      case ROP_NEQ:
        answer = cmp == 0 ? 0 : 1;
        break;
      case ROP_LT:
        answer = cmp < 0 ? 1 : 0;
        break;
      case ROP_LTE:
        answer = cmp <= 0 ? 1 : 0;
        break;
      case ROP_GT:
        answer = cmp > 0 ? 1 : 0;
        break;
      case ROP_GTE:
        answer = cmp >= 0 ? 1 : 0;
        break;
      default:
        answer = 0;
    }
    free(strleft);
    free(strright);
  } else {
    left = expr();
    op = relop();
    right = expr();
    switch(op) {
      case ROP_EQ:
        answer = (left == right) ? 1 : 0;
        break;
      case ROP_NEQ:
        answer = (left != right) ? 1 : 0;
        break;
      case ROP_LT:
        answer = (left < right) ? 1 : 0;
        break;
      case ROP_LTE:
        answer = (left <= right) ? 1 : 0;
        break;
      case ROP_GT:
        answer = (left > right) ? 1 : 0;
        break;
      case ROP_GTE:
        answer = (left >= right) ? 1 : 0;
        break;
      default:
       errorflag = 1;
       return 0;
    }
  }

  return answer;
}

/*
  the IF statement.
  if jump taken, returns new line no, else returns 0
*/
int do_if(void)
{
  int condition;
  int jump;

  condition = boolexpr();
  match(THEN);
  jump = integer( expr() );
  if(condition)
    return jump;
  else
    return EXEC_NEXTLINE;
}

/*
  the GOTO satement
  returns new line number
*/
int do_goto(void)
{
  return integer( expr() );
}

/*
*/
int exitfor(char *id) {
  char nextid[32];
  int answer;
  int len;
  const char *savestring = string;
  while((string = strchr(string, '\n'))) {
    errorflag = 0;
    token = gettoken(string);
    match(VALUE);
    if(token->tokennum == NEXT) {
      match(NEXT);
      if(token->tokennum == FLTID || token->tokennum == DIMFLTID) {
        getid(string, nextid, &len);
        if(!id || !strcmp(id, nextid)) {
          answer = getnextline(string);
          string = savestring;
          token = gettoken(string);
          return answer ? answer : EXEC_HALT;
        }
      }
    }
  }

  seterror(ERR_NONEXT);
  return EXEC_HALT;
}

int do_exitfor(void) {
  if (nfors) {
    int nextline=exitfor(0);
    if (nextline > 0) nfors--;
    return nextline;
  } else {
    seterror(ERR_NOFOR);
    return EXEC_HALT;
  }
}

/*
  The FOR statement.

  Pushes the for stack.
  Returns line to jump to, or EXEC_HALT to end program

*/
int do_for(void) {
  LVALUE lv;
  char id[32];
  int len;
  double initval;
  double toval;
  double stepval;

  getid(string, id, &len);

  lvalue(&lv);
  if(lv.type != FLTID) {
    seterror(ERR_BADTYPE);
    return EXEC_HALT;
  }
  match(EQUALS);
  initval = expr();
  match(TO);
  toval = expr();
  if(token->tokennum == STEP) {
    match(STEP);
    stepval = expr();
  } else {
    stepval = 1.0;
  }

  *lv.dval = initval;

  if(nfors > MINIBASIC_MAXFORS - 1) {
    seterror(ERR_TOOMANYFORS);
    return EXEC_HALT;
  }

  if((stepval < 0 && initval < toval) || (stepval > 0 && initval > toval)) {
    return exitfor(id);
  } else {
    strcpy(forstack[nfors].id, id);
    forstack[nfors].nextline = getnextline(string);
    forstack[nfors].step = stepval;
    forstack[nfors].toval = toval;
    nfors++;
    return EXEC_NEXTLINE;
  }
}

/*
  the NEXT statement
  updates the counting index, and returns line to jump to
*/
int do_next(void) {
  char id[32];
  int len;
  LVALUE lv;

  if(nfors) {
    getid(string, id, &len);
    lvalue(&lv);
    if(lv.type != FLTID) {
      seterror(ERR_BADTYPE);
      return EXEC_HALT;
    }
    *lv.dval += forstack[nfors-1].step;
    if( (forstack[nfors-1].step < 0 && *lv.dval < forstack[nfors-1].toval) ||
      (forstack[nfors-1].step > 0 && *lv.dval > forstack[nfors-1].toval) ) {
      nfors--;
      return EXEC_NEXTLINE;
    } else {
        return forstack[nfors-1].nextline;
    }
  } else {
    seterror(ERR_NOFOR);
    return EXEC_HALT;
  }
}


/*
  the INPUT statement
*/
int do_input(void) {
  LVALUE lv;
  char buff[1024];
  char *prompt;
  char *end;

  prompt = stringexpr();
  match(COMMA);
  lvalue(&lv);
  infunc(prompt, buff, sizeof(buff));
  end=buff;
  free(prompt);

  if (lv.type == FLTID) {
    while(sscanf(end, "%lf", lv.dval) != 1) {
      if(!*end) {
        seterror(ERR_EOF);
        return EXEC_HALT;
      }
      end++;
    }
  } else if (lv.type == STRID) {
    if(*lv.sval) {
      free(*lv.sval);
      *lv.sval = 0;
    }
    if(!*end) {
      seterror(ERR_EOF);
      return EXEC_HALT;
    }
    end = strchr(buff, '\n');
    if(!end) {
      seterror(ERR_INPUTTOOLONG);
      return EXEC_HALT;
    }
    *end = 0;
    *lv.sval = mystrdup(buff);
    if(!*lv.sval) {
      seterror(ERR_OUTOFMEMORY);
      return EXEC_HALT;
    }
  }
  return EXEC_NEXTLINE;
}

int do_readfile(void)
{
  char id[32];
  int len;
  char line[MINIBASIC_MAXPRINT];
  char *filename = stringexpr();
  match(COMMA);
  getid(string, id, &len);
  match(STRID);
  strcat(id,"(");
  DIMVAR* dimvar=finddimvar(id);
  if (!dimvar) {
      seterror(ERR_NOSUCHVARIABLE);
      return EXEC_HALT;
  }
  if (dimvar->type != STRID) {
      seterror(ERR_TYPEMISMATCH);
      return EXEC_HALT;
  }
  int linecnt=0;
  std::ifstream myfile(filename);
  if (myfile.is_open()) {
    while (!myfile.eof())
    {
      myfile.getline (line,MINIBASIC_MAXPRINT);
      line[MINIBASIC_MAXPRINT-1]=0;
      if (linecnt < dimvar->dim[0]) {
        dimvar->str[linecnt] = mystrdup(line);
        if(!dimvar->str[linecnt]) {
          seterror(ERR_OUTOFMEMORY);
          return EXEC_HALT;
        }
      }
      linecnt++;
    }
    myfile.close();
  }
  free(filename);
  return EXEC_NEXTLINE;
}

/*
  the REM statement.
*/
int do_rem(void)
{
  while (*string && *string != '\n')
    string++;
  return EXEC_NEXTLINE;
}

/*
  Parse a line. High level parse function
*/
int line(void) {
  int answer = 0;
  const char *str;

  match(VALUE);  /* line number */
  TOKEN *lasttok=token;
  if (token->tokentype == TOK_COMMAND) {
    //printf("command: %3.0f %s\n",dvalue,token->name);
    match(token->tokennum);
    answer = (this->*lasttok->cfunc)();
  } else {
    seterror(ERR_SYNTAX);
  }

  if(token->tokennum != EOS) {
    /* check for a newline */
    str = string;
    while(isspace(*str)) {
      if(*str == '\n') break;
      str++;
    }
    if(*str != '\n') seterror(ERR_SYNTAX);
  }

  return answer;
}


int AddToken(const char *tokstring, int tokentype) {
  strcpy(tokentable[ntokens].name, tokstring);
  tokentable[ntokens].length=strlen(tokstring);
  tokentable[ntokens].tokennum=ntokens;
  tokentable[ntokens].tokentype=tokentype;
  return ntokens++;
}

int AddCommand(const char *tokstring, CommandPtr processfunc) {
  int toknum=AddToken(tokstring,TOK_COMMAND);
  tokentable[toknum].cfunc=processfunc;
  return toknum;
}

int AddNumericFunction(const char *tokstring, NumericFuncPtr processfunc) {
  int toknum=AddToken(tokstring,TOK_NFUNC);
  tokentable[toknum].nfunc=processfunc;
  return toknum;
}

int AddStringFunction(const char *tokstring, StringFuncPtr processfunc) {
  int toknum=AddToken(tokstring,TOK_SFUNC);
  tokentable[toknum].sfunc=processfunc;
  return toknum;
}


/*
  Interpret a BASIC script
  Returns: true on success, false on error condition.
*/
virtual bool runFromLineIdx(int curline = 0) {
  int nextline;
  bool answer = true;
  char msgbuf[100];

  while(curline != -1) {
    string = lines[curline].str;
    token = gettoken(string);
    errorflag = 0;

    nextline = line();
    if(errorflag) {
      reporterror(lines[curline].no);
      answer = false;
      nextline = EXEC_HALT;
    }

    if(nextline == EXEC_HALT)
      break;

    if(nextline == EXEC_NEXTLINE) {
      curline++;
      if(curline == nlines) break;
    } else {
      curline = findline(nextline);
      if(curline == -1) {
        sprintf(msgbuf, "line %d not found\n", nextline);
        sendErrorMsg(msgbuf);
        answer = false;
        break;
      }
    }
  }

  return answer;
}



public:

MiniBasicClass()
{
  lines = 0;
  nlines = 0;
  scriptname = 0;
  script = 0;
  ntokens = 0;
  variables = 0;
  nvariables = 0;
  dimvariables = 0;
  ndimvariables = 0;

  emptystring[0]='\0';

  ERRORTOK=AddToken("", TOK_OTHER);
  VALUE   =AddToken("", TOK_OTHER);
  STRID   =AddToken("", TOK_OTHER);
  FLTID   =AddToken("", TOK_OTHER);
  DIMFLTID=AddToken("", TOK_OTHER);
  DIMSTRID=AddToken("", TOK_OTHER);

  firstchartoken=ntokens;
  EOS      =AddToken("",   TOK_CHAR);
  EOL      =AddToken("\n", TOK_CHAR);
  DIV      =AddToken("/",  TOK_CHAR);
  MULT     =AddToken("*",  TOK_CHAR);
  OPAREN   =AddToken("(",  TOK_CHAR);
  CPAREN   =AddToken(")",  TOK_CHAR);
  PLUS     =AddToken("+",  TOK_CHAR);
  MINUS    =AddToken("-",  TOK_CHAR);
  SHRIEK   =AddToken("!",  TOK_CHAR);
  COMMA    =AddToken(",",  TOK_CHAR);
  QUOTE    =AddToken("\"", TOK_CHAR);
  EQUALS   =AddToken("=",  TOK_CHAR);
  LESS     =AddToken("<",  TOK_CHAR);
  GREATER  =AddToken(">",  TOK_CHAR);
  SEMICOLON=AddToken(";",  TOK_CHAR);

  AddNumericFunction("e",     &MiniBasicClass::do_e);
  AddNumericFunction("SIN",   &MiniBasicClass::do_sin);
  AddNumericFunction("COS",   &MiniBasicClass::do_cos);
  AddNumericFunction("TAN",   &MiniBasicClass::do_tan);
  AddNumericFunction("LN",    &MiniBasicClass::do_ln);
  AddNumericFunction("POW",   &MiniBasicClass::do_pow);
  AddNumericFunction("PI",    &MiniBasicClass::do_pi);
  AddNumericFunction("SQRT",  &MiniBasicClass::do_sqrt);
  AddNumericFunction("ABS",   &MiniBasicClass::do_abs);
  AddNumericFunction("LEN",   &MiniBasicClass::do_len);
  AddNumericFunction("ASCII", &MiniBasicClass::do_ascii);
  AddNumericFunction("ASIN",  &MiniBasicClass::do_asin);
  AddNumericFunction("ACOS",  &MiniBasicClass::do_acos);
  AddNumericFunction("ATAN",  &MiniBasicClass::do_atan);
  AddNumericFunction("INT",   &MiniBasicClass::do_int);
  AddNumericFunction("RND",   &MiniBasicClass::do_rnd);
  AddNumericFunction("VAL",   &MiniBasicClass::do_val);
  AddNumericFunction("VALLEN",&MiniBasicClass::do_vallen);
  AddNumericFunction("INSTR", &MiniBasicClass::do_instr);
  AddNumericFunction("NOW",   &MiniBasicClass::do_now);
  AddNumericFunction("DATE",  &MiniBasicClass::do_date);
  AddNumericFunction("DATESERIAL", &MiniBasicClass::do_dateserial);
  AddNumericFunction("TIMESERIAL", &MiniBasicClass::do_timeserial);
  AddNumericFunction("YEAR",   &MiniBasicClass::do_year);
  AddNumericFunction("MONTH",  &MiniBasicClass::do_month);
  AddNumericFunction("DAY",    &MiniBasicClass::do_day);
  AddNumericFunction("HOUR",   &MiniBasicClass::do_hour);
  AddNumericFunction("MINUTE", &MiniBasicClass::do_minute);
  AddNumericFunction("SECOND", &MiniBasicClass::do_second);

  AddStringFunction("CHR$",    &MiniBasicClass::do_chr);
  AddStringFunction("STR$",    &MiniBasicClass::do_str);
  AddStringFunction("LEFT$",   &MiniBasicClass::do_left);
  AddStringFunction("RIGHT$",  &MiniBasicClass::do_right);
  AddStringFunction("MID$",    &MiniBasicClass::do_mid);
  AddStringFunction("STRING$", &MiniBasicClass::do_string);
  AddStringFunction("FORMATDATETIME$", &MiniBasicClass::do_format);

  AddCommand("PRINT", &MiniBasicClass::do_print);
  AddCommand("LET",   &MiniBasicClass::do_let);
  AddCommand("DIM",   &MiniBasicClass::do_dim);
  AddCommand("IF",    &MiniBasicClass::do_if);
  AddCommand("GOTO",  &MiniBasicClass::do_goto);
  AddCommand("INPUT", &MiniBasicClass::do_input);
  AddCommand("REM",   &MiniBasicClass::do_rem);
  AddCommand("FOR",   &MiniBasicClass::do_for);
  NEXT=AddCommand("NEXT",  &MiniBasicClass::do_next);
  AddCommand("EXITFOR",&MiniBasicClass::do_exitfor);
  AddCommand("READFILE",&MiniBasicClass::do_readfile);

  THEN    =AddToken("THEN", TOK_OTHER);
  AND     =AddToken("AND",  TOK_OTHER);
  OR      =AddToken("OR",   TOK_OTHER);
  TO      =AddToken("TO",   TOK_OTHER);
  STEP    =AddToken("STEP", TOK_OTHER);
  MOD     =AddToken("MOD",  TOK_OTHER);
}

~MiniBasicClass()
{
  cleanup();
}


/*
  Set script to be executed
  Params: script_text - text of basic script to be run
  Returns: true on success, false on failure
*/
bool setScript(const char *script_name, const char *script_text) {
  cleanup();
  scriptname=mystrdup(script_name);
  script=mystrdup(script_text);
  return setup();
}


/*
  Interpret BASIC script starting at first line
  Returns: true on success, false on error condition.
*/
virtual bool run() {
  return runFromLineIdx(0);
}

/*
  Interpret BASIC script starting at line linenum
  Returns: true on success, false on error condition.
*/
bool runat(int linenum) {
  char msgbuf[100];
  int lineIdx = findline(linenum);
  if(lineIdx == -1) {
    sprintf(msgbuf, "line %d not found\n", linenum);
    sendErrorMsg(msgbuf);
    return false;
  } else {
    return runFromLineIdx(lineIdx);
  }
}

};
