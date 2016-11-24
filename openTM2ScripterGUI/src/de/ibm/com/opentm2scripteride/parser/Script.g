/*
 * Copyright (C) 2014, International Business Machines
 * Corporation and others.  All Rights Reserved.
 */

grammar Script;

options {
  language = Java;
}

@parser::header{
  package de.ibm.com.opentm2scripteride.parser;
  
  import java.util.ArrayList;
  import de.ibm.com.opentm2scripteride.models.ScriptModel;
  import de.ibm.com.opentm2scripteride.models.CommandModel;
}

@lexer::header {
  package de.ibm.com.opentm2scripteride.parser;
}

/*------------------------------------------------------------------
 * PARSER RULES
 *------------------------------------------------------------------*/

//ScriptModel is simply an array of CommandModels to be filled
script returns [ScriptModel scriptModel]: 
	{ $scriptModel = new ScriptModel(); }
	( r = robust_expression { $scriptModel.add($r.cm); } )*
	;
	
//A robust expression, that means it will either return a correct CommandModel
//or a CommandModel of the type "unknown" if we had a parsing error
robust_expression returns [CommandModel cm]:
    (expression)=> e = expression   { $cm = $e.cm; }
    | u = unknown                   { $cm = $u.cm; }
    ;
    catch [RecognitionException re] {
      $cm = CommandModel.newById(re.line - 1, "unknown");
      re.input.consume();
    }
	
//Constructs an "unknown" CommandModel of the line
unknown returns [CommandModel cm]:
    c = crapline  { String txt = $c.text;
                    if (txt != null && !txt.isEmpty()) {
                      $cm = CommandModel.newById(0, "unknown");
                      $cm.getBody().setParameter("text", txt);
                    }
                  }
    n = NL        { if ($cm != null) $cm.setStartLine($n.getLine() - 1); }
    ;
    
//A crappy line with any words we can't parse correctly otherwise
crapline: ( '$' | '%' | '"' | ',' | WS )*;
  
//Passes the CommandModel, either from a block command or from a function command
expression returns [CommandModel cm]:
    c = commentblock     { $cm = $c.cm; }
	| m = marker           { $cm = $m.cm; }
    | s = controlstruct 	 { $cm = $s.cm; }
    | f = function         { $cm = $f.cm; }
    ;
      

      
//CommandModel for a function command. Basically a CommandBody. Can't have any children
function returns [CommandModel cm]:
   SPACE?
	 c = NORMALWORD
	 a = argumentlist	{ $cm = new CommandModel( $c.getLine() - 1, $c.text, $a.list); }
	 NL
	 ;
	 
//A block of commands or the command itself if it's only one
commentblock returns [CommandModel cm]:
   { $cm = CommandModel.newById(0, "commentblock"); }
   ( c = comment { $cm.add( $c.cm ); } )+
   { 
     if ($cm.size() == 1) {
       $cm = $cm.get(0);
     } else {
       $cm.setStartLine( $cm.get(0).getStartLine() );
     }
   }
   ;
	 
//A comment line
comment returns [CommandModel cm]:
  c = COMMENT   { $cm = new CommandModel( $c.getLine() - 1, "*", $c.text.substring(1)); }
  NL
  ;

//An argument
argument:(
            QUOTE
          | CONSTANT
          | VARIABLE
          | NORMALWORD
          | SPACE
          | keyword
          | WORD
         )*
         ;      
         
//Any keyword
keyword: ( IF | ELSEIF | ELSE | ENDIF | FOR | ENDFOR | TESTCASE | END);
                      
//A stringlist representing the arguments
argumentlist returns [List list]: 
  { $list = new ArrayList<String>(); }
       a = argument    { $list.add($a.text.trim()); }
  (','  b = argument    { $list.add($b.text.trim()); } )*
  ;
  

//a line beginning with a hash marker "#"
marker returns  [CommandModel cm]:
  HASHMARKER SPACE? n = NORMALWORD SPACE? NL { $cm = new CommandModel($n.getLine() - 1, "#", $n.text); };
  
//Passes the CommandModel, either from a condition or from a for loop
controlstruct returns [CommandModel cm]:
     i = ifexpr  		{ $cm = $i.cm; } 
    | f = for_block 	{ $cm = $f.cm; }
	| t = test_block    { $cm = $t.cm; }
    ;

 
//A CommandModel as a wrapper around if, elseif and else blocks, which are the children.
ifexpr returns [CommandModel cm]:
	{ $cm = CommandModel.newById(0, "condition"); }
	   ib = if_block 		  { $cm.add($ib.cm); $cm.setStartLine($ib.cm.getStartLine()); }
	( eib = elseif_block 	{ $cm.add($eib.cm); } )*
	(  eb = else_block 		{ $cm.add($eb.cm);  }  )?
	SPACE* ENDIF NL
	;

//A CommandModel representing the IF-part of a condition block
if_block returns [CommandModel cm]:
	SPACE* a = IF be = boolexpr 	   { $cm = new CommandModel($a.getLine() - 1, "if", $be.string); } 
	NL
	( e = robust_expression   { $cm.add($e.cm); } )*
	;

//A CommandModel representing the ELSEIF-part of a condition block
elseif_block returns [CommandModel cm]:
	SPACE* a = ELSEIF be = boolexpr 	{ $cm = new CommandModel($a.getLine() - 1, "elseif", $be.string); }
	NL
	( e = robust_expression  	{ $cm.add($e.cm); } )*
	;
	
//A CommandModel representing the ELSE-part of a condition block
else_block returns [CommandModel cm]:
	SPACE* a = ELSE 			 		        { $cm = new CommandModel($a.getLine() - 1, "else"); } 
	NL
	( e = robust_expression  	{ $cm.add($e.cm); } )*
	;
	
	
//String representing the boolean expression of the if clause TODO: do it better.
boolexpr returns [String string]:
	a = argument	{ $string = $a.text.trim(); }//wlp add trim
	BOOLOP	        { $string += " " + $BOOLOP.text + " "; }             
	b = argument 	{ $string += $b.text.trim(); }
	;
	
forarguments returns [List list]:
   {list = new ArrayList<String>();}
    i = NORMALWORD  ',' {$list.add($i.text);}
	c = argument    ',' {$list.add($c.text.trim());}
	be = boolexpr   ',' {$list.add($be.string);}
	a = function {$list.add($a.cm.getFrame().getCommand()+" "+$a.cm.getBody().getParameters().get("name"));}
    ;
    
//CommandModel representing a for loop, children are body of loop
for_block returns [CommandModel cm]:
	 SPACE* f = FOR SPACE+ p = forarguments { $cm = new CommandModel($f.getLine() - 1, "for", $p.list); } 
	 ( e = robust_expression       { $cm.add(e); } )*
	 SPACE* ENDFOR NL;


testarguments returns [List list]:
    {list = new ArrayList<String>();}
	id = argument  ','    {$list.add($id.text.trim());}
	des = argument     {$list.add($des.text.trim());}
	(',' exp = NORMALWORD      {$list.add($exp.text.trim());} )?
	;
	
test_block returns [CommandModel cm]:
    SPACE* t = TESTCASE SPACE p = testarguments {$cm = new CommandModel($t.getLine() - 1, "TestCase", $p.list);}
	NL
	(f = function       { $cm.add($f.cm); })*
	SPACE* END NL;

/*------------------------------------------------------------------
 * LEXER RULES
 *------------------------------------------------------------------*/

//a comment
COMMENT:    { getCharPositionInLine() == 0 }?=>SPACE* '*' ~('\r'|'\n')*;

//a marker
HASHMARKER: { getCharPositionInLine() == 0 }? WS* '#';

//keywords, case insensiive
ENDIF: E N D I F;
IF: I F;
ELSEIF: E L S E I F;
ELSE: E L S E;
FOR: F O R;
ENDFOR: E N D F O R;
TESTCASE: T E S T C A S E;
END: E N D;


//any word without special characters
NORMALWORD: NORMALCHAR+;

//a boolean operation
BOOLOP: '=='
    | '<' ('=')?
    | '>' ('=')?
    | '!=';

//variable and constant
VARIABLE: '$' NORMALCHAR+ '$';
CONSTANT: '%' NORMALCHAR+ '%';

//an escaped string
QUOTE: '(' ~(NEWLINE|')')* ')'
      | '"' ~(NEWLINE|'"')* '"';


//any word
WORD: MOSTCHAR+;

//the newline indicator (or multiple)
NL: NEWLINE+;

//spaces
SPACE: WS+;

//fragments
fragment NEWLINE: '\r'|'\n';
fragment WS: ' ' | '\t' | '\f';
fragment SPECIALCHAR: '$'|'%'|','|'"'|'<'|'>'|'='|'!';
fragment MOSTCHAR: ~(SPECIALCHAR|NEWLINE|WS);
fragment NORMALCHAR: ('a'..'z'|'0'..'9'|'A'..'Z'|'-'|'_');

//letters
fragment A: 'A' | 'a';
fragment C: 'C' | 'c';
fragment D: 'D' | 'd';
fragment E: 'E' | 'e';
fragment F: 'F' | 'f';
fragment I: 'I' | 'i';
fragment L: 'L' | 'l';
fragment N: 'N' | 'n';
fragment O: 'O' | 'o';
fragment R: 'R' | 'r';
fragment S: 'S' | 's';
fragment T: 'T' | 't';
