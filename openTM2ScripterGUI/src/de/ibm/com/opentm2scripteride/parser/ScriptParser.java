/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
// $ANTLR 3.4 C:\\eclipseworkspace\\openTM2ScripterGUI-new\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g 2014-11-03 14:08:06

  package de.ibm.com.opentm2scripteride.parser;
  
  import java.util.ArrayList;
  import de.ibm.com.opentm2scripteride.models.ScriptModel;
  import de.ibm.com.opentm2scripteride.models.CommandModel;


import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;

@SuppressWarnings({"all", "warnings", "unchecked"})
public class ScriptParser extends Parser {
    public static final String[] tokenNames = new String[] {
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "A", "BOOLOP", "C", "COMMENT", "CONSTANT", "D", "E", "ELSE", "ELSEIF", "END", "ENDFOR", "ENDIF", "F", "FOR", "HASHMARKER", "I", "IF", "L", "MOSTCHAR", "N", "NEWLINE", "NL", "NORMALCHAR", "NORMALWORD", "O", "QUOTE", "R", "S", "SPACE", "SPECIALCHAR", "T", "TESTCASE", "VARIABLE", "WORD", "WS", "'\"'", "'$'", "'%'", "','"
    };

    public static final int EOF=-1;
    public static final int T__39=39;
    public static final int T__40=40;
    public static final int T__41=41;
    public static final int T__42=42;
    public static final int A=4;
    public static final int BOOLOP=5;
    public static final int C=6;
    public static final int COMMENT=7;
    public static final int CONSTANT=8;
    public static final int D=9;
    public static final int E=10;
    public static final int ELSE=11;
    public static final int ELSEIF=12;
    public static final int END=13;
    public static final int ENDFOR=14;
    public static final int ENDIF=15;
    public static final int F=16;
    public static final int FOR=17;
    public static final int HASHMARKER=18;
    public static final int I=19;
    public static final int IF=20;
    public static final int L=21;
    public static final int MOSTCHAR=22;
    public static final int N=23;
    public static final int NEWLINE=24;
    public static final int NL=25;
    public static final int NORMALCHAR=26;
    public static final int NORMALWORD=27;
    public static final int O=28;
    public static final int QUOTE=29;
    public static final int R=30;
    public static final int S=31;
    public static final int SPACE=32;
    public static final int SPECIALCHAR=33;
    public static final int T=34;
    public static final int TESTCASE=35;
    public static final int VARIABLE=36;
    public static final int WORD=37;
    public static final int WS=38;

    // delegates
    public Parser[] getDelegates() {
        return new Parser[] {};
    }

    // delegators


    public ScriptParser(TokenStream input) {
        this(input, new RecognizerSharedState());
    }
    public ScriptParser(TokenStream input, RecognizerSharedState state) {
        super(input, state);
    }

    public String[] getTokenNames() { return ScriptParser.tokenNames; }
    public String getGrammarFileName() { return "C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g"; }



    // $ANTLR start "script"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:29:1: script returns [ScriptModel scriptModel] : (r= robust_expression )* ;
    public final ScriptModel script() throws RecognitionException {
        ScriptModel scriptModel = null;


        CommandModel r =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:29:41: ( (r= robust_expression )* )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:30:2: (r= robust_expression )*
            {
            if ( state.backtracking==0 ) { scriptModel = new ScriptModel(); }

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:31:2: (r= robust_expression )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( (LA1_0==COMMENT||(LA1_0 >= FOR && LA1_0 <= HASHMARKER)||LA1_0==IF||LA1_0==NL||LA1_0==NORMALWORD||LA1_0==SPACE||LA1_0==TESTCASE||(LA1_0 >= WS && LA1_0 <= 42)) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:31:4: r= robust_expression
            	    {
            	    pushFollow(FOLLOW_robust_expression_in_script62);
            	    r=robust_expression();

            	    state._fsp--;
            	    if (state.failed) return scriptModel;

            	    if ( state.backtracking==0 ) { scriptModel.add(r); }

            	    }
            	    break;

            	default :
            	    break loop1;
                }
            } while (true);


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return scriptModel;
    }
    // $ANTLR end "script"



    // $ANTLR start "robust_expression"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:36:1: robust_expression returns [CommandModel cm] : ( ( expression )=>e= expression |u= unknown );
    public final CommandModel robust_expression() throws RecognitionException {
        CommandModel cm = null;


        CommandModel e =null;

        CommandModel u =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:36:44: ( ( expression )=>e= expression |u= unknown )
            int alt2=2;
            int LA2_0 = input.LA(1);

            if ( (LA2_0==COMMENT) && (synpred1_Script())) {
                alt2=1;
            }
            else if ( (LA2_0==HASHMARKER) && (synpred1_Script())) {
                alt2=1;
            }
            else if ( (LA2_0==SPACE) && (synpred1_Script())) {
                alt2=1;
            }
            else if ( (LA2_0==IF) && (synpred1_Script())) {
                alt2=1;
            }
            else if ( (LA2_0==FOR) && (synpred1_Script())) {
                alt2=1;
            }
            else if ( (LA2_0==TESTCASE) && (synpred1_Script())) {
                alt2=1;
            }
            else if ( (LA2_0==NORMALWORD) && (synpred1_Script())) {
                alt2=1;
            }
            else if ( (LA2_0==NL||(LA2_0 >= WS && LA2_0 <= 42)) ) {
                alt2=2;
            }
            else {
                if (state.backtracking>0) {state.failed=true; return cm;}
                NoViableAltException nvae =
                    new NoViableAltException("", 2, 0, input);

                throw nvae;

            }
            switch (alt2) {
                case 1 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:37:5: ( expression )=>e= expression
                    {
                    pushFollow(FOLLOW_expression_in_robust_expression96);
                    e=expression();

                    state._fsp--;
                    if (state.failed) return cm;

                    if ( state.backtracking==0 ) { cm = e; }

                    }
                    break;
                case 2 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:38:7: u= unknown
                    {
                    pushFollow(FOLLOW_unknown_in_robust_expression112);
                    u=unknown();

                    state._fsp--;
                    if (state.failed) return cm;

                    if ( state.backtracking==0 ) { cm = u; }

                    }
                    break;

            }
        }
        catch (RecognitionException re) {

                  cm = CommandModel.newById(re.line - 1, "unknown");
                  re.input.consume();
                
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "robust_expression"



    // $ANTLR start "unknown"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:46:1: unknown returns [CommandModel cm] : c= crapline n= NL ;
    public final CommandModel unknown() throws RecognitionException {
        CommandModel cm = null;


        Token n=null;
        ScriptParser.crapline_return c =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:46:34: (c= crapline n= NL )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:47:5: c= crapline n= NL
            {
            pushFollow(FOLLOW_crapline_in_unknown168);
            c=crapline();

            state._fsp--;
            if (state.failed) return cm;

            if ( state.backtracking==0 ) { String txt = (c!=null?input.toString(c.start,c.stop):null);
                                if (txt != null && !txt.isEmpty()) {
                                  cm = CommandModel.newById(0, "unknown");
                                  cm.getBody().setParameter("text", txt);
                                }
                              }

            n=(Token)match(input,NL,FOLLOW_NL_in_unknown181); if (state.failed) return cm;

            if ( state.backtracking==0 ) { if (cm != null) cm.setStartLine(n.getLine() - 1); }

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "unknown"


    public static class crapline_return extends ParserRuleReturnScope {
    };


    // $ANTLR start "crapline"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:57:1: crapline : ( '$' | '%' | '\"' | ',' | WS )* ;
    public final ScriptParser.crapline_return crapline() throws RecognitionException {
        ScriptParser.crapline_return retval = new ScriptParser.crapline_return();
        retval.start = input.LT(1);


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:57:9: ( ( '$' | '%' | '\"' | ',' | WS )* )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:57:11: ( '$' | '%' | '\"' | ',' | WS )*
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:57:11: ( '$' | '%' | '\"' | ',' | WS )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( ((LA3_0 >= WS && LA3_0 <= 42)) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            	    {
            	    if ( (input.LA(1) >= WS && input.LA(1) <= 42) ) {
            	        input.consume();
            	        state.errorRecovery=false;
            	        state.failed=false;
            	    }
            	    else {
            	        if (state.backtracking>0) {state.failed=true; return retval;}
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop3;
                }
            } while (true);


            }

            retval.stop = input.LT(-1);


        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return retval;
    }
    // $ANTLR end "crapline"



    // $ANTLR start "expression"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:60:1: expression returns [CommandModel cm] : (c= commentblock |m= marker |s= controlstruct |f= function );
    public final CommandModel expression() throws RecognitionException {
        CommandModel cm = null;


        CommandModel c =null;

        CommandModel m =null;

        CommandModel s =null;

        CommandModel f =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:60:37: (c= commentblock |m= marker |s= controlstruct |f= function )
            int alt4=4;
            switch ( input.LA(1) ) {
            case COMMENT:
                {
                alt4=1;
                }
                break;
            case HASHMARKER:
                {
                alt4=2;
                }
                break;
            case SPACE:
                {
                int LA4_3 = input.LA(2);

                if ( (LA4_3==FOR||LA4_3==IF||LA4_3==SPACE||LA4_3==TESTCASE) ) {
                    alt4=3;
                }
                else if ( (LA4_3==NORMALWORD) ) {
                    alt4=4;
                }
                else {
                    if (state.backtracking>0) {state.failed=true; return cm;}
                    NoViableAltException nvae =
                        new NoViableAltException("", 4, 3, input);

                    throw nvae;

                }
                }
                break;
            case FOR:
            case IF:
            case TESTCASE:
                {
                alt4=3;
                }
                break;
            case NORMALWORD:
                {
                alt4=4;
                }
                break;
            default:
                if (state.backtracking>0) {state.failed=true; return cm;}
                NoViableAltException nvae =
                    new NoViableAltException("", 4, 0, input);

                throw nvae;

            }

            switch (alt4) {
                case 1 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:61:5: c= commentblock
                    {
                    pushFollow(FOLLOW_commentblock_in_expression250);
                    c=commentblock();

                    state._fsp--;
                    if (state.failed) return cm;

                    if ( state.backtracking==0 ) { cm = c; }

                    }
                    break;
                case 2 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:62:4: m= marker
                    {
                    pushFollow(FOLLOW_marker_in_expression265);
                    m=marker();

                    state._fsp--;
                    if (state.failed) return cm;

                    if ( state.backtracking==0 ) { cm = m; }

                    }
                    break;
                case 3 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:63:7: s= controlstruct
                    {
                    pushFollow(FOLLOW_controlstruct_in_expression289);
                    s=controlstruct();

                    state._fsp--;
                    if (state.failed) return cm;

                    if ( state.backtracking==0 ) { cm = s; }

                    }
                    break;
                case 4 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:64:7: f= function
                    {
                    pushFollow(FOLLOW_function_in_expression305);
                    f=function();

                    state._fsp--;
                    if (state.failed) return cm;

                    if ( state.backtracking==0 ) { cm = f; }

                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "expression"



    // $ANTLR start "function"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:70:1: function returns [CommandModel cm] : ( SPACE )? c= NORMALWORD a= argumentlist NL ;
    public final CommandModel function() throws RecognitionException {
        CommandModel cm = null;


        Token c=null;
        List a =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:70:35: ( ( SPACE )? c= NORMALWORD a= argumentlist NL )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:71:4: ( SPACE )? c= NORMALWORD a= argumentlist NL
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:71:4: ( SPACE )?
            int alt5=2;
            int LA5_0 = input.LA(1);

            if ( (LA5_0==SPACE) ) {
                alt5=1;
            }
            switch (alt5) {
                case 1 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:71:4: SPACE
                    {
                    match(input,SPACE,FOLLOW_SPACE_in_function349); if (state.failed) return cm;

                    }
                    break;

            }


            c=(Token)match(input,NORMALWORD,FOLLOW_NORMALWORD_in_function358); if (state.failed) return cm;

            pushFollow(FOLLOW_argumentlist_in_function366);
            a=argumentlist();

            state._fsp--;
            if (state.failed) return cm;

            if ( state.backtracking==0 ) { cm = new CommandModel( c.getLine() - 1, (c!=null?c.getText():null), a); }

            match(input,NL,FOLLOW_NL_in_function372); if (state.failed) return cm;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "function"



    // $ANTLR start "commentblock"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:78:1: commentblock returns [CommandModel cm] : (c= comment )+ ;
    public final CommandModel commentblock() throws RecognitionException {
        CommandModel cm = null;


        CommandModel c =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:78:39: ( (c= comment )+ )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:79:4: (c= comment )+
            {
            if ( state.backtracking==0 ) { cm = CommandModel.newById(0, "commentblock"); }

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:80:4: (c= comment )+
            int cnt6=0;
            loop6:
            do {
                int alt6=2;
                int LA6_0 = input.LA(1);

                if ( (LA6_0==COMMENT) ) {
                    alt6=1;
                }


                switch (alt6) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:80:6: c= comment
            	    {
            	    pushFollow(FOLLOW_comment_in_commentblock403);
            	    c=comment();

            	    state._fsp--;
            	    if (state.failed) return cm;

            	    if ( state.backtracking==0 ) { cm.add( c ); }

            	    }
            	    break;

            	default :
            	    if ( cnt6 >= 1 ) break loop6;
            	    if (state.backtracking>0) {state.failed=true; return cm;}
                        EarlyExitException eee =
                            new EarlyExitException(6, input);
                        throw eee;
                }
                cnt6++;
            } while (true);


            if ( state.backtracking==0 ) { 
                 if (cm.size() == 1) {
                   cm = cm.get(0);
                 } else {
                   cm.setStartLine( cm.get(0).getStartLine() );
                 }
               }

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "commentblock"



    // $ANTLR start "comment"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:91:1: comment returns [CommandModel cm] : c= COMMENT NL ;
    public final CommandModel comment() throws RecognitionException {
        CommandModel cm = null;


        Token c=null;

        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:91:34: (c= COMMENT NL )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:92:3: c= COMMENT NL
            {
            c=(Token)match(input,COMMENT,FOLLOW_COMMENT_in_comment437); if (state.failed) return cm;

            if ( state.backtracking==0 ) { cm = new CommandModel( c.getLine() - 1, "*", (c!=null?c.getText():null).substring(1)); }

            match(input,NL,FOLLOW_NL_in_comment445); if (state.failed) return cm;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "comment"


    public static class argument_return extends ParserRuleReturnScope {
    };


    // $ANTLR start "argument"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:97:1: argument : ( QUOTE | CONSTANT | VARIABLE | NORMALWORD | SPACE | keyword | WORD )* ;
    public final ScriptParser.argument_return argument() throws RecognitionException {
        ScriptParser.argument_return retval = new ScriptParser.argument_return();
        retval.start = input.LT(1);


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:97:9: ( ( QUOTE | CONSTANT | VARIABLE | NORMALWORD | SPACE | keyword | WORD )* )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:97:10: ( QUOTE | CONSTANT | VARIABLE | NORMALWORD | SPACE | keyword | WORD )*
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:97:10: ( QUOTE | CONSTANT | VARIABLE | NORMALWORD | SPACE | keyword | WORD )*
            loop7:
            do {
                int alt7=8;
                switch ( input.LA(1) ) {
                case QUOTE:
                    {
                    alt7=1;
                    }
                    break;
                case CONSTANT:
                    {
                    alt7=2;
                    }
                    break;
                case VARIABLE:
                    {
                    alt7=3;
                    }
                    break;
                case NORMALWORD:
                    {
                    alt7=4;
                    }
                    break;
                case SPACE:
                    {
                    alt7=5;
                    }
                    break;
                case ELSE:
                case ELSEIF:
                case END:
                case ENDFOR:
                case ENDIF:
                case FOR:
                case IF:
                case TESTCASE:
                    {
                    alt7=6;
                    }
                    break;
                case WORD:
                    {
                    alt7=7;
                    }
                    break;

                }

                switch (alt7) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:98:13: QUOTE
            	    {
            	    match(input,QUOTE,FOLLOW_QUOTE_in_argument469); if (state.failed) return retval;

            	    }
            	    break;
            	case 2 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:99:13: CONSTANT
            	    {
            	    match(input,CONSTANT,FOLLOW_CONSTANT_in_argument483); if (state.failed) return retval;

            	    }
            	    break;
            	case 3 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:100:13: VARIABLE
            	    {
            	    match(input,VARIABLE,FOLLOW_VARIABLE_in_argument497); if (state.failed) return retval;

            	    }
            	    break;
            	case 4 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:101:13: NORMALWORD
            	    {
            	    match(input,NORMALWORD,FOLLOW_NORMALWORD_in_argument511); if (state.failed) return retval;

            	    }
            	    break;
            	case 5 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:102:13: SPACE
            	    {
            	    match(input,SPACE,FOLLOW_SPACE_in_argument525); if (state.failed) return retval;

            	    }
            	    break;
            	case 6 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:103:13: keyword
            	    {
            	    pushFollow(FOLLOW_keyword_in_argument539);
            	    keyword();

            	    state._fsp--;
            	    if (state.failed) return retval;

            	    }
            	    break;
            	case 7 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:104:13: WORD
            	    {
            	    match(input,WORD,FOLLOW_WORD_in_argument553); if (state.failed) return retval;

            	    }
            	    break;

            	default :
            	    break loop7;
                }
            } while (true);


            }

            retval.stop = input.LT(-1);


        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return retval;
    }
    // $ANTLR end "argument"



    // $ANTLR start "keyword"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:109:1: keyword : ( IF | ELSEIF | ELSE | ENDIF | FOR | ENDFOR | TESTCASE | END ) ;
    public final void keyword() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:109:8: ( ( IF | ELSEIF | ELSE | ENDIF | FOR | ENDFOR | TESTCASE | END ) )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( (input.LA(1) >= ELSE && input.LA(1) <= ENDIF)||input.LA(1)==FOR||input.LA(1)==IF||input.LA(1)==TESTCASE ) {
                input.consume();
                state.errorRecovery=false;
                state.failed=false;
            }
            else {
                if (state.backtracking>0) {state.failed=true; return ;}
                MismatchedSetException mse = new MismatchedSetException(null,input);
                throw mse;
            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return ;
    }
    // $ANTLR end "keyword"



    // $ANTLR start "argumentlist"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:112:1: argumentlist returns [List list] :a= argument ( ',' b= argument )* ;
    public final List argumentlist() throws RecognitionException {
        List list = null;


        ScriptParser.argument_return a =null;

        ScriptParser.argument_return b =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:112:33: (a= argument ( ',' b= argument )* )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:113:3: a= argument ( ',' b= argument )*
            {
            if ( state.backtracking==0 ) { list = new ArrayList<String>(); }

            pushFollow(FOLLOW_argument_in_argumentlist679);
            a=argument();

            state._fsp--;
            if (state.failed) return list;

            if ( state.backtracking==0 ) { list.add((a!=null?input.toString(a.start,a.stop):null).trim()); }

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:115:3: ( ',' b= argument )*
            loop8:
            do {
                int alt8=2;
                int LA8_0 = input.LA(1);

                if ( (LA8_0==42) ) {
                    alt8=1;
                }


                switch (alt8) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:115:4: ',' b= argument
            	    {
            	    match(input,42,FOLLOW_42_in_argumentlist689); if (state.failed) return list;

            	    pushFollow(FOLLOW_argument_in_argumentlist696);
            	    b=argument();

            	    state._fsp--;
            	    if (state.failed) return list;

            	    if ( state.backtracking==0 ) { list.add((b!=null?input.toString(b.start,b.stop):null).trim()); }

            	    }
            	    break;

            	default :
            	    break loop8;
                }
            } while (true);


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return list;
    }
    // $ANTLR end "argumentlist"



    // $ANTLR start "marker"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:120:1: marker returns [CommandModel cm] : HASHMARKER ( SPACE )? n= NORMALWORD ( SPACE )? NL ;
    public final CommandModel marker() throws RecognitionException {
        CommandModel cm = null;


        Token n=null;

        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:120:34: ( HASHMARKER ( SPACE )? n= NORMALWORD ( SPACE )? NL )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:121:3: HASHMARKER ( SPACE )? n= NORMALWORD ( SPACE )? NL
            {
            match(input,HASHMARKER,FOLLOW_HASHMARKER_in_marker725); if (state.failed) return cm;

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:121:14: ( SPACE )?
            int alt9=2;
            int LA9_0 = input.LA(1);

            if ( (LA9_0==SPACE) ) {
                alt9=1;
            }
            switch (alt9) {
                case 1 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:121:14: SPACE
                    {
                    match(input,SPACE,FOLLOW_SPACE_in_marker727); if (state.failed) return cm;

                    }
                    break;

            }


            n=(Token)match(input,NORMALWORD,FOLLOW_NORMALWORD_in_marker734); if (state.failed) return cm;

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:121:36: ( SPACE )?
            int alt10=2;
            int LA10_0 = input.LA(1);

            if ( (LA10_0==SPACE) ) {
                alt10=1;
            }
            switch (alt10) {
                case 1 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:121:36: SPACE
                    {
                    match(input,SPACE,FOLLOW_SPACE_in_marker736); if (state.failed) return cm;

                    }
                    break;

            }


            match(input,NL,FOLLOW_NL_in_marker739); if (state.failed) return cm;

            if ( state.backtracking==0 ) { cm = new CommandModel(n.getLine() - 1, "#", (n!=null?n.getText():null)); }

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "marker"



    // $ANTLR start "controlstruct"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:124:1: controlstruct returns [CommandModel cm] : (i= ifexpr |f= for_block |t= test_block );
    public final CommandModel controlstruct() throws RecognitionException {
        CommandModel cm = null;


        CommandModel i =null;

        CommandModel f =null;

        CommandModel t =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:124:40: (i= ifexpr |f= for_block |t= test_block )
            int alt11=3;
            alt11 = dfa11.predict(input);
            switch (alt11) {
                case 1 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:125:6: i= ifexpr
                    {
                    pushFollow(FOLLOW_ifexpr_in_controlstruct764);
                    i=ifexpr();

                    state._fsp--;
                    if (state.failed) return cm;

                    if ( state.backtracking==0 ) { cm = i; }

                    }
                    break;
                case 2 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:126:7: f= for_block
                    {
                    pushFollow(FOLLOW_for_block_in_controlstruct782);
                    f=for_block();

                    state._fsp--;
                    if (state.failed) return cm;

                    if ( state.backtracking==0 ) { cm = f; }

                    }
                    break;
                case 3 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:127:4: t= test_block
                    {
                    pushFollow(FOLLOW_test_block_in_controlstruct794);
                    t=test_block();

                    state._fsp--;
                    if (state.failed) return cm;

                    if ( state.backtracking==0 ) { cm = t; }

                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "controlstruct"



    // $ANTLR start "ifexpr"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:132:1: ifexpr returns [CommandModel cm] :ib= if_block (eib= elseif_block )* (eb= else_block )? ( SPACE )* ENDIF NL ;
    public final CommandModel ifexpr() throws RecognitionException {
        CommandModel cm = null;


        CommandModel ib =null;

        CommandModel eib =null;

        CommandModel eb =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:132:33: (ib= if_block (eib= elseif_block )* (eb= else_block )? ( SPACE )* ENDIF NL )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:133:2: ib= if_block (eib= elseif_block )* (eb= else_block )? ( SPACE )* ENDIF NL
            {
            if ( state.backtracking==0 ) { cm = CommandModel.newById(0, "condition"); }

            pushFollow(FOLLOW_if_block_in_ifexpr829);
            ib=if_block();

            state._fsp--;
            if (state.failed) return cm;

            if ( state.backtracking==0 ) { cm.add(ib); cm.setStartLine(ib.getStartLine()); }

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:135:2: (eib= elseif_block )*
            loop12:
            do {
                int alt12=2;
                alt12 = dfa12.predict(input);
                switch (alt12) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:135:4: eib= elseif_block
            	    {
            	    pushFollow(FOLLOW_elseif_block_in_ifexpr844);
            	    eib=elseif_block();

            	    state._fsp--;
            	    if (state.failed) return cm;

            	    if ( state.backtracking==0 ) { cm.add(eib); }

            	    }
            	    break;

            	default :
            	    break loop12;
                }
            } while (true);


            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:136:2: (eb= else_block )?
            int alt13=2;
            alt13 = dfa13.predict(input);
            switch (alt13) {
                case 1 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:136:5: eb= else_block
                    {
                    pushFollow(FOLLOW_else_block_in_ifexpr860);
                    eb=else_block();

                    state._fsp--;
                    if (state.failed) return cm;

                    if ( state.backtracking==0 ) { cm.add(eb);  }

                    }
                    break;

            }


            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:137:2: ( SPACE )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( (LA14_0==SPACE) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:137:2: SPACE
            	    {
            	    match(input,SPACE,FOLLOW_SPACE_in_ifexpr871); if (state.failed) return cm;

            	    }
            	    break;

            	default :
            	    break loop14;
                }
            } while (true);


            match(input,ENDIF,FOLLOW_ENDIF_in_ifexpr874); if (state.failed) return cm;

            match(input,NL,FOLLOW_NL_in_ifexpr876); if (state.failed) return cm;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "ifexpr"



    // $ANTLR start "if_block"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:141:1: if_block returns [CommandModel cm] : ( SPACE )* a= IF be= boolexpr NL (e= robust_expression )* ;
    public final CommandModel if_block() throws RecognitionException {
        CommandModel cm = null;


        Token a=null;
        String be =null;

        CommandModel e =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:141:35: ( ( SPACE )* a= IF be= boolexpr NL (e= robust_expression )* )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:142:2: ( SPACE )* a= IF be= boolexpr NL (e= robust_expression )*
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:142:2: ( SPACE )*
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( (LA15_0==SPACE) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:142:2: SPACE
            	    {
            	    match(input,SPACE,FOLLOW_SPACE_in_if_block891); if (state.failed) return cm;

            	    }
            	    break;

            	default :
            	    break loop15;
                }
            } while (true);


            a=(Token)match(input,IF,FOLLOW_IF_in_if_block898); if (state.failed) return cm;

            pushFollow(FOLLOW_boolexpr_in_if_block904);
            be=boolexpr();

            state._fsp--;
            if (state.failed) return cm;

            if ( state.backtracking==0 ) { cm = new CommandModel(a.getLine() - 1, "if", be); }

            match(input,NL,FOLLOW_NL_in_if_block914); if (state.failed) return cm;

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:144:2: (e= robust_expression )*
            loop16:
            do {
                int alt16=2;
                alt16 = dfa16.predict(input);
                switch (alt16) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:144:4: e= robust_expression
            	    {
            	    pushFollow(FOLLOW_robust_expression_in_if_block923);
            	    e=robust_expression();

            	    state._fsp--;
            	    if (state.failed) return cm;

            	    if ( state.backtracking==0 ) { cm.add(e); }

            	    }
            	    break;

            	default :
            	    break loop16;
                }
            } while (true);


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "if_block"



    // $ANTLR start "elseif_block"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:148:1: elseif_block returns [CommandModel cm] : ( SPACE )* a= ELSEIF be= boolexpr NL (e= robust_expression )* ;
    public final CommandModel elseif_block() throws RecognitionException {
        CommandModel cm = null;


        Token a=null;
        String be =null;

        CommandModel e =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:148:39: ( ( SPACE )* a= ELSEIF be= boolexpr NL (e= robust_expression )* )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:149:2: ( SPACE )* a= ELSEIF be= boolexpr NL (e= robust_expression )*
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:149:2: ( SPACE )*
            loop17:
            do {
                int alt17=2;
                int LA17_0 = input.LA(1);

                if ( (LA17_0==SPACE) ) {
                    alt17=1;
                }


                switch (alt17) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:149:2: SPACE
            	    {
            	    match(input,SPACE,FOLLOW_SPACE_in_elseif_block945); if (state.failed) return cm;

            	    }
            	    break;

            	default :
            	    break loop17;
                }
            } while (true);


            a=(Token)match(input,ELSEIF,FOLLOW_ELSEIF_in_elseif_block952); if (state.failed) return cm;

            pushFollow(FOLLOW_boolexpr_in_elseif_block958);
            be=boolexpr();

            state._fsp--;
            if (state.failed) return cm;

            if ( state.backtracking==0 ) { cm = new CommandModel(a.getLine() - 1, "elseif", be); }

            match(input,NL,FOLLOW_NL_in_elseif_block964); if (state.failed) return cm;

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:151:2: (e= robust_expression )*
            loop18:
            do {
                int alt18=2;
                alt18 = dfa18.predict(input);
                switch (alt18) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:151:4: e= robust_expression
            	    {
            	    pushFollow(FOLLOW_robust_expression_in_elseif_block973);
            	    e=robust_expression();

            	    state._fsp--;
            	    if (state.failed) return cm;

            	    if ( state.backtracking==0 ) { cm.add(e); }

            	    }
            	    break;

            	default :
            	    break loop18;
                }
            } while (true);


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "elseif_block"



    // $ANTLR start "else_block"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:155:1: else_block returns [CommandModel cm] : ( SPACE )* a= ELSE NL (e= robust_expression )* ;
    public final CommandModel else_block() throws RecognitionException {
        CommandModel cm = null;


        Token a=null;
        CommandModel e =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:155:37: ( ( SPACE )* a= ELSE NL (e= robust_expression )* )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:156:2: ( SPACE )* a= ELSE NL (e= robust_expression )*
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:156:2: ( SPACE )*
            loop19:
            do {
                int alt19=2;
                int LA19_0 = input.LA(1);

                if ( (LA19_0==SPACE) ) {
                    alt19=1;
                }


                switch (alt19) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:156:2: SPACE
            	    {
            	    match(input,SPACE,FOLLOW_SPACE_in_else_block996); if (state.failed) return cm;

            	    }
            	    break;

            	default :
            	    break loop19;
                }
            } while (true);


            a=(Token)match(input,ELSE,FOLLOW_ELSE_in_else_block1003); if (state.failed) return cm;

            if ( state.backtracking==0 ) { cm = new CommandModel(a.getLine() - 1, "else"); }

            match(input,NL,FOLLOW_NL_in_else_block1023); if (state.failed) return cm;

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:158:2: (e= robust_expression )*
            loop20:
            do {
                int alt20=2;
                alt20 = dfa20.predict(input);
                switch (alt20) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:158:4: e= robust_expression
            	    {
            	    pushFollow(FOLLOW_robust_expression_in_else_block1032);
            	    e=robust_expression();

            	    state._fsp--;
            	    if (state.failed) return cm;

            	    if ( state.backtracking==0 ) { cm.add(e); }

            	    }
            	    break;

            	default :
            	    break loop20;
                }
            } while (true);


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "else_block"



    // $ANTLR start "boolexpr"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:163:1: boolexpr returns [String string] : a= argument BOOLOP b= argument ;
    public final String boolexpr() throws RecognitionException {
        String string = null;


        Token BOOLOP1=null;
        ScriptParser.argument_return a =null;

        ScriptParser.argument_return b =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:163:33: (a= argument BOOLOP b= argument )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:164:2: a= argument BOOLOP b= argument
            {
            pushFollow(FOLLOW_argument_in_boolexpr1061);
            a=argument();

            state._fsp--;
            if (state.failed) return string;

            if ( state.backtracking==0 ) { string = (a!=null?input.toString(a.start,a.stop):null).trim(); }

            BOOLOP1=(Token)match(input,BOOLOP,FOLLOW_BOOLOP_in_boolexpr1066); if (state.failed) return string;

            if ( state.backtracking==0 ) { string += " " + (BOOLOP1!=null?BOOLOP1.getText():null) + " "; }

            pushFollow(FOLLOW_argument_in_boolexpr1096);
            b=argument();

            state._fsp--;
            if (state.failed) return string;

            if ( state.backtracking==0 ) { string += (b!=null?input.toString(b.start,b.stop):null).trim(); }

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return string;
    }
    // $ANTLR end "boolexpr"



    // $ANTLR start "forarguments"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:169:1: forarguments returns [List list] :i= NORMALWORD ',' c= argument ',' be= boolexpr ',' a= function ;
    public final List forarguments() throws RecognitionException {
        List list = null;


        Token i=null;
        ScriptParser.argument_return c =null;

        String be =null;

        CommandModel a =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:169:33: (i= NORMALWORD ',' c= argument ',' be= boolexpr ',' a= function )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:170:4: i= NORMALWORD ',' c= argument ',' be= boolexpr ',' a= function
            {
            if ( state.backtracking==0 ) {list = new ArrayList<String>();}

            i=(Token)match(input,NORMALWORD,FOLLOW_NORMALWORD_in_forarguments1126); if (state.failed) return list;

            match(input,42,FOLLOW_42_in_forarguments1129); if (state.failed) return list;

            if ( state.backtracking==0 ) {list.add((i!=null?i.getText():null));}

            pushFollow(FOLLOW_argument_in_forarguments1138);
            c=argument();

            state._fsp--;
            if (state.failed) return list;

            match(input,42,FOLLOW_42_in_forarguments1143); if (state.failed) return list;

            if ( state.backtracking==0 ) {list.add((c!=null?input.toString(c.start,c.stop):null).trim());}

            pushFollow(FOLLOW_boolexpr_in_forarguments1152);
            be=boolexpr();

            state._fsp--;
            if (state.failed) return list;

            match(input,42,FOLLOW_42_in_forarguments1156); if (state.failed) return list;

            if ( state.backtracking==0 ) {list.add(be);}

            pushFollow(FOLLOW_function_in_forarguments1165);
            a=function();

            state._fsp--;
            if (state.failed) return list;

            if ( state.backtracking==0 ) {list.add(a.getFrame().getCommand()+" "+a.getBody().getParameters().get("name"));}

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return list;
    }
    // $ANTLR end "forarguments"



    // $ANTLR start "for_block"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:178:1: for_block returns [CommandModel cm] : ( SPACE )* f= FOR ( SPACE )+ p= forarguments (e= robust_expression )* ( SPACE )* ENDFOR NL ;
    public final CommandModel for_block() throws RecognitionException {
        CommandModel cm = null;


        Token f=null;
        List p =null;

        CommandModel e =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:178:36: ( ( SPACE )* f= FOR ( SPACE )+ p= forarguments (e= robust_expression )* ( SPACE )* ENDFOR NL )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:179:3: ( SPACE )* f= FOR ( SPACE )+ p= forarguments (e= robust_expression )* ( SPACE )* ENDFOR NL
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:179:3: ( SPACE )*
            loop21:
            do {
                int alt21=2;
                int LA21_0 = input.LA(1);

                if ( (LA21_0==SPACE) ) {
                    alt21=1;
                }


                switch (alt21) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:179:3: SPACE
            	    {
            	    match(input,SPACE,FOLLOW_SPACE_in_for_block1190); if (state.failed) return cm;

            	    }
            	    break;

            	default :
            	    break loop21;
                }
            } while (true);


            f=(Token)match(input,FOR,FOLLOW_FOR_in_for_block1197); if (state.failed) return cm;

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:179:18: ( SPACE )+
            int cnt22=0;
            loop22:
            do {
                int alt22=2;
                int LA22_0 = input.LA(1);

                if ( (LA22_0==SPACE) ) {
                    alt22=1;
                }


                switch (alt22) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:179:18: SPACE
            	    {
            	    match(input,SPACE,FOLLOW_SPACE_in_for_block1199); if (state.failed) return cm;

            	    }
            	    break;

            	default :
            	    if ( cnt22 >= 1 ) break loop22;
            	    if (state.backtracking>0) {state.failed=true; return cm;}
                        EarlyExitException eee =
                            new EarlyExitException(22, input);
                        throw eee;
                }
                cnt22++;
            } while (true);


            pushFollow(FOLLOW_forarguments_in_for_block1206);
            p=forarguments();

            state._fsp--;
            if (state.failed) return cm;

            if ( state.backtracking==0 ) { cm = new CommandModel(f.getLine() - 1, "for", p); }

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:180:3: (e= robust_expression )*
            loop23:
            do {
                int alt23=2;
                alt23 = dfa23.predict(input);
                switch (alt23) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:180:5: e= robust_expression
            	    {
            	    pushFollow(FOLLOW_robust_expression_in_for_block1219);
            	    e=robust_expression();

            	    state._fsp--;
            	    if (state.failed) return cm;

            	    if ( state.backtracking==0 ) { cm.add(e); }

            	    }
            	    break;

            	default :
            	    break loop23;
                }
            } while (true);


            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:181:3: ( SPACE )*
            loop24:
            do {
                int alt24=2;
                int LA24_0 = input.LA(1);

                if ( (LA24_0==SPACE) ) {
                    alt24=1;
                }


                switch (alt24) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:181:3: SPACE
            	    {
            	    match(input,SPACE,FOLLOW_SPACE_in_for_block1234); if (state.failed) return cm;

            	    }
            	    break;

            	default :
            	    break loop24;
                }
            } while (true);


            match(input,ENDFOR,FOLLOW_ENDFOR_in_for_block1237); if (state.failed) return cm;

            match(input,NL,FOLLOW_NL_in_for_block1239); if (state.failed) return cm;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "for_block"



    // $ANTLR start "testarguments"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:184:1: testarguments returns [List list] :id= argument ',' des= argument ( ',' exp= NORMALWORD )? ;
    public final List testarguments() throws RecognitionException {
        List list = null;


        Token exp=null;
        ScriptParser.argument_return id =null;

        ScriptParser.argument_return des =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:184:34: (id= argument ',' des= argument ( ',' exp= NORMALWORD )? )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:185:5: id= argument ',' des= argument ( ',' exp= NORMALWORD )?
            {
            if ( state.backtracking==0 ) {list = new ArrayList<String>();}

            pushFollow(FOLLOW_argument_in_testarguments1262);
            id=argument();

            state._fsp--;
            if (state.failed) return list;

            match(input,42,FOLLOW_42_in_testarguments1265); if (state.failed) return list;

            if ( state.backtracking==0 ) {list.add((id!=null?input.toString(id.start,id.stop):null).trim());}

            pushFollow(FOLLOW_argument_in_testarguments1277);
            des=argument();

            state._fsp--;
            if (state.failed) return list;

            if ( state.backtracking==0 ) {list.add((des!=null?input.toString(des.start,des.stop):null).trim());}

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:188:2: ( ',' exp= NORMALWORD )?
            int alt25=2;
            int LA25_0 = input.LA(1);

            if ( (LA25_0==42) ) {
                alt25=1;
            }
            switch (alt25) {
                case 1 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:188:3: ',' exp= NORMALWORD
                    {
                    match(input,42,FOLLOW_42_in_testarguments1287); if (state.failed) return list;

                    exp=(Token)match(input,NORMALWORD,FOLLOW_NORMALWORD_in_testarguments1293); if (state.failed) return list;

                    if ( state.backtracking==0 ) {list.add((exp!=null?exp.getText():null).trim());}

                    }
                    break;

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return list;
    }
    // $ANTLR end "testarguments"



    // $ANTLR start "test_block"
    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:191:1: test_block returns [CommandModel cm] : ( SPACE )* t= TESTCASE SPACE p= testarguments NL (f= function )* ( SPACE )* END NL ;
    public final CommandModel test_block() throws RecognitionException {
        CommandModel cm = null;


        Token t=null;
        List p =null;

        CommandModel f =null;


        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:191:37: ( ( SPACE )* t= TESTCASE SPACE p= testarguments NL (f= function )* ( SPACE )* END NL )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:192:5: ( SPACE )* t= TESTCASE SPACE p= testarguments NL (f= function )* ( SPACE )* END NL
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:192:5: ( SPACE )*
            loop26:
            do {
                int alt26=2;
                int LA26_0 = input.LA(1);

                if ( (LA26_0==SPACE) ) {
                    alt26=1;
                }


                switch (alt26) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:192:5: SPACE
            	    {
            	    match(input,SPACE,FOLLOW_SPACE_in_test_block1321); if (state.failed) return cm;

            	    }
            	    break;

            	default :
            	    break loop26;
                }
            } while (true);


            t=(Token)match(input,TESTCASE,FOLLOW_TESTCASE_in_test_block1328); if (state.failed) return cm;

            match(input,SPACE,FOLLOW_SPACE_in_test_block1330); if (state.failed) return cm;

            pushFollow(FOLLOW_testarguments_in_test_block1336);
            p=testarguments();

            state._fsp--;
            if (state.failed) return cm;

            if ( state.backtracking==0 ) {cm = new CommandModel(t.getLine() - 1, "TestCase", p);}

            match(input,NL,FOLLOW_NL_in_test_block1341); if (state.failed) return cm;

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:194:2: (f= function )*
            loop27:
            do {
                int alt27=2;
                int LA27_0 = input.LA(1);

                if ( (LA27_0==SPACE) ) {
                    int LA27_1 = input.LA(2);

                    if ( (LA27_1==NORMALWORD) ) {
                        alt27=1;
                    }


                }
                else if ( (LA27_0==NORMALWORD) ) {
                    alt27=1;
                }


                switch (alt27) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:194:3: f= function
            	    {
            	    pushFollow(FOLLOW_function_in_test_block1349);
            	    f=function();

            	    state._fsp--;
            	    if (state.failed) return cm;

            	    if ( state.backtracking==0 ) { cm.add(f); }

            	    }
            	    break;

            	default :
            	    break loop27;
                }
            } while (true);


            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:195:2: ( SPACE )*
            loop28:
            do {
                int alt28=2;
                int LA28_0 = input.LA(1);

                if ( (LA28_0==SPACE) ) {
                    alt28=1;
                }


                switch (alt28) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:195:2: SPACE
            	    {
            	    match(input,SPACE,FOLLOW_SPACE_in_test_block1362); if (state.failed) return cm;

            	    }
            	    break;

            	default :
            	    break loop28;
                }
            } while (true);


            match(input,END,FOLLOW_END_in_test_block1365); if (state.failed) return cm;

            match(input,NL,FOLLOW_NL_in_test_block1367); if (state.failed) return cm;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }

        finally {
        	// do for sure before leaving
        }
        return cm;
    }
    // $ANTLR end "test_block"

    // $ANTLR start synpred1_Script
    public final void synpred1_Script_fragment() throws RecognitionException {
        // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:37:5: ( expression )
        // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:37:6: expression
        {
        pushFollow(FOLLOW_expression_in_synpred1_Script88);
        expression();

        state._fsp--;
        if (state.failed) return ;

        }

    }
    // $ANTLR end synpred1_Script

    // Delegated rules

    public final boolean synpred1_Script() {
        state.backtracking++;
        int start = input.mark();
        try {
            synpred1_Script_fragment(); // can never throw exception
        } catch (RecognitionException re) {
            System.err.println("impossible: "+re);
        }
        boolean success = !state.failed;
        input.rewind(start);
        state.backtracking--;
        state.failed=false;
        return success;
    }


    protected DFA11 dfa11 = new DFA11(this);
    protected DFA12 dfa12 = new DFA12(this);
    protected DFA13 dfa13 = new DFA13(this);
    protected DFA16 dfa16 = new DFA16(this);
    protected DFA18 dfa18 = new DFA18(this);
    protected DFA20 dfa20 = new DFA20(this);
    protected DFA23 dfa23 = new DFA23(this);
    static final String DFA11_eotS =
        "\5\uffff";
    static final String DFA11_eofS =
        "\5\uffff";
    static final String DFA11_minS =
        "\2\21\3\uffff";
    static final String DFA11_maxS =
        "\2\43\3\uffff";
    static final String DFA11_acceptS =
        "\2\uffff\1\1\1\2\1\3";
    static final String DFA11_specialS =
        "\5\uffff}>";
    static final String[] DFA11_transitionS = {
            "\1\3\2\uffff\1\2\13\uffff\1\1\2\uffff\1\4",
            "\1\3\2\uffff\1\2\13\uffff\1\1\2\uffff\1\4",
            "",
            "",
            ""
    };

    static final short[] DFA11_eot = DFA.unpackEncodedString(DFA11_eotS);
    static final short[] DFA11_eof = DFA.unpackEncodedString(DFA11_eofS);
    static final char[] DFA11_min = DFA.unpackEncodedStringToUnsignedChars(DFA11_minS);
    static final char[] DFA11_max = DFA.unpackEncodedStringToUnsignedChars(DFA11_maxS);
    static final short[] DFA11_accept = DFA.unpackEncodedString(DFA11_acceptS);
    static final short[] DFA11_special = DFA.unpackEncodedString(DFA11_specialS);
    static final short[][] DFA11_transition;

    static {
        int numStates = DFA11_transitionS.length;
        DFA11_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA11_transition[i] = DFA.unpackEncodedString(DFA11_transitionS[i]);
        }
    }

    class DFA11 extends DFA {

        public DFA11(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 11;
            this.eot = DFA11_eot;
            this.eof = DFA11_eof;
            this.min = DFA11_min;
            this.max = DFA11_max;
            this.accept = DFA11_accept;
            this.special = DFA11_special;
            this.transition = DFA11_transition;
        }
        public String getDescription() {
            return "124:1: controlstruct returns [CommandModel cm] : (i= ifexpr |f= for_block |t= test_block );";
        }
    }
    static final String DFA12_eotS =
        "\4\uffff";
    static final String DFA12_eofS =
        "\4\uffff";
    static final String DFA12_minS =
        "\2\13\2\uffff";
    static final String DFA12_maxS =
        "\2\40\2\uffff";
    static final String DFA12_acceptS =
        "\2\uffff\1\2\1\1";
    static final String DFA12_specialS =
        "\4\uffff}>";
    static final String[] DFA12_transitionS = {
            "\1\2\1\3\2\uffff\1\2\20\uffff\1\1",
            "\1\2\1\3\2\uffff\1\2\20\uffff\1\1",
            "",
            ""
    };

    static final short[] DFA12_eot = DFA.unpackEncodedString(DFA12_eotS);
    static final short[] DFA12_eof = DFA.unpackEncodedString(DFA12_eofS);
    static final char[] DFA12_min = DFA.unpackEncodedStringToUnsignedChars(DFA12_minS);
    static final char[] DFA12_max = DFA.unpackEncodedStringToUnsignedChars(DFA12_maxS);
    static final short[] DFA12_accept = DFA.unpackEncodedString(DFA12_acceptS);
    static final short[] DFA12_special = DFA.unpackEncodedString(DFA12_specialS);
    static final short[][] DFA12_transition;

    static {
        int numStates = DFA12_transitionS.length;
        DFA12_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA12_transition[i] = DFA.unpackEncodedString(DFA12_transitionS[i]);
        }
    }

    class DFA12 extends DFA {

        public DFA12(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 12;
            this.eot = DFA12_eot;
            this.eof = DFA12_eof;
            this.min = DFA12_min;
            this.max = DFA12_max;
            this.accept = DFA12_accept;
            this.special = DFA12_special;
            this.transition = DFA12_transition;
        }
        public String getDescription() {
            return "()* loopback of 135:2: (eib= elseif_block )*";
        }
    }
    static final String DFA13_eotS =
        "\4\uffff";
    static final String DFA13_eofS =
        "\4\uffff";
    static final String DFA13_minS =
        "\2\13\2\uffff";
    static final String DFA13_maxS =
        "\2\40\2\uffff";
    static final String DFA13_acceptS =
        "\2\uffff\1\1\1\2";
    static final String DFA13_specialS =
        "\4\uffff}>";
    static final String[] DFA13_transitionS = {
            "\1\2\3\uffff\1\3\20\uffff\1\1",
            "\1\2\3\uffff\1\3\20\uffff\1\1",
            "",
            ""
    };

    static final short[] DFA13_eot = DFA.unpackEncodedString(DFA13_eotS);
    static final short[] DFA13_eof = DFA.unpackEncodedString(DFA13_eofS);
    static final char[] DFA13_min = DFA.unpackEncodedStringToUnsignedChars(DFA13_minS);
    static final char[] DFA13_max = DFA.unpackEncodedStringToUnsignedChars(DFA13_maxS);
    static final short[] DFA13_accept = DFA.unpackEncodedString(DFA13_acceptS);
    static final short[] DFA13_special = DFA.unpackEncodedString(DFA13_specialS);
    static final short[][] DFA13_transition;

    static {
        int numStates = DFA13_transitionS.length;
        DFA13_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA13_transition[i] = DFA.unpackEncodedString(DFA13_transitionS[i]);
        }
    }

    class DFA13 extends DFA {

        public DFA13(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 13;
            this.eot = DFA13_eot;
            this.eof = DFA13_eof;
            this.min = DFA13_min;
            this.max = DFA13_max;
            this.accept = DFA13_accept;
            this.special = DFA13_special;
            this.transition = DFA13_transition;
        }
        public String getDescription() {
            return "136:2: (eb= else_block )?";
        }
    }
    static final String DFA16_eotS =
        "\5\uffff";
    static final String DFA16_eofS =
        "\5\uffff";
    static final String DFA16_minS =
        "\1\7\1\13\2\uffff\1\13";
    static final String DFA16_maxS =
        "\1\52\1\43\2\uffff\1\43";
    static final String DFA16_acceptS =
        "\2\uffff\1\2\1\1\1\uffff";
    static final String DFA16_specialS =
        "\5\uffff}>";
    static final String[] DFA16_transitionS = {
            "\1\3\3\uffff\2\2\2\uffff\1\2\1\uffff\2\3\1\uffff\1\3\4\uffff"+
            "\1\3\1\uffff\1\3\4\uffff\1\1\2\uffff\1\3\2\uffff\5\3",
            "\2\2\2\uffff\1\2\1\uffff\1\3\2\uffff\1\3\6\uffff\1\3\4\uffff"+
            "\1\4\2\uffff\1\3",
            "",
            "",
            "\2\2\2\uffff\1\2\1\uffff\1\3\2\uffff\1\3\13\uffff\1\4\2\uffff"+
            "\1\3"
    };

    static final short[] DFA16_eot = DFA.unpackEncodedString(DFA16_eotS);
    static final short[] DFA16_eof = DFA.unpackEncodedString(DFA16_eofS);
    static final char[] DFA16_min = DFA.unpackEncodedStringToUnsignedChars(DFA16_minS);
    static final char[] DFA16_max = DFA.unpackEncodedStringToUnsignedChars(DFA16_maxS);
    static final short[] DFA16_accept = DFA.unpackEncodedString(DFA16_acceptS);
    static final short[] DFA16_special = DFA.unpackEncodedString(DFA16_specialS);
    static final short[][] DFA16_transition;

    static {
        int numStates = DFA16_transitionS.length;
        DFA16_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA16_transition[i] = DFA.unpackEncodedString(DFA16_transitionS[i]);
        }
    }

    class DFA16 extends DFA {

        public DFA16(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 16;
            this.eot = DFA16_eot;
            this.eof = DFA16_eof;
            this.min = DFA16_min;
            this.max = DFA16_max;
            this.accept = DFA16_accept;
            this.special = DFA16_special;
            this.transition = DFA16_transition;
        }
        public String getDescription() {
            return "()* loopback of 144:2: (e= robust_expression )*";
        }
    }
    static final String DFA18_eotS =
        "\5\uffff";
    static final String DFA18_eofS =
        "\5\uffff";
    static final String DFA18_minS =
        "\1\7\1\13\2\uffff\1\13";
    static final String DFA18_maxS =
        "\1\52\1\43\2\uffff\1\43";
    static final String DFA18_acceptS =
        "\2\uffff\1\2\1\1\1\uffff";
    static final String DFA18_specialS =
        "\5\uffff}>";
    static final String[] DFA18_transitionS = {
            "\1\3\3\uffff\2\2\2\uffff\1\2\1\uffff\2\3\1\uffff\1\3\4\uffff"+
            "\1\3\1\uffff\1\3\4\uffff\1\1\2\uffff\1\3\2\uffff\5\3",
            "\2\2\2\uffff\1\2\1\uffff\1\3\2\uffff\1\3\6\uffff\1\3\4\uffff"+
            "\1\4\2\uffff\1\3",
            "",
            "",
            "\2\2\2\uffff\1\2\1\uffff\1\3\2\uffff\1\3\13\uffff\1\4\2\uffff"+
            "\1\3"
    };

    static final short[] DFA18_eot = DFA.unpackEncodedString(DFA18_eotS);
    static final short[] DFA18_eof = DFA.unpackEncodedString(DFA18_eofS);
    static final char[] DFA18_min = DFA.unpackEncodedStringToUnsignedChars(DFA18_minS);
    static final char[] DFA18_max = DFA.unpackEncodedStringToUnsignedChars(DFA18_maxS);
    static final short[] DFA18_accept = DFA.unpackEncodedString(DFA18_acceptS);
    static final short[] DFA18_special = DFA.unpackEncodedString(DFA18_specialS);
    static final short[][] DFA18_transition;

    static {
        int numStates = DFA18_transitionS.length;
        DFA18_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA18_transition[i] = DFA.unpackEncodedString(DFA18_transitionS[i]);
        }
    }

    class DFA18 extends DFA {

        public DFA18(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 18;
            this.eot = DFA18_eot;
            this.eof = DFA18_eof;
            this.min = DFA18_min;
            this.max = DFA18_max;
            this.accept = DFA18_accept;
            this.special = DFA18_special;
            this.transition = DFA18_transition;
        }
        public String getDescription() {
            return "()* loopback of 151:2: (e= robust_expression )*";
        }
    }
    static final String DFA20_eotS =
        "\5\uffff";
    static final String DFA20_eofS =
        "\5\uffff";
    static final String DFA20_minS =
        "\1\7\1\17\2\uffff\1\17";
    static final String DFA20_maxS =
        "\1\52\1\43\2\uffff\1\43";
    static final String DFA20_acceptS =
        "\2\uffff\1\2\1\1\1\uffff";
    static final String DFA20_specialS =
        "\5\uffff}>";
    static final String[] DFA20_transitionS = {
            "\1\3\7\uffff\1\2\1\uffff\2\3\1\uffff\1\3\4\uffff\1\3\1\uffff"+
            "\1\3\4\uffff\1\1\2\uffff\1\3\2\uffff\5\3",
            "\1\2\1\uffff\1\3\2\uffff\1\3\6\uffff\1\3\4\uffff\1\4\2\uffff"+
            "\1\3",
            "",
            "",
            "\1\2\1\uffff\1\3\2\uffff\1\3\13\uffff\1\4\2\uffff\1\3"
    };

    static final short[] DFA20_eot = DFA.unpackEncodedString(DFA20_eotS);
    static final short[] DFA20_eof = DFA.unpackEncodedString(DFA20_eofS);
    static final char[] DFA20_min = DFA.unpackEncodedStringToUnsignedChars(DFA20_minS);
    static final char[] DFA20_max = DFA.unpackEncodedStringToUnsignedChars(DFA20_maxS);
    static final short[] DFA20_accept = DFA.unpackEncodedString(DFA20_acceptS);
    static final short[] DFA20_special = DFA.unpackEncodedString(DFA20_specialS);
    static final short[][] DFA20_transition;

    static {
        int numStates = DFA20_transitionS.length;
        DFA20_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA20_transition[i] = DFA.unpackEncodedString(DFA20_transitionS[i]);
        }
    }

    class DFA20 extends DFA {

        public DFA20(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 20;
            this.eot = DFA20_eot;
            this.eof = DFA20_eof;
            this.min = DFA20_min;
            this.max = DFA20_max;
            this.accept = DFA20_accept;
            this.special = DFA20_special;
            this.transition = DFA20_transition;
        }
        public String getDescription() {
            return "()* loopback of 158:2: (e= robust_expression )*";
        }
    }
    static final String DFA23_eotS =
        "\5\uffff";
    static final String DFA23_eofS =
        "\5\uffff";
    static final String DFA23_minS =
        "\1\7\1\16\2\uffff\1\16";
    static final String DFA23_maxS =
        "\1\52\1\43\2\uffff\1\43";
    static final String DFA23_acceptS =
        "\2\uffff\1\2\1\1\1\uffff";
    static final String DFA23_specialS =
        "\5\uffff}>";
    static final String[] DFA23_transitionS = {
            "\1\3\6\uffff\1\2\2\uffff\2\3\1\uffff\1\3\4\uffff\1\3\1\uffff"+
            "\1\3\4\uffff\1\1\2\uffff\1\3\2\uffff\5\3",
            "\1\2\2\uffff\1\3\2\uffff\1\3\6\uffff\1\3\4\uffff\1\4\2\uffff"+
            "\1\3",
            "",
            "",
            "\1\2\2\uffff\1\3\2\uffff\1\3\13\uffff\1\4\2\uffff\1\3"
    };

    static final short[] DFA23_eot = DFA.unpackEncodedString(DFA23_eotS);
    static final short[] DFA23_eof = DFA.unpackEncodedString(DFA23_eofS);
    static final char[] DFA23_min = DFA.unpackEncodedStringToUnsignedChars(DFA23_minS);
    static final char[] DFA23_max = DFA.unpackEncodedStringToUnsignedChars(DFA23_maxS);
    static final short[] DFA23_accept = DFA.unpackEncodedString(DFA23_acceptS);
    static final short[] DFA23_special = DFA.unpackEncodedString(DFA23_specialS);
    static final short[][] DFA23_transition;

    static {
        int numStates = DFA23_transitionS.length;
        DFA23_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA23_transition[i] = DFA.unpackEncodedString(DFA23_transitionS[i]);
        }
    }

    class DFA23 extends DFA {

        public DFA23(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 23;
            this.eot = DFA23_eot;
            this.eof = DFA23_eof;
            this.min = DFA23_min;
            this.max = DFA23_max;
            this.accept = DFA23_accept;
            this.special = DFA23_special;
            this.transition = DFA23_transition;
        }
        public String getDescription() {
            return "()* loopback of 180:3: (e= robust_expression )*";
        }
    }
 

    public static final BitSet FOLLOW_robust_expression_in_script62 = new BitSet(new long[]{0x000007C90A160082L});
    public static final BitSet FOLLOW_expression_in_robust_expression96 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_unknown_in_robust_expression112 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_crapline_in_unknown168 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_NL_in_unknown181 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_commentblock_in_expression250 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_marker_in_expression265 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_controlstruct_in_expression289 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_function_in_expression305 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_SPACE_in_function349 = new BitSet(new long[]{0x0000000008000000L});
    public static final BitSet FOLLOW_NORMALWORD_in_function358 = new BitSet(new long[]{0x000004392812F900L});
    public static final BitSet FOLLOW_argumentlist_in_function366 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_NL_in_function372 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_comment_in_commentblock403 = new BitSet(new long[]{0x0000000000000082L});
    public static final BitSet FOLLOW_COMMENT_in_comment437 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_NL_in_comment445 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_QUOTE_in_argument469 = new BitSet(new long[]{0x000000392812F902L});
    public static final BitSet FOLLOW_CONSTANT_in_argument483 = new BitSet(new long[]{0x000000392812F902L});
    public static final BitSet FOLLOW_VARIABLE_in_argument497 = new BitSet(new long[]{0x000000392812F902L});
    public static final BitSet FOLLOW_NORMALWORD_in_argument511 = new BitSet(new long[]{0x000000392812F902L});
    public static final BitSet FOLLOW_SPACE_in_argument525 = new BitSet(new long[]{0x000000392812F902L});
    public static final BitSet FOLLOW_keyword_in_argument539 = new BitSet(new long[]{0x000000392812F902L});
    public static final BitSet FOLLOW_WORD_in_argument553 = new BitSet(new long[]{0x000000392812F902L});
    public static final BitSet FOLLOW_argument_in_argumentlist679 = new BitSet(new long[]{0x0000040000000002L});
    public static final BitSet FOLLOW_42_in_argumentlist689 = new BitSet(new long[]{0x000004392812F900L});
    public static final BitSet FOLLOW_argument_in_argumentlist696 = new BitSet(new long[]{0x0000040000000002L});
    public static final BitSet FOLLOW_HASHMARKER_in_marker725 = new BitSet(new long[]{0x0000000108000000L});
    public static final BitSet FOLLOW_SPACE_in_marker727 = new BitSet(new long[]{0x0000000008000000L});
    public static final BitSet FOLLOW_NORMALWORD_in_marker734 = new BitSet(new long[]{0x0000000102000000L});
    public static final BitSet FOLLOW_SPACE_in_marker736 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_NL_in_marker739 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ifexpr_in_controlstruct764 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_for_block_in_controlstruct782 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_test_block_in_controlstruct794 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_if_block_in_ifexpr829 = new BitSet(new long[]{0x0000000100009800L});
    public static final BitSet FOLLOW_elseif_block_in_ifexpr844 = new BitSet(new long[]{0x0000000100009800L});
    public static final BitSet FOLLOW_else_block_in_ifexpr860 = new BitSet(new long[]{0x0000000100008000L});
    public static final BitSet FOLLOW_SPACE_in_ifexpr871 = new BitSet(new long[]{0x0000000100008000L});
    public static final BitSet FOLLOW_ENDIF_in_ifexpr874 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_NL_in_ifexpr876 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_SPACE_in_if_block891 = new BitSet(new long[]{0x0000000100100000L});
    public static final BitSet FOLLOW_IF_in_if_block898 = new BitSet(new long[]{0x000000392812F920L});
    public static final BitSet FOLLOW_boolexpr_in_if_block904 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_NL_in_if_block914 = new BitSet(new long[]{0x000007C90A160082L});
    public static final BitSet FOLLOW_robust_expression_in_if_block923 = new BitSet(new long[]{0x000007C90A160082L});
    public static final BitSet FOLLOW_SPACE_in_elseif_block945 = new BitSet(new long[]{0x0000000100001000L});
    public static final BitSet FOLLOW_ELSEIF_in_elseif_block952 = new BitSet(new long[]{0x000000392812F920L});
    public static final BitSet FOLLOW_boolexpr_in_elseif_block958 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_NL_in_elseif_block964 = new BitSet(new long[]{0x000007C90A160082L});
    public static final BitSet FOLLOW_robust_expression_in_elseif_block973 = new BitSet(new long[]{0x000007C90A160082L});
    public static final BitSet FOLLOW_SPACE_in_else_block996 = new BitSet(new long[]{0x0000000100000800L});
    public static final BitSet FOLLOW_ELSE_in_else_block1003 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_NL_in_else_block1023 = new BitSet(new long[]{0x000007C90A160082L});
    public static final BitSet FOLLOW_robust_expression_in_else_block1032 = new BitSet(new long[]{0x000007C90A160082L});
    public static final BitSet FOLLOW_argument_in_boolexpr1061 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_BOOLOP_in_boolexpr1066 = new BitSet(new long[]{0x000000392812F900L});
    public static final BitSet FOLLOW_argument_in_boolexpr1096 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_NORMALWORD_in_forarguments1126 = new BitSet(new long[]{0x0000040000000000L});
    public static final BitSet FOLLOW_42_in_forarguments1129 = new BitSet(new long[]{0x000004392812F900L});
    public static final BitSet FOLLOW_argument_in_forarguments1138 = new BitSet(new long[]{0x0000040000000000L});
    public static final BitSet FOLLOW_42_in_forarguments1143 = new BitSet(new long[]{0x000000392812F920L});
    public static final BitSet FOLLOW_boolexpr_in_forarguments1152 = new BitSet(new long[]{0x0000040000000000L});
    public static final BitSet FOLLOW_42_in_forarguments1156 = new BitSet(new long[]{0x0000000108000000L});
    public static final BitSet FOLLOW_function_in_forarguments1165 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_SPACE_in_for_block1190 = new BitSet(new long[]{0x0000000100020000L});
    public static final BitSet FOLLOW_FOR_in_for_block1197 = new BitSet(new long[]{0x0000000100000000L});
    public static final BitSet FOLLOW_SPACE_in_for_block1199 = new BitSet(new long[]{0x0000000108000000L});
    public static final BitSet FOLLOW_forarguments_in_for_block1206 = new BitSet(new long[]{0x000007C90A164080L});
    public static final BitSet FOLLOW_robust_expression_in_for_block1219 = new BitSet(new long[]{0x000007C90A164080L});
    public static final BitSet FOLLOW_SPACE_in_for_block1234 = new BitSet(new long[]{0x0000000100004000L});
    public static final BitSet FOLLOW_ENDFOR_in_for_block1237 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_NL_in_for_block1239 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_argument_in_testarguments1262 = new BitSet(new long[]{0x0000040000000000L});
    public static final BitSet FOLLOW_42_in_testarguments1265 = new BitSet(new long[]{0x000004392812F900L});
    public static final BitSet FOLLOW_argument_in_testarguments1277 = new BitSet(new long[]{0x0000040000000002L});
    public static final BitSet FOLLOW_42_in_testarguments1287 = new BitSet(new long[]{0x0000000008000000L});
    public static final BitSet FOLLOW_NORMALWORD_in_testarguments1293 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_SPACE_in_test_block1321 = new BitSet(new long[]{0x0000000900000000L});
    public static final BitSet FOLLOW_TESTCASE_in_test_block1328 = new BitSet(new long[]{0x0000000100000000L});
    public static final BitSet FOLLOW_SPACE_in_test_block1330 = new BitSet(new long[]{0x000004392812F900L});
    public static final BitSet FOLLOW_testarguments_in_test_block1336 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_NL_in_test_block1341 = new BitSet(new long[]{0x0000000108002000L});
    public static final BitSet FOLLOW_function_in_test_block1349 = new BitSet(new long[]{0x0000000108002000L});
    public static final BitSet FOLLOW_SPACE_in_test_block1362 = new BitSet(new long[]{0x0000000100002000L});
    public static final BitSet FOLLOW_END_in_test_block1365 = new BitSet(new long[]{0x0000000002000000L});
    public static final BitSet FOLLOW_NL_in_test_block1367 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_expression_in_synpred1_Script88 = new BitSet(new long[]{0x0000000000000002L});

}