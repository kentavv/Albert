#ifndef _HELP_H_
#define _HELP_H_

/*******************************************************************/
/***  FILE :     Help.h                                          ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  MODIFIED:  David Lee (8/20/92) - Added help for change     ***/
/***                                   command.                  ***/
/***             Trent Whiteley (8/20/93) - rewrote the help     ***/
/***                                   messages                  ***/
/*******************************************************************/

#define    NUM_COMMANDS    23
#define    MAX_LINE        80

static int helpLines = 0;
static int helpCols = 0;

typedef struct Help_struct {
    char *help_topic;
    char *help_text;
} Help_struct;

static struct Help_struct Help_lines[NUM_COMMANDS] = {
    "H",
"\tHelp Topics\n\n\
    b -- build command\n\
    c -- change command\n\
    d -- display command\n\
    f -- field command\n\
    g -- generators command\n\
    h -- help command\n\
    i -- identity command\n\
    o -- output command\n\
    p -- polynomial command\n\
    q -- quit command\n\
    r -- remove command\n\
    s -- save command\n\
    t -- type command\n\
    v -- view command\n\
    x -- xpand command\n\
    A -- Albert Introduction\n\
    B -- Bibliography\n\
    G -- Glossary of terms\n\
    H -- This Help menu\n\
    I -- Invoking Albert\n\
    N -- Nonassociative polynomial\n\
    S -- System definition file \".albert\"\n\
    W -- Warnings to the user\n\n",
    "A",
"\n\n\
\t\tAlbert\n\n\
Albert is an interactive research tool to assist the\n\
specialist in the study of nonassociative algebra.  The main\n\
problem addressed by Albert is the recognition of polynomial\n\
identities.  Roughly, Albert works in the following way.\n\
Suppose a user wishes to study alternative algebras. These\n\
are algebras defined by the two polynomial identities\n\
(yx)x - y(xx) and (xx)y - x(xy), known respectively as the\n\
right and left alternative laws.  In particular, the user\n\
wishes to know if, in the presence of the right and left\n\
alternative laws, (a,b,c) * [a,b] is also an identity.  Here\n\
(a,b,c) denotes (ab)c - a(bc), [a,b] denotes ab - ba, and\n\
x * y denotes xy + yx.  The user first supplies Albert with\n\
the right and left alternative laws, using the identity\n\
command.  Next, the user supplies the problem type.  This\n\
refers to the number and degree of letters in the target\n\
polynomial.  For example in this problem, each term of the\n\
target polynomial has two a's, two b'x, and one c, and so the\n\
problem type is 2a2b1c.  This is entered using the generators\n\
command.  It may be that over certain rings of scalars the\n\
polynomial is an identity, but over others it is not an\n\
identity.  Albert allows the user to supply the ring of\n\
scalars, but currently the user must select a Galois field\n\
Z(p) in which p is a prime at most 251.  This is done using\n\
the field command.  If no field is entered, the default field\n\
Z(251) is chosen.\n\n\
In deciding whether a given polynomial is an identity or not,\n\
Albert internally constructs a certain homomorphic image of\n\
the free algebra.  It is not necessary that the user\n\
understand the theory of free algebras, nor is it necessary\n\
to understand the algorithms Albert employs to create them.\n\
The user need only be aware that Albert builds a\n\
multiplication table for this free algebra.  The user\n\
instructs Albert to begin the construction using the build\n\
command.  Once this construction has been completed, the user\n\
can query whether the polynomial (a,b,c) * [a,b] is an\n\
identity using the polynomial command.  In fact, the user can\n\
ask Albert about any homogeneous polynomial p(a,b,c) having\n\
at most two a's, two b's and one c in each term.  For\n\
example, the polynomial (a,b,(a,b,c)) + [b,a](a,b,c) could\n\
also be tested.  With Albert, polynomials and identities may\n\
be entered from the keyboard using associators, commutators,\n\
and much of the familiar notation used in nonassociative\n\
right theory.  See the help section entitled Polynomial\n\
Expression Language for a complete description of how\n\
polynomials and identities can be entered in Albert.\n\n\
Albert has a small set of commands, and meaningful research\n\
can be conducted using only the identity command, generators\n\
command, build command, and polynomial commands.  Commands\n\
can always be entered in a shortened form, by using any\n\
prefix of the command.  Thus, the command build can be given\n\
by \"bu\" or simply with \"b\".\n\n\
The program owes its name to A. A. Albert whose work was\n\
pioneering in nonassociative ring theory.  It was designed\n\
and implemented at Clemson University by David P. Jacobs,\n\
Sekhar Muddana, and Jeff Offutt, and partially supported by\n\
NSF Grant #CCR8905534.  Kurti Prabhu also helped during the\n\
early design.  Subsequent features have been implemented by\n\
David Lee and Trent Whiteley.  The idea of constructing free\n\
nonassociative algebras was motivated by several papers of E.\n\
Kleinfeld.  Irvin Hentzel has also assisted in the program's\n\
development through helpful discussions.\n\n",
    "I",
"\n\n\
\t\tInvoking Albert\n\n\
Albert is invoked on the command line by giving its name\n\
followed by two arguments.\n\n\
albert -d dimlimit -a dirname\n\n\
The arguments are optional.  Here, dirname refers to the\n\
directory location where albert will get the \".albert\"\n\
file.  If this argument is not given, albert will look for\n\
it in the current directory.\n\n\
Here dimlimit refers to the dimension limit that albert will\n\
use.  When the build command is given, and a construction is\n\
begun for an algebra whose dimension exceeds this limit, the\n\
construction will fail.  This dimension limit will be in\n\
effect for the duration of the session.  Thus the user should\n\
give an adequate setting.  However, setting this number too\n\
large will cause albert to run extraordinarily slow, so some\n\
care should be used.\n\n\
Legitimate values for dimension limits are multiples of 500\n\
up to 10,000.  If no dimension limit is given, the default\n\
is 500.\n\n",
    "b",
"\n\n\
\t\tbuild\n\n\
This command invokes Albert to begin construction of the\n\
algebra defined by the current configuration.  Albert\n\
constructs the algebra using the current set of identities,\n\
problem type and field stored in the current configuration.\n\
Status information is printed during the construction.  An\n\
old multiplication table is destroyed.\n\n\
A problem type must have been defined with the generators\n\
command.  Likely reasons for this command to fail are\n\
insufficient memory, insufficient time, or exceeding the\n\
dimension limit.\n\n",
    "c",
"\n\n\
\t\tchange\n\n\
Most of the time Albert's computing time and memory are spent\n\
performing matrix computations.  These matrices tend to be\n\
sparse (i.e. have relatively few nonzero entries).  There are\n\
two ways to store matrices:  As ordinary two-dimensional\n\
arrays, in which all matrix elements are stored, or\n\
alternately using a data structure in which only the nonzero\n\
entries are stored.  Of course in such a sparse\n\
implementation, there is more overhead associated with each\n\
entry.  Consequently, for dense matrices the traditional\n\
two-dimensional array would use less memory, while a sparse\n\
implementation would be more efficient for sparse matrices.\n\n\
Albert can use either matrix implementation.  With the sparse\n\
method, the overhead per entry is now about eight times that\n\
of the traditional method, and so the sparse implementation\n\
will be most beneficial for matrices whose density never\n\
exceeds 12%.\n\n\
The matrix densities that occur in Albert vary wildly.  Thus,\n\
a sparse method will sometimes benefit memory utilization,\n\
and sometimes it will hinder memory utilization.  For this\n\
reason, the change command allows the user to toggle between\n\
the two methods.  This switch will not effect the results of\n\
the computation, only the time and memory needed by Albert to\n\
obtain the results.  Since many interesting results often\n\
seem to be just beyond Albert's \"reach\", it is hoped that\n\
this fine tuning knob may enable more problems to be solved.\n\n\
A useful methodology is to begin a problem with the new\n\
(default) sparse setting.  It is likely this will be the best\n\
method.  But, if the problem is unable to finish due to lack\n\
of memory, there is a chance the \"change\" toggle will\n\
help, especially if the user feels that the matrix densities\n\
are exceeding 12%.\n\n",
    "d",
"\n\n\
\t\tdisplay\n\n\
Typing display causes Albert to display the current set of\n\
defining identities, field, problem type and information\n\
about the multiplication table, if present.\n\n",
    "f",
"\n\n\
\t\tfield [number]\n\n\
This command changes the field of scalars, and this\n\
information is stored as part of the current configuration.\n\
When the field is changed, any resident multiplication table\n\
is destroyed.  For example,\n\n\
\tfield 17\n\n\
will cause subsequent algebras to be constructed over the\n\
field Z(17).  When Albert is first entered, the default field\n\
Z(251) is selected.  This field remains in effect until the\n\
field is changed.\n\n",
    "g",
"\n\n\
\t\tgenerators [problem type]\n\n\
Before beginning the construction of an algebra, Albert must\n\
know what the generators will be, and the degrees of the\n\
generators.  This information is referred to as the problem\n\
type, and the generators command is used to define it. This\n\
problem type is stored in the current configuration.\n\
Entering a new problem type destroys any existing problem\n\
type and any multiplication table, if present. For example,\n\n\
\tgenerators aabcc\n\
\tgen aabcc\n\
\tg 2ab2c\n\n\
Here \"problem type\" is a string of lower-case letters\n\
indicating the generators and degrees used in the problem.\n\
For example,\n\n\
\taabcc\n\n\
indicates that the algebra to be built will be generated by\n\
a, b, and c, and will be spanned by words in these letters\n\
having at most two a's, one b, and two c's.  This word must\n\
have degree at least two.  this can also be entered in\n\
abbreviated form as\n\n\
\t2a1b2c\n\
\t2ab2c\n\
\tb2a2c\n\n",
    "i",
"\n\n\
\t\tidentity [polynomial]\n\n\
Examples:\n\n\
\tidentity (x,x,[y,x])\n\
\ti (x,(x,(x,y,x),x),x)\n\n\
This command appends the polynomial to the current set of\n\
identities.  Albert assigns a unique number to the entered\n\
identity for future use.  Entering a new identity destroys\n\
any existing multiplication table in memory.  Names defined\n\
in the \".albert\" file can also be used to enter a\n\
polynomial.  The entered polynomial must be homogeneous.\n\
Albert linearizes all identities without telling the user.\n\n",
    "o",
"\n\n\
\t\toutput [b | m]\n\n\
This command outputs the basis table or the multiplication\n\
table to the printer, provided the table exists.  The\n\
argument specifies the table to be output (b - basis table,\n\
m - multiplication table).  For example, typing\n\n\
\toutput b\n\n\
will output the current basis table to the printer.\n\n",
    "p",
"\n\n\
\t\tpolynomial [polynomial]\n\n\
This command allows the user to query if a polynomial is an\n\
identity.  After a multiplication table has been constructed\n\
using build, this command may be used to test whether the\n\
given polynomial is zero in the resident algebra.  The\n\
polynomial must be homogeneous.  For example, typing\n\n\
\tp 2((ba)a)a + ((aa)a)b - 3((aa)b)a\n\n\
might cause Albert to respond with:\n\n\
\tPolynomial is not an identity.\n\n\
A multiplication table must exist and the degree of each\n\
generator (in the problem type) must be greater than or equal\n\
to the degree of the polynomial in each letter.\n\n",
    "q",
"\n\n\
\t\tquit\n\n\
This command exits the user from Albert.  Any multiplication\n\
table or other information is lost.\n\n",
    "r",
"\n\n\
\t\tremove [number | *]\n\n\
This command removes one or more identities from the current\n\
set of identities.  For example,\n\n\
\tremove 2\n\n\
would remove the identity whose number is 2.  The remaining\n\
identities are renumbered after deletion of the identity.  An\n\
asterisk (*) can be used in place of number to remove all\n\
existing identities:\n\n\
\tr *\n\n\
The remove command will destroy a resident multiplication\n\
table, if present.\n\n",
    "s",
"\n\n\
\t\tsave [b | m]\n\n\
This command saves the basis table or the multiplication\n\
table to a file specified by the user, provided the table\n\
and the file already exist.  After typing the save command,\n\
the user will receive the following prompt:\n\n\
\tFile Name -->\n\n\
The user may enter a directory path along with the file name.\n\
If a file name is entered it will be written to the current\n\
directory.  The argument, \"b\" or \"m\", specifies the table\n\
to be output (b - basis table, m - multiplication table).\n\
For example, typing\n\n\
\tsave b\n\
\tFile Name --> Mult.table\n\n\
will save the current multiplication table in Mult.table in\n\
the current directory.\n\n",
    "t",
"\n\n\
\t\ttype [word]\n\n\
This command determines the association type of a word.  For\n\
nonassociative words having the same degree, Albert assigns a\n\
unique number to each association type.  Usually this is not\n\
important, unless the user wishes to use the W (artificial\n\
word) operator described elsewhere.\n\n\
For example, the following commands reveal how the five\n\
association types of degree 5 are numbered.\n\n\
\t-->type ((aa)a)a\n\n\
\tThe association type of the word = 1.\n\n\
\t-->type (a(aa))a\n\n\
\tThe association type of the word = 2.\n\n\
\t-->type (aa)(aa)\n\n\
\tThe association type of the word = 3.\n\n\
\t-->type a((aa)a)\n\n\
\tThe association type of the word = 4.\n\n\
\t-->type a(a(aa))\n\n\
\tThe association type of the word = 5.\n\n",
    "v",
"\n\n\
\t\tview [b | m]\n\n\
This command outputs the basis table or the multiplication\n\
table to the screen, provided they exist.  The argument\n\
specifies the table to be output (b - basis table, m -\n\
multiplication table).  For example, typing\n\n\
\tview b\n\n\
will output the current basis table to the screen.\n\n",
    "x",
"\n\n\
\t\txpand [polynomial]\n\n\
This command is used to see the expanded form of a\n\
nonassociative polynomial.  For example, typing\n\n\
\txpand (x,y,z)\n\n\
would cause Albert to respond with:\n\n\
\t(xy)z - x(yz)\n\n\
The command is used for information purposes only and does\n\
not affect the current configuration.  The polynomial must be\n\
homogeneous.\n\n",
    "h",
"\n\n\
\t\thelp [topic | ]\n\n\
This command provides detailed information about the topic\n\
named as the parameter.  If no topic is given, a list of all\n\
help topics is displayed.\n\n",
    "N",
"\n\n\
\t\tNonassociative Polynomial Expressions\n\n\
The identity command, polynomial command, and xpand command\n\
require a nonassociative polynomial to be entered.\n\
Nonassociative polynomials are constructed by applying\n\
operators to variables.  All variables must be lower case\n\
letters.  The following is a list of the available operators.\n\n\
 1. addition:  x+y\n\n\
 2. subtraction:  x-y\n\n\
 3. unary minus:  -x\n\n\
 4. scalar product:  3x (The scalar is interpreted to be from\n\
    the ring of integers.)\n\n\
 5. juxtaposed product:  xy\n\n\
 6. commutator:  [x, y] = xy - yx\n\n\
 7. associator:  (x, y, z) = (xy)z -x(yz)\n\n\
 8. Jordan product:  x * y = xy + yx\n\n\
 9. Jordan associator:  < x, y, z > = (x * y) * z - x * (y * z)\n\n\
10. Jacobi:  J(x, y, z) = (xy) z + (yz)x + (zx)y\n\n\
11. left associated exponential:  x^3 = (xx)x\n\n\
12. left/right multiplications\n\
    The general from of these expressions is\n\n\
\t{A y1 y2 ... yK}\n\n\
    where each yi is of the form x' or x`.  Here A can be a\n\
    more complicated expression.\n\
    x` denotes left multiplication by x, and x' denotes right\n\
    multiplication.\n\
    A sequence of such operators are applied from left to\n\
    right, and surrounded by braces.\n\
    For example,\n\n\
\t{xy`z'u'} denotes ((yx)z)u\n\n\
\t{xy`z'u'(s(ts))`} denotes (s(ts))((yx)z)u\n\n\
\t{xy'z'} - {zy`x`} denotes (xy)z - x(yz)\n\n\
13. artificial word W{n; a:b:a:c:d}\n\
    Here n is called an association type.  Often it is\n\
    cumbersome to type in long parenthesized expressions.  To\n\
    simplify the typing, the artificial word construct can be\n\
    used.  W{n; a:b:a:c:d} represents the unique word having\n\
    letters a,b,a,c,d and association type n.  The\n\
    association type of a word can be found with the type\n\
    command.  For example, \"type x((xx)x)\" tells that\n\
    x((xx)x) has association type 4.  Thus W{4; a:a:c:b}\n\
    means a((ac)b).\n\
    Note that typing in a number n that exceeds the number\n\
    of degree n (association) types is an error.\n\n\
The operators can be intermixed in any arbitrary fashion.\n\
For example the following are valid polynomials:\n\n\
\t(x,[x,y,x)\n\n\
\tJ(x, <x,y,z>, z)\n\n\
\t[x^3, (x,x*y,y)]\n\n\
\t[[[x,y],z],w]\n\n\
\t(x(yz))w + w(x(yz))\n\n\
The order in which operators are applied is controlled using\n\
parenthesis.  Thus, one may write (x*y)*z or x*(y*z).  An\n\
ambiguous expression such as x*y*z is illegal.  Most\n\
expressions have the same common meaning as they do in\n\
mathematics.  For example, scalar multiplication and unary\n\
minus(-) have higher precedence over addition and subtraction\n\
and, therefore, 3(a,b,c) + (c,b,a) means (3(a,b,c)) +\n\
(c,b,a).  However, there are some caveats.  When used in the\n\
presence of \"^\" or \"*\", juxtaposition has higher\n\
precedence:\n\n\
\txy^3 means ((xy)(xy))(xy), not x(y^{2}y)\n\n\
\txy*x means (xy)*y\n\n",
    "S",
"\n\n\
\t\tThe \".albert\" File\n\n\
The \".albert\" file contains definitions for making it\n\
easier to enter identities.  This file can be edited by the\n\
user outside of Albert.  Arbitrarily many definitions can be\n\
given.  When Albert is initialized, the \".albert\" file is\n\
read into memory.  Definitions occurring within this file can\n\
be used in subsequent commands, by surrounding the defined\n\
entity with $'s.  For example, suppose the \".albert\" file\n\
contains the definition\n\n\
malcev		(xy)(xz) - ((xyz)x - ((yz)x)x - ((zx)x)y\n\n\
Then from within Albert, one now could enter the command\n\
\"identity $malcev$\".\n\n\
Definitions in the \".albert\" file can depend on other\n\
definitions, as in\n\n\
CommutatorMalcev	[w, $malcev$]\n\n\
A definition need not occur before its use.  However,\n\
circular definitions will cause great problems.  Long\n\
definitions can be continued on another line by placing a\n\
backslash at the end of the line.\n\n\
glennie\n\
 2((2((2(zx)x - z(xx))y)y - (2(zx)x - z(xx))(yy))z)(xy)      \\\n\
 + 2(2((2(zx)x - z(xx))y)y  - (2(zx)x - z(xx))(yy))(z(xy))   \\\n\
 - 2((2((2(zx)x - z(xx))y)y - (2(zx)x-z(xx))(yy))(xy))z      \\\n\
 - 2((2((2((xy)z)z-(xy)(zz))x)x - (2((xy)z)z-(xy)(zz))(xx)y)y\\\n\
 + (2((2((xy)z)z-(xy)(zz))x)x - (2((xy)z)z-(xy)(zz))(xx))(yy)\\\n\
 - (2((2((2(zy)y-z(yy))x)x - (2(zy)y-z(yy))(xx))z)(yx)       \\\n\
 + 2(2((2(zy)y-z(yy))x)x - (2(zy)y-z(yy))(xx))(z(yx))        \\\n\
 - 2((2((2(zy)y-z(yy))x)x - (2(zy)y-z(yy))(xx))(yx))z        \\\n\
 - 2((2((2((yx)z)z-(yx)(zz))y)y-(2((yx)z)z-(yx)(zz))(yy))x)x \\\n\
 + (2((2((yx)z)z-(yx)(zz))y)y-(2((yx)z)z-(yx)(zz))(yy))(xx))\n\n\
The \".albert\" file can include blank lines.  Comments begin\n\
with % and extend to the end of a line.  A typical \".albert\"\n\
file might look like this.\n\n\
%Various identities satisfied by right alternative rings char\n\
%not 2:  See \"Semiprime Locally (-1,1) Rings with Minimal\n\
%Condition, Hentzel & Smith, Albegras, Groups, and Geometries\n\
%2\" (1985), p 28.\n\
RightMoufang   (x,yz,y) - (x,z,y)y\n\
RA.1           (y,z,xx) - (y,x,zx + xz)\n\
RA.2           [xy,z] - x[y,z] - [x,z]y - 2(x,y,z) - (z, x,y)\n\
RA.3           (z,x,wy) + (z,w,xy) - (z,x,y)w + (z,w,y)x\n\
RA.4           ([x,y],w,z) - [x,(y,w,z)] + [y,(x,w,z)] \\\n\
                           - (y,x,[w,z]) + (x,y,[w,z])\n\n",
    "W",
"\n\n\
\t\tWarnings\n\n\
1. All polynomials and defining identities must be\n\
homogeneous.  That is, each must be expressible as a linear\n\
combination of words each having the same degree in each\n\
variable.  This restriction is not very severe, since any\n\
nonhomogeneous polynomial can be replaced by its homogeneous\n\
parts.  This replacement will not affect the results given a\n\
sufficiently high characteristic for the field.\n\n\
2.  Albert internally linearizes any defining identity that\n\
is not multilinear.  Thus the right alternative identity\n\
(x,y,y) is always interpreted as (yx)z - y(xz) +\n\
(yz)x - y(zx).  However, if a defining identity is not\n\
multilinear, the user is advised not to linearize it before\n\
entering it, but rather enter it in its non-multilinear form.\n\
In most cases, this allows Albert to treat it more\n\
efficiently, since the identity's symmetry can be exploited,\n\
in turn saving computer memory.\n\n\
3.  The main obstacle faced by Albert is insufficient computer\n\
memory.  In general, the larger the degree of the problem\n\
type, the more memory Albert requires.  Given two problems\n\
involving the same degree and the same defining identities,\n\
Albert will cope best with the one having fewer letters.  The\n\
defining identities influence the problem, too.  Defining\n\
identities such as the commutative law (as in the case of\n\
Jordan algebras) and the anticommutative law \"drive down\"\n\
the dimension of the free algebra, thus enabling larger\n\
problems to be solved than might otherwise be possible.  If\n\
Albert is unable to complete a problem having degree n,\n\
adding additional identities of degree less than n may allow\n\
Albert to finish.\n\n\
4.  Like any large program, the possibility is high for\n\
errors.  Please report any suspected bugs or general comments\n\
to dpj@cs.clemson.edu.\n\n",
    "G",
"\n\n\
\t\tGlossary\n\n\
By \"configuration\", we mean the current set of identities,\n\
the current Galois field, current list of generators and\n\
their degrees (i.e. problem type), and other information\n\
Albert will use to construct its next algebra.\n\n\
The word \"polynomial\" means nonassociative polynomial.\n\n\
An \"identity\" is a nonassociative polynomial that is\n\
identically zero over a certain algebra.\n\n\
The \"dimension limit\" is the maximum dimension of a\n\
nonassociative algebra that Albert can construct.  This limit\n\
can be set by the user when Albert is invoked.\n\n\
\"Sparse\" refers to certain matrices that contain mostly\n\
zeros.  Albert employs a lot of matrix arithmetic, and most of\n\
its matrices are sparse.  Consequently, Albert uses special\n\
data structures that can efficiently store sparse matrices.\n\
Mostly, these data structures benefit the user, but sometimes\n\
not.  The user can turn this off using the change command.\n\n\
A \"homogeneous\" polynomial is one which, in each generator,\n\
has the same degree for each terms.\n\n\
A \"multilinear\" polynomial is a homogeneous polynomial\n\
having 1 in all its generators.  Every polynomial can be\n\
\"linearized\".  The resulting polynomial is equivalent, given\n\
a high enough characteristic.\n\n",
    "B",
"\n\n\
\t\tBibliography\n\n\
The following is an Albert tutorial:\n\n\
    D.P. Jacobs, S.V. Muddana, A.J. Offutt,\n\
    \"A Computer Algebra System for Nonassociative Identities,\n\
    Hadronic Mechanics and Nonpotential Interactions\",\n\
    Proceedings of the Fifth International Conference,\n\
    Cedar Falls, Myung, H.C. (Ed.),\n\
    Nova Science Publishers, Inc., New York, 1993.\n\n\
The following paper is an application, and illustrates a\n\
methodology for using Albert:\n\n\
    I. R. Hentzel, D.P. Jacobs, and E. Kleinfeld,\n\
    \"Rings with (a,b,c) = (a,c,b) and (a,[b,c],d) = 0:\n\
    A Case Study Using Albert\",\n\
    Int. J. of Computational Mathematics, 50, to appear.\n\n\
Two general texts on nonassociative algebra:\n\n\
    K.A. Zhevlakov, and A.M. Slin'ko, I.P. Shestakov,\n\
    A.I. Shirsho v,\n\
    \"Rings That Are Nearly Associative\",\n\
    Academic Press, New York, 1982.\n\n\
    R.D. Schafer,\n\
    \"An Introduction to Nonassociative Algebras\",\n\
    Academic Press, New York, 1966.\n\n\
The following paper gives the algorithm Albert uses to\n\
construct its algebras:\n\n\
    I.R. Hentzel and D. Pokrass Jacobs,\n\
    \"A Dynamic Programming Method for Building Free Algebras,\n\
    Computers & Mathematics with Applications\",\n\
    22 (1991), 12, 61-66.\n\n"
};

#endif
