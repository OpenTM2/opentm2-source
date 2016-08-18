/*
 * Created on 22.09.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import java.util.MissingResourceException;
import java.util.ResourceBundle;

/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class Messages
{
  private static final String         BUNDLE_NAME     = "matchlistviewer.messages";              //$NON-NLS-1$

  private static final ResourceBundle RESOURCE_BUNDLE = ResourceBundle.getBundle( BUNDLE_NAME );

  private Messages()
  {
  }

  public static String getString( String key )
  {
    // TODO Auto-generated method stub
    try
    {
      return RESOURCE_BUNDLE.getString( key );
    }
    catch ( MissingResourceException e )
    {
      return '!' + key + '!';
    }
  }
}
