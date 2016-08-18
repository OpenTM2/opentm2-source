/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/
// $ANTLR 3.4 C:\\eclipseworkspace\\openTM2ScripterGUI-new\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g 2014-11-03 14:08:06

  package de.ibm.com.opentm2scripteride.parser;


import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;

@SuppressWarnings({"all", "warnings", "unchecked"})
public class ScriptLexer extends Lexer {
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
    // delegators
    public Lexer[] getDelegates() {
        return new Lexer[] {};
    }

    public ScriptLexer() {} 
    public ScriptLexer(CharStream input) {
        this(input, new RecognizerSharedState());
    }
    public ScriptLexer(CharStream input, RecognizerSharedState state) {
        super(input,state);
    }
    public String getGrammarFileName() { return "C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g"; }

    // $ANTLR start "T__39"
    public final void mT__39() throws RecognitionException {
        try {
            int _type = T__39;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:11:7: ( '\"' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:11:9: '\"'
            {
            match('\"'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "T__39"

    // $ANTLR start "T__40"
    public final void mT__40() throws RecognitionException {
        try {
            int _type = T__40;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:12:7: ( '$' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:12:9: '$'
            {
            match('$'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "T__40"

    // $ANTLR start "T__41"
    public final void mT__41() throws RecognitionException {
        try {
            int _type = T__41;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:13:7: ( '%' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:13:9: '%'
            {
            match('%'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "T__41"

    // $ANTLR start "T__42"
    public final void mT__42() throws RecognitionException {
        try {
            int _type = T__42;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:14:7: ( ',' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:14:9: ','
            {
            match(','); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "T__42"

    // $ANTLR start "COMMENT"
    public final void mCOMMENT() throws RecognitionException {
        try {
            int _type = COMMENT;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:202:8: ({...}? => ( SPACE )* '*' (~ ( '\\r' | '\\n' ) )* )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:202:13: {...}? => ( SPACE )* '*' (~ ( '\\r' | '\\n' ) )*
            {
            if ( !(( getCharPositionInLine() == 0 )) ) {
                throw new FailedPredicateException(input, "COMMENT", " getCharPositionInLine() == 0 ");
            }

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:202:48: ( SPACE )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( (LA1_0=='\t'||LA1_0=='\f'||LA1_0==' ') ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:202:48: SPACE
            	    {
            	    mSPACE(); 


            	    }
            	    break;

            	default :
            	    break loop1;
                }
            } while (true);


            match('*'); 

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:202:59: (~ ( '\\r' | '\\n' ) )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( ((LA2_0 >= '\u0000' && LA2_0 <= '\t')||(LA2_0 >= '\u000B' && LA2_0 <= '\f')||(LA2_0 >= '\u000E' && LA2_0 <= '\uFFFF')) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            	    {
            	    if ( (input.LA(1) >= '\u0000' && input.LA(1) <= '\t')||(input.LA(1) >= '\u000B' && input.LA(1) <= '\f')||(input.LA(1) >= '\u000E' && input.LA(1) <= '\uFFFF') ) {
            	        input.consume();
            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "COMMENT"

    // $ANTLR start "HASHMARKER"
    public final void mHASHMARKER() throws RecognitionException {
        try {
            int _type = HASHMARKER;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:205:11: ({...}? ( WS )* '#' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:205:13: {...}? ( WS )* '#'
            {
            if ( !(( getCharPositionInLine() == 0 )) ) {
                throw new FailedPredicateException(input, "HASHMARKER", " getCharPositionInLine() == 0 ");
            }

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:205:47: ( WS )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( (LA3_0=='\t'||LA3_0=='\f'||LA3_0==' ') ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            	    {
            	    if ( input.LA(1)=='\t'||input.LA(1)=='\f'||input.LA(1)==' ' ) {
            	        input.consume();
            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop3;
                }
            } while (true);


            match('#'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "HASHMARKER"

    // $ANTLR start "ENDIF"
    public final void mENDIF() throws RecognitionException {
        try {
            int _type = ENDIF;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:208:6: ( E N D I F )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:208:8: E N D I F
            {
            mE(); 


            mN(); 


            mD(); 


            mI(); 


            mF(); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "ENDIF"

    // $ANTLR start "IF"
    public final void mIF() throws RecognitionException {
        try {
            int _type = IF;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:209:3: ( I F )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:209:5: I F
            {
            mI(); 


            mF(); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "IF"

    // $ANTLR start "ELSEIF"
    public final void mELSEIF() throws RecognitionException {
        try {
            int _type = ELSEIF;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:210:7: ( E L S E I F )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:210:9: E L S E I F
            {
            mE(); 


            mL(); 


            mS(); 


            mE(); 


            mI(); 


            mF(); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "ELSEIF"

    // $ANTLR start "ELSE"
    public final void mELSE() throws RecognitionException {
        try {
            int _type = ELSE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:211:5: ( E L S E )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:211:7: E L S E
            {
            mE(); 


            mL(); 


            mS(); 


            mE(); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "ELSE"

    // $ANTLR start "FOR"
    public final void mFOR() throws RecognitionException {
        try {
            int _type = FOR;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:212:4: ( F O R )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:212:6: F O R
            {
            mF(); 


            mO(); 


            mR(); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "FOR"

    // $ANTLR start "ENDFOR"
    public final void mENDFOR() throws RecognitionException {
        try {
            int _type = ENDFOR;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:213:7: ( E N D F O R )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:213:9: E N D F O R
            {
            mE(); 


            mN(); 


            mD(); 


            mF(); 


            mO(); 


            mR(); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "ENDFOR"

    // $ANTLR start "TESTCASE"
    public final void mTESTCASE() throws RecognitionException {
        try {
            int _type = TESTCASE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:214:9: ( T E S T C A S E )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:214:11: T E S T C A S E
            {
            mT(); 


            mE(); 


            mS(); 


            mT(); 


            mC(); 


            mA(); 


            mS(); 


            mE(); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "TESTCASE"

    // $ANTLR start "END"
    public final void mEND() throws RecognitionException {
        try {
            int _type = END;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:215:4: ( E N D )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:215:6: E N D
            {
            mE(); 


            mN(); 


            mD(); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "END"

    // $ANTLR start "NORMALWORD"
    public final void mNORMALWORD() throws RecognitionException {
        try {
            int _type = NORMALWORD;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:219:11: ( ( NORMALCHAR )+ )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:219:13: ( NORMALCHAR )+
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:219:13: ( NORMALCHAR )+
            int cnt4=0;
            loop4:
            do {
                int alt4=2;
                int LA4_0 = input.LA(1);

                if ( (LA4_0=='-'||(LA4_0 >= '0' && LA4_0 <= '9')||(LA4_0 >= 'A' && LA4_0 <= 'Z')||LA4_0=='_'||(LA4_0 >= 'a' && LA4_0 <= 'z')) ) {
                    alt4=1;
                }


                switch (alt4) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            	    {
            	    if ( input.LA(1)=='-'||(input.LA(1) >= '0' && input.LA(1) <= '9')||(input.LA(1) >= 'A' && input.LA(1) <= 'Z')||input.LA(1)=='_'||(input.LA(1) >= 'a' && input.LA(1) <= 'z') ) {
            	        input.consume();
            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt4 >= 1 ) break loop4;
                        EarlyExitException eee =
                            new EarlyExitException(4, input);
                        throw eee;
                }
                cnt4++;
            } while (true);


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "NORMALWORD"

    // $ANTLR start "BOOLOP"
    public final void mBOOLOP() throws RecognitionException {
        try {
            int _type = BOOLOP;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:222:7: ( '==' | '<' ( '=' )? | '>' ( '=' )? | '!=' )
            int alt7=4;
            switch ( input.LA(1) ) {
            case '=':
                {
                alt7=1;
                }
                break;
            case '<':
                {
                alt7=2;
                }
                break;
            case '>':
                {
                alt7=3;
                }
                break;
            case '!':
                {
                alt7=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("", 7, 0, input);

                throw nvae;

            }

            switch (alt7) {
                case 1 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:222:9: '=='
                    {
                    match("=="); 



                    }
                    break;
                case 2 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:223:7: '<' ( '=' )?
                    {
                    match('<'); 

                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:223:11: ( '=' )?
                    int alt5=2;
                    int LA5_0 = input.LA(1);

                    if ( (LA5_0=='=') ) {
                        alt5=1;
                    }
                    switch (alt5) {
                        case 1 :
                            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:223:12: '='
                            {
                            match('='); 

                            }
                            break;

                    }


                    }
                    break;
                case 3 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:224:7: '>' ( '=' )?
                    {
                    match('>'); 

                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:224:11: ( '=' )?
                    int alt6=2;
                    int LA6_0 = input.LA(1);

                    if ( (LA6_0=='=') ) {
                        alt6=1;
                    }
                    switch (alt6) {
                        case 1 :
                            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:224:12: '='
                            {
                            match('='); 

                            }
                            break;

                    }


                    }
                    break;
                case 4 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:225:7: '!='
                    {
                    match("!="); 



                    }
                    break;

            }
            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "BOOLOP"

    // $ANTLR start "VARIABLE"
    public final void mVARIABLE() throws RecognitionException {
        try {
            int _type = VARIABLE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:228:9: ( '$' ( NORMALCHAR )+ '$' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:228:11: '$' ( NORMALCHAR )+ '$'
            {
            match('$'); 

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:228:15: ( NORMALCHAR )+
            int cnt8=0;
            loop8:
            do {
                int alt8=2;
                int LA8_0 = input.LA(1);

                if ( (LA8_0=='-'||(LA8_0 >= '0' && LA8_0 <= '9')||(LA8_0 >= 'A' && LA8_0 <= 'Z')||LA8_0=='_'||(LA8_0 >= 'a' && LA8_0 <= 'z')) ) {
                    alt8=1;
                }


                switch (alt8) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            	    {
            	    if ( input.LA(1)=='-'||(input.LA(1) >= '0' && input.LA(1) <= '9')||(input.LA(1) >= 'A' && input.LA(1) <= 'Z')||input.LA(1)=='_'||(input.LA(1) >= 'a' && input.LA(1) <= 'z') ) {
            	        input.consume();
            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt8 >= 1 ) break loop8;
                        EarlyExitException eee =
                            new EarlyExitException(8, input);
                        throw eee;
                }
                cnt8++;
            } while (true);


            match('$'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "VARIABLE"

    // $ANTLR start "CONSTANT"
    public final void mCONSTANT() throws RecognitionException {
        try {
            int _type = CONSTANT;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:229:9: ( '%' ( NORMALCHAR )+ '%' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:229:11: '%' ( NORMALCHAR )+ '%'
            {
            match('%'); 

            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:229:15: ( NORMALCHAR )+
            int cnt9=0;
            loop9:
            do {
                int alt9=2;
                int LA9_0 = input.LA(1);

                if ( (LA9_0=='-'||(LA9_0 >= '0' && LA9_0 <= '9')||(LA9_0 >= 'A' && LA9_0 <= 'Z')||LA9_0=='_'||(LA9_0 >= 'a' && LA9_0 <= 'z')) ) {
                    alt9=1;
                }


                switch (alt9) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            	    {
            	    if ( input.LA(1)=='-'||(input.LA(1) >= '0' && input.LA(1) <= '9')||(input.LA(1) >= 'A' && input.LA(1) <= 'Z')||input.LA(1)=='_'||(input.LA(1) >= 'a' && input.LA(1) <= 'z') ) {
            	        input.consume();
            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt9 >= 1 ) break loop9;
                        EarlyExitException eee =
                            new EarlyExitException(9, input);
                        throw eee;
                }
                cnt9++;
            } while (true);


            match('%'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "CONSTANT"

    // $ANTLR start "QUOTE"
    public final void mQUOTE() throws RecognitionException {
        try {
            int _type = QUOTE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:232:6: ( '(' (~ ( NEWLINE | ')' ) )* ')' | '\"' (~ ( NEWLINE | '\"' ) )* '\"' )
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0=='(') ) {
                alt12=1;
            }
            else if ( (LA12_0=='\"') ) {
                alt12=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("", 12, 0, input);

                throw nvae;

            }
            switch (alt12) {
                case 1 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:232:8: '(' (~ ( NEWLINE | ')' ) )* ')'
                    {
                    match('('); 

                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:232:12: (~ ( NEWLINE | ')' ) )*
                    loop10:
                    do {
                        int alt10=2;
                        int LA10_0 = input.LA(1);

                        if ( ((LA10_0 >= '\u0000' && LA10_0 <= '\t')||(LA10_0 >= '\u000B' && LA10_0 <= '\f')||(LA10_0 >= '\u000E' && LA10_0 <= '(')||(LA10_0 >= '*' && LA10_0 <= '\uFFFF')) ) {
                            alt10=1;
                        }


                        switch (alt10) {
                    	case 1 :
                    	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
                    	    {
                    	    if ( (input.LA(1) >= '\u0000' && input.LA(1) <= '\t')||(input.LA(1) >= '\u000B' && input.LA(1) <= '\f')||(input.LA(1) >= '\u000E' && input.LA(1) <= '(')||(input.LA(1) >= '*' && input.LA(1) <= '\uFFFF') ) {
                    	        input.consume();
                    	    }
                    	    else {
                    	        MismatchedSetException mse = new MismatchedSetException(null,input);
                    	        recover(mse);
                    	        throw mse;
                    	    }


                    	    }
                    	    break;

                    	default :
                    	    break loop10;
                        }
                    } while (true);


                    match(')'); 

                    }
                    break;
                case 2 :
                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:233:9: '\"' (~ ( NEWLINE | '\"' ) )* '\"'
                    {
                    match('\"'); 

                    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:233:13: (~ ( NEWLINE | '\"' ) )*
                    loop11:
                    do {
                        int alt11=2;
                        int LA11_0 = input.LA(1);

                        if ( ((LA11_0 >= '\u0000' && LA11_0 <= '\t')||(LA11_0 >= '\u000B' && LA11_0 <= '\f')||(LA11_0 >= '\u000E' && LA11_0 <= '!')||(LA11_0 >= '#' && LA11_0 <= '\uFFFF')) ) {
                            alt11=1;
                        }


                        switch (alt11) {
                    	case 1 :
                    	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
                    	    {
                    	    if ( (input.LA(1) >= '\u0000' && input.LA(1) <= '\t')||(input.LA(1) >= '\u000B' && input.LA(1) <= '\f')||(input.LA(1) >= '\u000E' && input.LA(1) <= '!')||(input.LA(1) >= '#' && input.LA(1) <= '\uFFFF') ) {
                    	        input.consume();
                    	    }
                    	    else {
                    	        MismatchedSetException mse = new MismatchedSetException(null,input);
                    	        recover(mse);
                    	        throw mse;
                    	    }


                    	    }
                    	    break;

                    	default :
                    	    break loop11;
                        }
                    } while (true);


                    match('\"'); 

                    }
                    break;

            }
            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "QUOTE"

    // $ANTLR start "WORD"
    public final void mWORD() throws RecognitionException {
        try {
            int _type = WORD;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:237:5: ( ( MOSTCHAR )+ )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:237:7: ( MOSTCHAR )+
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:237:7: ( MOSTCHAR )+
            int cnt13=0;
            loop13:
            do {
                int alt13=2;
                int LA13_0 = input.LA(1);

                if ( ((LA13_0 >= '\u0000' && LA13_0 <= '\b')||LA13_0=='\u000B'||(LA13_0 >= '\u000E' && LA13_0 <= '\u001F')||LA13_0=='#'||(LA13_0 >= '&' && LA13_0 <= '+')||(LA13_0 >= '-' && LA13_0 <= ';')||(LA13_0 >= '?' && LA13_0 <= '\uFFFF')) ) {
                    alt13=1;
                }


                switch (alt13) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            	    {
            	    if ( (input.LA(1) >= '\u0000' && input.LA(1) <= '\b')||input.LA(1)=='\u000B'||(input.LA(1) >= '\u000E' && input.LA(1) <= '\u001F')||input.LA(1)=='#'||(input.LA(1) >= '&' && input.LA(1) <= '+')||(input.LA(1) >= '-' && input.LA(1) <= ';')||(input.LA(1) >= '?' && input.LA(1) <= '\uFFFF') ) {
            	        input.consume();
            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt13 >= 1 ) break loop13;
                        EarlyExitException eee =
                            new EarlyExitException(13, input);
                        throw eee;
                }
                cnt13++;
            } while (true);


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "WORD"

    // $ANTLR start "NL"
    public final void mNL() throws RecognitionException {
        try {
            int _type = NL;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:240:3: ( ( NEWLINE )+ )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:240:5: ( NEWLINE )+
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:240:5: ( NEWLINE )+
            int cnt14=0;
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( (LA14_0=='\n'||LA14_0=='\r') ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            	    {
            	    if ( input.LA(1)=='\n'||input.LA(1)=='\r' ) {
            	        input.consume();
            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt14 >= 1 ) break loop14;
                        EarlyExitException eee =
                            new EarlyExitException(14, input);
                        throw eee;
                }
                cnt14++;
            } while (true);


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "NL"

    // $ANTLR start "SPACE"
    public final void mSPACE() throws RecognitionException {
        try {
            int _type = SPACE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:243:6: ( ( WS )+ )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:243:8: ( WS )+
            {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:243:8: ( WS )+
            int cnt15=0;
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( (LA15_0=='\t'||LA15_0=='\f'||LA15_0==' ') ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            	    {
            	    if ( input.LA(1)=='\t'||input.LA(1)=='\f'||input.LA(1)==' ' ) {
            	        input.consume();
            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt15 >= 1 ) break loop15;
                        EarlyExitException eee =
                            new EarlyExitException(15, input);
                        throw eee;
                }
                cnt15++;
            } while (true);


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "SPACE"

    // $ANTLR start "NEWLINE"
    public final void mNEWLINE() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:246:17: ( '\\r' | '\\n' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='\n'||input.LA(1)=='\r' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "NEWLINE"

    // $ANTLR start "WS"
    public final void mWS() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:247:12: ( ' ' | '\\t' | '\\f' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='\t'||input.LA(1)=='\f'||input.LA(1)==' ' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "WS"

    // $ANTLR start "SPECIALCHAR"
    public final void mSPECIALCHAR() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:248:21: ( '$' | '%' | ',' | '\"' | '<' | '>' | '=' | '!' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( (input.LA(1) >= '!' && input.LA(1) <= '\"')||(input.LA(1) >= '$' && input.LA(1) <= '%')||input.LA(1)==','||(input.LA(1) >= '<' && input.LA(1) <= '>') ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "SPECIALCHAR"

    // $ANTLR start "MOSTCHAR"
    public final void mMOSTCHAR() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:249:18: (~ ( SPECIALCHAR | NEWLINE | WS ) )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( (input.LA(1) >= '\u0000' && input.LA(1) <= '\b')||input.LA(1)=='\u000B'||(input.LA(1) >= '\u000E' && input.LA(1) <= '\u001F')||input.LA(1)=='#'||(input.LA(1) >= '&' && input.LA(1) <= '+')||(input.LA(1) >= '-' && input.LA(1) <= ';')||(input.LA(1) >= '?' && input.LA(1) <= '\uFFFF') ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "MOSTCHAR"

    // $ANTLR start "NORMALCHAR"
    public final void mNORMALCHAR() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:250:20: ( ( 'a' .. 'z' | '0' .. '9' | 'A' .. 'Z' | '-' | '_' ) )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='-'||(input.LA(1) >= '0' && input.LA(1) <= '9')||(input.LA(1) >= 'A' && input.LA(1) <= 'Z')||input.LA(1)=='_'||(input.LA(1) >= 'a' && input.LA(1) <= 'z') ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "NORMALCHAR"

    // $ANTLR start "A"
    public final void mA() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:253:11: ( 'A' | 'a' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='A'||input.LA(1)=='a' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "A"

    // $ANTLR start "C"
    public final void mC() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:254:11: ( 'C' | 'c' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='C'||input.LA(1)=='c' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "C"

    // $ANTLR start "D"
    public final void mD() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:255:11: ( 'D' | 'd' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='D'||input.LA(1)=='d' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "D"

    // $ANTLR start "E"
    public final void mE() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:256:11: ( 'E' | 'e' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='E'||input.LA(1)=='e' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "E"

    // $ANTLR start "F"
    public final void mF() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:257:11: ( 'F' | 'f' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='F'||input.LA(1)=='f' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "F"

    // $ANTLR start "I"
    public final void mI() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:258:11: ( 'I' | 'i' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='I'||input.LA(1)=='i' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "I"

    // $ANTLR start "L"
    public final void mL() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:259:11: ( 'L' | 'l' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='L'||input.LA(1)=='l' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "L"

    // $ANTLR start "N"
    public final void mN() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:260:11: ( 'N' | 'n' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='N'||input.LA(1)=='n' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "N"

    // $ANTLR start "O"
    public final void mO() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:261:11: ( 'O' | 'o' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='O'||input.LA(1)=='o' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "O"

    // $ANTLR start "R"
    public final void mR() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:262:11: ( 'R' | 'r' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='R'||input.LA(1)=='r' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "R"

    // $ANTLR start "S"
    public final void mS() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:263:11: ( 'S' | 's' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='S'||input.LA(1)=='s' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "S"

    // $ANTLR start "T"
    public final void mT() throws RecognitionException {
        try {
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:264:11: ( 'T' | 't' )
            // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:
            {
            if ( input.LA(1)=='T'||input.LA(1)=='t' ) {
                input.consume();
            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;
            }


            }


        }
        finally {
        	// do for sure before leaving
        }
    }
    // $ANTLR end "T"

    public void mTokens() throws RecognitionException {
        // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:8: ( T__39 | T__40 | T__41 | T__42 | COMMENT | HASHMARKER | ENDIF | IF | ELSEIF | ELSE | FOR | ENDFOR | TESTCASE | END | NORMALWORD | BOOLOP | VARIABLE | CONSTANT | QUOTE | WORD | NL | SPACE )
        int alt16=22;
        alt16 = dfa16.predict(input);
        switch (alt16) {
            case 1 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:10: T__39
                {
                mT__39(); 


                }
                break;
            case 2 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:16: T__40
                {
                mT__40(); 


                }
                break;
            case 3 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:22: T__41
                {
                mT__41(); 


                }
                break;
            case 4 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:28: T__42
                {
                mT__42(); 


                }
                break;
            case 5 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:34: COMMENT
                {
                mCOMMENT(); 


                }
                break;
            case 6 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:42: HASHMARKER
                {
                mHASHMARKER(); 


                }
                break;
            case 7 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:53: ENDIF
                {
                mENDIF(); 


                }
                break;
            case 8 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:59: IF
                {
                mIF(); 


                }
                break;
            case 9 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:62: ELSEIF
                {
                mELSEIF(); 


                }
                break;
            case 10 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:69: ELSE
                {
                mELSE(); 


                }
                break;
            case 11 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:74: FOR
                {
                mFOR(); 


                }
                break;
            case 12 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:78: ENDFOR
                {
                mENDFOR(); 


                }
                break;
            case 13 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:85: TESTCASE
                {
                mTESTCASE(); 


                }
                break;
            case 14 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:94: END
                {
                mEND(); 


                }
                break;
            case 15 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:98: NORMALWORD
                {
                mNORMALWORD(); 


                }
                break;
            case 16 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:109: BOOLOP
                {
                mBOOLOP(); 


                }
                break;
            case 17 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:116: VARIABLE
                {
                mVARIABLE(); 


                }
                break;
            case 18 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:125: CONSTANT
                {
                mCONSTANT(); 


                }
                break;
            case 19 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:134: QUOTE
                {
                mQUOTE(); 


                }
                break;
            case 20 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:140: WORD
                {
                mWORD(); 


                }
                break;
            case 21 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:145: NL
                {
                mNL(); 


                }
                break;
            case 22 :
                // C:\\eclipseworkspace\\openTM2ScripterGUI\\src\\de\\ibm\\com\\opentm2scripteride\\parser\\Script.g:1:148: SPACE
                {
                mSPACE(); 


                }
                break;

        }

    }


    protected DFA16 dfa16 = new DFA16(this);
    static final String DFA16_eotS =
        "\1\uffff\1\21\1\23\1\25\1\uffff\1\27\1\33\1\34\5\35\1\uffff\1\17"+
        "\13\uffff\1\45\3\uffff\2\35\1\50\2\35\1\17\1\22\1\uffff\1\53\1\35"+
        "\1\uffff\1\57\1\35\1\uffff\2\35\1\63\1\uffff\1\35\1\66\1\35\1\uffff"+
        "\2\35\1\uffff\1\72\1\73\1\35\2\uffff\1\35\1\76\1\uffff";
    static final String DFA16_eofS =
        "\77\uffff";
    static final String DFA16_minS =
        "\2\0\2\55\1\uffff\1\11\7\0\1\uffff\1\0\13\uffff\3\0\1\uffff\12\0"+
        "\1\uffff\2\0\1\uffff\3\0\1\uffff\3\0\1\uffff\2\0\1\uffff\3\0\2\uffff"+
        "\2\0\1\uffff";
    static final String DFA16_maxS =
        "\2\uffff\2\172\1\uffff\1\52\7\uffff\1\uffff\1\uffff\13\uffff\1\uffff"+
        "\2\0\1\uffff\7\uffff\1\0\2\uffff\1\uffff\2\uffff\1\uffff\3\uffff"+
        "\1\uffff\3\uffff\1\uffff\2\uffff\1\uffff\3\uffff\2\uffff\2\uffff"+
        "\1\uffff";
    static final String DFA16_acceptS =
        "\4\uffff\1\4\10\uffff\1\20\1\uffff\1\24\1\25\1\1\1\23\1\2\1\21\1"+
        "\3\1\22\1\26\1\5\1\6\3\uffff\1\17\12\uffff\1\10\2\uffff\1\16\3\uffff"+
        "\1\13\3\uffff\1\12\2\uffff\1\7\3\uffff\1\14\1\11\2\uffff\1\15";
    static final String DFA16_specialS =
        "\1\16\1\6\3\uffff\1\23\1\13\1\10\1\32\1\20\1\45\1\27\1\14\1\uffff"+
        "\1\33\13\uffff\1\44\1\42\1\46\1\uffff\1\37\1\2\1\11\1\30\1\24\1"+
        "\3\1\34\1\43\1\0\1\26\1\uffff\1\35\1\4\1\uffff\1\21\1\36\1\17\1"+
        "\uffff\1\40\1\15\1\25\1\uffff\1\12\1\5\1\uffff\1\1\1\7\1\22\2\uffff"+
        "\1\31\1\41\1\uffff}>";
    static final String[] DFA16_transitionS = {
            "\11\17\1\5\1\20\1\17\1\5\1\20\22\17\1\5\1\15\1\1\1\7\1\2\1\3"+
            "\2\17\1\16\1\17\1\6\1\17\1\4\1\14\2\17\12\14\2\17\3\15\2\17"+
            "\4\14\1\10\1\12\2\14\1\11\12\14\1\13\6\14\4\17\1\14\1\17\4\14"+
            "\1\10\1\12\2\14\1\11\12\14\1\13\6\14\uff85\17",
            "\12\22\1\uffff\2\22\1\uffff\ufff2\22",
            "\1\24\2\uffff\12\24\7\uffff\32\24\4\uffff\1\24\1\uffff\32\24",
            "\1\26\2\uffff\12\26\7\uffff\32\26\4\uffff\1\26\1\uffff\32\26",
            "",
            "\1\5\2\uffff\1\5\23\uffff\1\5\2\uffff\1\31\6\uffff\1\30",
            "\11\32\1\30\1\uffff\1\32\1\30\1\uffff\22\32\3\30\1\32\2\30"+
            "\6\32\1\30\17\32\3\30\uffc1\32",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\17\17\3\uffff\uffc1\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\13\14\1\37\1\14\1"+
            "\36\14\14\4\17\1\14\1\17\13\14\1\37\1\14\1\36\14\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\5\14\1\40\24\14\4"+
            "\17\1\14\1\17\5\14\1\40\24\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\16\14\1\41\13\14"+
            "\4\17\1\14\1\17\16\14\1\41\13\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\4\14\1\42\25\14\4"+
            "\17\1\14\1\17\4\14\1\42\25\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\32\14\4\17\1\14\1"+
            "\17\32\14\uff85\17",
            "",
            "\11\43\1\22\1\uffff\1\43\1\22\1\uffff\22\43\3\22\1\43\2\22"+
            "\3\43\1\44\2\43\1\22\17\43\3\22\uffc1\43",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "\11\32\1\30\1\uffff\1\32\1\30\1\uffff\22\32\3\30\1\32\2\30"+
            "\6\32\1\30\17\32\3\30\uffc1\32",
            "\1\uffff",
            "\1\uffff",
            "",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\3\14\1\46\26\14\4"+
            "\17\1\14\1\17\3\14\1\46\26\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\22\14\1\47\7\14\4"+
            "\17\1\14\1\17\22\14\1\47\7\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\32\14\4\17\1\14\1"+
            "\17\32\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\21\14\1\51\10\14"+
            "\4\17\1\14\1\17\21\14\1\51\10\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\22\14\1\52\7\14\4"+
            "\17\1\14\1\17\22\14\1\52\7\14\uff85\17",
            "\11\43\1\22\1\uffff\1\43\1\22\1\uffff\22\43\3\22\1\43\2\22"+
            "\3\43\1\44\2\43\1\22\17\43\3\22\uffc1\43",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\17\17\3\uffff\uffc1\17",
            "\1\uffff",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\5\14\1\55\2\14\1"+
            "\54\21\14\4\17\1\14\1\17\5\14\1\55\2\14\1\54\21\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\4\14\1\56\25\14\4"+
            "\17\1\14\1\17\4\14\1\56\25\14\uff85\17",
            "",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\32\14\4\17\1\14\1"+
            "\17\32\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\23\14\1\60\6\14\4"+
            "\17\1\14\1\17\23\14\1\60\6\14\uff85\17",
            "",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\5\14\1\61\24\14\4"+
            "\17\1\14\1\17\5\14\1\61\24\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\16\14\1\62\13\14"+
            "\4\17\1\14\1\17\16\14\1\62\13\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\10\14\1\64\21\14"+
            "\4\17\1\14\1\17\10\14\1\64\21\14\uff85\17",
            "",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\2\14\1\65\27\14\4"+
            "\17\1\14\1\17\2\14\1\65\27\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\32\14\4\17\1\14\1"+
            "\17\32\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\21\14\1\67\10\14"+
            "\4\17\1\14\1\17\21\14\1\67\10\14\uff85\17",
            "",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\5\14\1\70\24\14\4"+
            "\17\1\14\1\17\5\14\1\70\24\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\1\71\31\14\4\17\1"+
            "\14\1\17\1\71\31\14\uff85\17",
            "",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\32\14\4\17\1\14\1"+
            "\17\32\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\32\14\4\17\1\14\1"+
            "\17\32\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\22\14\1\74\7\14\4"+
            "\17\1\14\1\17\22\14\1\74\7\14\uff85\17",
            "",
            "",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\4\14\1\75\25\14\4"+
            "\17\1\14\1\17\4\14\1\75\25\14\uff85\17",
            "\11\17\2\uffff\1\17\2\uffff\22\17\3\uffff\1\17\2\uffff\6\17"+
            "\1\uffff\1\14\2\17\12\14\2\17\3\uffff\2\17\32\14\4\17\1\14\1"+
            "\17\32\14\uff85\17",
            ""
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
            return "1:1: Tokens : ( T__39 | T__40 | T__41 | T__42 | COMMENT | HASHMARKER | ENDIF | IF | ELSEIF | ELSE | FOR | ENDFOR | TESTCASE | END | NORMALWORD | BOOLOP | VARIABLE | CONSTANT | QUOTE | WORD | NL | SPACE );";
        }
        public int specialStateTransition(int s, IntStream _input) throws NoViableAltException {
            IntStream input = _input;
        	int _s = s;
            switch ( s ) {
                    case 0 : 
                        int LA16_38 = input.LA(1);

                        s = -1;
                        if ( (LA16_38=='I'||LA16_38=='i') ) {s = 44;}

                        else if ( (LA16_38=='F'||LA16_38=='f') ) {s = 45;}

                        else if ( (LA16_38=='-'||(LA16_38 >= '0' && LA16_38 <= '9')||(LA16_38 >= 'A' && LA16_38 <= 'E')||(LA16_38 >= 'G' && LA16_38 <= 'H')||(LA16_38 >= 'J' && LA16_38 <= 'Z')||LA16_38=='_'||(LA16_38 >= 'a' && LA16_38 <= 'e')||(LA16_38 >= 'g' && LA16_38 <= 'h')||(LA16_38 >= 'j' && LA16_38 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_38 >= '\u0000' && LA16_38 <= '\b')||LA16_38=='\u000B'||(LA16_38 >= '\u000E' && LA16_38 <= '\u001F')||LA16_38=='#'||(LA16_38 >= '&' && LA16_38 <= '+')||(LA16_38 >= '.' && LA16_38 <= '/')||(LA16_38 >= ':' && LA16_38 <= ';')||(LA16_38 >= '?' && LA16_38 <= '@')||(LA16_38 >= '[' && LA16_38 <= '^')||LA16_38=='`'||(LA16_38 >= '{' && LA16_38 <= '\uFFFF')) ) {s = 15;}

                        else s = 43;

                        if ( s>=0 ) return s;
                        break;

                    case 1 : 
                        int LA16_55 = input.LA(1);

                        s = -1;
                        if ( (LA16_55=='-'||(LA16_55 >= '0' && LA16_55 <= '9')||(LA16_55 >= 'A' && LA16_55 <= 'Z')||LA16_55=='_'||(LA16_55 >= 'a' && LA16_55 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_55 >= '\u0000' && LA16_55 <= '\b')||LA16_55=='\u000B'||(LA16_55 >= '\u000E' && LA16_55 <= '\u001F')||LA16_55=='#'||(LA16_55 >= '&' && LA16_55 <= '+')||(LA16_55 >= '.' && LA16_55 <= '/')||(LA16_55 >= ':' && LA16_55 <= ';')||(LA16_55 >= '?' && LA16_55 <= '@')||(LA16_55 >= '[' && LA16_55 <= '^')||LA16_55=='`'||(LA16_55 >= '{' && LA16_55 <= '\uFFFF')) ) {s = 15;}

                        else s = 58;

                        if ( s>=0 ) return s;
                        break;

                    case 2 : 
                        int LA16_31 = input.LA(1);

                        s = -1;
                        if ( (LA16_31=='S'||LA16_31=='s') ) {s = 39;}

                        else if ( (LA16_31=='-'||(LA16_31 >= '0' && LA16_31 <= '9')||(LA16_31 >= 'A' && LA16_31 <= 'R')||(LA16_31 >= 'T' && LA16_31 <= 'Z')||LA16_31=='_'||(LA16_31 >= 'a' && LA16_31 <= 'r')||(LA16_31 >= 't' && LA16_31 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_31 >= '\u0000' && LA16_31 <= '\b')||LA16_31=='\u000B'||(LA16_31 >= '\u000E' && LA16_31 <= '\u001F')||LA16_31=='#'||(LA16_31 >= '&' && LA16_31 <= '+')||(LA16_31 >= '.' && LA16_31 <= '/')||(LA16_31 >= ':' && LA16_31 <= ';')||(LA16_31 >= '?' && LA16_31 <= '@')||(LA16_31 >= '[' && LA16_31 <= '^')||LA16_31=='`'||(LA16_31 >= '{' && LA16_31 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 3 : 
                        int LA16_35 = input.LA(1);

                        s = -1;
                        if ( (LA16_35==')') ) {s = 36;}

                        else if ( ((LA16_35 >= '\u0000' && LA16_35 <= '\b')||LA16_35=='\u000B'||(LA16_35 >= '\u000E' && LA16_35 <= '\u001F')||LA16_35=='#'||(LA16_35 >= '&' && LA16_35 <= '(')||(LA16_35 >= '*' && LA16_35 <= '+')||(LA16_35 >= '-' && LA16_35 <= ';')||(LA16_35 >= '?' && LA16_35 <= '\uFFFF')) ) {s = 35;}

                        else if ( (LA16_35=='\t'||LA16_35=='\f'||(LA16_35 >= ' ' && LA16_35 <= '\"')||(LA16_35 >= '$' && LA16_35 <= '%')||LA16_35==','||(LA16_35 >= '<' && LA16_35 <= '>')) ) {s = 18;}

                        else s = 15;

                        if ( s>=0 ) return s;
                        break;

                    case 4 : 
                        int LA16_42 = input.LA(1);

                        s = -1;
                        if ( (LA16_42=='T'||LA16_42=='t') ) {s = 48;}

                        else if ( (LA16_42=='-'||(LA16_42 >= '0' && LA16_42 <= '9')||(LA16_42 >= 'A' && LA16_42 <= 'S')||(LA16_42 >= 'U' && LA16_42 <= 'Z')||LA16_42=='_'||(LA16_42 >= 'a' && LA16_42 <= 's')||(LA16_42 >= 'u' && LA16_42 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_42 >= '\u0000' && LA16_42 <= '\b')||LA16_42=='\u000B'||(LA16_42 >= '\u000E' && LA16_42 <= '\u001F')||LA16_42=='#'||(LA16_42 >= '&' && LA16_42 <= '+')||(LA16_42 >= '.' && LA16_42 <= '/')||(LA16_42 >= ':' && LA16_42 <= ';')||(LA16_42 >= '?' && LA16_42 <= '@')||(LA16_42 >= '[' && LA16_42 <= '^')||LA16_42=='`'||(LA16_42 >= '{' && LA16_42 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 5 : 
                        int LA16_53 = input.LA(1);

                        s = -1;
                        if ( (LA16_53=='A'||LA16_53=='a') ) {s = 57;}

                        else if ( (LA16_53=='-'||(LA16_53 >= '0' && LA16_53 <= '9')||(LA16_53 >= 'B' && LA16_53 <= 'Z')||LA16_53=='_'||(LA16_53 >= 'b' && LA16_53 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_53 >= '\u0000' && LA16_53 <= '\b')||LA16_53=='\u000B'||(LA16_53 >= '\u000E' && LA16_53 <= '\u001F')||LA16_53=='#'||(LA16_53 >= '&' && LA16_53 <= '+')||(LA16_53 >= '.' && LA16_53 <= '/')||(LA16_53 >= ':' && LA16_53 <= ';')||(LA16_53 >= '?' && LA16_53 <= '@')||(LA16_53 >= '[' && LA16_53 <= '^')||LA16_53=='`'||(LA16_53 >= '{' && LA16_53 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 6 : 
                        int LA16_1 = input.LA(1);

                        s = -1;
                        if ( ((LA16_1 >= '\u0000' && LA16_1 <= '\t')||(LA16_1 >= '\u000B' && LA16_1 <= '\f')||(LA16_1 >= '\u000E' && LA16_1 <= '\uFFFF')) ) {s = 18;}

                        else s = 17;

                        if ( s>=0 ) return s;
                        break;

                    case 7 : 
                        int LA16_56 = input.LA(1);

                        s = -1;
                        if ( (LA16_56=='-'||(LA16_56 >= '0' && LA16_56 <= '9')||(LA16_56 >= 'A' && LA16_56 <= 'Z')||LA16_56=='_'||(LA16_56 >= 'a' && LA16_56 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_56 >= '\u0000' && LA16_56 <= '\b')||LA16_56=='\u000B'||(LA16_56 >= '\u000E' && LA16_56 <= '\u001F')||LA16_56=='#'||(LA16_56 >= '&' && LA16_56 <= '+')||(LA16_56 >= '.' && LA16_56 <= '/')||(LA16_56 >= ':' && LA16_56 <= ';')||(LA16_56 >= '?' && LA16_56 <= '@')||(LA16_56 >= '[' && LA16_56 <= '^')||LA16_56=='`'||(LA16_56 >= '{' && LA16_56 <= '\uFFFF')) ) {s = 15;}

                        else s = 59;

                        if ( s>=0 ) return s;
                        break;

                    case 8 : 
                        int LA16_7 = input.LA(1);

                        s = -1;
                        if ( ((LA16_7 >= '\u0000' && LA16_7 <= '\b')||LA16_7=='\u000B'||(LA16_7 >= '\u000E' && LA16_7 <= '\u001F')||LA16_7=='#'||(LA16_7 >= '&' && LA16_7 <= '+')||(LA16_7 >= '-' && LA16_7 <= ';')||(LA16_7 >= '?' && LA16_7 <= '\uFFFF')) ) {s = 15;}

                        else s = 28;

                        if ( s>=0 ) return s;
                        break;

                    case 9 : 
                        int LA16_32 = input.LA(1);

                        s = -1;
                        if ( (LA16_32=='-'||(LA16_32 >= '0' && LA16_32 <= '9')||(LA16_32 >= 'A' && LA16_32 <= 'Z')||LA16_32=='_'||(LA16_32 >= 'a' && LA16_32 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_32 >= '\u0000' && LA16_32 <= '\b')||LA16_32=='\u000B'||(LA16_32 >= '\u000E' && LA16_32 <= '\u001F')||LA16_32=='#'||(LA16_32 >= '&' && LA16_32 <= '+')||(LA16_32 >= '.' && LA16_32 <= '/')||(LA16_32 >= ':' && LA16_32 <= ';')||(LA16_32 >= '?' && LA16_32 <= '@')||(LA16_32 >= '[' && LA16_32 <= '^')||LA16_32=='`'||(LA16_32 >= '{' && LA16_32 <= '\uFFFF')) ) {s = 15;}

                        else s = 40;

                        if ( s>=0 ) return s;
                        break;

                    case 10 : 
                        int LA16_52 = input.LA(1);

                        s = -1;
                        if ( (LA16_52=='F'||LA16_52=='f') ) {s = 56;}

                        else if ( (LA16_52=='-'||(LA16_52 >= '0' && LA16_52 <= '9')||(LA16_52 >= 'A' && LA16_52 <= 'E')||(LA16_52 >= 'G' && LA16_52 <= 'Z')||LA16_52=='_'||(LA16_52 >= 'a' && LA16_52 <= 'e')||(LA16_52 >= 'g' && LA16_52 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_52 >= '\u0000' && LA16_52 <= '\b')||LA16_52=='\u000B'||(LA16_52 >= '\u000E' && LA16_52 <= '\u001F')||LA16_52=='#'||(LA16_52 >= '&' && LA16_52 <= '+')||(LA16_52 >= '.' && LA16_52 <= '/')||(LA16_52 >= ':' && LA16_52 <= ';')||(LA16_52 >= '?' && LA16_52 <= '@')||(LA16_52 >= '[' && LA16_52 <= '^')||LA16_52=='`'||(LA16_52 >= '{' && LA16_52 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 11 : 
                        int LA16_6 = input.LA(1);

                         
                        int index16_6 = input.index();
                        input.rewind();

                        s = -1;
                        if ( ((LA16_6 >= '\u0000' && LA16_6 <= '\b')||LA16_6=='\u000B'||(LA16_6 >= '\u000E' && LA16_6 <= '\u001F')||LA16_6=='#'||(LA16_6 >= '&' && LA16_6 <= '+')||(LA16_6 >= '-' && LA16_6 <= ';')||(LA16_6 >= '?' && LA16_6 <= '\uFFFF')) ) {s = 26;}

                        else if ( (LA16_6=='\t'||LA16_6=='\f'||(LA16_6 >= ' ' && LA16_6 <= '\"')||(LA16_6 >= '$' && LA16_6 <= '%')||LA16_6==','||(LA16_6 >= '<' && LA16_6 <= '>')) && (( getCharPositionInLine() == 0 ))) {s = 24;}

                        else s = 27;

                         
                        input.seek(index16_6);

                        if ( s>=0 ) return s;
                        break;

                    case 12 : 
                        int LA16_12 = input.LA(1);

                        s = -1;
                        if ( (LA16_12=='-'||(LA16_12 >= '0' && LA16_12 <= '9')||(LA16_12 >= 'A' && LA16_12 <= 'Z')||LA16_12=='_'||(LA16_12 >= 'a' && LA16_12 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_12 >= '\u0000' && LA16_12 <= '\b')||LA16_12=='\u000B'||(LA16_12 >= '\u000E' && LA16_12 <= '\u001F')||LA16_12=='#'||(LA16_12 >= '&' && LA16_12 <= '+')||(LA16_12 >= '.' && LA16_12 <= '/')||(LA16_12 >= ':' && LA16_12 <= ';')||(LA16_12 >= '?' && LA16_12 <= '@')||(LA16_12 >= '[' && LA16_12 <= '^')||LA16_12=='`'||(LA16_12 >= '{' && LA16_12 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 13 : 
                        int LA16_49 = input.LA(1);

                        s = -1;
                        if ( (LA16_49=='-'||(LA16_49 >= '0' && LA16_49 <= '9')||(LA16_49 >= 'A' && LA16_49 <= 'Z')||LA16_49=='_'||(LA16_49 >= 'a' && LA16_49 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_49 >= '\u0000' && LA16_49 <= '\b')||LA16_49=='\u000B'||(LA16_49 >= '\u000E' && LA16_49 <= '\u001F')||LA16_49=='#'||(LA16_49 >= '&' && LA16_49 <= '+')||(LA16_49 >= '.' && LA16_49 <= '/')||(LA16_49 >= ':' && LA16_49 <= ';')||(LA16_49 >= '?' && LA16_49 <= '@')||(LA16_49 >= '[' && LA16_49 <= '^')||LA16_49=='`'||(LA16_49 >= '{' && LA16_49 <= '\uFFFF')) ) {s = 15;}

                        else s = 54;

                        if ( s>=0 ) return s;
                        break;

                    case 14 : 
                        int LA16_0 = input.LA(1);

                        s = -1;
                        if ( (LA16_0=='\"') ) {s = 1;}

                        else if ( (LA16_0=='$') ) {s = 2;}

                        else if ( (LA16_0=='%') ) {s = 3;}

                        else if ( (LA16_0==',') ) {s = 4;}

                        else if ( (LA16_0=='\t'||LA16_0=='\f'||LA16_0==' ') ) {s = 5;}

                        else if ( (LA16_0=='*') ) {s = 6;}

                        else if ( (LA16_0=='#') ) {s = 7;}

                        else if ( (LA16_0=='E'||LA16_0=='e') ) {s = 8;}

                        else if ( (LA16_0=='I'||LA16_0=='i') ) {s = 9;}

                        else if ( (LA16_0=='F'||LA16_0=='f') ) {s = 10;}

                        else if ( (LA16_0=='T'||LA16_0=='t') ) {s = 11;}

                        else if ( (LA16_0=='-'||(LA16_0 >= '0' && LA16_0 <= '9')||(LA16_0 >= 'A' && LA16_0 <= 'D')||(LA16_0 >= 'G' && LA16_0 <= 'H')||(LA16_0 >= 'J' && LA16_0 <= 'S')||(LA16_0 >= 'U' && LA16_0 <= 'Z')||LA16_0=='_'||(LA16_0 >= 'a' && LA16_0 <= 'd')||(LA16_0 >= 'g' && LA16_0 <= 'h')||(LA16_0 >= 'j' && LA16_0 <= 's')||(LA16_0 >= 'u' && LA16_0 <= 'z')) ) {s = 12;}

                        else if ( (LA16_0=='!'||(LA16_0 >= '<' && LA16_0 <= '>')) ) {s = 13;}

                        else if ( (LA16_0=='(') ) {s = 14;}

                        else if ( ((LA16_0 >= '\u0000' && LA16_0 <= '\b')||LA16_0=='\u000B'||(LA16_0 >= '\u000E' && LA16_0 <= '\u001F')||(LA16_0 >= '&' && LA16_0 <= '\'')||LA16_0==')'||LA16_0=='+'||(LA16_0 >= '.' && LA16_0 <= '/')||(LA16_0 >= ':' && LA16_0 <= ';')||(LA16_0 >= '?' && LA16_0 <= '@')||(LA16_0 >= '[' && LA16_0 <= '^')||LA16_0=='`'||(LA16_0 >= '{' && LA16_0 <= '\uFFFF')) ) {s = 15;}

                        else if ( (LA16_0=='\n'||LA16_0=='\r') ) {s = 16;}

                        if ( s>=0 ) return s;
                        break;

                    case 15 : 
                        int LA16_46 = input.LA(1);

                        s = -1;
                        if ( (LA16_46=='I'||LA16_46=='i') ) {s = 52;}

                        else if ( (LA16_46=='-'||(LA16_46 >= '0' && LA16_46 <= '9')||(LA16_46 >= 'A' && LA16_46 <= 'H')||(LA16_46 >= 'J' && LA16_46 <= 'Z')||LA16_46=='_'||(LA16_46 >= 'a' && LA16_46 <= 'h')||(LA16_46 >= 'j' && LA16_46 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_46 >= '\u0000' && LA16_46 <= '\b')||LA16_46=='\u000B'||(LA16_46 >= '\u000E' && LA16_46 <= '\u001F')||LA16_46=='#'||(LA16_46 >= '&' && LA16_46 <= '+')||(LA16_46 >= '.' && LA16_46 <= '/')||(LA16_46 >= ':' && LA16_46 <= ';')||(LA16_46 >= '?' && LA16_46 <= '@')||(LA16_46 >= '[' && LA16_46 <= '^')||LA16_46=='`'||(LA16_46 >= '{' && LA16_46 <= '\uFFFF')) ) {s = 15;}

                        else s = 51;

                        if ( s>=0 ) return s;
                        break;

                    case 16 : 
                        int LA16_9 = input.LA(1);

                        s = -1;
                        if ( (LA16_9=='F'||LA16_9=='f') ) {s = 32;}

                        else if ( (LA16_9=='-'||(LA16_9 >= '0' && LA16_9 <= '9')||(LA16_9 >= 'A' && LA16_9 <= 'E')||(LA16_9 >= 'G' && LA16_9 <= 'Z')||LA16_9=='_'||(LA16_9 >= 'a' && LA16_9 <= 'e')||(LA16_9 >= 'g' && LA16_9 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_9 >= '\u0000' && LA16_9 <= '\b')||LA16_9=='\u000B'||(LA16_9 >= '\u000E' && LA16_9 <= '\u001F')||LA16_9=='#'||(LA16_9 >= '&' && LA16_9 <= '+')||(LA16_9 >= '.' && LA16_9 <= '/')||(LA16_9 >= ':' && LA16_9 <= ';')||(LA16_9 >= '?' && LA16_9 <= '@')||(LA16_9 >= '[' && LA16_9 <= '^')||LA16_9=='`'||(LA16_9 >= '{' && LA16_9 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 17 : 
                        int LA16_44 = input.LA(1);

                        s = -1;
                        if ( (LA16_44=='F'||LA16_44=='f') ) {s = 49;}

                        else if ( (LA16_44=='-'||(LA16_44 >= '0' && LA16_44 <= '9')||(LA16_44 >= 'A' && LA16_44 <= 'E')||(LA16_44 >= 'G' && LA16_44 <= 'Z')||LA16_44=='_'||(LA16_44 >= 'a' && LA16_44 <= 'e')||(LA16_44 >= 'g' && LA16_44 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_44 >= '\u0000' && LA16_44 <= '\b')||LA16_44=='\u000B'||(LA16_44 >= '\u000E' && LA16_44 <= '\u001F')||LA16_44=='#'||(LA16_44 >= '&' && LA16_44 <= '+')||(LA16_44 >= '.' && LA16_44 <= '/')||(LA16_44 >= ':' && LA16_44 <= ';')||(LA16_44 >= '?' && LA16_44 <= '@')||(LA16_44 >= '[' && LA16_44 <= '^')||LA16_44=='`'||(LA16_44 >= '{' && LA16_44 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 18 : 
                        int LA16_57 = input.LA(1);

                        s = -1;
                        if ( (LA16_57=='S'||LA16_57=='s') ) {s = 60;}

                        else if ( (LA16_57=='-'||(LA16_57 >= '0' && LA16_57 <= '9')||(LA16_57 >= 'A' && LA16_57 <= 'R')||(LA16_57 >= 'T' && LA16_57 <= 'Z')||LA16_57=='_'||(LA16_57 >= 'a' && LA16_57 <= 'r')||(LA16_57 >= 't' && LA16_57 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_57 >= '\u0000' && LA16_57 <= '\b')||LA16_57=='\u000B'||(LA16_57 >= '\u000E' && LA16_57 <= '\u001F')||LA16_57=='#'||(LA16_57 >= '&' && LA16_57 <= '+')||(LA16_57 >= '.' && LA16_57 <= '/')||(LA16_57 >= ':' && LA16_57 <= ';')||(LA16_57 >= '?' && LA16_57 <= '@')||(LA16_57 >= '[' && LA16_57 <= '^')||LA16_57=='`'||(LA16_57 >= '{' && LA16_57 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 19 : 
                        int LA16_5 = input.LA(1);

                         
                        int index16_5 = input.index();
                        input.rewind();

                        s = -1;
                        if ( (LA16_5=='*') && (( getCharPositionInLine() == 0 ))) {s = 24;}

                        else if ( (LA16_5=='\t'||LA16_5=='\f'||LA16_5==' ') ) {s = 5;}

                        else if ( (LA16_5=='#') ) {s = 25;}

                        else s = 23;

                         
                        input.seek(index16_5);

                        if ( s>=0 ) return s;
                        break;

                    case 20 : 
                        int LA16_34 = input.LA(1);

                        s = -1;
                        if ( (LA16_34=='S'||LA16_34=='s') ) {s = 42;}

                        else if ( (LA16_34=='-'||(LA16_34 >= '0' && LA16_34 <= '9')||(LA16_34 >= 'A' && LA16_34 <= 'R')||(LA16_34 >= 'T' && LA16_34 <= 'Z')||LA16_34=='_'||(LA16_34 >= 'a' && LA16_34 <= 'r')||(LA16_34 >= 't' && LA16_34 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_34 >= '\u0000' && LA16_34 <= '\b')||LA16_34=='\u000B'||(LA16_34 >= '\u000E' && LA16_34 <= '\u001F')||LA16_34=='#'||(LA16_34 >= '&' && LA16_34 <= '+')||(LA16_34 >= '.' && LA16_34 <= '/')||(LA16_34 >= ':' && LA16_34 <= ';')||(LA16_34 >= '?' && LA16_34 <= '@')||(LA16_34 >= '[' && LA16_34 <= '^')||LA16_34=='`'||(LA16_34 >= '{' && LA16_34 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 21 : 
                        int LA16_50 = input.LA(1);

                        s = -1;
                        if ( (LA16_50=='R'||LA16_50=='r') ) {s = 55;}

                        else if ( (LA16_50=='-'||(LA16_50 >= '0' && LA16_50 <= '9')||(LA16_50 >= 'A' && LA16_50 <= 'Q')||(LA16_50 >= 'S' && LA16_50 <= 'Z')||LA16_50=='_'||(LA16_50 >= 'a' && LA16_50 <= 'q')||(LA16_50 >= 's' && LA16_50 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_50 >= '\u0000' && LA16_50 <= '\b')||LA16_50=='\u000B'||(LA16_50 >= '\u000E' && LA16_50 <= '\u001F')||LA16_50=='#'||(LA16_50 >= '&' && LA16_50 <= '+')||(LA16_50 >= '.' && LA16_50 <= '/')||(LA16_50 >= ':' && LA16_50 <= ';')||(LA16_50 >= '?' && LA16_50 <= '@')||(LA16_50 >= '[' && LA16_50 <= '^')||LA16_50=='`'||(LA16_50 >= '{' && LA16_50 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 22 : 
                        int LA16_39 = input.LA(1);

                        s = -1;
                        if ( (LA16_39=='E'||LA16_39=='e') ) {s = 46;}

                        else if ( (LA16_39=='-'||(LA16_39 >= '0' && LA16_39 <= '9')||(LA16_39 >= 'A' && LA16_39 <= 'D')||(LA16_39 >= 'F' && LA16_39 <= 'Z')||LA16_39=='_'||(LA16_39 >= 'a' && LA16_39 <= 'd')||(LA16_39 >= 'f' && LA16_39 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_39 >= '\u0000' && LA16_39 <= '\b')||LA16_39=='\u000B'||(LA16_39 >= '\u000E' && LA16_39 <= '\u001F')||LA16_39=='#'||(LA16_39 >= '&' && LA16_39 <= '+')||(LA16_39 >= '.' && LA16_39 <= '/')||(LA16_39 >= ':' && LA16_39 <= ';')||(LA16_39 >= '?' && LA16_39 <= '@')||(LA16_39 >= '[' && LA16_39 <= '^')||LA16_39=='`'||(LA16_39 >= '{' && LA16_39 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 23 : 
                        int LA16_11 = input.LA(1);

                        s = -1;
                        if ( (LA16_11=='E'||LA16_11=='e') ) {s = 34;}

                        else if ( (LA16_11=='-'||(LA16_11 >= '0' && LA16_11 <= '9')||(LA16_11 >= 'A' && LA16_11 <= 'D')||(LA16_11 >= 'F' && LA16_11 <= 'Z')||LA16_11=='_'||(LA16_11 >= 'a' && LA16_11 <= 'd')||(LA16_11 >= 'f' && LA16_11 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_11 >= '\u0000' && LA16_11 <= '\b')||LA16_11=='\u000B'||(LA16_11 >= '\u000E' && LA16_11 <= '\u001F')||LA16_11=='#'||(LA16_11 >= '&' && LA16_11 <= '+')||(LA16_11 >= '.' && LA16_11 <= '/')||(LA16_11 >= ':' && LA16_11 <= ';')||(LA16_11 >= '?' && LA16_11 <= '@')||(LA16_11 >= '[' && LA16_11 <= '^')||LA16_11=='`'||(LA16_11 >= '{' && LA16_11 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 24 : 
                        int LA16_33 = input.LA(1);

                        s = -1;
                        if ( (LA16_33=='R'||LA16_33=='r') ) {s = 41;}

                        else if ( (LA16_33=='-'||(LA16_33 >= '0' && LA16_33 <= '9')||(LA16_33 >= 'A' && LA16_33 <= 'Q')||(LA16_33 >= 'S' && LA16_33 <= 'Z')||LA16_33=='_'||(LA16_33 >= 'a' && LA16_33 <= 'q')||(LA16_33 >= 's' && LA16_33 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_33 >= '\u0000' && LA16_33 <= '\b')||LA16_33=='\u000B'||(LA16_33 >= '\u000E' && LA16_33 <= '\u001F')||LA16_33=='#'||(LA16_33 >= '&' && LA16_33 <= '+')||(LA16_33 >= '.' && LA16_33 <= '/')||(LA16_33 >= ':' && LA16_33 <= ';')||(LA16_33 >= '?' && LA16_33 <= '@')||(LA16_33 >= '[' && LA16_33 <= '^')||LA16_33=='`'||(LA16_33 >= '{' && LA16_33 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 25 : 
                        int LA16_60 = input.LA(1);

                        s = -1;
                        if ( (LA16_60=='E'||LA16_60=='e') ) {s = 61;}

                        else if ( (LA16_60=='-'||(LA16_60 >= '0' && LA16_60 <= '9')||(LA16_60 >= 'A' && LA16_60 <= 'D')||(LA16_60 >= 'F' && LA16_60 <= 'Z')||LA16_60=='_'||(LA16_60 >= 'a' && LA16_60 <= 'd')||(LA16_60 >= 'f' && LA16_60 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_60 >= '\u0000' && LA16_60 <= '\b')||LA16_60=='\u000B'||(LA16_60 >= '\u000E' && LA16_60 <= '\u001F')||LA16_60=='#'||(LA16_60 >= '&' && LA16_60 <= '+')||(LA16_60 >= '.' && LA16_60 <= '/')||(LA16_60 >= ':' && LA16_60 <= ';')||(LA16_60 >= '?' && LA16_60 <= '@')||(LA16_60 >= '[' && LA16_60 <= '^')||LA16_60=='`'||(LA16_60 >= '{' && LA16_60 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 26 : 
                        int LA16_8 = input.LA(1);

                        s = -1;
                        if ( (LA16_8=='N'||LA16_8=='n') ) {s = 30;}

                        else if ( (LA16_8=='L'||LA16_8=='l') ) {s = 31;}

                        else if ( (LA16_8=='-'||(LA16_8 >= '0' && LA16_8 <= '9')||(LA16_8 >= 'A' && LA16_8 <= 'K')||LA16_8=='M'||(LA16_8 >= 'O' && LA16_8 <= 'Z')||LA16_8=='_'||(LA16_8 >= 'a' && LA16_8 <= 'k')||LA16_8=='m'||(LA16_8 >= 'o' && LA16_8 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_8 >= '\u0000' && LA16_8 <= '\b')||LA16_8=='\u000B'||(LA16_8 >= '\u000E' && LA16_8 <= '\u001F')||LA16_8=='#'||(LA16_8 >= '&' && LA16_8 <= '+')||(LA16_8 >= '.' && LA16_8 <= '/')||(LA16_8 >= ':' && LA16_8 <= ';')||(LA16_8 >= '?' && LA16_8 <= '@')||(LA16_8 >= '[' && LA16_8 <= '^')||LA16_8=='`'||(LA16_8 >= '{' && LA16_8 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 27 : 
                        int LA16_14 = input.LA(1);

                        s = -1;
                        if ( ((LA16_14 >= '\u0000' && LA16_14 <= '\b')||LA16_14=='\u000B'||(LA16_14 >= '\u000E' && LA16_14 <= '\u001F')||LA16_14=='#'||(LA16_14 >= '&' && LA16_14 <= '(')||(LA16_14 >= '*' && LA16_14 <= '+')||(LA16_14 >= '-' && LA16_14 <= ';')||(LA16_14 >= '?' && LA16_14 <= '\uFFFF')) ) {s = 35;}

                        else if ( (LA16_14==')') ) {s = 36;}

                        else if ( (LA16_14=='\t'||LA16_14=='\f'||(LA16_14 >= ' ' && LA16_14 <= '\"')||(LA16_14 >= '$' && LA16_14 <= '%')||LA16_14==','||(LA16_14 >= '<' && LA16_14 <= '>')) ) {s = 18;}

                        else s = 15;

                        if ( s>=0 ) return s;
                        break;

                    case 28 : 
                        int LA16_36 = input.LA(1);

                        s = -1;
                        if ( ((LA16_36 >= '\u0000' && LA16_36 <= '\b')||LA16_36=='\u000B'||(LA16_36 >= '\u000E' && LA16_36 <= '\u001F')||LA16_36=='#'||(LA16_36 >= '&' && LA16_36 <= '+')||(LA16_36 >= '-' && LA16_36 <= ';')||(LA16_36 >= '?' && LA16_36 <= '\uFFFF')) ) {s = 15;}

                        else s = 18;

                        if ( s>=0 ) return s;
                        break;

                    case 29 : 
                        int LA16_41 = input.LA(1);

                        s = -1;
                        if ( (LA16_41=='-'||(LA16_41 >= '0' && LA16_41 <= '9')||(LA16_41 >= 'A' && LA16_41 <= 'Z')||LA16_41=='_'||(LA16_41 >= 'a' && LA16_41 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_41 >= '\u0000' && LA16_41 <= '\b')||LA16_41=='\u000B'||(LA16_41 >= '\u000E' && LA16_41 <= '\u001F')||LA16_41=='#'||(LA16_41 >= '&' && LA16_41 <= '+')||(LA16_41 >= '.' && LA16_41 <= '/')||(LA16_41 >= ':' && LA16_41 <= ';')||(LA16_41 >= '?' && LA16_41 <= '@')||(LA16_41 >= '[' && LA16_41 <= '^')||LA16_41=='`'||(LA16_41 >= '{' && LA16_41 <= '\uFFFF')) ) {s = 15;}

                        else s = 47;

                        if ( s>=0 ) return s;
                        break;

                    case 30 : 
                        int LA16_45 = input.LA(1);

                        s = -1;
                        if ( (LA16_45=='O'||LA16_45=='o') ) {s = 50;}

                        else if ( (LA16_45=='-'||(LA16_45 >= '0' && LA16_45 <= '9')||(LA16_45 >= 'A' && LA16_45 <= 'N')||(LA16_45 >= 'P' && LA16_45 <= 'Z')||LA16_45=='_'||(LA16_45 >= 'a' && LA16_45 <= 'n')||(LA16_45 >= 'p' && LA16_45 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_45 >= '\u0000' && LA16_45 <= '\b')||LA16_45=='\u000B'||(LA16_45 >= '\u000E' && LA16_45 <= '\u001F')||LA16_45=='#'||(LA16_45 >= '&' && LA16_45 <= '+')||(LA16_45 >= '.' && LA16_45 <= '/')||(LA16_45 >= ':' && LA16_45 <= ';')||(LA16_45 >= '?' && LA16_45 <= '@')||(LA16_45 >= '[' && LA16_45 <= '^')||LA16_45=='`'||(LA16_45 >= '{' && LA16_45 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 31 : 
                        int LA16_30 = input.LA(1);

                        s = -1;
                        if ( (LA16_30=='D'||LA16_30=='d') ) {s = 38;}

                        else if ( (LA16_30=='-'||(LA16_30 >= '0' && LA16_30 <= '9')||(LA16_30 >= 'A' && LA16_30 <= 'C')||(LA16_30 >= 'E' && LA16_30 <= 'Z')||LA16_30=='_'||(LA16_30 >= 'a' && LA16_30 <= 'c')||(LA16_30 >= 'e' && LA16_30 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_30 >= '\u0000' && LA16_30 <= '\b')||LA16_30=='\u000B'||(LA16_30 >= '\u000E' && LA16_30 <= '\u001F')||LA16_30=='#'||(LA16_30 >= '&' && LA16_30 <= '+')||(LA16_30 >= '.' && LA16_30 <= '/')||(LA16_30 >= ':' && LA16_30 <= ';')||(LA16_30 >= '?' && LA16_30 <= '@')||(LA16_30 >= '[' && LA16_30 <= '^')||LA16_30=='`'||(LA16_30 >= '{' && LA16_30 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 32 : 
                        int LA16_48 = input.LA(1);

                        s = -1;
                        if ( (LA16_48=='C'||LA16_48=='c') ) {s = 53;}

                        else if ( (LA16_48=='-'||(LA16_48 >= '0' && LA16_48 <= '9')||(LA16_48 >= 'A' && LA16_48 <= 'B')||(LA16_48 >= 'D' && LA16_48 <= 'Z')||LA16_48=='_'||(LA16_48 >= 'a' && LA16_48 <= 'b')||(LA16_48 >= 'd' && LA16_48 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_48 >= '\u0000' && LA16_48 <= '\b')||LA16_48=='\u000B'||(LA16_48 >= '\u000E' && LA16_48 <= '\u001F')||LA16_48=='#'||(LA16_48 >= '&' && LA16_48 <= '+')||(LA16_48 >= '.' && LA16_48 <= '/')||(LA16_48 >= ':' && LA16_48 <= ';')||(LA16_48 >= '?' && LA16_48 <= '@')||(LA16_48 >= '[' && LA16_48 <= '^')||LA16_48=='`'||(LA16_48 >= '{' && LA16_48 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 33 : 
                        int LA16_61 = input.LA(1);

                        s = -1;
                        if ( (LA16_61=='-'||(LA16_61 >= '0' && LA16_61 <= '9')||(LA16_61 >= 'A' && LA16_61 <= 'Z')||LA16_61=='_'||(LA16_61 >= 'a' && LA16_61 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_61 >= '\u0000' && LA16_61 <= '\b')||LA16_61=='\u000B'||(LA16_61 >= '\u000E' && LA16_61 <= '\u001F')||LA16_61=='#'||(LA16_61 >= '&' && LA16_61 <= '+')||(LA16_61 >= '.' && LA16_61 <= '/')||(LA16_61 >= ':' && LA16_61 <= ';')||(LA16_61 >= '?' && LA16_61 <= '@')||(LA16_61 >= '[' && LA16_61 <= '^')||LA16_61=='`'||(LA16_61 >= '{' && LA16_61 <= '\uFFFF')) ) {s = 15;}

                        else s = 62;

                        if ( s>=0 ) return s;
                        break;

                    case 34 : 
                        int LA16_27 = input.LA(1);

                         
                        int index16_27 = input.index();
                        input.rewind();

                        s = -1;
                        if ( (( getCharPositionInLine() == 0 )) ) {s = 24;}

                        else if ( (true) ) {s = 15;}

                         
                        input.seek(index16_27);

                        if ( s>=0 ) return s;
                        break;

                    case 35 : 
                        int LA16_37 = input.LA(1);

                         
                        int index16_37 = input.index();
                        input.rewind();

                        s = -1;
                        if ( (( getCharPositionInLine() == 0 )) ) {s = 24;}

                        else if ( (true) ) {s = 15;}

                         
                        input.seek(index16_37);

                        if ( s>=0 ) return s;
                        break;

                    case 36 : 
                        int LA16_26 = input.LA(1);

                         
                        int index16_26 = input.index();
                        input.rewind();

                        s = -1;
                        if ( ((LA16_26 >= '\u0000' && LA16_26 <= '\b')||LA16_26=='\u000B'||(LA16_26 >= '\u000E' && LA16_26 <= '\u001F')||LA16_26=='#'||(LA16_26 >= '&' && LA16_26 <= '+')||(LA16_26 >= '-' && LA16_26 <= ';')||(LA16_26 >= '?' && LA16_26 <= '\uFFFF')) ) {s = 26;}

                        else if ( (LA16_26=='\t'||LA16_26=='\f'||(LA16_26 >= ' ' && LA16_26 <= '\"')||(LA16_26 >= '$' && LA16_26 <= '%')||LA16_26==','||(LA16_26 >= '<' && LA16_26 <= '>')) && (( getCharPositionInLine() == 0 ))) {s = 24;}

                        else s = 37;

                         
                        input.seek(index16_26);

                        if ( s>=0 ) return s;
                        break;

                    case 37 : 
                        int LA16_10 = input.LA(1);

                        s = -1;
                        if ( (LA16_10=='O'||LA16_10=='o') ) {s = 33;}

                        else if ( (LA16_10=='-'||(LA16_10 >= '0' && LA16_10 <= '9')||(LA16_10 >= 'A' && LA16_10 <= 'N')||(LA16_10 >= 'P' && LA16_10 <= 'Z')||LA16_10=='_'||(LA16_10 >= 'a' && LA16_10 <= 'n')||(LA16_10 >= 'p' && LA16_10 <= 'z')) ) {s = 12;}

                        else if ( ((LA16_10 >= '\u0000' && LA16_10 <= '\b')||LA16_10=='\u000B'||(LA16_10 >= '\u000E' && LA16_10 <= '\u001F')||LA16_10=='#'||(LA16_10 >= '&' && LA16_10 <= '+')||(LA16_10 >= '.' && LA16_10 <= '/')||(LA16_10 >= ':' && LA16_10 <= ';')||(LA16_10 >= '?' && LA16_10 <= '@')||(LA16_10 >= '[' && LA16_10 <= '^')||LA16_10=='`'||(LA16_10 >= '{' && LA16_10 <= '\uFFFF')) ) {s = 15;}

                        else s = 29;

                        if ( s>=0 ) return s;
                        break;

                    case 38 : 
                        int LA16_28 = input.LA(1);

                         
                        int index16_28 = input.index();
                        input.rewind();

                        s = -1;
                        if ( (( getCharPositionInLine() == 0 )) ) {s = 25;}

                        else if ( (true) ) {s = 15;}

                         
                        input.seek(index16_28);

                        if ( s>=0 ) return s;
                        break;
            }
            NoViableAltException nvae =
                new NoViableAltException(getDescription(), 16, _s, input);
            error(nvae);
            throw nvae;
        }

    }
 

}