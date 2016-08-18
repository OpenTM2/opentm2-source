/*
 * Created on 20.09.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import java.awt.Dimension;
import javax.swing.JComponent;

/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
/** A component that takes up space to create empty boxes. */
class SpaceCanvas extends JComponent
{
    public Dimension getMinimumSize()
    {
        Dimension d = getParent().getSize();
        int w = 60, h = 20;
        if (d.height > h)
            h = d.height - 1;

        return new Dimension(w,h);
    }

    public Dimension getPreferredSize() { return getMinimumSize(); }
}