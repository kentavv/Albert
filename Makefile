#Makefile for the Albert program.
#
# Trent Whiteley September 1993
BIN=../bin

CC=gcc

#CFLAGS=-g -O0
CFLAGS=-O2

LDFLAGS=
LIBS=-lcurses -ltermcap -lm

OBJECTS=driver.o Get_Command.o Po_create_poly.o Po_routines.o Id_routines.o \
        Ty_routines.o Generators.o Field.o Po_parse_poly.o Help.o \
        Po_expand_poly.o Strings.o Po_parse_exptext.o Po_semantics.o \
        Po_syn_stack.o Po_prod_bst.o Memory_routines.o \
        Build.o Scalar_arithmetic.o Alg_elements.o Mult_table.o \
        Basis_table.o CreateMatrix.o ExtractMatrix.o \
        ReduceMatrix.o Type_table.o GenerateEquations.o Multpart.o \
        CreateSubs.o PerformSub.o SparseReduceMatrix.o node_mgt.o \
	initGlobals.o

albert:			$(OBJECTS)
			$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

driver.o:		Build_defs.h Po_parse_exptext.h Id_routines.h Get_Command.h \
			Basis_table.h

Get_Command.o:		Get_Command.h

Po_create_poly.o:	Po_create_poly.h Po_parse_poly.h \
			Po_semantics.h Po_prod_bst.h Po_parse_exptext.h

Po_routines.o:		Build_defs.h Po_parse_exptext.h Debug.h \
			Alg_elements.h

Id_routines.o:		Id_routines.h Po_parse_exptext.h

Ty_routines.o:		Po_parse_exptext.h Ty_routines.h

Generators.o:		Build_defs.h Po_parse_exptext.h

Field.o:		Field.h Build_defs.h

Po_parse_poly.o:	Po_parse_poly.h Po_syn_stack.h \
			Po_semantics.h Po_prod_bst.h

Help.o:			Help.h

Po_expand_poly.o:	Po_parse_poly.h Po_semantics.h

Strings.o:		

Po_parse_exptext.o:	Po_parse_exptext.h

Po_semantics.o:		Po_parse_poly.h Po_syn_stack.h \
			Po_semantics.h

Po_syn_stack.o:		Po_syn_stack.h

Po_prod_bst.o:		Po_prod_bst.h

Memory_routines.o:	Memory_routines.h Po_parse_poly.h Po_prod_bst.h \
			Po_parse_exptext.h Id_routines.h

Build.o:		Build_defs.h Mult_table.h CreateMatrix.h \
			Po_parse_exptext.h Id_routines.h Sparse_structs.h \
			Sparse_defs.h Debug.h

Scalar_arithmetic.o:	Build_defs.h

Alg_elements.o:		Build_defs.h Alg_elements.h

Mult_table.o:		Build_defs.h Alg_elements.h Mult_table.h

Basis_table.o:		Build_defs.h Basis_table.h

CreateMatrix.o:		Build_defs.h CreateMatrix.h \
			Sparse_structs.h Sparse_defs.h

ExtractMatrix.o:	Build_defs.h CreateMatrix.h \
			Mult_table.h Sparse_structs.h Sparse_defs.h

ReduceMatrix.o:		Build_defs.h CreateMatrix.h

Type_table.o:		Build_defs.h Type_table.h

GenerateEquations.o:	Build_defs.h CreateMatrix.h \
			Po_parse_exptext.h Debug.h

Multpart.o:		Build_defs.h Type_table.h CreateMatrix.h \
			Po_parse_exptext.h Debug.h

CreateSubs.o:		Build_defs.h Type_table.h CreateMatrix.h \
			Po_parse_exptext.h Debug.h

PerformSub.o:		Build_defs.h Alg_elements.h CreateMatrix.h \
			Po_parse_exptext.h PerformSub.h Debug.h

SparseReduceMatrix.o:	Build_defs.h Sparse_structs.h \
			Sparse_defs.h

node_mgt.o:		Sparse_structs.h Sparse_defs.h Debug.h node_mgt.h

initGlobals.o:		Build_defs.h Basis_table.h Mult_table.h


doxygen:
	doxygen doxygen_config

install: albert
	/bin/rm -f ${BIN}/albert4
	cp albert ${BIN}/albert4
	chmod 775 ${BIN}/albert4


clean:
	- rm -f albert *.o *~ *# *.core core

