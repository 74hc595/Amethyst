\ This includes the contents of the following files in the Forth 2012 Standard
\ Test Suite:
\   ttester.fs
\   core.fr
\   coreplustest.fth
\   coreexttest.fth
\   doubletest.fth
\   exceptiontest.fth
\   stringtest.fth
\
\ Three modifications have been made to make the test suite work with this
\ implementation:
\    1. BASE and >IN are byte-sized variables. Replaced cell-sized fetch/stores
\       (e.g. BASE @) with byte-sized equivalents. (e.g. BASE C@)
\    2. STATE does not exist, but STATE@ does.
\    3. Adapted syntax of IMMEDIATE. In this implementation, words must be
\       declared IMMEDIATE at definition time, and IMMEDIATE cannot be applied
\       to words created with CONSTANT, 2CONSTANT, VARIABLE, 2VARIABLE, VALUE,
\       and 2VALUE.

\ 
\ https://github.com/gerryjackson/forth2012-test-suite

\ This file contains the code for ttester, a utility for testing Forth words,
\ as developed by several authors (see below), together with some explanations
\ of its use.
\ Floating-point support has been removed.

\ ttester is based on the original tester suite by Hayes:
\ From: John Hayes S1I
\ Subject: tester.fr
\ Date: Mon, 27 Nov 95 13:10:09 PST  
\ (C) 1995 JOHNS HOPKINS UNIVERSITY / APPLIED PHYSICS LABORATORY
\ MAY BE DISTRIBUTED FREELY AS LONG AS THIS COPYRIGHT NOTICE REMAINS.
\ VERSION 1.1
\ All the subsequent changes have been placed in the public domain.
\ The primary changes from the original are the replacement of "{" by "T{"
\ and "}" by "}T" (to avoid conflicts with the uses of { for locals and }
\ for FSL arrays), modifications so that the stack is allowed to be non-empty
\ before T{, and extensions for the handling of floating point tests.
\ Code for testing equality of floating point values comes
\ from ftester.fs written by David N. Williams, based on the idea of
\ approximate equality in Dirk Zoller's float.4th.
\ Further revisions were provided by Anton Ertl, including the ability
\ to handle either integrated or separate floating point stacks.
\ Revision history and possibly newer versions can be found at
\ http://www.complang.tuwien.ac.at/cvsweb/cgi-bin/cvsweb/gforth/test/ttester.fs
\ Explanatory material and minor reformatting (no code changes) by
\ C. G. Montgomery March 2009, with helpful comments from David Williams
\ and Krishna Myneni.

\ Usage:

\ The basic usage takes the form  T{ <code> -> <expected stack> }T .
\ This executes  <code>  and compares the resulting stack contents with
\ the  <expected stack>  values, and reports any discrepancy between the
\ two sets of values.
\ For example:
\ T{ 1 2 3 swap -> 1 3 2 }T  ok
\ T{ 1 2 3 swap -> 1 2 2 }T INCORRECT RESULT: T{ 1 2 3 swap -> 1 2 2 }T ok
\ T{ 1 2 3 swap -> 1 2 }T WRONG NUMBER OF RESULTS: T{ 1 2 3 swap -> 1 2 }T ok

BASE C@
DECIMAL

VARIABLE ACTUAL-DEPTH \ stack record
CREATE ACTUAL-RESULTS 32 CELLS ALLOT
VARIABLE START-DEPTH
VARIABLE XCURSOR      \ for ...}T
VARIABLE ERROR-XT

: ERROR ERROR-XT @ EXECUTE ;   \ for vectoring of error reporting

: EMPTY-STACK \ ( ... -- ) empty stack; handles underflowed stack too.
  DEPTH START-DEPTH @ < IF
    DEPTH START-DEPTH @ SWAP DO 0 LOOP
  THEN
  DEPTH START-DEPTH @ > IF
    DEPTH START-DEPTH @ DO DROP LOOP
  THEN
;

VARIABLE HALT-ON-ERROR
TRUE HALT-ON-ERROR !

: ERROR1 \ ( C-ADDR U -- ) display an error message 
  \ followed by the line that had the error.
  TYPE SOURCE TYPE CR \ display line corresponding to error
  EMPTY-STACK \ throw away everything else
  BEGIN HALT-ON-ERROR @ WHILE REPEAT
;

' ERROR1 ERROR-XT !

: T{ \ ( -- ) syntactic sugar.
  DEPTH START-DEPTH ! 0 XCURSOR !
;

: -> \ ( ... -- ) record depth and contents of stack.
  DEPTH DUP ACTUAL-DEPTH ! \ record depth
  START-DEPTH @ > IF \ if there is something on the stack
    DEPTH START-DEPTH @ - 0 DO ACTUAL-RESULTS I CELLS + ! LOOP \ save them
  THEN
;

: }T \ ( ... -- ) COMPARE STACK (EXPECTED) CONTENTS WITH SAVED
     \ (ACTUAL) CONTENTS.
  DEPTH ACTUAL-DEPTH @ = IF \ if depths match
    DEPTH START-DEPTH @ > IF \ if there is something on the stack
      DEPTH START-DEPTH @ - 0 DO \ for each stack item
        ACTUAL-RESULTS I CELLS + @ \ compare actual with expected
        <> IF S" INCORRECT RESULT: " ERROR LEAVE THEN
      LOOP
    THEN
  ELSE \ depth mismatch
    S" WRONG NUMBER OF RESULTS: " ERROR
  THEN
;

: ...}T ( -- )
  XCURSOR @ START-DEPTH @ + ACTUAL-DEPTH @ <> IF
    S" NUMBER OF CELL RESULTS BEFORE '->' DOES NOT MATCH ...}T SPECIFICATION: "
    ERROR
  ELSE DEPTH START-DEPTH @ = 0= IF
    S" NUMBER OF CELL RESULTS BEFORE AND AFTER '->' DOES NOT MATCH: " ERROR
  THEN THEN
;

: XTESTER ( X -- )
  DEPTH 0= ACTUAL-DEPTH @ XCURSOR @ START-DEPTH @ + 1+ < OR IF
    S" NUMBER OF CELL RESULTS AFTER '->' BELOW ...}T SPECIFICATION: " ERROR
    EXIT
  ELSE ACTUAL-RESULTS XCURSOR @ CELLS + @ <> IF
    S" INCORRECT CELL RESULT: " ERROR
  THEN THEN
  1 XCURSOR +!
;

: X}T XTESTER ...}T ;
: XX}T XTESTER XTESTER ...}T ;
: XXX}T XTESTER XTESTER XTESTER ...}T ;
: XXXX}T XTESTER XTESTER XTESTER XTESTER ...}T ;

BASE C!

\ Set the following flag to TRUE for more verbose output; this may 
\ allow you to tell which test caused your system to hang. 
VARIABLE VERBOSE
FALSE VERBOSE !

: TESTING \ ( -- ) TALKING COMMENT.
   SOURCE VERBOSE @
   IF DUP >R TYPE CR R> >IN C!
   ELSE >IN C! DROP
   THEN ;

: SKIPPED POSTPONE \ ;

\ From: John Hayes S1I
\ Subject: core.fr
\ Date: Mon, 27 Nov 95 13:10

\ (C) 1995 JOHNS HOPKINS UNIVERSITY / APPLIED PHYSICS LABORATORY
\ MAY BE DISTRIBUTED FREELY AS LONG AS THIS COPYRIGHT NOTICE REMAINS.
\ VERSION 1.2
\ THIS PROGRAM TESTS THE CORE WORDS OF AN ANS FORTH SYSTEM.
\ THE PROGRAM ASSUMES A TWO'S COMPLEMENT IMPLEMENTATION WHERE
\ THE RANGE OF SIGNED NUMBERS IS -2^(N-1) ... 2^(N-1)-1 AND
\ THE RANGE OF UNSIGNED NUMBERS IS 0 ... 2^(N)-1.
\ I HAVEN'T FIGURED OUT HOW TO TEST KEY, QUIT, ABORT, OR ABORT"...
\ I ALSO HAVEN'T THOUGHT OF A WAY TO TEST ENVIRONMENT?...

CR
TESTING CORE WORDS
HEX

\ ------------------------------------------------------------------------
TESTING BASIC ASSUMPTIONS

T{ -> }T               \ START WITH CLEAN SLATE
( TEST IF ANY BITS ARE SET; ANSWER IN BASE 1 )
T{ : BITSSET? IF 0 0 ELSE 0 THEN ; -> }T
T{  0 BITSSET? -> 0 }T      ( ZERO IS ALL BITS CLEAR )
T{  1 BITSSET? -> 0 0 }T      ( OTHER NUMBER HAVE AT LEAST ONE BIT )
T{ -1 BITSSET? -> 0 0 }T

\ ------------------------------------------------------------------------
TESTING BOOLEANS: INVERT AND OR XOR

T{ 0 0 AND -> 0 }T
T{ 0 1 AND -> 0 }T
T{ 1 0 AND -> 0 }T
T{ 1 1 AND -> 1 }T

T{ 0 INVERT 1 AND -> 1 }T
T{ 1 INVERT 1 AND -> 0 }T

0    CONSTANT 0S
0 INVERT CONSTANT 1S

T{ 0S INVERT -> 1S }T
T{ 1S INVERT -> 0S }T

T{ 0S 0S AND -> 0S }T
T{ 0S 1S AND -> 0S }T
T{ 1S 0S AND -> 0S }T
T{ 1S 1S AND -> 1S }T

T{ 0S 0S OR -> 0S }T
T{ 0S 1S OR -> 1S }T
T{ 1S 0S OR -> 1S }T
T{ 1S 1S OR -> 1S }T

T{ 0S 0S XOR -> 0S }T
T{ 0S 1S XOR -> 1S }T
T{ 1S 0S XOR -> 1S }T
T{ 1S 1S XOR -> 0S }T

\ ------------------------------------------------------------------------
TESTING 2* 2/ LSHIFT RSHIFT

( WE TRUST 1S, INVERT, AND BITSSET?; WE WILL CONFIRM RSHIFT LATER )
1S 1 RSHIFT INVERT CONSTANT MSB
T{ MSB BITSSET? -> 0 0 }T

T{ 0S 2* -> 0S }T
T{ 1 2* -> 2 }T
T{ 4000 2* -> 8000 }T
T{ 1S 2* 1 XOR -> 1S }T
T{ MSB 2* -> 0S }T

T{ 0S 2/ -> 0S }T
T{ 1 2/ -> 0 }T
T{ 4000 2/ -> 2000 }T
T{ 1S 2/ -> 1S }T            \ MSB PROPOGATED
T{ 1S 1 XOR 2/ -> 1S }T
T{ MSB 2/ MSB AND -> MSB }T

T{ 1 0 LSHIFT -> 1 }T
T{ 1 1 LSHIFT -> 2 }T
T{ 1 2 LSHIFT -> 4 }T
T{ 1 F LSHIFT -> 8000 }T         \ BIGGEST GUARANTEED SHIFT
T{ 1S 1 LSHIFT 1 XOR -> 1S }T
T{ MSB 1 LSHIFT -> 0 }T

T{ 1 0 RSHIFT -> 1 }T
T{ 1 1 RSHIFT -> 0 }T
T{ 2 1 RSHIFT -> 1 }T
T{ 4 2 RSHIFT -> 1 }T
T{ 8000 F RSHIFT -> 1 }T         \ BIGGEST
T{ MSB 1 RSHIFT MSB AND -> 0 }T      \ RSHIFT ZERO FILLS MSBS
T{ MSB 1 RSHIFT 2* -> MSB }T

0 INVERT CONSTANT MAX-UINT
0 INVERT 1 RSHIFT CONSTANT MAX-INT
0 INVERT 1 RSHIFT INVERT CONSTANT MIN-INT
0 INVERT 1 RSHIFT CONSTANT MID-UINT
0 INVERT 1 RSHIFT INVERT CONSTANT MID-UINT+1

0 CONSTANT <FALSE>
-1 CONSTANT <TRUE>


T{ 0 0= -> <TRUE> }T
T{ 1 0= -> <FALSE> }T
T{ 2 0= -> <FALSE> }T
T{ -1 0= -> <FALSE> }T
T{ MAX-UINT 0= -> <FALSE> }T
T{ MIN-INT 0= -> <FALSE> }T
T{ MAX-INT 0= -> <FALSE> }T

T{ 0 0 = -> <TRUE> }T
T{ 1 1 = -> <TRUE> }T
T{ -1 -1 = -> <TRUE> }T
T{ 1 0 = -> <FALSE> }T
T{ -1 0 = -> <FALSE> }T
T{ 0 1 = -> <FALSE> }T
T{ 0 -1 = -> <FALSE> }T

T{ 0 0< -> <FALSE> }T
T{ -1 0< -> <TRUE> }T
T{ MIN-INT 0< -> <TRUE> }T
T{ 1 0< -> <FALSE> }T
T{ MAX-INT 0< -> <FALSE> }T

T{ 0 1 < -> <TRUE> }T
T{ 1 2 < -> <TRUE> }T
T{ -1 0 < -> <TRUE> }T
T{ -1 1 < -> <TRUE> }T
T{ MIN-INT 0 < -> <TRUE> }T
T{ MIN-INT MAX-INT < -> <TRUE> }T
T{ 0 MAX-INT < -> <TRUE> }T
T{ 0 0 < -> <FALSE> }T
T{ 1 1 < -> <FALSE> }T
T{ 1 0 < -> <FALSE> }T
T{ 2 1 < -> <FALSE> }T
T{ 0 -1 < -> <FALSE> }T
T{ 1 -1 < -> <FALSE> }T
T{ 0 MIN-INT < -> <FALSE> }T
T{ MAX-INT MIN-INT < -> <FALSE> }T
T{ MAX-INT 0 < -> <FALSE> }T

T{ 0 1 > -> <FALSE> }T
T{ 1 2 > -> <FALSE> }T
T{ -1 0 > -> <FALSE> }T
T{ -1 1 > -> <FALSE> }T
T{ MIN-INT 0 > -> <FALSE> }T
T{ MIN-INT MAX-INT > -> <FALSE> }T
T{ 0 MAX-INT > -> <FALSE> }T
T{ 0 0 > -> <FALSE> }T
T{ 1 1 > -> <FALSE> }T
T{ 1 0 > -> <TRUE> }T
T{ 2 1 > -> <TRUE> }T
T{ 0 -1 > -> <TRUE> }T
T{ 1 -1 > -> <TRUE> }T
T{ 0 MIN-INT > -> <TRUE> }T
T{ MAX-INT MIN-INT > -> <TRUE> }T
T{ MAX-INT 0 > -> <TRUE> }T

T{ 0 1 U< -> <TRUE> }T
T{ 1 2 U< -> <TRUE> }T
T{ 0 MID-UINT U< -> <TRUE> }T
T{ 0 MAX-UINT U< -> <TRUE> }T
T{ MID-UINT MAX-UINT U< -> <TRUE> }T
T{ 0 0 U< -> <FALSE> }T
T{ 1 1 U< -> <FALSE> }T
T{ 1 0 U< -> <FALSE> }T
T{ 2 1 U< -> <FALSE> }T
T{ MID-UINT 0 U< -> <FALSE> }T
T{ MAX-UINT 0 U< -> <FALSE> }T
T{ MAX-UINT MID-UINT U< -> <FALSE> }T

T{ 0 1 MIN -> 0 }T
T{ 1 2 MIN -> 1 }T
T{ -1 0 MIN -> -1 }T
T{ -1 1 MIN -> -1 }T
T{ MIN-INT 0 MIN -> MIN-INT }T
T{ MIN-INT MAX-INT MIN -> MIN-INT }T
T{ 0 MAX-INT MIN -> 0 }T
T{ 0 0 MIN -> 0 }T
T{ 1 1 MIN -> 1 }T
T{ 1 0 MIN -> 0 }T
T{ 2 1 MIN -> 1 }T
T{ 0 -1 MIN -> -1 }T
T{ 1 -1 MIN -> -1 }T
T{ 0 MIN-INT MIN -> MIN-INT }T
T{ MAX-INT MIN-INT MIN -> MIN-INT }T
T{ MAX-INT 0 MIN -> 0 }T

T{ 0 1 MAX -> 1 }T
T{ 1 2 MAX -> 2 }T
T{ -1 0 MAX -> 0 }T
T{ -1 1 MAX -> 1 }T
T{ MIN-INT 0 MAX -> 0 }T
T{ MIN-INT MAX-INT MAX -> MAX-INT }T
T{ 0 MAX-INT MAX -> MAX-INT }T
T{ 0 0 MAX -> 0 }T
T{ 1 1 MAX -> 1 }T
T{ 1 0 MAX -> 1 }T
T{ 2 1 MAX -> 2 }T
T{ 0 -1 MAX -> 0 }T
T{ 1 -1 MAX -> 1 }T
T{ 0 MIN-INT MAX -> 0 }T
T{ MAX-INT MIN-INT MAX -> MAX-INT }T
T{ MAX-INT 0 MAX -> MAX-INT }T

\ ------------------------------------------------------------------------
TESTING STACK OPS: 2DROP 2DUP 2OVER 2SWAP ?DUP DEPTH DROP DUP OVER ROT SWAP

T{ 1 2 2DROP -> }T
T{ 1 2 2DUP -> 1 2 1 2 }T
T{ 1 2 3 4 2OVER -> 1 2 3 4 1 2 }T
T{ 1 2 3 4 2SWAP -> 3 4 1 2 }T
T{ 0 ?DUP -> 0 }T
T{ 1 ?DUP -> 1 1 }T
T{ -1 ?DUP -> -1 -1 }T
T{ DEPTH -> 0 }T
T{ 0 DEPTH -> 0 1 }T
T{ 0 1 DEPTH -> 0 1 2 }T
T{ 0 DROP -> }T
T{ 1 2 DROP -> 1 }T
T{ 1 DUP -> 1 1 }T
T{ 1 2 OVER -> 1 2 1 }T
T{ 1 2 3 ROT -> 2 3 1 }T
T{ 1 2 SWAP -> 2 1 }T

\ ------------------------------------------------------------------------
TESTING >R R> R@

T{ : GR1 >R R> ; -> }T
T{ : GR2 >R R@ R> DROP ; -> }T
T{ 123 GR1 -> 123 }T
T{ 123 GR2 -> 123 }T
T{ 1S GR1 -> 1S }T   ( RETURN STACK HOLDS CELLS )

\ ------------------------------------------------------------------------
TESTING ADD/SUBTRACT: + - 1+ 1- ABS NEGATE

T{ 0 5 + -> 5 }T
T{ 5 0 + -> 5 }T
T{ 0 -5 + -> -5 }T
T{ -5 0 + -> -5 }T
T{ 1 2 + -> 3 }T
T{ 1 -2 + -> -1 }T
T{ -1 2 + -> 1 }T
T{ -1 -2 + -> -3 }T
T{ -1 1 + -> 0 }T
T{ MID-UINT 1 + -> MID-UINT+1 }T

T{ 0 5 - -> -5 }T
T{ 5 0 - -> 5 }T
T{ 0 -5 - -> 5 }T
T{ -5 0 - -> -5 }T
T{ 1 2 - -> -1 }T
T{ 1 -2 - -> 3 }T
T{ -1 2 - -> -3 }T
T{ -1 -2 - -> 1 }T
T{ 0 1 - -> -1 }T
T{ MID-UINT+1 1 - -> MID-UINT }T

T{ 0 1+ -> 1 }T
T{ -1 1+ -> 0 }T
T{ 1 1+ -> 2 }T
T{ MID-UINT 1+ -> MID-UINT+1 }T

T{ 2 1- -> 1 }T
T{ 1 1- -> 0 }T
T{ 0 1- -> -1 }T
T{ MID-UINT+1 1- -> MID-UINT }T

T{ 0 NEGATE -> 0 }T
T{ 1 NEGATE -> -1 }T
T{ -1 NEGATE -> 1 }T
T{ 2 NEGATE -> -2 }T
T{ -2 NEGATE -> 2 }T

T{ 0 ABS -> 0 }T
T{ 1 ABS -> 1 }T
T{ -1 ABS -> 1 }T
T{ MIN-INT ABS -> MID-UINT+1 }T

\ ------------------------------------------------------------------------
TESTING MULTIPLY: S>D * M* UM*

T{ 0 S>D -> 0 0 }T
T{ 1 S>D -> 1 0 }T
T{ 2 S>D -> 2 0 }T
T{ -1 S>D -> -1 -1 }T
T{ -2 S>D -> -2 -1 }T
T{ MIN-INT S>D -> MIN-INT -1 }T
T{ MAX-INT S>D -> MAX-INT 0 }T

T{ 0 0 M* -> 0 S>D }T
T{ 0 1 M* -> 0 S>D }T
T{ 1 0 M* -> 0 S>D }T
T{ 1 2 M* -> 2 S>D }T
T{ 2 1 M* -> 2 S>D }T
T{ 3 3 M* -> 9 S>D }T
T{ -3 3 M* -> -9 S>D }T
T{ 3 -3 M* -> -9 S>D }T
T{ -3 -3 M* -> 9 S>D }T
T{ 0 MIN-INT M* -> 0 S>D }T
T{ 1 MIN-INT M* -> MIN-INT S>D }T
T{ 2 MIN-INT M* -> 0 1S }T
T{ 0 MAX-INT M* -> 0 S>D }T
T{ 1 MAX-INT M* -> MAX-INT S>D }T
T{ 2 MAX-INT M* -> MAX-INT 1 LSHIFT 0 }T
T{ MIN-INT MIN-INT M* -> 0 MSB 1 RSHIFT }T
T{ MAX-INT MIN-INT M* -> MSB MSB 2/ }T
T{ MAX-INT MAX-INT M* -> 1 MSB 2/ INVERT }T

T{ 0 0 * -> 0 }T            \ TEST IDENTITIES
T{ 0 1 * -> 0 }T
T{ 1 0 * -> 0 }T
T{ 1 2 * -> 2 }T
T{ 2 1 * -> 2 }T
T{ 3 3 * -> 9 }T
T{ -3 3 * -> -9 }T
T{ 3 -3 * -> -9 }T
T{ -3 -3 * -> 9 }T

T{ MID-UINT+1 1 RSHIFT 2 * -> MID-UINT+1 }T
T{ MID-UINT+1 2 RSHIFT 4 * -> MID-UINT+1 }T
T{ MID-UINT+1 1 RSHIFT MID-UINT+1 OR 2 * -> MID-UINT+1 }T

T{ 0 0 UM* -> 0 0 }T
T{ 0 1 UM* -> 0 0 }T
T{ 1 0 UM* -> 0 0 }T
T{ 1 2 UM* -> 2 0 }T
T{ 2 1 UM* -> 2 0 }T
T{ 3 3 UM* -> 9 0 }T

T{ MID-UINT+1 1 RSHIFT 2 UM* -> MID-UINT+1 0 }T
T{ MID-UINT+1 2 UM* -> 0 1 }T
T{ MID-UINT+1 4 UM* -> 0 2 }T
T{ 1S 2 UM* -> 1S 1 LSHIFT 1 }T
T{ MAX-UINT MAX-UINT UM* -> 1 1 INVERT }T


\ ------------------------------------------------------------------------
TESTING DIVIDE: FM/MOD SM/REM UM/MOD */ */MOD / /MOD MOD

T{ 0 S>D 1 FM/MOD -> 0 0 }T
T{ 1 S>D 1 FM/MOD -> 0 1 }T
T{ 2 S>D 1 FM/MOD -> 0 2 }T
T{ -1 S>D 1 FM/MOD -> 0 -1 }T
T{ -2 S>D 1 FM/MOD -> 0 -2 }T
T{ 0 S>D -1 FM/MOD -> 0 0 }T
T{ 1 S>D -1 FM/MOD -> 0 -1 }T
T{ 2 S>D -1 FM/MOD -> 0 -2 }T
T{ -1 S>D -1 FM/MOD -> 0 1 }T
T{ -2 S>D -1 FM/MOD -> 0 2 }T
T{ 2 S>D 2 FM/MOD -> 0 1 }T
T{ -1 S>D -1 FM/MOD -> 0 1 }T
T{ -2 S>D -2 FM/MOD -> 0 1 }T
T{  7 S>D  3 FM/MOD -> 1 2 }T
T{  7 S>D -3 FM/MOD -> -2 -3 }T
T{ -7 S>D  3 FM/MOD -> 2 -3 }T
T{ -7 S>D -3 FM/MOD -> -1 2 }T
T{ MAX-INT S>D 1 FM/MOD -> 0 MAX-INT }T
T{ MIN-INT S>D 1 FM/MOD -> 0 MIN-INT }T
T{ MAX-INT S>D MAX-INT FM/MOD -> 0 1 }T
T{ MIN-INT S>D MIN-INT FM/MOD -> 0 1 }T
T{ 1S 1 4 FM/MOD -> 3 MAX-INT }T
T{ 1 MIN-INT M* 1 FM/MOD -> 0 MIN-INT }T
T{ 1 MIN-INT M* MIN-INT FM/MOD -> 0 1 }T
T{ 2 MIN-INT M* 2 FM/MOD -> 0 MIN-INT }T
T{ 2 MIN-INT M* MIN-INT FM/MOD -> 0 2 }T
T{ 1 MAX-INT M* 1 FM/MOD -> 0 MAX-INT }T
T{ 1 MAX-INT M* MAX-INT FM/MOD -> 0 1 }T
T{ 2 MAX-INT M* 2 FM/MOD -> 0 MAX-INT }T
T{ 2 MAX-INT M* MAX-INT FM/MOD -> 0 2 }T
T{ MIN-INT MIN-INT M* MIN-INT FM/MOD -> 0 MIN-INT }T
T{ MIN-INT MAX-INT M* MIN-INT FM/MOD -> 0 MAX-INT }T
T{ MIN-INT MAX-INT M* MAX-INT FM/MOD -> 0 MIN-INT }T
T{ MAX-INT MAX-INT M* MAX-INT FM/MOD -> 0 MAX-INT }T

T{ 0 S>D 1 SM/REM -> 0 0 }T
T{ 1 S>D 1 SM/REM -> 0 1 }T
T{ 2 S>D 1 SM/REM -> 0 2 }T
T{ -1 S>D 1 SM/REM -> 0 -1 }T
T{ -2 S>D 1 SM/REM -> 0 -2 }T
T{ 0 S>D -1 SM/REM -> 0 0 }T
T{ 1 S>D -1 SM/REM -> 0 -1 }T
T{ 2 S>D -1 SM/REM -> 0 -2 }T
T{ -1 S>D -1 SM/REM -> 0 1 }T
T{ -2 S>D -1 SM/REM -> 0 2 }T
T{ 2 S>D 2 SM/REM -> 0 1 }T
T{ -1 S>D -1 SM/REM -> 0 1 }T
T{ -2 S>D -2 SM/REM -> 0 1 }T
T{  7 S>D  3 SM/REM -> 1 2 }T
T{  7 S>D -3 SM/REM -> 1 -2 }T
T{ -7 S>D  3 SM/REM -> -1 -2 }T
T{ -7 S>D -3 SM/REM -> -1 2 }T
T{ MAX-INT S>D 1 SM/REM -> 0 MAX-INT }T
T{ MIN-INT S>D 1 SM/REM -> 0 MIN-INT }T
T{ MAX-INT S>D MAX-INT SM/REM -> 0 1 }T
T{ MIN-INT S>D MIN-INT SM/REM -> 0 1 }T
T{ 1S 1 4 SM/REM -> 3 MAX-INT }T
T{ 2 MIN-INT M* 2 SM/REM -> 0 MIN-INT }T
T{ 2 MIN-INT M* MIN-INT SM/REM -> 0 2 }T
T{ 2 MAX-INT M* 2 SM/REM -> 0 MAX-INT }T
T{ 2 MAX-INT M* MAX-INT SM/REM -> 0 2 }T
T{ MIN-INT MIN-INT M* MIN-INT SM/REM -> 0 MIN-INT }T
T{ MIN-INT MAX-INT M* MIN-INT SM/REM -> 0 MAX-INT }T
T{ MIN-INT MAX-INT M* MAX-INT SM/REM -> 0 MIN-INT }T
T{ MAX-INT MAX-INT M* MAX-INT SM/REM -> 0 MAX-INT }T

T{ 0 0 1 UM/MOD -> 0 0 }T
T{ 1 0 1 UM/MOD -> 0 1 }T
T{ 1 0 2 UM/MOD -> 1 0 }T
T{ 3 0 2 UM/MOD -> 1 1 }T
T{ MAX-UINT 2 UM* 2 UM/MOD -> 0 MAX-UINT }T
T{ MAX-UINT 2 UM* MAX-UINT UM/MOD -> 0 2 }T
T{ MAX-UINT MAX-UINT UM* MAX-UINT UM/MOD -> 0 MAX-UINT }T

: IFFLOORED
   [ -3 2 / -2 = INVERT ] LITERAL IF POSTPONE \ THEN ;

: IFSYM
   [ -3 2 / -1 = INVERT ] LITERAL IF POSTPONE \ THEN ;

\ THE SYSTEM MIGHT DO EITHER FLOORED OR SYMMETRIC DIVISION.
\ SINCE WE HAVE ALREADY TESTED M*, FM/MOD, AND SM/REM WE CAN USE THEM IN TEST.

IFFLOORED : T/MOD  >R S>D R> FM/MOD ;
IFFLOORED : T/     T/MOD SWAP DROP ;
IFFLOORED : TMOD   T/MOD DROP ;
IFFLOORED : T*/MOD >R M* R> FM/MOD ;
IFFLOORED : T*/    T*/MOD SWAP DROP ;
IFSYM     : T/MOD  >R S>D R> SM/REM ;
IFSYM     : T/     T/MOD SWAP DROP ;
IFSYM     : TMOD   T/MOD DROP ;
IFSYM     : T*/MOD >R M* R> SM/REM ;
IFSYM     : T*/    T*/MOD SWAP DROP ;

T{ 0 1 /MOD -> 0 1 T/MOD }T
T{ 1 1 /MOD -> 1 1 T/MOD }T
T{ 2 1 /MOD -> 2 1 T/MOD }T
T{ -1 1 /MOD -> -1 1 T/MOD }T
T{ -2 1 /MOD -> -2 1 T/MOD }T
T{ 0 -1 /MOD -> 0 -1 T/MOD }T
T{ 1 -1 /MOD -> 1 -1 T/MOD }T
T{ 2 -1 /MOD -> 2 -1 T/MOD }T
T{ -1 -1 /MOD -> -1 -1 T/MOD }T
T{ -2 -1 /MOD -> -2 -1 T/MOD }T
T{ 2 2 /MOD -> 2 2 T/MOD }T
T{ -1 -1 /MOD -> -1 -1 T/MOD }T
T{ -2 -2 /MOD -> -2 -2 T/MOD }T
T{ 7 3 /MOD -> 7 3 T/MOD }T
T{ 7 -3 /MOD -> 7 -3 T/MOD }T
T{ -7 3 /MOD -> -7 3 T/MOD }T
T{ -7 -3 /MOD -> -7 -3 T/MOD }T
T{ MAX-INT 1 /MOD -> MAX-INT 1 T/MOD }T
T{ MIN-INT 1 /MOD -> MIN-INT 1 T/MOD }T
T{ MAX-INT MAX-INT /MOD -> MAX-INT MAX-INT T/MOD }T
T{ MIN-INT MIN-INT /MOD -> MIN-INT MIN-INT T/MOD }T

T{ 0 1 / -> 0 1 T/ }T
T{ 1 1 / -> 1 1 T/ }T
T{ 2 1 / -> 2 1 T/ }T
T{ -1 1 / -> -1 1 T/ }T
T{ -2 1 / -> -2 1 T/ }T
T{ 0 -1 / -> 0 -1 T/ }T
T{ 1 -1 / -> 1 -1 T/ }T
T{ 2 -1 / -> 2 -1 T/ }T
T{ -1 -1 / -> -1 -1 T/ }T
T{ -2 -1 / -> -2 -1 T/ }T
T{ 2 2 / -> 2 2 T/ }T
T{ -1 -1 / -> -1 -1 T/ }T
T{ -2 -2 / -> -2 -2 T/ }T
T{ 7 3 / -> 7 3 T/ }T
T{ 7 -3 / -> 7 -3 T/ }T
T{ -7 3 / -> -7 3 T/ }T
T{ -7 -3 / -> -7 -3 T/ }T
T{ MAX-INT 1 / -> MAX-INT 1 T/ }T
T{ MIN-INT 1 / -> MIN-INT 1 T/ }T
T{ MAX-INT MAX-INT / -> MAX-INT MAX-INT T/ }T
T{ MIN-INT MIN-INT / -> MIN-INT MIN-INT T/ }T

T{ 0 1 MOD -> 0 1 TMOD }T
T{ 1 1 MOD -> 1 1 TMOD }T
T{ 2 1 MOD -> 2 1 TMOD }T
T{ -1 1 MOD -> -1 1 TMOD }T
T{ -2 1 MOD -> -2 1 TMOD }T
T{ 0 -1 MOD -> 0 -1 TMOD }T
T{ 1 -1 MOD -> 1 -1 TMOD }T
T{ 2 -1 MOD -> 2 -1 TMOD }T
T{ -1 -1 MOD -> -1 -1 TMOD }T
T{ -2 -1 MOD -> -2 -1 TMOD }T
T{ 2 2 MOD -> 2 2 TMOD }T
T{ -1 -1 MOD -> -1 -1 TMOD }T
T{ -2 -2 MOD -> -2 -2 TMOD }T
T{ 7 3 MOD -> 7 3 TMOD }T
T{ 7 -3 MOD -> 7 -3 TMOD }T
T{ -7 3 MOD -> -7 3 TMOD }T
T{ -7 -3 MOD -> -7 -3 TMOD }T
T{ MAX-INT 1 MOD -> MAX-INT 1 TMOD }T
T{ MIN-INT 1 MOD -> MIN-INT 1 TMOD }T
T{ MAX-INT MAX-INT MOD -> MAX-INT MAX-INT TMOD }T
T{ MIN-INT MIN-INT MOD -> MIN-INT MIN-INT TMOD }T

T{ 0 2 1 */ -> 0 2 1 T*/ }T
T{ 1 2 1 */ -> 1 2 1 T*/ }T
T{ 2 2 1 */ -> 2 2 1 T*/ }T
T{ -1 2 1 */ -> -1 2 1 T*/ }T
T{ -2 2 1 */ -> -2 2 1 T*/ }T
T{ 0 2 -1 */ -> 0 2 -1 T*/ }T
T{ 1 2 -1 */ -> 1 2 -1 T*/ }T
T{ 2 2 -1 */ -> 2 2 -1 T*/ }T
T{ -1 2 -1 */ -> -1 2 -1 T*/ }T
T{ -2 2 -1 */ -> -2 2 -1 T*/ }T
T{ 2 2 2 */ -> 2 2 2 T*/ }T
T{ -1 2 -1 */ -> -1 2 -1 T*/ }T
T{ -2 2 -2 */ -> -2 2 -2 T*/ }T
T{ 7 2 3 */ -> 7 2 3 T*/ }T
T{ 7 2 -3 */ -> 7 2 -3 T*/ }T
T{ -7 2 3 */ -> -7 2 3 T*/ }T
T{ -7 2 -3 */ -> -7 2 -3 T*/ }T
T{ MAX-INT 2 MAX-INT */ -> MAX-INT 2 MAX-INT T*/ }T
T{ MIN-INT 2 MIN-INT */ -> MIN-INT 2 MIN-INT T*/ }T

T{ 0 2 1 */MOD -> 0 2 1 T*/MOD }T
T{ 1 2 1 */MOD -> 1 2 1 T*/MOD }T
T{ 2 2 1 */MOD -> 2 2 1 T*/MOD }T
T{ -1 2 1 */MOD -> -1 2 1 T*/MOD }T
T{ -2 2 1 */MOD -> -2 2 1 T*/MOD }T
T{ 0 2 -1 */MOD -> 0 2 -1 T*/MOD }T
T{ 1 2 -1 */MOD -> 1 2 -1 T*/MOD }T
T{ 2 2 -1 */MOD -> 2 2 -1 T*/MOD }T
T{ -1 2 -1 */MOD -> -1 2 -1 T*/MOD }T
T{ -2 2 -1 */MOD -> -2 2 -1 T*/MOD }T
T{ 2 2 2 */MOD -> 2 2 2 T*/MOD }T
T{ -1 2 -1 */MOD -> -1 2 -1 T*/MOD }T
T{ -2 2 -2 */MOD -> -2 2 -2 T*/MOD }T
T{ 7 2 3 */MOD -> 7 2 3 T*/MOD }T
T{ 7 2 -3 */MOD -> 7 2 -3 T*/MOD }T
T{ -7 2 3 */MOD -> -7 2 3 T*/MOD }T
T{ -7 2 -3 */MOD -> -7 2 -3 T*/MOD }T
T{ MAX-INT 2 MAX-INT */MOD -> MAX-INT 2 MAX-INT T*/MOD }T
T{ MIN-INT 2 MIN-INT */MOD -> MIN-INT 2 MIN-INT T*/MOD }T

\ ------------------------------------------------------------------------
TESTING HERE , @ ! CELL+ CELLS C, C@ C! CHARS 2@ 2! ALIGN ALIGNED +! ALLOT

HERE 1 ALLOT
HERE
CONSTANT 2NDA
CONSTANT 1STA
T{ 1STA 2NDA U< -> <TRUE> }T      \ HERE MUST GROW WITH ALLOT
T{ 1STA 1+ -> 2NDA }T         \ ... BY ONE ADDRESS UNIT
( MISSING TEST: NEGATIVE ALLOT )

\ Added by GWJ so that ALIGN can be used before , (comma) is tested
1 ALIGNED CONSTANT ALMNT   \ -- 1|2|4|8 for 8|16|32|64 bit alignment
ALIGN
T{ HERE 1 ALLOT ALIGN HERE SWAP - ALMNT = -> <TRUE> }T
\ End of extra test

HERE 1 ,
HERE 2 ,
CONSTANT 2ND
CONSTANT 1ST
T{ 1ST 2ND U< -> <TRUE> }T         \ HERE MUST GROW WITH ALLOT
T{ 1ST CELL+ -> 2ND }T         \ ... BY ONE CELL
T{ 1ST 1 CELLS + -> 2ND }T
T{ 1ST @ 2ND @ -> 1 2 }T
T{ 5 1ST ! -> }T
T{ 1ST @ 2ND @ -> 5 2 }T
T{ 6 2ND ! -> }T
T{ 1ST @ 2ND @ -> 5 6 }T
T{ 1ST 2@ -> 6 5 }T
T{ 2 1 1ST 2! -> }T
T{ 1ST 2@ -> 2 1 }T
T{ 1S 1ST !  1ST @ -> 1S }T      \ CAN STORE CELL-WIDE VALUE

HERE 1 C,
HERE 2 C,
CONSTANT 2NDC
CONSTANT 1STC
T{ 1STC 2NDC U< -> <TRUE> }T      \ HERE MUST GROW WITH ALLOT
T{ 1STC CHAR+ -> 2NDC }T         \ ... BY ONE CHAR
T{ 1STC 1 CHARS + -> 2NDC }T
T{ 1STC C@ 2NDC C@ -> 1 2 }T
T{ 3 1STC C! -> }T
T{ 1STC C@ 2NDC C@ -> 3 2 }T
T{ 4 2NDC C! -> }T
T{ 1STC C@ 2NDC C@ -> 3 4 }T

ALIGN 1 ALLOT HERE ALIGN HERE 3 CELLS ALLOT
CONSTANT A-ADDR  CONSTANT UA-ADDR
T{ UA-ADDR ALIGNED -> A-ADDR }T
T{    1 A-ADDR C!  A-ADDR C@ ->    1 }T
T{ 1234 A-ADDR  !  A-ADDR  @ -> 1234 }T
T{ 123 456 A-ADDR 2!  A-ADDR 2@ -> 123 456 }T
T{ 2 A-ADDR CHAR+ C!  A-ADDR CHAR+ C@ -> 2 }T
T{ 3 A-ADDR CELL+ C!  A-ADDR CELL+ C@ -> 3 }T
T{ 1234 A-ADDR CELL+ !  A-ADDR CELL+ @ -> 1234 }T
T{ 123 456 A-ADDR CELL+ 2!  A-ADDR CELL+ 2@ -> 123 456 }T

: BITS ( X -- U )
   0 SWAP BEGIN DUP WHILE DUP MSB AND IF >R 1+ R> THEN 2* REPEAT DROP ;
( CHARACTERS >= 1 AU, <= SIZE OF CELL, >= 8 BITS )
T{ 1 CHARS 1 < -> <FALSE> }T
T{ 1 CHARS 1 CELLS > -> <FALSE> }T
( TBD: HOW TO FIND NUMBER OF BITS? )

( CELLS >= 1 AU, INTEGRAL MULTIPLE OF CHAR SIZE, >= 16 BITS )
T{ 1 CELLS 1 < -> <FALSE> }T
T{ 1 CELLS 1 CHARS MOD -> 0 }T
T{ 1S BITS 10 < -> <FALSE> }T

T{ 0 1ST ! -> }T
T{ 1 1ST +! -> }T
T{ 1ST @ -> 1 }T
T{ -1 1ST +! 1ST @ -> 0 }T

\ ------------------------------------------------------------------------
TESTING CHAR [CHAR] [ ] BL S"

T{ BL -> 20 }T
T{ CHAR X -> 58 }T
T{ CHAR HELLO -> 48 }T
T{ : GC1 [CHAR] X ; -> }T
T{ : GC2 [CHAR] HELLO ; -> }T
T{ GC1 -> 58 }T
T{ GC2 -> 48 }T
T{ : GC3 [ GC1 ] LITERAL ; -> }T
T{ GC3 -> 58 }T
T{ : GC4 S" XY" ; -> }T
T{ GC4 SWAP DROP -> 2 }T
T{ GC4 DROP DUP C@ SWAP CHAR+ C@ -> 58 59 }T

\ ------------------------------------------------------------------------
TESTING ' ['] FIND EXECUTE IMMEDIATE COUNT LITERAL POSTPONE STATE@

T{ : GT1 123 ; -> }T
T{ ' GT1 EXECUTE -> 123 }T
T{ : GT2 IMMEDIATE ['] GT1 ; -> }T
T{ GT2 EXECUTE -> 123 }T
HERE 3 C, CHAR G C, CHAR T C, CHAR 1 C, CONSTANT GT1STRING
HERE 3 C, CHAR G C, CHAR T C, CHAR 2 C, CONSTANT GT2STRING
T{ GT1STRING FIND -> ' GT1 -1 }T
T{ GT2STRING FIND -> ' GT2 1 }T
( HOW TO SEARCH FOR NON-EXISTENT WORD? )
T{ : GT3 GT2 LITERAL ; -> }T
T{ GT3 -> ' GT1 }T
T{ GT1STRING COUNT -> GT1STRING CHAR+ 3 }T

T{ : GT4 IMMEDIATE POSTPONE GT1 ; -> }T
T{ : GT5 GT4 ; -> }T
T{ GT5 -> 123 }T
T{ : GT6 IMMEDIATE 345 ; -> }T
T{ : GT7 POSTPONE GT6 ; -> }T
T{ GT7 -> 345 }T

T{ : GT8 IMMEDIATE STATE@ ; -> }T
T{ GT8 -> 0 }T
T{ : GT9 GT8 LITERAL ; -> }T
T{ GT9 0= -> <FALSE> }T

\ ------------------------------------------------------------------------
TESTING IF ELSE THEN BEGIN WHILE REPEAT UNTIL RECURSE

T{ : GI1 IF 123 THEN ; -> }T
T{ : GI2 IF 123 ELSE 234 THEN ; -> }T
T{ 0 GI1 -> }T
T{ 1 GI1 -> 123 }T
T{ -1 GI1 -> 123 }T
T{ 0 GI2 -> 234 }T
T{ 1 GI2 -> 123 }T
T{ -1 GI1 -> 123 }T

T{ : GI3 BEGIN DUP 5 < WHILE DUP 1+ REPEAT ; -> }T
T{ 0 GI3 -> 0 1 2 3 4 5 }T
T{ 4 GI3 -> 4 5 }T
T{ 5 GI3 -> 5 }T
T{ 6 GI3 -> 6 }T

T{ : GI4 BEGIN DUP 1+ DUP 5 > UNTIL ; -> }T
T{ 3 GI4 -> 3 4 5 6 }T
T{ 5 GI4 -> 5 6 }T
T{ 6 GI4 -> 6 7 }T

T{ : GI5 BEGIN DUP 2 >
         WHILE DUP 5 < WHILE DUP 1+ REPEAT 123 ELSE 345 THEN ; -> }T
T{ 1 GI5 -> 1 345 }T
T{ 2 GI5 -> 2 345 }T
T{ 3 GI5 -> 3 4 5 123 }T
T{ 4 GI5 -> 4 5 123 }T
T{ 5 GI5 -> 5 123 }T

T{ : GI6 ( N -- 0,1,..N ) DUP IF DUP >R 1- RECURSE R> THEN ; -> }T
T{ 0 GI6 -> 0 }T
T{ 1 GI6 -> 0 1 }T
T{ 2 GI6 -> 0 1 2 }T
T{ 3 GI6 -> 0 1 2 3 }T
T{ 4 GI6 -> 0 1 2 3 4 }T

\ ------------------------------------------------------------------------
TESTING DO LOOP +LOOP I J UNLOOP LEAVE EXIT

T{ : GD1 DO I LOOP ; -> }T
T{ 4 1 GD1 -> 1 2 3 }T
T{ 2 -1 GD1 -> -1 0 1 }T
T{ MID-UINT+1 MID-UINT GD1 -> MID-UINT }T

T{ : GD2 DO I -1 +LOOP ; -> }T
T{ 1 4 GD2 -> 4 3 2 1 }T
T{ -1 2 GD2 -> 2 1 0 -1 }T
T{ MID-UINT MID-UINT+1 GD2 -> MID-UINT+1 MID-UINT }T

T{ : GD3 DO 1 0 DO J LOOP LOOP ; -> }T
T{ 4 1 GD3 -> 1 2 3 }T
T{ 2 -1 GD3 -> -1 0 1 }T
T{ MID-UINT+1 MID-UINT GD3 -> MID-UINT }T

T{ : GD4 DO 1 0 DO J LOOP -1 +LOOP ; -> }T
T{ 1 4 GD4 -> 4 3 2 1 }T
T{ -1 2 GD4 -> 2 1 0 -1 }T
T{ MID-UINT MID-UINT+1 GD4 -> MID-UINT+1 MID-UINT }T

T{ : GD5 123 SWAP 0 DO I 4 > IF DROP 234 LEAVE THEN LOOP ; -> }T
T{ 1 GD5 -> 123 }T
T{ 5 GD5 -> 123 }T
T{ 6 GD5 -> 234 }T

T{ : GD6  ( PAT: T{0 0},{0 0}{1 0}{1 1},{0 0}{1 0}{1 1}{2 0}{2 1}{2 2} )
   0 SWAP 0 DO
      I 1+ 0 DO I J + 3 = IF I UNLOOP I UNLOOP EXIT THEN 1+ LOOP
    LOOP ; -> }T
T{ 1 GD6 -> 1 }T
T{ 2 GD6 -> 3 }T
T{ 3 GD6 -> 4 1 2 }T

\ ------------------------------------------------------------------------
TESTING DEFINING WORDS: : ; CONSTANT VARIABLE CREATE DOES> >BODY

T{ 123 CONSTANT X123 -> }T
T{ X123 -> 123 }T
T{ : EQU CONSTANT ; -> }T
T{ X123 EQU Y123 -> }T
T{ Y123 -> 123 }T

T{ VARIABLE V1 -> }T
T{ 123 V1 ! -> }T
T{ V1 @ -> 123 }T

T{ : NOP : POSTPONE ; ; -> }T
T{ NOP NOP1 NOP NOP2 -> }T
T{ NOP1 -> }T
T{ NOP2 -> }T

T{ : DOES1 DOES> @ 1 + ; -> }T
T{ : DOES2 DOES> @ 2 + ; -> }T
T{ CREATE CR1 -> }T
T{ CR1 -> HERE }T
T{ ' CR1 >BODY -> HERE }T
T{ 1 , -> }T
T{ CR1 @ -> 1 }T
T{ DOES1 -> }T
T{ CR1 -> 2 }T
T{ DOES2 -> }T
T{ CR1 -> 3 }T

T{ : WEIRD: CREATE DOES> 1 + DOES> 2 + ; -> }T
T{ WEIRD: W1 -> }T
T{ ' W1 >BODY -> HERE }T
T{ W1 -> HERE 1 + }T
T{ W1 -> HERE 2 + }T

\ ------------------------------------------------------------------------
TESTING EVALUATE

: GE1 IMMEDIATE S" 123" ;
: GE2 IMMEDIATE S" 123 1+" ;
: GE3 S" : GE4 345 ;" ;
: GE5 IMMEDIATE EVALUATE ;

T{ GE1 EVALUATE -> 123 }T         ( TEST EVALUATE IN INTERP. STATE )
T{ GE2 EVALUATE -> 124 }T
T{ GE3 EVALUATE -> }T
T{ GE4 -> 345 }T

T{ :: GE6 GE1 GE5 ; -> }T         ( TEST EVALUATE IN COMPILE STATE )
T{ GE6 -> 123 }T
T{ :: GE7 GE2 GE5 ; -> }T
T{ GE7 -> 124 }T

\ ------------------------------------------------------------------------
TESTING SOURCE >IN WORD

: GS1 S" SOURCE" 2DUP EVALUATE
       >R SWAP >R = R> R> = ;
T{ GS1 -> <TRUE> <TRUE> }T

VARIABLE SCANS
: RESCAN?  -1 SCANS +! SCANS @ IF 0 >IN C! THEN ;

T{ 2 SCANS !
345 RESCAN?
-> 345 345 }T

: GS2  5 SCANS ! S" 123 RESCAN?" EVALUATE ;
T{ GS2 -> 123 123 123 123 123 }T

: GS3 WORD COUNT SWAP C@ ;
T{ BL GS3 HELLO -> 5 CHAR H }T
T{ CHAR " GS3 GOODBYE" -> 7 CHAR G }T
T{ BL GS3
DROP -> 0 }T            \ BLANK LINE RETURN ZERO-LENGTH STRING

: GS4 SOURCE >IN C! DROP ;
T{ GS4 123 456
-> }T

\ ------------------------------------------------------------------------
TESTING <# # #S #> HOLD SIGN BASE >NUMBER HEX DECIMAL

: S=  \ ( ADDR1 C1 ADDR2 C2 -- T/F ) COMPARE TWO STRINGS.
   >R SWAP R@ = IF         \ MAKE SURE STRINGS HAVE SAME LENGTH
      R> ?DUP IF         \ IF NON-EMPTY STRINGS
    0 DO
       OVER C@ OVER C@ - IF 2DROP <FALSE> UNLOOP EXIT THEN
       SWAP CHAR+ SWAP CHAR+
         LOOP
      THEN
      2DROP <TRUE>         \ IF WE GET HERE, STRINGS MATCH
   ELSE
      R> DROP 2DROP <FALSE>      \ LENGTHS MISMATCH
   THEN ;

: GP1  <# 41 HOLD 42 HOLD 0 0 #> S" BA" S= ;
T{ GP1 -> <TRUE> }T

: GP2  <# -1 SIGN 0 SIGN -1 SIGN 0 0 #> S" --" S= ;
T{ GP2 -> <TRUE> }T

: GP3  <# 1 0 # # #> S" 01" S= ;
T{ GP3 -> <TRUE> }T

: GP4  <# 1 0 #S #> S" 1" S= ;
T{ GP4 -> <TRUE> }T

24 CONSTANT MAX-BASE         \ BASE 2 .. 36
: COUNT-BITS
   0 0 INVERT BEGIN DUP WHILE >R 1+ R> 2* REPEAT DROP ;
COUNT-BITS 2* CONSTANT #BITS-UD      \ NUMBER OF BITS IN UD

: GP5
   BASE C@ <TRUE>
   MAX-BASE 1+ 2 DO         \ FOR EACH POSSIBLE BASE
      I BASE C!            \ TBD: ASSUMES BASE WORKS
      I 0 <# #S #> S" 10" S= AND
   LOOP
   SWAP BASE C! ;
T{ GP5 -> <TRUE> }T

: GP6
   BASE C@ >R  2 BASE C!
   MAX-UINT MAX-UINT <# #S #>      \ MAXIMUM UD TO BINARY
   R> BASE C!            \ S: C-ADDR U
   DUP #BITS-UD = SWAP
   0 DO               \ S: C-ADDR FLAG
      OVER C@ [CHAR] 1 = AND      \ ALL ONES
      >R CHAR+ R>
   LOOP SWAP DROP ;
T{ GP6 -> <TRUE> }T

: GP7
   BASE C@ >R    MAX-BASE BASE C!
   <TRUE>
   A 0 DO
      I 0 <# #S #>
      1 = SWAP C@ I 30 + = AND AND
   LOOP
   MAX-BASE A DO
      I 0 <# #S #>
      1 = SWAP C@ 41 I A - + = AND AND
   LOOP
   R> BASE C! ;

T{ GP7 -> <TRUE> }T

\ >NUMBER TESTS
CREATE GN-BUF 0 C,
: GN-STRING   GN-BUF 1 ;
: GN-CONSUMED   GN-BUF CHAR+ 0 ;
: GN'      [CHAR] ' WORD CHAR+ C@ GN-BUF C!  GN-STRING ;

T{ 0 0 GN' 0' >NUMBER -> 0 0 GN-CONSUMED }T
T{ 0 0 GN' 1' >NUMBER -> 1 0 GN-CONSUMED }T
T{ 1 0 GN' 1' >NUMBER -> BASE C@ 1+ 0 GN-CONSUMED }T
T{ 0 0 GN' -' >NUMBER -> 0 0 GN-STRING }T   \ SHOULD FAIL TO CONVERT THESE
T{ 0 0 GN' +' >NUMBER -> 0 0 GN-STRING }T
T{ 0 0 GN' .' >NUMBER -> 0 0 GN-STRING }T

: >NUMBER-BASED
   BASE C@ >R BASE C! >NUMBER R> BASE C! ;

T{ 0 0 GN' 2' 10 >NUMBER-BASED -> 2 0 GN-CONSUMED }T
T{ 0 0 GN' 2'  2 >NUMBER-BASED -> 0 0 GN-STRING }T
T{ 0 0 GN' F' 10 >NUMBER-BASED -> F 0 GN-CONSUMED }T
T{ 0 0 GN' G' 10 >NUMBER-BASED -> 0 0 GN-STRING }T
T{ 0 0 GN' G' MAX-BASE >NUMBER-BASED -> 10 0 GN-CONSUMED }T
T{ 0 0 GN' Z' MAX-BASE >NUMBER-BASED -> 23 0 GN-CONSUMED }T

: GN1   \ ( UD BASE -- UD' LEN ) UD SHOULD EQUAL UD' AND LEN SHOULD BE ZERO.
   BASE C@ >R BASE C!
   <# #S #>
   0 0 2SWAP >NUMBER SWAP DROP      \ RETURN LENGTH ONLY
   R> BASE C! ;
T{ 0 0 2 GN1 -> 0 0 0 }T
T{ MAX-UINT 0 2 GN1 -> MAX-UINT 0 0 }T
T{ MAX-UINT DUP 2 GN1 -> MAX-UINT DUP 0 }T
T{ 0 0 MAX-BASE GN1 -> 0 0 0 }T
T{ MAX-UINT 0 MAX-BASE GN1 -> MAX-UINT 0 0 }T
T{ MAX-UINT DUP MAX-BASE GN1 -> MAX-UINT DUP 0 }T

: GN2   \ ( -- 16 10 )
   BASE C@ >R  HEX BASE C@  DECIMAL BASE C@  R> BASE C! ;
T{ GN2 -> 10 A }T

\ ------------------------------------------------------------------------
TESTING FILL MOVE

CREATE FBUF 00 C, 00 C, 00 C,
CREATE SBUF 12 C, 34 C, 56 C,
: SEEBUF FBUF C@  FBUF CHAR+ C@  FBUF CHAR+ CHAR+ C@ ;

T{ FBUF 0 20 FILL -> }T
T{ SEEBUF -> 00 00 00 }T

T{ FBUF 1 20 FILL -> }T
T{ SEEBUF -> 20 00 00 }T

T{ FBUF 3 20 FILL -> }T
T{ SEEBUF -> 20 20 20 }T

T{ FBUF FBUF 3 CHARS MOVE -> }T      \ BIZARRE SPECIAL CASE
T{ SEEBUF -> 20 20 20 }T

T{ SBUF FBUF 0 CHARS MOVE -> }T
T{ SEEBUF -> 20 20 20 }T

T{ SBUF FBUF 1 CHARS MOVE -> }T
T{ SEEBUF -> 12 20 20 }T

T{ SBUF FBUF 3 CHARS MOVE -> }T
T{ SEEBUF -> 12 34 56 }T

T{ FBUF FBUF CHAR+ 2 CHARS MOVE -> }T
T{ SEEBUF -> 12 12 34 }T

T{ FBUF CHAR+ FBUF 2 CHARS MOVE -> }T
T{ SEEBUF -> 12 34 34 }T

\ ------------------------------------------------------------------------
TESTING OUTPUT: . ." CR EMIT SPACE SPACES TYPE U.

: OUTPUT-TEST
   ." YOU SHOULD SEE THE STANDARD GRAPHIC CHARACTERS:" CR
   41 BL DO I EMIT LOOP CR
   61 41 DO I EMIT LOOP CR
   7F 61 DO I EMIT LOOP CR
   ." YOU SHOULD SEE 0-9 SEPARATED BY A SPACE:" CR
   9 1+ 0 DO I . LOOP CR
   ." YOU SHOULD SEE 0-9 (WITH NO SPACES):" CR
   [CHAR] 9 1+ [CHAR] 0 DO I 0 SPACES EMIT LOOP CR
   ." YOU SHOULD SEE A-G SEPARATED BY A SPACE:" CR
   [CHAR] G 1+ [CHAR] A DO I EMIT SPACE LOOP CR
   ." YOU SHOULD SEE 0-5 SEPARATED BY TWO SPACES:" CR
   5 1+ 0 DO I [CHAR] 0 + EMIT 2 SPACES LOOP CR
   ." YOU SHOULD SEE TWO SEPARATE LINES:" CR
   S" LINE 1" TYPE CR S" LINE 2" TYPE CR
   ." YOU SHOULD SEE THE NUMBER RANGES OF SIGNED AND UNSIGNED NUMBERS:" CR
   ."   SIGNED: " MIN-INT . MAX-INT . CR
   ." UNSIGNED: " 0 U. MAX-UINT U. CR
;

T{ OUTPUT-TEST -> }T


\ ------------------------------------------------------------------------
TESTING INPUT: ACCEPT

CREATE ABUF 50 CHARS ALLOT

: ACCEPT-TEST
   CR ." PLEASE TYPE UP TO 80 CHARACTERS:" CR
   ABUF 50 ACCEPT
   CR ." RECEIVED: " [CHAR] " EMIT
   ABUF SWAP TYPE [CHAR] " EMIT CR
;

T{ ACCEPT-TEST -> }T

\ ------------------------------------------------------------------------
TESTING DICTIONARY SEARCH RULES

T{ : GDX   123 ; : GDX   GDX 234 ; -> }T

T{ GDX -> 123 234 }T

CR .( End of Core word set tests) CR


\ Additional tests on the the ANS Forth Core word set

\ This program was written by Gerry Jackson in 2007, with contributions from
\ others where indicated, and is in the public domain - it can be distributed
\ and/or modified in any way but please retain this notice.

\ This program is distributed in the hope that it will be useful,
\ but WITHOUT ANY WARRANTY; without even the implied warranty of
\ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\ The tests are not claimed to be comprehensive or correct 

\ ------------------------------------------------------------------------------
\ The tests are based on John Hayes test program for the core word set
\
\ This file provides some more tests on Core words where the original Hayes
\ tests are thought to be incomplete
\
\ Words tested in this file are:
\     DO I +LOOP RECURSE ELSE >IN IMMEDIATE FIND IF...BEGIN...REPEAT ALLOT DOES>
\ and
\     Parsing behaviour
\     Number prefixes # $ % and 'A' character input
\     Definition names
\ ------------------------------------------------------------------------------
\ Assumptions and dependencies:
\     - tester.fr or ttester.fs has been loaded prior to this file
\     - core.fr has been loaded so that constants <TRUE> MAX-INT, MIN-INT and
\       MAX-UINT are defined
\ ------------------------------------------------------------------------------

DECIMAL

TESTING DO +LOOP with run-time increment, negative increment, infinite loop
\ Contributed by Reinhold Straub

VARIABLE ITERATIONS
VARIABLE INCREMENT
: GD7 ( LIMIT START INCREMENT -- )
   INCREMENT !
   0 ITERATIONS !
   DO
      1 ITERATIONS +!
      I
      ITERATIONS @  6 = IF LEAVE THEN
      INCREMENT @
   +LOOP ITERATIONS @
;

T{  4  4 -1 GD7 -> 4 1 }T
T{  1  4 -1 GD7 -> 4 3 2 1 4 }T
T{  4  1 -1 GD7 -> 1 0 -1 -2 -3 -4 6 }T
T{  4  1  0 GD7 -> 1 1 1 1 1 1 6 }T
T{  0  0  0 GD7 -> 0 0 0 0 0 0 6 }T
T{  1  4  0 GD7 -> 4 4 4 4 4 4 6 }T
T{  1  4  1 GD7 -> 4 5 6 7 8 9 6 }T
T{  4  1  1 GD7 -> 1 2 3 3 }T
T{  4  4  1 GD7 -> 4 5 6 7 8 9 6 }T
T{  2 -1 -1 GD7 -> -1 -2 -3 -4 -5 -6 6 }T
T{ -1  2 -1 GD7 -> 2 1 0 -1 4 }T
T{  2 -1  0 GD7 -> -1 -1 -1 -1 -1 -1 6 }T
T{ -1  2  0 GD7 -> 2 2 2 2 2 2 6 }T
T{ -1  2  1 GD7 -> 2 3 4 5 6 7 6 }T
T{  2 -1  1 GD7 -> -1 0 1 3 }T
T{ -20 30 -10 GD7 -> 30 20 10 0 -10 -20 6 }T
T{ -20 31 -10 GD7 -> 31 21 11 1 -9 -19 6 }T
T{ -20 29 -10 GD7 -> 29 19 9 -1 -11 5 }T

\ ------------------------------------------------------------------------------
TESTING DO +LOOP with large and small increments

\ Contributed by Andrew Haley

MAX-UINT 8 RSHIFT 1+ CONSTANT USTEP
USTEP NEGATE CONSTANT -USTEP
MAX-INT 7 RSHIFT 1+ CONSTANT STEP
STEP NEGATE CONSTANT -STEP

VARIABLE BUMP

T{ : GD8 BUMP ! DO 1+ BUMP @ +LOOP ; -> }T

T{ 0 MAX-UINT 0 USTEP GD8 -> 256 }T
T{ 0 0 MAX-UINT -USTEP GD8 -> 256 }T

T{ 0 MAX-INT MIN-INT STEP GD8 -> 256 }T
T{ 0 MIN-INT MAX-INT -STEP GD8 -> 256 }T

\ Two's complement arithmetic, wraps around modulo wordsize
\ Only tested if the Forth system does wrap around, use of conditional
\ compilation deliberately avoided

MAX-INT 1+ MIN-INT = CONSTANT +WRAP?
MIN-INT 1- MAX-INT = CONSTANT -WRAP?
MAX-UINT 1+ 0=       CONSTANT +UWRAP?
0 1- MAX-UINT =      CONSTANT -UWRAP?

: GD9  ( n limit start step f result -- )
   >R IF GD8 ELSE 2DROP 2DROP R@ THEN -> R> }T
;

T{ 0 0 0  USTEP +UWRAP? 256 GD9
T{ 0 0 0 -USTEP -UWRAP?   1 GD9
T{ 0 MIN-INT MAX-INT  STEP +WRAP? 1 GD9
T{ 0 MAX-INT MIN-INT -STEP -WRAP? 1 GD9

\ ------------------------------------------------------------------------------
TESTING DO +LOOP with maximum and minimum increments

: (-MI) MAX-INT DUP NEGATE + 0= IF MAX-INT NEGATE ELSE -32767 THEN ;
(-MI) CONSTANT -MAX-INT

T{ 0 1 0 MAX-INT GD8  -> 1 }T
T{ 0 -MAX-INT NEGATE -MAX-INT OVER GD8  -> 2 }T

T{ 0 MAX-INT  0 MAX-INT GD8  -> 1 }T
T{ 0 MAX-INT  1 MAX-INT GD8  -> 1 }T
T{ 0 MAX-INT -1 MAX-INT GD8  -> 2 }T
T{ 0 MAX-INT DUP 1- MAX-INT GD8  -> 1 }T

T{ 0 MIN-INT 1+   0 MIN-INT GD8  -> 1 }T
T{ 0 MIN-INT 1+  -1 MIN-INT GD8  -> 1 }T
T{ 0 MIN-INT 1+   1 MIN-INT GD8  -> 2 }T
T{ 0 MIN-INT 1+ DUP MIN-INT GD8  -> 1 }T

\ ------------------------------------------------------------------------------
\ TESTING +LOOP setting I to an arbitrary value

\ The specification for +LOOP permits the loop index I to be set to any value
\ including a value outside the range given to the corresponding  DO.

\ SET-I is a helper to set I in a DO ... +LOOP to a given value
\ n2 is the value of I in a DO ... +LOOP
\ n3 is a test value
\ If n2=n3 then return n1-n2 else return 1
: SET-I  ( n1 n2 n3 -- n1-n2 | 1 ) 
   OVER = IF - ELSE 2DROP 1 THEN
;

: -SET-I ( n1 n2 n3 -- n1-n2 | -1 )
   SET-I DUP 1 = IF NEGATE THEN
;

: PL1 20 1 DO I 18 I 3 SET-I +LOOP ;
T{ PL1 -> 1 2 3 18 19 }T
: PL2 20 1 DO I 20 I 2 SET-I +LOOP ;
T{ PL2 -> 1 2 }T
: PL3 20 5 DO I 19 I 2 SET-I DUP 1 = IF DROP 0 I 6 SET-I THEN +LOOP ;
T{ PL3 -> 5 6 0 1 2 19 }T
: PL4 20 1 DO I MAX-INT I 4 SET-I +LOOP ;
T{ PL4 -> 1 2 3 4 }T
: PL5 -20 -1 DO I -19 I -3 -SET-I +LOOP ;
T{ PL5 -> -1 -2 -3 -19 -20 }T
: PL6 -20 -1 DO I -21 I -4 -SET-I +LOOP ;
T{ PL6 -> -1 -2 -3 -4 }T
: PL7 -20 -1 DO I MIN-INT I -5 -SET-I +LOOP ;
T{ PL7 -> -1 -2 -3 -4 -5 }T
: PL8 -20 -5 DO I -20 I -2 -SET-I DUP -1 = IF DROP 0 I -6 -SET-I THEN +LOOP ;
T{ PL8 -> -5 -6 0 -1 -2 -20 }T

\ ------------------------------------------------------------------------------
TESTING multiple RECURSEs in one colon definition

: ACK ( m n -- u )    \ Ackermann function, from Rosetta Code
   OVER 0= IF  NIP 1+ EXIT  THEN       \ ack(0, n) = n+1
   SWAP 1- SWAP                        ( -- m-1 n )
   DUP  0= IF  1+  RECURSE EXIT  THEN  \ ack(m, 0) = ack(m-1, 1)
   1- OVER 1+ SWAP RECURSE RECURSE     \ ack(m, n) = ack(m-1, ack(m,n-1))
;

T{ 0 0 ACK ->  1 }T
T{ 3 0 ACK ->  5 }T
T{ 2 4 ACK -> 11 }T

\ ------------------------------------------------------------------------------
TESTING multiple ELSE's in an IF statement
\ Discussed on comp.lang.forth and accepted as valid ANS Forth

: MELSE IF 1 ELSE 2 ELSE 3 ELSE 4 ELSE 5 THEN ;
T{ 0 MELSE -> 2 4 }T
T{ -1 MELSE -> 1 3 5 }T

\ ------------------------------------------------------------------------------
\ TESTING manipulation of >IN in interpreter mode

\ changing ! to C! and +! to C+! messes up the character offsets and makes the
\ test fail. rather than recompute the values, just define aliases
: _ C! ; : +_ C+! ;
T{ 12345 DEPTH OVER 9 < 34 AND + 3 + >IN _ -> 12345 2345 345 45 5 }T
T{ 14145 8115 ?DUP 0= 34 AND >IN +_ TUCK MOD 14 >IN _ GCD CALCULATION -> 15 }T

\ ------------------------------------------------------------------------------
\ TESTING IMMEDIATE with CONSTANT  VARIABLE and CREATE [ ... DOES> ]
\ not currently supported by this implementation
SKIPPED T{ 123 CONSTANT IW1 IMMEDIATE IW1 -> 123 }T
SKIPPED T{ : IW2 IW1 LITERAL ; IW2 -> 123 }T
SKIPPED T{ VARIABLE IW3 IMMEDIATE 234 IW3 ! IW3 @ -> 234 }T
SKIPPED T{ : IW4 IW3 [ @ ] LITERAL ; IW4 -> 234 }T
SKIPPED T{ :NONAME [ 345 ] IW3 [ ! ] ; DROP IW3 @ -> 345 }T
SKIPPED T{ CREATE IW5 456 , IMMEDIATE -> }T
SKIPPED T{ :NONAME IW5 [ @ IW3 ! ] ; DROP IW3 @ -> 456 }T
SKIPPED T{ : IW6 CREATE , IMMEDIATE DOES> @ 1+ ; -> }T
SKIPPED T{ 111 IW6 IW7 IW7 -> 112 }T
SKIPPED T{ : IW8 IW7 LITERAL 1+ ; IW8 -> 113 }T
SKIPPED T{ : IW9 CREATE , DOES> @ 2 + IMMEDIATE ; -> }T
SKIPPED : FIND-IW BL WORD FIND NIP ;  ( -- 0 | 1 | -1 )
SKIPPED T{ 222 IW9 IW10 FIND-IW IW10 -> -1 }T   \ IW10 is not immediate
SKIPPED T{ IW10 FIND-IW IW10 -> 224 1 }T        \ IW10 becomes immediate

\ ------------------------------------------------------------------------------
TESTING that IMMEDIATE doesn't toggle a flag

VARIABLE IT1 0 IT1 !
: IT2 IMMEDIATE IMMEDIATE 1234 IT1 ! ;
T{ : IT3 IT2 ; IT1 @ -> 1234 }T

\ ------------------------------------------------------------------------------
TESTING parsing behaviour of S" ." and (
\ which should parse to just beyond the terminating character no space needed

T{ : GC5 S" A string"2DROP ; GC5 -> }T
T{ ( A comment)1234 -> 1234 }T
T{ : PB1 CR ." You should see 2345: "." 2345"( A comment) CR ; PB1 -> }T
 
\ ------------------------------------------------------------------------------
TESTING number prefixes # $ % and 'c' character input
\ Adapted from the Forth 200X Draft 14.5 document

VARIABLE OLD-BASE
DECIMAL BASE C@ OLD-BASE C!
T{ #1289 -> 1289 }T
T{ #-1289 -> -1289 }T
T{ $12eF -> 4847 }T
T{ $-12eF -> -4847 }T
T{ %10010110 -> 150 }T
T{ %-10010110 -> -150 }T
T{ 'z' -> 122 }T
T{ 'Z' -> 90 }T
\ Check BASE is unchanged
T{ BASE C@ OLD-BASE C@ = -> <TRUE> }T

\ Repeat in Hex mode
16 OLD-BASE C! 16 BASE C!
T{ #1289 -> 509 }T
T{ #-1289 -> -509 }T
T{ $12eF -> 12EF }T
T{ $-12eF -> -12EF }T
T{ %10010110 -> 96 }T
\ T{ %-10010110 -> -96 }T
T{ 'z' -> 7a }T
T{ 'Z' -> 5a }T
\ Check BASE is unchanged
T{ BASE C@ OLD-BASE C@ = -> <TRUE> }T   \ 2

DECIMAL
\ Check number prefixes in compile mode
T{ : nmp  #8327 $-2cbe %011010111 ''' ; nmp -> 8327 -11454 215 39 }T

\ ------------------------------------------------------------------------------
TESTING definition names
\ should support {1..31} graphical characters
: !"#$%&'()*+,-./0123456789:;<=>? 1 ;
T{ !"#$%&'()*+,-./0123456789:;<=>? -> 1 }T
: @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^ 2 ;
T{ @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^ -> 2 }T
: _`abcdefghijklmnopqrstuvwxyz{|} 3 ;
T{ _`abcdefghijklmnopqrstuvwxyz{|} -> 3 }T
: _`abcdefghijklmnopqrstuvwxyz{|~ 4 ;     \ Last character different
T{ _`abcdefghijklmnopqrstuvwxyz{|~ -> 4 }T
T{ _`abcdefghijklmnopqrstuvwxyz{|} -> 3 }T

\ ------------------------------------------------------------------------------
TESTING FIND with a zero length string and a non-existent word

CREATE EMPTYSTRING 0 C,
: EMPTYSTRING-FIND-CHECK ( c-addr 0 | xt 1 | xt -1 -- t|f )
    DUP IF ." FIND returns a TRUE value for an empty string!" CR THEN
    0= SWAP EMPTYSTRING = = ;
T{ EMPTYSTRING FIND EMPTYSTRING-FIND-CHECK -> <TRUE> }T

CREATE NON-EXISTENT-WORD   \ Same as in exceptiontest.fth
       15 C, CHAR $ C, CHAR $ C, CHAR Q C, CHAR W C, CHAR E C, CHAR Q C,
   CHAR W C, CHAR E C, CHAR Q C, CHAR W C, CHAR E C, CHAR R C, CHAR T C,
   CHAR $ C, CHAR $ C,
T{ NON-EXISTENT-WORD FIND -> NON-EXISTENT-WORD 0 }T

\ ------------------------------------------------------------------------------
TESTING IF ... BEGIN ... REPEAT (unstructured)

T{ : UNS1 DUP 0 > IF 9 SWAP BEGIN 1+ DUP 3 > IF EXIT THEN REPEAT ; -> }T
T{ -6 UNS1 -> -6 }T
T{  1 UNS1 -> 9 4 }T

\ ------------------------------------------------------------------------------
TESTING DOES> doesn't cause a problem with a CREATEd address

: MAKE-2CONST DOES> 2@ ;
T{ CREATE 2K 3 , 2K , MAKE-2CONST 2K -> ' 2K >BODY 3 }T

\ ------------------------------------------------------------------------------
TESTING ALLOT ( n -- ) where n <= 0

T{ HERE 5 ALLOT -5 ALLOT HERE = -> <TRUE> }T
T{ HERE 0 ALLOT HERE = -> <TRUE> }T
 
\ ------------------------------------------------------------------------------

CR .( End of additional Core tests) CR

: STR1  S" abcd" ;  : STR2  S" abcde" ;
: STR3  S" abCd" ;  : STR4  S" wbcd"  ;
: S"" S" " ;

\ To test the ANS Forth Core Extension word set

\ This program was written by Gerry Jackson in 2006, with contributions from
\ others where indicated, and is in the public domain - it can be distributed
\ and/or modified in any way but please retain this notice.

\ This program is distributed in the hope that it will be useful,
\ but WITHOUT ANY WARRANTY; without even the implied warranty of
\ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\ The tests are not claimed to be comprehensive or correct 

\ ------------------------------------------------------------------------------
\ Version 0.13 28 October 2015
\              Replace <FALSE> and <TRUE> with FALSE and TRUE to avoid
\              dependence on Core tests
\              Moved SAVE-INPUT and RESTORE-INPUT tests in a file to filetest.fth
\              Use of 2VARIABLE (from optional wordset) replaced with CREATE.
\              Minor lower to upper case conversions.
\              Calls to COMPARE replaced by S= (in utilities.fth) to avoid use
\              of a word from an optional word set.
\              UNUSED tests revised as UNUSED UNUSED = may return FALSE when an
\              implementation has the data stack sharing unused dataspace.
\              Double number input dependency removed from the HOLDS tests.
\              Minor case sensitivities removed in definition names.
\         0.11 25 April 2015
\              Added tests for PARSE-NAME HOLDS BUFFER:
\              S\" tests added
\              DEFER IS ACTION-OF DEFER! DEFER@ tests added
\              Empty CASE statement test added
\              [COMPILE] tests removed because it is obsolescent in Forth 2012
\         0.10 1 August 2014
\             Added tests contributed by James Bowman for:
\                <> U> 0<> 0> NIP TUCK ROLL PICK 2>R 2R@ 2R>
\                HEX WITHIN UNUSED AGAIN MARKER
\             Added tests for:
\                .R U.R ERASE PAD REFILL SOURCE-ID 
\             Removed ABORT from NeverExecuted to enable Win32
\             to continue after failure of RESTORE-INPUT.
\             Removed max-intx which is no longer used.
\         0.7 6 June 2012 Extra CASE test added
\         0.6 1 April 2012 Tests placed in the public domain.
\             SAVE-INPUT & RESTORE-INPUT tests, position
\             of T{ moved so that tests work with ttester.fs
\             CONVERT test deleted - obsolete word removed from Forth 200X
\             IMMEDIATE VALUEs tested
\             RECURSE with :NONAME tested
\             PARSE and .( tested
\             Parsing behaviour of C" added
\         0.5 14 September 2011 Removed the double [ELSE] from the
\             initial SAVE-INPUT & RESTORE-INPUT test
\         0.4 30 November 2009  max-int replaced with max-intx to
\             avoid redefinition warnings.
\         0.3  6 March 2009 { and } replaced with T{ and }T
\                           CONVERT test now independent of cell size
\         0.2  20 April 2007 ANS Forth words changed to upper case
\                            Tests qd3 to qd6 by Reinhold Straub
\         0.1  Oct 2006 First version released
\ -----------------------------------------------------------------------------
\ The tests are based on John Hayes test program for the core word set

\ Words tested in this file are:
\     .( .R 0<> 0> 2>R 2R> 2R@ :NONAME <> ?DO AGAIN C" CASE COMPILE, ENDCASE
\     ENDOF ERASE FALSE HEX MARKER NIP OF PAD PARSE PICK REFILL
\     RESTORE-INPUT ROLL SAVE-INPUT SOURCE-ID TO TRUE TUCK U.R U> UNUSED
\     VALUE WITHIN [COMPILE]

\ Words not tested or partially tested:
\     \ because it has been extensively used already and is, hence, unnecessary
\     REFILL and SOURCE-ID from the user input device which are not possible
\     when testing from a file such as this one
\     UNUSED (partially tested) as the value returned is system dependent
\     Obsolescent words #TIB CONVERT EXPECT QUERY SPAN TIB as they have been
\     removed from the Forth 2012 standard

\ Results from words that output to the user output device have to visually
\ checked for correctness. These are .R U.R .(

\ -----------------------------------------------------------------------------
\ Assumptions & dependencies:
\     - tester.fr (or ttester.fs), errorreport.fth and utilities.fth have been
\       included prior to this file
\     - the Core word set available
\ -----------------------------------------------------------------------------
TESTING Core Extension words

DECIMAL

TESTING TRUE FALSE

T{ TRUE  -> 0 INVERT }T
T{ FALSE -> 0 }T

\ -----------------------------------------------------------------------------
TESTING <> U>   (contributed by James Bowman)

T{ 0 0 <> -> FALSE }T
T{ 1 1 <> -> FALSE }T
T{ -1 -1 <> -> FALSE }T
T{ 1 0 <> -> TRUE }T
T{ -1 0 <> -> TRUE }T
T{ 0 1 <> -> TRUE }T
T{ 0 -1 <> -> TRUE }T

T{ 0 1 U> -> FALSE }T
T{ 1 2 U> -> FALSE }T
T{ 0 MID-UINT U> -> FALSE }T
T{ 0 MAX-UINT U> -> FALSE }T
T{ MID-UINT MAX-UINT U> -> FALSE }T
T{ 0 0 U> -> FALSE }T
T{ 1 1 U> -> FALSE }T
T{ 1 0 U> -> TRUE }T
T{ 2 1 U> -> TRUE }T
T{ MID-UINT 0 U> -> TRUE }T
T{ MAX-UINT 0 U> -> TRUE }T
T{ MAX-UINT MID-UINT U> -> TRUE }T

\ -----------------------------------------------------------------------------
TESTING 0<> 0>   (contributed by James Bowman)

T{ 0 0<> -> FALSE }T
T{ 1 0<> -> TRUE }T
T{ 2 0<> -> TRUE }T
T{ -1 0<> -> TRUE }T
T{ MAX-UINT 0<> -> TRUE }T
T{ MIN-INT 0<> -> TRUE }T
T{ MAX-INT 0<> -> TRUE }T

T{ 0 0> -> FALSE }T
T{ -1 0> -> FALSE }T
T{ MIN-INT 0> -> FALSE }T
T{ 1 0> -> TRUE }T
T{ MAX-INT 0> -> TRUE }T

\ -----------------------------------------------------------------------------
TESTING NIP TUCK ROLL PICK   (contributed by James Bowman)

T{ 1 2 NIP -> 2 }T
T{ 1 2 3 NIP -> 1 3 }T

T{ 1 2 TUCK -> 2 1 2 }T
T{ 1 2 3 TUCK -> 1 3 2 3 }T

T{ : RO5 100 200 300 400 500 ; -> }T
T{ RO5 3 ROLL -> 100 300 400 500 200 }T
T{ RO5 2 ROLL -> RO5 ROT }T
T{ RO5 1 ROLL -> RO5 SWAP }T
T{ RO5 0 ROLL -> RO5 }T

T{ RO5 2 PICK -> 100 200 300 400 500 300 }T
T{ RO5 1 PICK -> RO5 OVER }T
T{ RO5 0 PICK -> RO5 DUP }T

\ -----------------------------------------------------------------------------
TESTING 2>R 2R@ 2R>   (contributed by James Bowman)

T{ : RR0 2>R 100 R> R> ; -> }T
T{ 300 400 RR0 -> 100 400 300 }T
T{ 200 300 400 RR0 -> 200 100 400 300 }T

T{ : RR1 2>R 100 2R@ R> R> ; -> }T
T{ 300 400 RR1 -> 100 300 400 400 300 }T
T{ 200 300 400 RR1 -> 200 100 300 400 400 300 }T

T{ : RR2 2>R 100 2R> ; -> }T
T{ 300 400 RR2 -> 100 300 400 }T
T{ 200 300 400 RR2 -> 200 100 300 400 }T

\ -----------------------------------------------------------------------------
TESTING HEX   (contributed by James Bowman)

T{ BASE C@ HEX BASE C@ DECIMAL BASE C@ - SWAP BASE C! -> 6 }T

\ -----------------------------------------------------------------------------
TESTING WITHIN   (contributed by James Bowman)

T{ 0 0 0 WITHIN -> FALSE }T
T{ 0 0 MID-UINT WITHIN -> TRUE }T
T{ 0 0 MID-UINT+1 WITHIN -> TRUE }T
T{ 0 0 MAX-UINT WITHIN -> TRUE }T
T{ 0 MID-UINT 0 WITHIN -> FALSE }T
T{ 0 MID-UINT MID-UINT WITHIN -> FALSE }T
T{ 0 MID-UINT MID-UINT+1 WITHIN -> FALSE }T
T{ 0 MID-UINT MAX-UINT WITHIN -> FALSE }T
T{ 0 MID-UINT+1 0 WITHIN -> FALSE }T
T{ 0 MID-UINT+1 MID-UINT WITHIN -> TRUE }T
T{ 0 MID-UINT+1 MID-UINT+1 WITHIN -> FALSE }T
T{ 0 MID-UINT+1 MAX-UINT WITHIN -> FALSE }T
T{ 0 MAX-UINT 0 WITHIN -> FALSE }T
T{ 0 MAX-UINT MID-UINT WITHIN -> TRUE }T
T{ 0 MAX-UINT MID-UINT+1 WITHIN -> TRUE }T
T{ 0 MAX-UINT MAX-UINT WITHIN -> FALSE }T
T{ MID-UINT 0 0 WITHIN -> FALSE }T
T{ MID-UINT 0 MID-UINT WITHIN -> FALSE }T
T{ MID-UINT 0 MID-UINT+1 WITHIN -> TRUE }T
T{ MID-UINT 0 MAX-UINT WITHIN -> TRUE }T
T{ MID-UINT MID-UINT 0 WITHIN -> TRUE }T
T{ MID-UINT MID-UINT MID-UINT WITHIN -> FALSE }T
T{ MID-UINT MID-UINT MID-UINT+1 WITHIN -> TRUE }T
T{ MID-UINT MID-UINT MAX-UINT WITHIN -> TRUE }T
T{ MID-UINT MID-UINT+1 0 WITHIN -> FALSE }T
T{ MID-UINT MID-UINT+1 MID-UINT WITHIN -> FALSE }T
T{ MID-UINT MID-UINT+1 MID-UINT+1 WITHIN -> FALSE }T
T{ MID-UINT MID-UINT+1 MAX-UINT WITHIN -> FALSE }T
T{ MID-UINT MAX-UINT 0 WITHIN -> FALSE }T
T{ MID-UINT MAX-UINT MID-UINT WITHIN -> FALSE }T
T{ MID-UINT MAX-UINT MID-UINT+1 WITHIN -> TRUE }T
T{ MID-UINT MAX-UINT MAX-UINT WITHIN -> FALSE }T
T{ MID-UINT+1 0 0 WITHIN -> FALSE }T
T{ MID-UINT+1 0 MID-UINT WITHIN -> FALSE }T
T{ MID-UINT+1 0 MID-UINT+1 WITHIN -> FALSE }T
T{ MID-UINT+1 0 MAX-UINT WITHIN -> TRUE }T
T{ MID-UINT+1 MID-UINT 0 WITHIN -> TRUE }T
T{ MID-UINT+1 MID-UINT MID-UINT WITHIN -> FALSE }T
T{ MID-UINT+1 MID-UINT MID-UINT+1 WITHIN -> FALSE }T
T{ MID-UINT+1 MID-UINT MAX-UINT WITHIN -> TRUE }T
T{ MID-UINT+1 MID-UINT+1 0 WITHIN -> TRUE }T
T{ MID-UINT+1 MID-UINT+1 MID-UINT WITHIN -> TRUE }T
T{ MID-UINT+1 MID-UINT+1 MID-UINT+1 WITHIN -> FALSE }T
T{ MID-UINT+1 MID-UINT+1 MAX-UINT WITHIN -> TRUE }T
T{ MID-UINT+1 MAX-UINT 0 WITHIN -> FALSE }T
T{ MID-UINT+1 MAX-UINT MID-UINT WITHIN -> FALSE }T
T{ MID-UINT+1 MAX-UINT MID-UINT+1 WITHIN -> FALSE }T
T{ MID-UINT+1 MAX-UINT MAX-UINT WITHIN -> FALSE }T
T{ MAX-UINT 0 0 WITHIN -> FALSE }T
T{ MAX-UINT 0 MID-UINT WITHIN -> FALSE }T
T{ MAX-UINT 0 MID-UINT+1 WITHIN -> FALSE }T
T{ MAX-UINT 0 MAX-UINT WITHIN -> FALSE }T
T{ MAX-UINT MID-UINT 0 WITHIN -> TRUE }T
T{ MAX-UINT MID-UINT MID-UINT WITHIN -> FALSE }T
T{ MAX-UINT MID-UINT MID-UINT+1 WITHIN -> FALSE }T
T{ MAX-UINT MID-UINT MAX-UINT WITHIN -> FALSE }T
T{ MAX-UINT MID-UINT+1 0 WITHIN -> TRUE }T
T{ MAX-UINT MID-UINT+1 MID-UINT WITHIN -> TRUE }T
T{ MAX-UINT MID-UINT+1 MID-UINT+1 WITHIN -> FALSE }T
T{ MAX-UINT MID-UINT+1 MAX-UINT WITHIN -> FALSE }T
T{ MAX-UINT MAX-UINT 0 WITHIN -> TRUE }T
T{ MAX-UINT MAX-UINT MID-UINT WITHIN -> TRUE }T
T{ MAX-UINT MAX-UINT MID-UINT+1 WITHIN -> TRUE }T
T{ MAX-UINT MAX-UINT MAX-UINT WITHIN -> FALSE }T

T{ MIN-INT MIN-INT MIN-INT WITHIN -> FALSE }T
T{ MIN-INT MIN-INT 0 WITHIN -> TRUE }T
T{ MIN-INT MIN-INT 1 WITHIN -> TRUE }T
T{ MIN-INT MIN-INT MAX-INT WITHIN -> TRUE }T
T{ MIN-INT 0 MIN-INT WITHIN -> FALSE }T
T{ MIN-INT 0 0 WITHIN -> FALSE }T
T{ MIN-INT 0 1 WITHIN -> FALSE }T
T{ MIN-INT 0 MAX-INT WITHIN -> FALSE }T
T{ MIN-INT 1 MIN-INT WITHIN -> FALSE }T
T{ MIN-INT 1 0 WITHIN -> TRUE }T
T{ MIN-INT 1 1 WITHIN -> FALSE }T
T{ MIN-INT 1 MAX-INT WITHIN -> FALSE }T
T{ MIN-INT MAX-INT MIN-INT WITHIN -> FALSE }T
T{ MIN-INT MAX-INT 0 WITHIN -> TRUE }T
T{ MIN-INT MAX-INT 1 WITHIN -> TRUE }T
T{ MIN-INT MAX-INT MAX-INT WITHIN -> FALSE }T
T{ 0 MIN-INT MIN-INT WITHIN -> FALSE }T
T{ 0 MIN-INT 0 WITHIN -> FALSE }T
T{ 0 MIN-INT 1 WITHIN -> TRUE }T
T{ 0 MIN-INT MAX-INT WITHIN -> TRUE }T
T{ 0 0 MIN-INT WITHIN -> TRUE }T
T{ 0 0 0 WITHIN -> FALSE }T
T{ 0 0 1 WITHIN -> TRUE }T
T{ 0 0 MAX-INT WITHIN -> TRUE }T
T{ 0 1 MIN-INT WITHIN -> FALSE }T
T{ 0 1 0 WITHIN -> FALSE }T
T{ 0 1 1 WITHIN -> FALSE }T
T{ 0 1 MAX-INT WITHIN -> FALSE }T
T{ 0 MAX-INT MIN-INT WITHIN -> FALSE }T
T{ 0 MAX-INT 0 WITHIN -> FALSE }T
T{ 0 MAX-INT 1 WITHIN -> TRUE }T
T{ 0 MAX-INT MAX-INT WITHIN -> FALSE }T
T{ 1 MIN-INT MIN-INT WITHIN -> FALSE }T
T{ 1 MIN-INT 0 WITHIN -> FALSE }T
T{ 1 MIN-INT 1 WITHIN -> FALSE }T
T{ 1 MIN-INT MAX-INT WITHIN -> TRUE }T
T{ 1 0 MIN-INT WITHIN -> TRUE }T
T{ 1 0 0 WITHIN -> FALSE }T
T{ 1 0 1 WITHIN -> FALSE }T
T{ 1 0 MAX-INT WITHIN -> TRUE }T
T{ 1 1 MIN-INT WITHIN -> TRUE }T
T{ 1 1 0 WITHIN -> TRUE }T
T{ 1 1 1 WITHIN -> FALSE }T
T{ 1 1 MAX-INT WITHIN -> TRUE }T
T{ 1 MAX-INT MIN-INT WITHIN -> FALSE }T
T{ 1 MAX-INT 0 WITHIN -> FALSE }T
T{ 1 MAX-INT 1 WITHIN -> FALSE }T
T{ 1 MAX-INT MAX-INT WITHIN -> FALSE }T
T{ MAX-INT MIN-INT MIN-INT WITHIN -> FALSE }T
T{ MAX-INT MIN-INT 0 WITHIN -> FALSE }T
T{ MAX-INT MIN-INT 1 WITHIN -> FALSE }T
T{ MAX-INT MIN-INT MAX-INT WITHIN -> FALSE }T
T{ MAX-INT 0 MIN-INT WITHIN -> TRUE }T
T{ MAX-INT 0 0 WITHIN -> FALSE }T
T{ MAX-INT 0 1 WITHIN -> FALSE }T
T{ MAX-INT 0 MAX-INT WITHIN -> FALSE }T
T{ MAX-INT 1 MIN-INT WITHIN -> TRUE }T
T{ MAX-INT 1 0 WITHIN -> TRUE }T
T{ MAX-INT 1 1 WITHIN -> FALSE }T
T{ MAX-INT 1 MAX-INT WITHIN -> FALSE }T
T{ MAX-INT MAX-INT MIN-INT WITHIN -> TRUE }T
T{ MAX-INT MAX-INT 0 WITHIN -> TRUE }T
T{ MAX-INT MAX-INT 1 WITHIN -> TRUE }T
T{ MAX-INT MAX-INT MAX-INT WITHIN -> FALSE }T

\ -----------------------------------------------------------------------------
TESTING UNUSED  (contributed by James Bowman & Peter Knaggs)

VARIABLE UNUSED0
T{ UNUSED DROP -> }T                  
T{ ALIGN UNUSED UNUSED0 ! 0 , UNUSED CELL+ UNUSED0 @ = -> TRUE }T
T{ UNUSED UNUSED0 ! 0 C, UNUSED CHAR+ UNUSED0 @ =
         -> TRUE }T  \ aligned -> unaligned
T{ UNUSED UNUSED0 ! 0 C, UNUSED CHAR+ UNUSED0 @ = -> TRUE }T  \ unaligned -> ?

\ -----------------------------------------------------------------------------
TESTING AGAIN   (contributed by James Bowman)

T{ : AG0 701 BEGIN DUP 7 MOD 0= IF EXIT THEN 1+ AGAIN ; -> }T
T{ AG0 -> 707 }T

\ -----------------------------------------------------------------------------
TESTING MARKER   (contributed by James Bowman)

T{ : MA? BL WORD FIND NIP 0<> ; -> }T
T{ MARKER MA0 -> }T
T{ : MA1 111 ; -> }T
T{ MARKER MA2 -> }T
T{ : MA1 222 ; -> }T
T{ MA? MA0 MA? MA1 MA? MA2 -> TRUE TRUE TRUE }T
T{ MA1 MA2 MA1 -> 222 111 }T
T{ MA? MA0 MA? MA1 MA? MA2 -> TRUE TRUE FALSE }T
T{ MA0 -> }T
T{ MA? MA0 MA? MA1 MA? MA2 -> FALSE FALSE FALSE }T

\ -----------------------------------------------------------------------------
TESTING ?DO

: QD ?DO I LOOP ;
T{ 789 789 QD -> }T
T{ -9876 -9876 QD -> }T
T{ 5 0 QD -> 0 1 2 3 4 }T

: QD1 ?DO I 10 +LOOP ;
T{ 50 1 QD1 -> 1 11 21 31 41 }T
T{ 50 0 QD1 -> 0 10 20 30 40 }T

: QD2 ?DO I 3 > IF LEAVE ELSE I THEN LOOP ;
T{ 5 -1 QD2 -> -1 0 1 2 3 }T

: QD3 ?DO I 1 +LOOP ;
T{ 4  4 QD3 -> }T
T{ 4  1 QD3 -> 1 2 3 }T
T{ 2 -1 QD3 -> -1 0 1 }T

: QD4 ?DO I -1 +LOOP ;
T{  4 4 QD4 -> }T
T{  1 4 QD4 -> 4 3 2 1 }T
T{ -1 2 QD4 -> 2 1 0 -1 }T

: QD5 ?DO I -10 +LOOP ;
T{   1 50 QD5 -> 50 40 30 20 10 }T
T{   0 50 QD5 -> 50 40 30 20 10 0 }T
T{ -25 10 QD5 -> 10 0 -10 -20 }T

VARIABLE ITERS
VARIABLE INCRMNT

: QD6 ( limit start increment -- )
   INCRMNT !
   0 ITERS !
   ?DO
      1 ITERS +!
      I
      ITERS @  6 = IF LEAVE THEN
      INCRMNT @
   +LOOP ITERS @
;

T{  4  4 -1 QD6 -> 0 }T
T{  1  4 -1 QD6 -> 4 3 2 1 4 }T
T{  4  1 -1 QD6 -> 1 0 -1 -2 -3 -4 6 }T
T{  4  1  0 QD6 -> 1 1 1 1 1 1 6 }T
T{  0  0  0 QD6 -> 0 }T
T{  1  4  0 QD6 -> 4 4 4 4 4 4 6 }T
T{  1  4  1 QD6 -> 4 5 6 7 8 9 6 }T
T{  4  1  1 QD6 -> 1 2 3 3 }T
T{  4  4  1 QD6 -> 0 }T
T{  2 -1 -1 QD6 -> -1 -2 -3 -4 -5 -6 6 }T
T{ -1  2 -1 QD6 -> 2 1 0 -1 4 }T
T{  2 -1  0 QD6 -> -1 -1 -1 -1 -1 -1 6 }T
T{ -1  2  0 QD6 -> 2 2 2 2 2 2 6 }T
T{ -1  2  1 QD6 -> 2 3 4 5 6 7 6 }T
T{  2 -1  1 QD6 -> -1 0 1 3 }T

\ -----------------------------------------------------------------------------
TESTING BUFFER:

T{ 8 BUFFER: BUF:TEST -> }T
T{ BUF:TEST DUP ALIGNED = -> TRUE }T
T{ 111 BUF:TEST ! 222 BUF:TEST CELL+ ! -> }T
T{ BUF:TEST @ BUF:TEST CELL+ @ -> 111 222 }T

\ -----------------------------------------------------------------------------
TESTING VALUE TO

T{ 111 VALUE VAL1 -999 VALUE VAL2 -> }T
T{ VAL1 -> 111 }T
T{ VAL2 -> -999 }T
T{ 222 TO VAL1 -> }T
T{ VAL1 -> 222 }T
T{ : VD1 VAL1 ; -> }T
T{ VD1 -> 222 }T
T{ : VD2 TO VAL2 ; -> }T
T{ VAL2 -> -999 }T
T{ -333 VD2 -> }T
T{ VAL2 -> -333 }T
T{ VAL1 -> 222 }T
T{ 123 VALUE VAL3 : VAL3 IMMEDIATE VAL3 ; VAL3 -> 123 }T \ modified for compatibility
T{ : VD3 VAL3 LITERAL ; VD3 -> 123 }T

\ -----------------------------------------------------------------------------
TESTING CASE OF ENDOF ENDCASE

: CS1 CASE 1 OF 111 ENDOF
           2 OF 222 ENDOF
           3 OF 333 ENDOF
           >R 999 R>
      ENDCASE
;

T{ 1 CS1 -> 111 }T
T{ 2 CS1 -> 222 }T
T{ 3 CS1 -> 333 }T
T{ 4 CS1 -> 999 }T

\ Nested CASE's

: CS2 >R CASE -1 OF CASE R@ 1 OF 100 ENDOF
                            2 OF 200 ENDOF
                           >R -300 R>
                    ENDCASE
                 ENDOF
              -2 OF CASE R@ 1 OF -99  ENDOF
                            >R -199 R>
                    ENDCASE
                 ENDOF
                 >R 299 R>
         ENDCASE R> DROP
;

T{ -1 1 CS2 ->  100 }T
T{ -1 2 CS2 ->  200 }T
T{ -1 3 CS2 -> -300 }T
T{ -2 1 CS2 -> -99  }T
T{ -2 2 CS2 -> -199 }T
T{  0 2 CS2 ->  299 }T

\ Boolean short circuiting using CASE

: CS3  ( N1 -- N2 )
   CASE 1- FALSE OF 11 ENDOF
        1- FALSE OF 22 ENDOF
        1- FALSE OF 33 ENDOF
        44 SWAP
   ENDCASE
;

T{ 1 CS3 -> 11 }T
T{ 2 CS3 -> 22 }T
T{ 3 CS3 -> 33 }T
T{ 9 CS3 -> 44 }T

\ Empty CASE statements with/without default

T{ : CS4 CASE ENDCASE ; 1 CS4 -> }T
T{ : CS5 CASE 2 SWAP ENDCASE ; 1 CS5 -> 2 }T
T{ : CS6 CASE 1 OF ENDOF 2 ENDCASE ; 1 CS6 -> }T
T{ : CS7 CASE 3 OF ENDOF 2 ENDCASE ; 1 CS7 -> 1 }T

\ -----------------------------------------------------------------------------
TESTING :NONAME RECURSE

VARIABLE NN1
VARIABLE NN2
:NONAME 1234 ; NN1 !
:NONAME 9876 ; NN2 !
T{ NN1 @ EXECUTE -> 1234 }T
T{ NN2 @ EXECUTE -> 9876 }T

T{ :NONAME ( n -- 0,1,..n ) DUP IF DUP >R 1- RECURSE R> THEN ;
   CONSTANT RN1 -> }T
T{ 0 RN1 EXECUTE -> 0 }T
T{ 4 RN1 EXECUTE -> 0 1 2 3 4 }T

:NONAME  ( n -- n1 )    \ Multiple RECURSEs in one definition
   1- DUP
   CASE 0 OF EXIT ENDOF
        1 OF 11 SWAP RECURSE ENDOF
        2 OF 22 SWAP RECURSE ENDOF
        3 OF 33 SWAP RECURSE ENDOF
        DROP ABS RECURSE EXIT
   ENDCASE
; CONSTANT RN2

T{  1 RN2 EXECUTE -> 0 }T
T{  2 RN2 EXECUTE -> 11 0 }T
T{  4 RN2 EXECUTE -> 33 22 11 0 }T
T{ 25 RN2 EXECUTE -> 33 22 11 0 }T

\ -----------------------------------------------------------------------------
TESTING C"

T{ : CQ1 C" 123" ; -> }T
T{ CQ1 COUNT EVALUATE -> 123 }T
T{ : CQ2 C" " ; -> }T
T{ CQ2 COUNT EVALUATE -> }T
T{ : CQ3 C" 2345"COUNT EVALUATE ; CQ3 -> 2345 }T

\ -----------------------------------------------------------------------------
TESTING COMPILE,

:NONAME DUP + ; CONSTANT DUP+
T{ : Q DUP+ COMPILE, ; -> }T
T{ : AS1 [ Q ] ; -> }T
T{ 123 AS1 -> 246 }T

\ -----------------------------------------------------------------------------
\ Cannot automatically test SAVE-INPUT and RESTORE-INPUT from a console source

TESTING SAVE-INPUT and RESTORE-INPUT with a string source

VARIABLE SI_INC 0 SI_INC !

: SI1
   SI_INC @ >IN C+!
   15 SI_INC !
;

: S$ S" SAVE-INPUT SI1 RESTORE-INPUT 12345" ;

T{ S$ EVALUATE SI_INC @ -> 0 2345 15 }T

\ -----------------------------------------------------------------------------
TESTING .(

CR CR .( Output from .() 
T{ CR .( You should see -9876: ) -9876 . -> }T
T{ CR .( and again: ).( -9876)CR -> }T

CR CR .( On the next 2 lines you should see First then Second messages:)
T{ : DOTP  CR ." Second message via ." [CHAR] " EMIT    \ Check .( is immediate
     [ CR ] .( First message via .( ) ; DOTP -> }T
CR CR
T{ : IMM? BL WORD FIND NIP ; IMM? .( -> 1 }T

\ -----------------------------------------------------------------------------
TESTING .R and U.R - has to handle different cell sizes

\ Create some large integers just below/above MAX and Min INTs
MAX-INT 73 79 */ CONSTANT LI1
MIN-INT 71 73 */ CONSTANT LI2

LI1 0 <# #S #> NIP CONSTANT LENLI1

: (.R&U.R)  ( u1 u2 -- )  \ u1 <= string length, u2 is required indentation
   TUCK + >R
   LI1 OVER SPACES  . CR R@    LI1 SWAP  .R CR
   LI2 OVER SPACES  . CR R@ 1+ LI2 SWAP  .R CR
   LI1 OVER SPACES U. CR R@    LI1 SWAP U.R CR
   LI2 SWAP SPACES U. CR R>    LI2 SWAP U.R CR
;

: .R&U.R  ( -- )
   CR ." You should see lines duplicated:" CR
   ." indented by 0 spaces" CR 0      0 (.R&U.R) CR
   ." indented by 0 spaces" CR LENLI1 0 (.R&U.R) CR \ Just fits required width
   ." indented by 5 spaces" CR LENLI1 5 (.R&U.R) CR
;

CR CR .( Output from .R and U.R)
T{ .R&U.R -> }T

\ -----------------------------------------------------------------------------
TESTING PAD ERASE
\ Must handle different size characters i.e. 1 CHARS >= 1 

84 CONSTANT CHARS/PAD      \ Minimum size of PAD in chars
CHARS/PAD CHARS CONSTANT AUS/PAD
: CHECKPAD  ( caddr u ch -- f )  \ f = TRUE if u chars = ch
   SWAP 0
   ?DO
      OVER I CHARS + C@ OVER <>
      IF 2DROP UNLOOP FALSE EXIT THEN
   LOOP  
   2DROP TRUE
;

T{ PAD DROP -> }T
T{ 0 INVERT PAD C! -> }T
T{ PAD C@ CONSTANT MAXCHAR -> }T
T{ PAD CHARS/PAD 2DUP MAXCHAR FILL MAXCHAR CHECKPAD -> TRUE }T
T{ PAD CHARS/PAD 2DUP CHARS ERASE 0 CHECKPAD -> TRUE }T
T{ PAD CHARS/PAD 2DUP MAXCHAR FILL PAD 0 ERASE MAXCHAR CHECKPAD -> TRUE }T
T{ PAD 43 CHARS + 9 CHARS ERASE -> }T
T{ PAD 43 MAXCHAR CHECKPAD -> TRUE }T
T{ PAD 43 CHARS + 9 0 CHECKPAD -> TRUE }T
T{ PAD 52 CHARS + CHARS/PAD 52 - MAXCHAR CHECKPAD -> TRUE }T

\ Check that use of WORD and pictured numeric output do not corrupt PAD
\ Minimum size of buffers for these are 33 chars and (2*n)+2 chars respectively
\ where n is number of bits per cell

PAD CHARS/PAD ERASE
2 BASE C!
MAX-UINT MAX-UINT <# #S CHAR 1 DUP HOLD HOLD #> 2DROP
DECIMAL
BL WORD 12345678123456781234567812345678 DROP
T{ PAD CHARS/PAD 0 CHECKPAD -> TRUE }T

\ -----------------------------------------------------------------------------
TESTING PARSE

T{ CHAR | PARSE 1234| DUP ROT ROT EVALUATE -> 4 1234 }T
T{ CHAR ^ PARSE  23 45 ^ DUP ROT ROT EVALUATE -> 7 23 45 }T
: PA1 [CHAR] $ PARSE DUP >R PAD SWAP CHARS MOVE PAD R> ;
T{ PA1 3456
   DUP ROT ROT EVALUATE -> 4 3456 }T
T{ CHAR A PARSE A SWAP DROP -> 0 }T
T{ CHAR Z PARSE
   SWAP DROP -> 0 }T
T{ CHAR " PARSE 4567 "DUP ROT ROT EVALUATE -> 5 4567 }T
 
\ -----------------------------------------------------------------------------
TESTING PARSE-NAME  (Forth 2012)
\ Adapted from the PARSE-NAME RfD tests

T{ PARSE-NAME abcd  STR1  S= -> TRUE }T        \ No leading spaces
T{ PARSE-NAME      abcde STR2 S= -> TRUE }T    \ Leading spaces

\ Test empty parse area, new lines are necessary
T{ PARSE-NAME
  NIP -> 0 }T
\ Empty parse area with spaces after PARSE-NAME
T{ PARSE-NAME         
  NIP -> 0 }T

T{ : PARSE-NAME-TEST ( "name1" "name2" -- n )
    PARSE-NAME PARSE-NAME S= ; -> }T
T{ PARSE-NAME-TEST abcd abcd  -> TRUE }T
T{ PARSE-NAME-TEST abcd   abcd  -> TRUE }T  \ Leading spaces
T{ PARSE-NAME-TEST abcde abcdf -> FALSE }T
T{ PARSE-NAME-TEST abcdf abcde -> FALSE }T
T{ PARSE-NAME-TEST abcde abcde
   -> TRUE }T         \ Parse to end of line
T{ PARSE-NAME-TEST abcde           abcde         
   -> TRUE }T         \ Leading and trailing spaces

\ -----------------------------------------------------------------------------
TESTING DEFER DEFER@ DEFER! IS ACTION-OF (Forth 2012)
\ Adapted from the Forth 200X RfD tests

T{ DEFER DEFER1 -> }T
T{ : MY-DEFER DEFER ; -> }T
T{ : IS-DEFER1 IS DEFER1 ; -> }T
T{ : ACTION-DEFER1 ACTION-OF DEFER1 ; -> }T
T{ : DEF! DEFER! ; -> }T
T{ : DEF@ DEFER@ ; -> }T

T{ ' * ' DEFER1 DEFER! -> }T
T{ 2 3 DEFER1 -> 6 }T
T{ ' DEFER1 DEFER@ -> ' * }T
T{ ' DEFER1 DEF@ -> ' * }T
T{ ACTION-OF DEFER1 -> ' * }T
T{ ACTION-DEFER1 -> ' * }T
T{ ' + IS DEFER1 -> }T
T{ 1 2 DEFER1 -> 3 }T
T{ ' DEFER1 DEFER@ -> ' + }T
T{ ' DEFER1 DEF@ -> ' + }T
T{ ACTION-OF DEFER1 -> ' + }T
T{ ACTION-DEFER1 -> ' + }T
T{ ' - IS-DEFER1 -> }T
T{ 1 2 DEFER1 -> -1 }T
T{ ' DEFER1 DEFER@ -> ' - }T
T{ ' DEFER1 DEF@ -> ' - }T
T{ ACTION-OF DEFER1 -> ' - }T
T{ ACTION-DEFER1 -> ' - }T

T{ MY-DEFER DEFER2 -> }T
T{ ' DUP IS DEFER2 -> }T
T{ 1 DEFER2 -> 1 1 }T

\ -----------------------------------------------------------------------------
TESTING HOLDS  (Forth 2012)

: HTEST S" Testing HOLDS" ;
: HTEST2 S" works" ;
: HTEST3 S" Testing HOLDS works 123" ;
T{ 0 0 <#  HTEST HOLDS #> HTEST S= -> TRUE }T
T{ 123 0 <# #S BL HOLD HTEST2 HOLDS BL HOLD HTEST HOLDS #>
   HTEST3 S= -> TRUE }T
T{ : HLD HOLDS ; -> }T
T{ 0 0 <#  HTEST HLD #> HTEST S= -> TRUE }T

\ -----------------------------------------------------------------------------
TESTING REFILL SOURCE-ID
\ REFILL and SOURCE-ID from the user input device can't be tested from a file,
\ can only be tested from a string via EVALUATE

T{ : RF1  S" REFILL" EVALUATE ; RF1 -> FALSE }T
T{ : SID1  S" SOURCE-ID" EVALUATE ; SID1 -> -1 }T

\ ------------------------------------------------------------------------------
TESTING S\"  (Forth 2012 compilation mode)
\ Extended the Forth 200X RfD tests
\ Note this tests the Core Ext definition of S\" which has unedfined
\ interpretation semantics. S\" in interpretation mode is tested in the tests on
\ the File-Access word set

T{ : SSQ1 S\" abc" S" abc" S= ; -> }T  \ No escapes
T{ SSQ1 -> TRUE }T
T{ : SSQ2 S\" " ; SSQ2 SWAP DROP -> 0 }T    \ Empty string

T{ : SSQ3 S\" \a\b\e\f\l\m\q\r\t\v\x0F0\x1Fa\xaBx\z\"\\" ; -> }T
T{ SSQ3 SWAP DROP          ->  20 }T    \ String length
T{ SSQ3 DROP            C@ ->   7 }T    \ \a   BEL  Bell
T{ SSQ3 DROP  1 CHARS + C@ ->   8 }T    \ \b   BS   Backspace
T{ SSQ3 DROP  2 CHARS + C@ ->  27 }T    \ \e   ESC  Escape
T{ SSQ3 DROP  3 CHARS + C@ ->  12 }T    \ \f   FF   Form feed
T{ SSQ3 DROP  4 CHARS + C@ ->  10 }T    \ \l   LF   Line feed
T{ SSQ3 DROP  5 CHARS + C@ ->  13 }T    \ \m        CR of CR/LF pair
T{ SSQ3 DROP  6 CHARS + C@ ->  10 }T    \           LF of CR/LF pair
T{ SSQ3 DROP  7 CHARS + C@ ->  34 }T    \ \q   "    Double Quote
T{ SSQ3 DROP  8 CHARS + C@ ->  13 }T    \ \r   CR   Carriage Return
T{ SSQ3 DROP  9 CHARS + C@ ->   9 }T    \ \t   TAB  Horizontal Tab
T{ SSQ3 DROP 10 CHARS + C@ ->  11 }T    \ \v   VT   Vertical Tab
T{ SSQ3 DROP 11 CHARS + C@ ->  15 }T    \ \x0F      Given Char
T{ SSQ3 DROP 12 CHARS + C@ ->  48 }T    \ 0    0    Digit follow on
T{ SSQ3 DROP 13 CHARS + C@ ->  31 }T    \ \x1F      Given Char
T{ SSQ3 DROP 14 CHARS + C@ ->  97 }T    \ a    a    Hex follow on
T{ SSQ3 DROP 15 CHARS + C@ -> 171 }T    \ \xaB      Insensitive Given Char
T{ SSQ3 DROP 16 CHARS + C@ -> 120 }T    \ x    x    Non hex follow on
T{ SSQ3 DROP 17 CHARS + C@ ->   0 }T    \ \z   NUL  No Character
T{ SSQ3 DROP 18 CHARS + C@ ->  34 }T    \ \"   "    Double Quote
T{ SSQ3 DROP 19 CHARS + C@ ->  92 }T    \ \\   \    Back Slash

\ The above does not test \n as this is a system dependent value.
\ Check it displays a new line
CR .( The next test should display:)
CR .( One line...)
CR .( another line)
T{ : SSQ4 S\" \nOne line...\nanotherLine\n" type ; SSQ4 -> }T

\ Test bare escapable characters appear as themselves
T{ : SSQ5 S\" abeflmnqrtvxz" S" abeflmnqrtvxz" S= ; SSQ5 -> TRUE }T

T{ : SSQ6 S\" a\""2DROP 1111 ; SSQ6 -> 1111 }T \ Parsing behaviour

T{ : SSQ7  S\" 111 : SSQ8 s\\\" 222\" EVALUATE ; SSQ8 333" EVALUATE ; -> }T
T{ SSQ7 -> 111 222 333 }T
T{ : SSQ9  S\" 11 : SSQ10 s\\\" \\x32\\x32\" EVALUATE ; SSQ10 33" EVALUATE ; -> }T
T{ SSQ9 -> 11 22 33 }T

CR .( End of Core Extension word tests) CR

\ To test the ANS Forth Double-Number word set and double number extensions

\ This program was written by Gerry Jackson in 2006, with contributions from
\ others where indicated, and is in the public domain - it can be distributed
\ and/or modified in any way but please retain this notice.

\ This program is distributed in the hope that it will be useful,
\ but WITHOUT ANY WARRANTY; without even the implied warranty of
\ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\ The tests are not claimed to be comprehensive or correct 
\ ------------------------------------------------------------------------------
\ Version 0.13  Assumptions and dependencies changed
\         0.12  1 August 2015 test D< acts on MS cells of double word
\         0.11  7 April 2015 2VALUE tested
\         0.6   1 April 2012 Tests placed in the public domain.
\               Immediate 2CONSTANTs and 2VARIABLEs tested
\         0.5   20 November 2009 Various constants renamed to avoid
\               redefinition warnings. <TRUE> and <FALSE> replaced
\               with TRUE and FALSE
\         0.4   6 March 2009 { and } replaced with T{ and }T
\               Tests rewritten to be independent of word size and
\               tests re-ordered
\         0.3   20 April 2007 ANS Forth words changed to upper case
\         0.2   30 Oct 2006 Updated following GForth test to include
\               various constants from core.fr
\         0.1   Oct 2006 First version released
\ ------------------------------------------------------------------------------
\ The tests are based on John Hayes test program for the core word set

\ Words tested in this file are:
\     2CONSTANT 2LITERAL 2VARIABLE D+ D- D. D.R D0< D0= D2* D2/
\     D< D= D>S DABS DMAX DMIN DNEGATE M*/ M+ 2ROT DU<
\ Also tests the interpreter and compiler reading a double number
\ ------------------------------------------------------------------------------
\ Assumptions and dependencies:
\     - tester.fr (or ttester.fs), errorreport.fth and utilities.fth have been
\       included prior to this file
\     - the Core word set is available and tested
\ ------------------------------------------------------------------------------
\ Constant definitions

DECIMAL
0 INVERT        CONSTANT 1SD
1SD 1 RSHIFT    CONSTANT MAX-INTD   \ 01...1
MAX-INTD INVERT CONSTANT MIN-INTD   \ 10...0
MAX-INTD 2/     CONSTANT HI-INT     \ 001...1
MIN-INTD 2/     CONSTANT LO-INT     \ 110...1

\ ------------------------------------------------------------------------------
TESTING interpreter and compiler reading double numbers, with/without prefixes

T{ 1. -> 1 0 }T
T{ -2. -> -2 -1 }T
T{ : RDL1 3. ; RDL1 -> 3 0 }T
T{ : RDL2 -4. ; RDL2 -> -4 -1 }T

CVARIABLE OLD-DBASE
DECIMAL BASE C@ OLD-DBASE C!
T{ #12346789. -> 12346789. }T
T{ #-12346789. -> -12346789. }T
T{ $12aBcDeF. -> 313249263. }T
T{ $-12AbCdEf. -> -313249263. }T
T{ %10010110. -> 150. }T
T{ %-10010110. -> -150. }T
\ Check BASE is unchanged
T{ BASE C@ OLD-DBASE C@ = -> <TRUE> }T

\ Repeat in Hex mode
16 OLD-DBASE C! 16 BASE C!
T{ #12346789. -> BC65A5. }T
T{ #-12346789. -> -BC65A5. }T
T{ $12aBcDeF. -> 12AbCdeF. }T
T{ $-12AbCdEf. -> -12ABCDef. }T
T{ %10010110. -> 96. }T
T{ %-10010110. -> -96. }T
\ Check BASE is unchanged
T{ BASE C@ OLD-DBASE C@ = -> <TRUE> }T   \ 2

DECIMAL
\ Check number prefixes in compile mode
T{ : dnmp  #8327. $-2cbe. %011010111. ; dnmp -> 8327. -11454. 215. }T

\ ------------------------------------------------------------------------------
TESTING 2CONSTANT

T{ 1 2 2CONSTANT 2C1 -> }T
T{ 2C1 -> 1 2 }T
T{ : CD1 2C1 ; -> }T
T{ CD1 -> 1 2 }T
T{ : CD2 2CONSTANT ; -> }T
T{ -1 -2 CD2 2C2 -> }T
T{ 2C2 -> -1 -2 }T
T{ 4 5 2CONSTANT 2C3 : 2C3 IMMEDIATE 2C3 ; 2C3 -> 4 5 }T \ modified for compatibility
T{ : CD6 2C3 2LITERAL ; CD6 -> 4 5 }T

\ ------------------------------------------------------------------------------
\ Some 2CONSTANTs for the following tests

1SD MAX-INTD 2CONSTANT MAX-2INT  \ 01...1
0   MIN-INTD 2CONSTANT MIN-2INT  \ 10...0
MAX-2INT 2/  2CONSTANT HI-2INT   \ 001...1
MIN-2INT 2/  2CONSTANT LO-2INT   \ 110...0

\ ------------------------------------------------------------------------------
TESTING DNEGATE

T{ 0. DNEGATE -> 0. }T
T{ 1. DNEGATE -> -1. }T
T{ -1. DNEGATE -> 1. }T
T{ MAX-2INT DNEGATE -> MIN-2INT SWAP 1+ SWAP }T
T{ MIN-2INT SWAP 1+ SWAP DNEGATE -> MAX-2INT }T

\ ------------------------------------------------------------------------------
TESTING D+ with small integers

T{  0.  5. D+ ->  5. }T
T{ -5.  0. D+ -> -5. }T
T{  1.  2. D+ ->  3. }T
T{  1. -2. D+ -> -1. }T
T{ -1.  2. D+ ->  1. }T
T{ -1. -2. D+ -> -3. }T
T{ -1.  1. D+ ->  0. }T

TESTING D+ with mid range integers

T{  0  0  0  5 D+ ->  0  5 }T
T{ -1  5  0  0 D+ -> -1  5 }T
T{  0  0  0 -5 D+ ->  0 -5 }T
T{  0 -5 -1  0 D+ -> -1 -5 }T
T{  0  1  0  2 D+ ->  0  3 }T
T{ -1  1  0 -2 D+ -> -1 -1 }T
T{  0 -1  0  2 D+ ->  0  1 }T
T{  0 -1 -1 -2 D+ -> -1 -3 }T
T{ -1 -1  0  1 D+ -> -1  0 }T
T{ MIN-INTD 0 2DUP D+ -> 0 1 }T
T{ MIN-INTD S>D MIN-INTD 0 D+ -> 0 0 }T

TESTING D+ with large double integers

T{ HI-2INT 1. D+ -> 0 HI-INT 1+ }T
T{ HI-2INT 2DUP D+ -> 1SD 1- MAX-INTD }T
T{ MAX-2INT MIN-2INT D+ -> -1. }T
T{ MAX-2INT LO-2INT D+ -> HI-2INT }T
T{ HI-2INT MIN-2INT D+ 1. D+ -> LO-2INT }T
T{ LO-2INT 2DUP D+ -> MIN-2INT }T

\ ------------------------------------------------------------------------------
TESTING D- with small integers

T{  0.  5. D- -> -5. }T
T{  5.  0. D- ->  5. }T
T{  0. -5. D- ->  5. }T
T{  1.  2. D- -> -1. }T
T{  1. -2. D- ->  3. }T
T{ -1.  2. D- -> -3. }T
T{ -1. -2. D- ->  1. }T
T{ -1. -1. D- ->  0. }T

TESTING D- with mid-range integers

T{  0  0  0  5 D- ->  0 -5 }T
T{ -1  5  0  0 D- -> -1  5 }T
T{  0  0 -1 -5 D- ->  1  4 }T
T{  0 -5  0  0 D- ->  0 -5 }T
T{ -1  1  0  2 D- -> -1 -1 }T
T{  0  1 -1 -2 D- ->  1  2 }T
T{  0 -1  0  2 D- ->  0 -3 }T
T{  0 -1  0 -2 D- ->  0  1 }T
T{  0  0  0  1 D- ->  0 -1 }T
T{ MIN-INTD 0 2DUP D- -> 0. }T
T{ MIN-INTD S>D MAX-INTD 0 D- -> 1 1SD }T

TESTING D- with large integers

T{ MAX-2INT MAX-2INT D- -> 0. }T
T{ MIN-2INT MIN-2INT D- -> 0. }T
T{ MAX-2INT HI-2INT  D- -> LO-2INT DNEGATE }T
T{ HI-2INT  LO-2INT  D- -> MAX-2INT }T
T{ LO-2INT  HI-2INT  D- -> MIN-2INT 1. D+ }T
T{ MIN-2INT MIN-2INT D- -> 0. }T
T{ MIN-2INT LO-2INT  D- -> LO-2INT }T

\ ------------------------------------------------------------------------------
TESTING D0< D0=

T{ 0. D0< -> FALSE }T
T{ 1. D0< -> FALSE }T
T{ MIN-INTD 0 D0< -> FALSE }T
T{ 0 MAX-INTD D0< -> FALSE }T
T{ MAX-2INT  D0< -> FALSE }T
T{ -1. D0< -> TRUE }T
T{ MIN-2INT D0< -> TRUE }T

T{ 1. D0= -> FALSE }T
T{ MIN-INTD 0 D0= -> FALSE }T
T{ MAX-2INT  D0= -> FALSE }T
T{ -1 MAX-INTD D0= -> FALSE }T
T{ 0. D0= -> TRUE }T
T{ -1. D0= -> FALSE }T
T{ 0 MIN-INTD D0= -> FALSE }T

\ ------------------------------------------------------------------------------
TESTING D2* D2/

T{ 0. D2* -> 0. D2* }T
T{ MIN-INTD 0 D2* -> 0 1 }T
T{ HI-2INT D2* -> MAX-2INT 1. D- }T
T{ LO-2INT D2* -> MIN-2INT }T

T{ 0. D2/ -> 0. }T
T{ 1. D2/ -> 0. }T
T{ 0 1 D2/ -> MIN-INTD 0 }T
T{ MAX-2INT D2/ -> HI-2INT }T
T{ -1. D2/ -> -1. }T
T{ MIN-2INT D2/ -> LO-2INT }T

\ ------------------------------------------------------------------------------
TESTING D< D=

T{  0.  1. D< -> TRUE  }T
T{  0.  0. D< -> FALSE }T
T{  1.  0. D< -> FALSE }T
T{ -1.  1. D< -> TRUE  }T
T{ -1.  0. D< -> TRUE  }T
T{ -2. -1. D< -> TRUE  }T
T{ -1. -2. D< -> FALSE }T
T{ 0 1   1. D< -> FALSE }T  \ Suggested by Helmut Eller
T{ 1.  0 1  D< -> TRUE  }T
T{ 0 -1 1 -2 D< -> FALSE }T
T{ 1 -2 0 -1 D< -> TRUE  }T
T{ -1. MAX-2INT D< -> TRUE }T
T{ MIN-2INT MAX-2INT D< -> TRUE }T
T{ MAX-2INT -1. D< -> FALSE }T
T{ MAX-2INT MIN-2INT D< -> FALSE }T
T{ MAX-2INT 2DUP -1. D+ D< -> FALSE }T
T{ MIN-2INT 2DUP  1. D+ D< -> TRUE  }T
T{ MAX-INTD S>D 2DUP 1. D+ D< -> TRUE }T \ Ensure D< acts on MS cells 

T{ -1. -1. D= -> TRUE  }T
T{ -1.  0. D= -> FALSE }T
T{ -1.  1. D= -> FALSE }T
T{  0. -1. D= -> FALSE }T
T{  0.  0. D= -> TRUE  }T
T{  0.  1. D= -> FALSE }T
T{  1. -1. D= -> FALSE }T
T{  1.  0. D= -> FALSE }T
T{  1.  1. D= -> TRUE  }T

T{ 0 -1 0 -1 D= -> TRUE  }T
T{ 0 -1 0  0 D= -> FALSE }T
T{ 0 -1 0  1 D= -> FALSE }T
T{ 0  0 0 -1 D= -> FALSE }T
T{ 0  0 0  0 D= -> TRUE  }T
T{ 0  0 0  1 D= -> FALSE }T
T{ 0  1 0 -1 D= -> FALSE }T
T{ 0  1 0  0 D= -> FALSE }T
T{ 0  1 0  1 D= -> TRUE  }T

T{ MAX-2INT MIN-2INT D= -> FALSE }T
T{ MAX-2INT 0. D= -> FALSE }T
T{ MAX-2INT MAX-2INT D= -> TRUE }T
T{ MAX-2INT HI-2INT  D= -> FALSE }T
T{ MAX-2INT MIN-2INT D= -> FALSE }T
T{ MIN-2INT MIN-2INT D= -> TRUE }T
T{ MIN-2INT LO-2INT  D=  -> FALSE }T
T{ MIN-2INT MAX-2INT D= -> FALSE }T

\ ------------------------------------------------------------------------------
TESTING 2LITERAL 2VARIABLE

T{ : CD3 [ MAX-2INT ] 2LITERAL ; -> }T
T{ CD3 -> MAX-2INT }T
T{ 2VARIABLE 2V1 -> }T
T{ 0. 2V1 2! -> }T
T{ 2V1 2@ -> 0. }T
T{ -1 -2 2V1 2! -> }T
T{ 2V1 2@ -> -1 -2 }T
T{ : CD4 2VARIABLE ; -> }T
T{ CD4 2V2 -> }T
T{ : CD5 2V2 2! ; -> }T
T{ -2 -1 CD5 -> }T
T{ 2V2 2@ -> -2 -1 }T
T{ 2VARIABLE 2V3 : 2V3 IMMEDIATE 2V3 ; 5 6 2V3 2! -> }T \ modified for compatibility
T{ 2V3 2@ -> 5 6 }T
T{ : CD7 2V3 [ 2@ ] 2LITERAL ; CD7 -> 5 6 }T
T{ : CD8 [ 6 7 ] 2V3 [ 2! ] ; 2V3 2@ -> 6 7 }T

\ ------------------------------------------------------------------------------
TESTING DMAX DMIN

T{  1.  2. DMAX -> 2. }T
T{  1.  0. DMAX -> 1. }T
T{  1. -1. DMAX -> 1. }T
T{  1.  1. DMAX -> 1. }T
T{  0.  1. DMAX -> 1. }T
T{  0. -1. DMAX -> 0. }T
T{ -1.  1. DMAX -> 1. }T
T{ -1. -2. DMAX -> -1. }T

T{ MAX-2INT HI-2INT  DMAX -> MAX-2INT }T
T{ MAX-2INT MIN-2INT DMAX -> MAX-2INT }T
T{ MIN-2INT MAX-2INT DMAX -> MAX-2INT }T
T{ MIN-2INT LO-2INT  DMAX -> LO-2INT  }T

T{ MAX-2INT  1. DMAX -> MAX-2INT }T
T{ MAX-2INT -1. DMAX -> MAX-2INT }T
T{ MIN-2INT  1. DMAX ->  1. }T
T{ MIN-2INT -1. DMAX -> -1. }T


T{  1.  2. DMIN ->  1. }T
T{  1.  0. DMIN ->  0. }T
T{  1. -1. DMIN -> -1. }T
T{  1.  1. DMIN ->  1. }T
T{  0.  1. DMIN ->  0. }T
T{  0. -1. DMIN -> -1. }T
T{ -1.  1. DMIN -> -1. }T
T{ -1. -2. DMIN -> -2. }T

T{ MAX-2INT HI-2INT  DMIN -> HI-2INT  }T
T{ MAX-2INT MIN-2INT DMIN -> MIN-2INT }T
T{ MIN-2INT MAX-2INT DMIN -> MIN-2INT }T
T{ MIN-2INT LO-2INT  DMIN -> MIN-2INT }T

T{ MAX-2INT  1. DMIN ->  1. }T
T{ MAX-2INT -1. DMIN -> -1. }T
T{ MIN-2INT  1. DMIN -> MIN-2INT }T
T{ MIN-2INT -1. DMIN -> MIN-2INT }T

\ ------------------------------------------------------------------------------
TESTING D>S DABS

T{  1234  0 D>S ->  1234 }T
T{ -1234 -1 D>S -> -1234 }T
T{ MAX-INTD  0 D>S -> MAX-INTD }T
T{ MIN-INTD -1 D>S -> MIN-INTD }T

T{  1. DABS -> 1. }T
T{ -1. DABS -> 1. }T
T{ MAX-2INT DABS -> MAX-2INT }T
T{ MIN-2INT 1. D+ DABS -> MAX-2INT }T

\ ------------------------------------------------------------------------------
TESTING M+ M*/

T{ HI-2INT   1 M+ -> HI-2INT   1. D+ }T
T{ MAX-2INT -1 M+ -> MAX-2INT -1. D+ }T
T{ MIN-2INT  1 M+ -> MIN-2INT  1. D+ }T
T{ LO-2INT  -1 M+ -> LO-2INT  -1. D+ }T

\ To correct the result if the division is floored, only used when
\ necessary i.e. negative quotient and remainder <> 0

: ?FLOORED [ -3 2 / -2 = ] LITERAL IF 1. D- THEN ;

T{  5.  7 11 M*/ ->  3. }T
T{  5. -7 11 M*/ -> -3. ?FLOORED }T    \ FLOORED -4.
T{ -5.  7 11 M*/ -> -3. ?FLOORED }T    \ FLOORED -4.
T{ -5. -7 11 M*/ ->  3. }T
T{ MAX-2INT  8 16 M*/ -> HI-2INT }T
T{ MAX-2INT -8 16 M*/ -> HI-2INT DNEGATE ?FLOORED }T  \ FLOORED SUBTRACT 1
T{ MIN-2INT  8 16 M*/ -> LO-2INT }T
T{ MIN-2INT -8 16 M*/ -> LO-2INT DNEGATE }T
T{ MAX-2INT MAX-INTD MAX-INTD M*/ -> MAX-2INT }T
T{ MAX-2INT MAX-INTD 2/ MAX-INTD M*/ -> MAX-INTD 1- HI-2INT NIP }T
T{ MIN-2INT LO-2INT NIP 1+ DUP 1- NEGATE M*/ -> 0 MAX-INTD 1- }T
T{ MIN-2INT LO-2INT NIP 1- MAX-INTD M*/ -> MIN-INTD 3 + HI-2INT NIP 2 + }T
T{ MAX-2INT LO-2INT NIP DUP NEGATE M*/ -> MAX-2INT DNEGATE }T
T{ MIN-2INT MAX-INTD DUP M*/ -> MIN-2INT }T

\ ------------------------------------------------------------------------------
TESTING D. D.R

\ Create some large double numbers
MAX-2INT 71 73 M*/ 2CONSTANT DBL1
MIN-2INT 73 79 M*/ 2CONSTANT DBL2

: D>ASCII  ( D -- CADDR U )
   DUP >R <# DABS #S R> SIGN #>    ( -- CADDR1 U )
   HERE SWAP 2DUP 2>R CHARS DUP ALLOT MOVE 2R>
;

DBL1 D>ASCII 2CONSTANT "DBL1"
DBL2 D>ASCII 2CONSTANT "DBL2"

: DOUBLEOUTPUT
   CR ." You should see lines duplicated:" CR
   5 SPACES "DBL1" TYPE CR
   5 SPACES DBL1 D. CR
   8 SPACES "DBL1" DUP >R TYPE CR
   5 SPACES DBL1 R> 3 + D.R CR
   5 SPACES "DBL2" TYPE CR
   5 SPACES DBL2 D. CR
   10 SPACES "DBL2" DUP >R TYPE CR
   5 SPACES DBL2 R> 5 + D.R CR
;

T{ DOUBLEOUTPUT -> }T

\ ------------------------------------------------------------------------------
TESTING 2ROT DU< (Double Number extension words)

T{ 1. 2. 3. 2ROT -> 2. 3. 1. }T
T{ MAX-2INT MIN-2INT 1. 2ROT -> MIN-2INT 1. MAX-2INT }T

T{  1.  1. DU< -> FALSE }T
T{  1. -1. DU< -> TRUE  }T
T{ -1.  1. DU< -> FALSE }T
T{ -1. -2. DU< -> FALSE }T
T{ 0 1   1. DU< -> FALSE }T
T{ 1.  0 1  DU< -> TRUE  }T
T{ 0 -1 1 -2 DU< -> FALSE }T
T{ 1 -2 0 -1 DU< -> TRUE  }T

T{ MAX-2INT HI-2INT  DU< -> FALSE }T
T{ HI-2INT  MAX-2INT DU< -> TRUE  }T
T{ MAX-2INT MIN-2INT DU< -> TRUE }T
T{ MIN-2INT MAX-2INT DU< -> FALSE }T
T{ MIN-2INT LO-2INT  DU< -> TRUE }T

\ ------------------------------------------------------------------------------
TESTING 2VALUE

T{ 1111 2222 2VALUE 2VAL -> }T
T{ 2VAL -> 1111 2222 }T
T{ 3333 4444 TO 2VAL -> }T
T{ 2VAL -> 3333 4444 }T
T{ : TO-2VAL TO 2VAL ; 5555 6666 TO-2VAL -> }T
T{ 2VAL -> 5555 6666 }T

\ ------------------------------------------------------------------------------

CR .( End of Double-Number word tests) CR

\ To test the ANS Forth Exception word set and extension words

\ This program was written by Gerry Jackson in 2006, with contributions from
\ others where indicated, and is in the public domain - it can be distributed
\ and/or modified in any way but please retain this notice.

\ This program is distributed in the hope that it will be useful,
\ but WITHOUT ANY WARRANTY; without even the implied warranty of
\ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\ The tests are not claimed to be comprehensive or correct 

\ ------------------------------------------------------------------------------
\ Version 0.13 13 Nov 2015 C6 rewritten to avoid use of CASE etc and hence
\              dependence on the Core extension word set.     
\         0.4 1 April 2012  Tests placed in the public domain.
\         0.3 6 March 2009 { and } replaced with T{ and }T
\         0.2 20 April 2007 ANS Forth words changed to upper case
\         0.1 Oct 2006 First version released

\ ------------------------------------------------------------------------------
\ The tests are based on John Hayes test program for the core word set
\
\ Words tested in this file are:
\     CATCH THROW ABORT ABORT"
\
\ ------------------------------------------------------------------------------
\ Assumptions and dependencies:
\     - the forth system under test throws an exception with throw
\       code -13 for a word not found by the text interpreter. The
\       undefined word used is $$qweqweqwert$$,  if this happens to be
\       a valid word in your system change the definition of t7 below
\     - tester.fr (or ttester.fs), errorreport.fth and utilities.fth have been
\       included prior to this file
\     - the Core word set available and tested
\     - CASE, OF, ENDOF and ENDCASE from the core extension wordset
\       are present and work correctly
\ ------------------------------------------------------------------------------
TESTING CATCH THROW

DECIMAL

: T1 9 ;
: C1 1 2 3 ['] T1 CATCH ;
T{ C1 -> 1 2 3 9 0 }T         \ No THROW executed

: T2 8 0 THROW ;
: C2 1 2 ['] T2 CATCH ;
T{ C2 -> 1 2 8 0 }T            \ 0 THROW does nothing

: T3 7 8 9 99 THROW ;
: C3 1 2 ['] T3 CATCH ;
T{ C3 -> 1 2 99 }T            \ Restores stack to CATCH depth

: T4 1- DUP 0> IF RECURSE ELSE 999 THROW -222 THEN ;
: C4 3 4 5 10 ['] T4 CATCH -111 ;
T{ C4 -> 3 4 5 0 999 -111 }T   \ Test return stack unwinding

: T5 2DROP 2DROP 9999 THROW ;
: C5 1 2 3 4 ['] T5 CATCH            \ Test depth restored correctly
   DEPTH >R DROP 2DROP 2DROP R> ;   \ after stack has been emptied
T{ C5 -> 5 }T

\ ------------------------------------------------------------------------------
TESTING ABORT ABORT"

-1  CONSTANT EXC_ABORT
-2  CONSTANT EXC_ABORT"
-13 CONSTANT EXC_UNDEF
: T6 ABORT ;

\ The 77 in T10 is necessary for the second ABORT" test as the data stack
\ is restored to a depth of 2 when THROW is executed. The 77 ensures the top
\ of stack value is known for the results check

: T10 77 SWAP ABORT" This should not be displayed" ;
: C6 CATCH
   >R   R@ EXC_ABORT  = IF 11
   ELSE R@ EXC_ABORT" = IF 12
   ELSE R@ EXC_UNDEF  = IF 13
   THEN THEN THEN R> DROP
;

T{ 1 2 ' T6 C6  -> 1 2 11 }T     \ Test that ABORT is caught
T{ 3 0 ' T10 C6 -> 3 77 }T       \ ABORT" does nothing
T{ 4 5 ' T10 C6 -> 4 77 12 }T    \ ABORT" caught, no message

\ ------------------------------------------------------------------------------
TESTING a system generated exception

: T7 S" 333 $$QWEQWEQWERT$$ 334" EVALUATE 335 ;
: T8 S" 222 T7 223" EVALUATE 224 ;
: T9 S" 111 112 T8 113" EVALUATE 114 ;

T{ 6 7 ' T9 C6 3 -> 6 7 13 3 }T         \ Test unlinking of sources

\ ------------------------------------------------------------------------------

CR .( End of Exception word tests) CR

64 CONSTANT SBUF-SIZE
CREATE SBUF1 SBUF-SIZE CHARS ALLOT
CREATE SBUF2 SBUF-SIZE CHARS ALLOT

\ ($") saves a counted string at (caddr)
: ($")  ( caddr "ccc" -- caddr' u )
   [CHAR] " PARSE ROT 2DUP C!       ( -- ca2 u2 ca)
   CHAR+ SWAP 2DUP 2>R CHARS MOVE   ( -- )  ( R: -- ca' u2 )
   2R>
;

: $"   ( "ccc" -- caddr u )  SBUF1 ($") ;
: $2"  ( "ccc" -- caddr u )  SBUF2 ($") ;

\ To test the ANS Forth String word set

\ This program was written by Gerry Jackson in 2006, with contributions from
\ others where indicated, and is in the public domain - it can be distributed
\ and/or modified in any way but please retain this notice.

\ This program is distributed in the hope that it will be useful,
\ but WITHOUT ANY WARRANTY; without even the implied warranty of
\ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\ The tests are not claimed to be comprehensive or correct 

\ ------------------------------------------------------------------------------
\ Version 0.13 13 Nov 2015 Interpretive use of S" replaced by $" from
\                          utilities.fth
\         0.11 25 April 2015 Tests for REPLACES SUBSTITUTE UNESCAPE added
\         0.6 1 April 2012 Tests placed in the public domain.
\         0.5 29 April 2010 Added tests for SEARCH and COMPARE with
\             all strings zero length (suggested by Krishna Myneni).
\             SLITERAL test amended in line with comp.lang.forth
\             discussion
\         0.4 30 November 2009 <TRUE> and <FALSE> replaced with TRUE
\             and FALSE
\         0.3 6 March 2009 { and } replaced with T{ and }T
\         0.2 20 April 2007 ANS Forth words changed to upper case
\         0.1 Oct 2006 First version released

\ ------------------------------------------------------------------------------
\ The tests are based on John Hayes test program for the core word set
\ and requires those files to have been loaded

\ Words tested in this file are:
\     -TRAILING /STRING BLANK CMOVE CMOVE> COMPARE SEARCH SLITERAL
\
\ ------------------------------------------------------------------------------
\ Assumptions, dependencies and notes:
\     - tester.fr (or ttester.fs), errorreport.fth and utilities.fth have been
\       included prior to this file
\     - the Core word set is available and tested
\     - COMPARE is case sensitive
\ ------------------------------------------------------------------------------

TESTING String word set

DECIMAL

T{ :  S1 S" abcdefghijklmnopqrstuvwxyz" ; -> }T
T{ :  S2 S" abc"   ; -> }T
T{ :  S3 S" jklmn" ; -> }T
T{ :  S4 S" z"     ; -> }T
T{ :  S5 S" mnoq"  ; -> }T
T{ :  S6 S" 12345" ; -> }T
T{ :  S7 S" "      ; -> }T
T{ :  S8 S" abc  " ; -> }T
T{ :  S9 S"      " ; -> }T
T{ : S10 S"    a " ; -> }T

\ ------------------------------------------------------------------------------
TESTING -TRAILING

T{  S1 -TRAILING -> S1 }T
T{  S8 -TRAILING -> S8 2 - }T
T{  S7 -TRAILING -> S7 }T
T{  S9 -TRAILING -> S9 DROP 0 }T
T{ S10 -TRAILING -> S10 1- }T

\ ------------------------------------------------------------------------------
TESTING /STRING

T{ S1  5 /STRING -> S1 SWAP 5 + SWAP 5 - }T
T{ S1 10 /STRING -4 /STRING -> S1 6 /STRING }T
T{ S1  0 /STRING -> S1 }T

\ ------------------------------------------------------------------------------
TESTING SEARCH

T{ S1 S2 SEARCH -> S1 TRUE }T
T{ S1 S3 SEARCH -> S1  9 /STRING TRUE }T
T{ S1 S4 SEARCH -> S1 25 /STRING TRUE }T
T{ S1 S5 SEARCH -> S1 FALSE }T
T{ S1 S6 SEARCH -> S1 FALSE }T
T{ S1 S7 SEARCH -> S1 TRUE }T
T{ S7 PAD 0 SEARCH -> S7 TRUE }T

\ ------------------------------------------------------------------------------
TESTING COMPARE

T{ S1 S1 COMPARE -> 0 }T
T{ S1 PAD SWAP CMOVE -> }T
T{ S1 PAD OVER COMPARE -> 0 }T
T{ S1 PAD 6 COMPARE -> 1 }T
T{ PAD 10 S1 COMPARE -> -1 }T
T{ S1 PAD 0 COMPARE -> 1 }T
T{ PAD 0 S1 COMPARE -> -1 }T
T{ S1 S6 COMPARE ->  1 }T
T{ S6 S1 COMPARE -> -1 }T
T{ S7 PAD 0 COMPARE -> 0 }T

T{ S1 $" abdde"  COMPARE -> -1 }T
T{ S1 $" abbde"  COMPARE ->  1 }T
T{ S1 $" abcdf"  COMPARE -> -1 }T
T{ S1 $" abcdee" COMPARE ->  1 }T

: S11 S" 0abc" ;
: S12 S" 0aBc" ;

T{ S11 S12  COMPARE -> 1 }T
T{ S12 S11  COMPARE -> -1 }T

\ ------------------------------------------------------------------------------
TESTING CMOVE and CMOVE>

PAD 30 CHARS 0 FILL
T{ S1 PAD SWAP CMOVE -> }T
T{ S1 PAD S1 SWAP DROP COMPARE -> 0 }T
T{ S6 PAD 10 CHARS + SWAP CMOVE -> }T
T{ $" abcdefghij12345pqrstuvwxyz" PAD S1 SWAP DROP COMPARE -> 0 }T
T{ PAD 15 CHARS + PAD CHAR+ 6 CMOVE -> }T
T{ $" apqrstuhij12345pqrstuvwxyz" PAD 26 COMPARE -> 0 }T
T{ PAD PAD 3 CHARS + 7 CMOVE -> }T
T{ $" apqapqapqa12345pqrstuvwxyz" PAD 26 COMPARE -> 0 }T
T{ PAD PAD CHAR+ 10 CMOVE -> }T
T{ $" aaaaaaaaaaa2345pqrstuvwxyz" PAD 26 COMPARE -> 0 }T
T{ S7 PAD 14 CHARS + SWAP CMOVE -> }T
T{ $" aaaaaaaaaaa2345pqrstuvwxyz" PAD 26 COMPARE -> 0 }T

PAD 30 CHARS 0 FILL

T{ S1 PAD SWAP CMOVE> -> }T
T{ S1 PAD S1 SWAP DROP COMPARE -> 0 }T
T{ S6 PAD 10 CHARS + SWAP CMOVE> -> }T
T{ $" abcdefghij12345pqrstuvwxyz" PAD S1 SWAP DROP COMPARE -> 0 }T
T{ PAD 15 CHARS + PAD CHAR+ 6 CMOVE> -> }T
T{ $" apqrstuhij12345pqrstuvwxyz" PAD 26 COMPARE -> 0 }T
T{ PAD 13 CHARS + PAD 10 CHARS + 7 CMOVE> -> }T
T{ $" apqrstuhijtrstrstrstuvwxyz" PAD 26 COMPARE -> 0 }T
T{ PAD 12 CHARS + PAD 11 CHARS + 10 CMOVE> -> }T
T{ $" apqrstuhijtvvvvvvvvvvvwxyz" PAD 26 COMPARE -> 0 }T
T{ S7 PAD 14 CHARS + SWAP CMOVE> -> }T
T{ $" apqrstuhijtvvvvvvvvvvvwxyz" PAD 26 COMPARE -> 0 }T

\ ------------------------------------------------------------------------------
TESTING BLANK

: S13 S" aaaaa      a" ;   \ Don't move this down as it might corrupt PAD

T{ PAD 25 CHAR a FILL -> }T
T{ PAD 5 CHARS + 6 BLANK -> }T
T{ PAD 12 S13 COMPARE -> 0 }T

\ ------------------------------------------------------------------------------
TESTING SLITERAL

T{ HERE DUP S1 DUP ALLOT ROT SWAP CMOVE S1 SWAP DROP 2CONSTANT S1A -> }T
T{ : S14 [ S1A ] SLITERAL ; -> }T
T{ S1A S14 COMPARE -> 0 }T
T{ S1A DROP S14 DROP = -> FALSE }T


\ ------------------------------------------------------------------------------

CR .( End of String word tests) CR


\ AHEAD is the only words in the Programming Tools wordset that we support
\ (and is testable)

DECIMAL

\ ------------------------------------------------------------------------------
TESTING AHEAD

T{ : PT1 AHEAD 1111 2222 THEN 3333 ; -> }T
T{ PT1 -> 3333 }T

CR .( End of tests) CR
