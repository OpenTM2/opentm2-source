/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/

// define an xml token class will look at a string
// and search for tokens

enum
{
	lcomment , rcomment, ltag, rtag, lcdata, entity, rcdata,
	doctag . eof
} ;

typedef token short ;

class xmltoken
{
	public :
		token gettoken(IString s);
	private :
		int index ;
		IString s;

		// set index to next non-space or non dbcs character
		// a do this since im not sure if the word functions
		// can point to a dbcs char or not
		int setnext()
		{
			int k = index ;
			char c ;
			while ( k < s.length() ) 
			{
				c = s[k] ;
            if ( IsDBCS(c) ) 
				{
					k+=2;
					continue ;
				}
				if ( isspace(c) )
				{
					k++ ;
					continue ;
				} /* else */
				// we must of found the character
				break ;
			} // while
			if ( k > s.length() ) return EOL ;
			return TRUE ;
		};
} ;

token xmltoken::gettoken(IString s)
{
	while ( gettoken() == eol ) 
	{
		if ( getline() == eof ) 
		{

		} /* if */

	} /* while */
} // gettoken
