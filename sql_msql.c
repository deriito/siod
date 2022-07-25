/*****************************************************************************/
/**                                                                         **/
/** Interface msql --- siod generated by Ralf Wieland      31.3.96          **/
/**                                                                         **/
/**  ALL RIGHTS RESERVED by GNU General Public License                      **/
/**                                                                         **/
/*****************************************************************************/

/* Note: This code was sent to me by rwieland@ralf.zalf.de
         on 11-APR-96. In the mail message he mentioned the following
         locations:


        msql info ..... http://Hughes.com.au/product/msql
        msql source ... ftp://ftp.bond.edu/pub/Minerva/msql
        this source ... ftp://www.zalf.de/pub/out/project

  I then hacked the code to work with the most recent siod release,
  but I did not fix the lack of proper storage management handling
  by a call to set_gc_hooks. Building:

      setenv EXTRA_PROGS "sql_msql.so"
      make osf1 # or whatever target is appropriate for the platform.

$Id: sql_msql.c,v 1.1 1996/10/17 18:40:18 gjc Exp $

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include "msql.h"
#include "siod.h"

static long tc_extra = 0;

#define extra_tc_association 1
#define extra_tc_statement 2
#define extra_mrow 3

/* internal function convers a LISP-statemant in a internal representation */

static  m_result *get_c_statement(LISP st)
{m_result *c;
 if ((TYPE(st) != tc_extra) ||
     (st->storage_as.string.dim != extra_tc_statement))
   err("not a statement",st);
 c = (m_result *)st->storage_as.string.data;
 return(c);
}

/* return an empty LISP-statement */

static LISP extcons(long typec)
{long flag;
 LISP s;
 flag = no_interrupt(1);
 s = cons(NIL,NIL);
 s->type = tc_extra;
 s->storage_as.string.dim = typec;
 no_interrupt(flag);
 return(s);}

/* newhandle is pointer to the actuall database */

static int newhandle=0;

/* opens a database */

LISP msql_open(LISP host, LISP db)
{long retval,flag;
 int value;
 char *c_host,*c_db;
 c_host = get_c_string(host);
 c_db =  get_c_string(db);
 flag = no_interrupt(1);
 /* only on an local machine maybe host = NULL */
 newhandle = msqlConnect(c_host);
 if(newhandle <0)
   err("unknown host:",host );
 value = msqlSelectDB(newhandle,c_db);
 if(value<0)
   err("unknown database: ",db);
 no_interrupt(flag);
 return(NIL);
}

/* close the opend database */

LISP msql_close(void)
{long flag;
 flag = no_interrupt(1);
 msqlClose(newhandle);
 no_interrupt(flag);
 return(NIL);
}

/* sql-query as a sting via LISP */

LISP msql_query(LISP str)
{long flag;
 LISP result;
 if(newhandle == 0) return(NIL);
 flag = no_interrupt(1);
 result = extcons(extra_tc_statement);
 if(msqlQuery(newhandle,get_c_string(str)) < 0)
   err("cant handle query: ",str);
 ( m_result *)result->storage_as.string.data = msqlStoreResult();
 if(result->storage_as.string.data == NULL)
   {
    no_interrupt(flag);
    return(NIL);
   }
 no_interrupt(flag);
 return(result);
} 

/* it is like msql-query but dont allocate memory */

LISP msql_update(LISP str)
{long flag;
 if(newhandle == 0) return(NIL);
 flag = no_interrupt(1);
 if(msqlQuery(newhandle,get_c_string(str)) < 0)
   err("cant handle query: ",str);
 no_interrupt(flag);
 return(NIL);
} 

/* how many fields ? */

LISP msql_numfields(LISP result)
{ return(flocons(msqlNumFields(get_c_statement(result))));
}

/* how many rows return from a select */

LISP msql_numrows(LISP result)
{ return(flocons(msqlNumRows(get_c_statement(result))));
}

/* rows in sequentiell order */

LISP msql_fetch_row(LISP s)
{long flag;
 int count;
 int i;
 m_field *tf;
 m_row thisrow;
 m_result *res;
 LISP liste, s1, anker;
 flag = no_interrupt(1);
 liste = cons(NIL,NIL);
 anker = liste;
 res = get_c_statement(s);
 count = msqlNumFields(res);
 if(thisrow = msqlFetchRow(res)){
 msqlFieldSeek(res,0);
 for(i=0; i<count; i++)
   {tf = msqlFetchField(res);
    if(thisrow[i])
      {switch(tf->type)
         {case INT_TYPE:
               liste->storage_as.cons.car = strcons(strlen(thisrow[i]),thisrow[i]); 
               s1 = cons(NIL,NIL);
               liste->storage_as.cons.cdr = s1;
               liste = liste->storage_as.cons.cdr;
               break;
	     case REAL_TYPE:
               liste->storage_as.cons.car = strcons(strlen(thisrow[i]),thisrow[i]); 
               s1 = cons(NIL,NIL);
               liste->storage_as.cons.cdr = s1;
               liste = liste->storage_as.cons.cdr;
               break;
	     case CHAR_TYPE:
               liste->storage_as.cons.car = strcons(strlen(thisrow[i]),thisrow[i]); 
               s1 = cons(NIL,NIL);
               liste->storage_as.cons.cdr = s1;
               liste = liste->storage_as.cons.cdr;
               break;
		  default:
                  err("erorr in database",s);
		  
	     }
     }
  }
no_interrupt(flag);
return(anker);
}
no_interrupt(flag);
return(NIL);
}

/* rows in direct access */

LISP msql_data_seek(LISP s, LISP cursor)
{long flag;
 int count, counter;
 int i;
 m_field *tf;
 m_row thisrow;
 m_result *res;
 LISP liste, s1, anker;
 counter = (int)get_c_long(cursor);
 flag = no_interrupt(1);
 liste = cons(NIL,NIL);
 anker = liste;
 res = get_c_statement(s);
 count = (int)msqlNumFields(res);
 msqlDataSeek(res,counter);
 if(thisrow = msqlFetchRow(res)){
 msqlFieldSeek(res,0);
 for(i=0; i<count; i++)
   {tf = msqlFetchField(res);
    if(thisrow[i])
      {switch(tf->type)
         {case INT_TYPE:
               liste->storage_as.cons.car = strcons(strlen(thisrow[i]),thisrow[i]); 
               s1 = cons(NIL,NIL);
               liste->storage_as.cons.cdr = s1;
               liste = liste->storage_as.cons.cdr;
               break;
	     case REAL_TYPE:
               liste->storage_as.cons.car = strcons(strlen(thisrow[i]),thisrow[i]); 
               s1 = cons(NIL,NIL);
               liste->storage_as.cons.cdr = s1;
               liste = liste->storage_as.cons.cdr;
               break;
	     case CHAR_TYPE:
               liste->storage_as.cons.car = strcons(strlen(thisrow[i]),thisrow[i]); 
               s1 = cons(NIL,NIL);
               liste->storage_as.cons.cdr = s1;
               liste = liste->storage_as.cons.cdr;
               break;
		  default:
                  no_interrupt(flag);
                  err("erorr in database",s);
		  
	     }
     }
  }
no_interrupt(flag);
return(anker);
}
no_interrupt(flag);
return(NIL);
}

/* list of table of database */

LISP msql_list_tables(void)
{long flag;
 int count;
 int i;
 m_field *tf;
 m_row thisrow;
 m_result *res;
 LISP liste, s1, anker;
 flag = no_interrupt(1);
 liste = cons(NIL,NIL);
 anker = liste;
 if((res = msqlListTables(newhandle)) == NULL)
   {return(NIL);}
 if((count = msqlNumFields(res)) != 1)
   {return(NIL);}
 while(thisrow = msqlFetchRow(res))
   {liste->storage_as.cons.car = strcons(strlen(thisrow[0])+1,thisrow[0]); 
    s1 = cons(NIL,NIL);
    liste->storage_as.cons.cdr = s1;
    liste = liste->storage_as.cons.cdr;
  }
no_interrupt(flag);
return(anker);
}

/* list of structure of selected table */

LISP msql_list_fields(LISP table)
{long flag;
 int count;
 int i;
 m_field *tf;
 m_row thisrow;
 m_result *res;
 flag = no_interrupt(1);
 if((res = msqlListFields(newhandle,get_c_string(table))) == NULL)
   {return(NIL);}
 count = msqlNumFields(res);
 for(i=0; i<count; i++)
   {tf = msqlFetchField(res);
    printf("\n");
    switch(tf->type)
      {
      case INT_TYPE:
        printf("int:   ");
        break;
      case CHAR_TYPE:
        printf("char:  ");
        break;
      case REAL_TYPE:
        printf("real:  ");
        break;
      }
    printf("%s %s %d ",tf->name,tf->table,tf->length);
    if(IS_PRI_KEY(tf->flags))
        printf("pri    ");
    if(IS_NOT_NULL(tf->flags))
        printf("not null ");
  }

no_interrupt(flag);
printf("\n");
return(NIL);
}

/* gives memory allocated free */

LISP msql_free_result(LISP s)
{m_result *res;
 res = get_c_statement(s);
 msqlFreeResult(res);
 return(NIL);
}

/* init function for scheme interpreter */

void init_sql_msql(void)
{tc_extra = allocate_user_tc();

 init_subr_2("msql-open",msql_open);
 init_subr_0("msql-close",msql_close);
 init_subr_1("msql-query",msql_query);
 init_subr_1("msql-update",msql_query);
 init_subr_1("msql-numfields",msql_numfields);
 init_subr_1("msql-numrows",msql_numrows);
 init_subr_1("msql-fetch-row",msql_fetch_row);
 init_subr_2("msql-data-seek",msql_data_seek);
 init_subr_1("msql-free-result",msql_free_result);
 init_subr_0("msql-tables",msql_list_tables);
 init_subr_1("msql-fields",msql_list_fields);
 ("Enhancements (C) Copyright 1996 GNU.\n");}
