/*
 * Created on 20.09.2005
 *
 *	Copyright (C) 1990-2015, International Business Machines
 *	Corporation and others. All rights reserved
 */
package transmemoptimizer;

import java.awt.BorderLayout;
import java.awt.FlowLayout;

import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.border.BevelBorder;
import javax.swing.border.EtchedBorder;
import javax.swing.border.SoftBevelBorder;

/**
 * @author Gerhard
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
/** Class to create a status panel. */
class StatusPanel extends JPanel
{
    JTextField statusMessage;
    JComponent secondBox;
    JComponent thirdBox;

    public JTextField getStatusMessage() { return statusMessage; }
    public JComponent getSecondBox() { return secondBox; }
    public JComponent getThirdBox() { return thirdBox; }

    public void setText(String sText)
    {
        statusMessage.setText(sText);
    }

    public StatusPanel()
    {
        this(null,null);
    }

    public StatusPanel(JComponent secondBox)
    {
        this(secondBox, null);
    }

    public StatusPanel(JComponent second, JComponent third)
    {
        this.secondBox = second;
        this.thirdBox = third;
        this.setLayout(new BorderLayout(2,2));
        JPanel eastPanel = new JPanel(new FlowLayout(FlowLayout.LEFT,2,0));
        this.setBorder(new SoftBevelBorder(BevelBorder.RAISED));
        statusMessage = new JTextField();
        statusMessage.setEditable(false);
        statusMessage.setBorder(new EtchedBorder(EtchedBorder.LOWERED));
        this.add("Center", statusMessage);
        if (secondBox == null)
            secondBox = new SpaceCanvas();
        secondBox.setBorder(new EtchedBorder(EtchedBorder.LOWERED));
        eastPanel.add(secondBox);

        if (thirdBox == null)
            thirdBox = new SpaceCanvas();
        thirdBox.setBorder(new EtchedBorder(EtchedBorder.LOWERED));
        eastPanel.add(thirdBox);
        this.add("East", eastPanel);
    }
}
