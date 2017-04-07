/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom;

import de.ibm.com.opentm2scripteride.MainApp;
import de.ibm.com.opentm2scripteride.gui.custom.editor.JEditTextArea;
import de.ibm.com.opentm2scripteride.models.CommandBody;
import de.ibm.com.opentm2scripteride.models.CommandModel;
import de.ibm.com.opentm2scripteride.models.ParameterFrame;
import java.util.List;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JTextField;
import javax.swing.SwingConstants;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.text.Element;
import net.miginfocom.swing.MigLayout;
import java.awt.BorderLayout;
import java.awt.Dialog;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;

/**
 * A dialog that represents one command of a script.
 * It shows which parameters the command has and lets the user check if there parameters a formed correctly.
 * After the user clicked on okay the command will be written back to both the CommandModel and the editor. 
 */
public class ScriptCommandDialog extends JDialog {

	private static final long serialVersionUID = 727835259070833902L; //generated
	
	protected JTextField mValues[];
	//protected JLabel mIcons[];
	protected JButton mOkButton;
	protected JButton mCancelButton;
	
	protected CommandBody mCommand;
	protected CommandModel mModel;
	protected List<ParameterFrame> mParas;
	
	protected ImageIcon mValidIcon; 
	protected ImageIcon mInvalidIcon;

	/**
	 * The constructor to create the dialog.
	 * @param cmd The CommandModel this Dialog corresponds to
	 */
	public ScriptCommandDialog(CommandModel cmd) {
		super(MainApp.getInstance().getMainWindow(), cmd.getFrame().getName(), Dialog.ModalityType.APPLICATION_MODAL);
		mCommand = cmd.getBody();
		mModel = cmd;
		mParas = mCommand.getFrame().getParameters();
		
		mValidIcon = new ImageIcon("resources/icons/checkbox.png");
		mInvalidIcon = new ImageIcon("resources/icons/error.png");
		createGui();
		Rectangle bounds = MainApp.getInstance().getMainWindow().getBounds();
		Dimension size = getSize();
		size.height -= 20;
		setBounds(bounds.x + bounds.width / 2 - size.width / 2, bounds.y + bounds.height / 2 - size.height / 2, size.width, size.height);
	}

	/**
	 * Creates the GUI of the dialog
	 */
	protected void createGui() {
		int paraCount = mParas.size();
		
		if (paraCount < 1) //nothing to do here. this object shouldn't have been created.
			return;
		
		int height = 70 * paraCount + 100;
		setBounds(100, 100, 500, height);
		setMinimumSize(new Dimension(600, Math.min(height+150, 600)));
		
		//ok/cancel buttons first
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				mOkButton = new JButton("OK");
				mOkButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent a) {
						savePressed();
					}
				});
				mOkButton.setActionCommand("OK");
				buttonPane.add(mOkButton);
				getRootPane().setDefaultButton(mOkButton);
			}
			{
				mCancelButton = new JButton("Cancel");
				mCancelButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent a) {
						close();
					}
				});
				mCancelButton.setActionCommand("Cancel");
				buttonPane.add(mCancelButton);
			}
		}
		
		mValues = new JTextField[paraCount];
		
		//Make sure it scrolls if it doesn't fit the window
		JScrollPane scroll = new JScrollPane();
		final JPanel panel = new JPanel();
		getContentPane().add(scroll, BorderLayout.CENTER);
		scroll.setViewportView(panel);
		panel.setLayout(new MigLayout("", "[100px:n, left][grow][]", "[] 5px [] 10px [grow]"));
		
		//add the title
		panel.add(new JLabel("Name"), "cell 0 0, alignx center, aligny top");
		panel.add(new JLabel("Value"), "cell 1 0, alignx center, aligny top");
		panel.add(new JPanel(), "cell 2 0, alignx right, aligny top");
		panel.add(new JSeparator(SwingConstants.HORIZONTAL), "cell 0 1, span 3, growx");
		
		//now create a row for each parameter
		for (int i = 0; i < paraCount; i++) {
			ParameterFrame curP = mParas.get(i);
		    String curId = curP.getId();

			//name/description panel
		    ParameterFramePanel framePanel = new ParameterFramePanel(curP, mCommand.getParameter(curId));
			panel.add(framePanel.getNamePanel(), "cell 0 " + (i + 2) + ", grow, aligny top");

			//set the value if already specified
			mValues[i] = framePanel.getTextField();
		    JTextField val = mValues[i];
		    
			//check the parameters if a key was released
			val.addKeyListener(new KeyListener() {
				public void keyReleased(KeyEvent e) {
					checkParameters();
				}
				public void keyPressed(KeyEvent e) {}
				public void keyTyped(KeyEvent e) {}
			});	
			
			// add document listener
			val.getDocument().addDocumentListener(new DocumentListener(){

				@Override
				public void insertUpdate(DocumentEvent e) {
					checkParameters();
				}

				@Override
				public void removeUpdate(DocumentEvent e) {
					checkParameters();
				}

				@Override
				public void changedUpdate(DocumentEvent e) {
				}
				
			});			
			
			// for lOptions, not add TestFiled
            panel.add( framePanel.getValueComponent(), "cell 1 "+(i + 2)+",growx,aligny top");
            			
			//first we set an invalid icon, later on it's going to be checked and set correctly
			//mIcons[i] = new JLabel(mInvalidIcon);
			//panel.add(mIcons[i], "cell 2 " + (i + 2) + ", alignx right, aligny top, gaptop 3px" );
		}

		//check ALL the parameters and adjust the icons!
		checkParameters();
		pack();
	}
	
	/**
	 * Checks all parameters and sets their icons properly to show the user if the parameter validates
	 */
	protected void checkParameters() {
		Boolean allValid = true;
		for (int i = 0; i < mParas.size(); i++) {
			if(mValues[i] == null)
				continue;
			
			Boolean valid = mParas.get(i).validates(mValues[i].getText());
			//mIcons[i].setIcon(valid ? mValidIcon : mInvalidIcon);
			allValid = allValid && valid;
		}
		//if everything is fine we enable the OK Button, otherwise we disable it
		mOkButton.setEnabled(allValid);
	}
	
	/**
	 * Closes the dialog
	 */
	protected void close() {
		setVisible(false);
		dispose();
	}
	
	/**
	 * Writes the changes back to the editor
	 */
	protected void writeToEditor() {
		JEditTextArea editor = MainApp.getInstance().getActiveEditor();
		Element line = editor.getDocument().getDefaultRootElement().getElement(mModel.getStartLine());
		//preserve indention
		String lineText = editor.getLineText(mModel.getStartLine());
		StringBuffer sb = new StringBuffer();
		for(int i = 0; i < lineText.length(); i++) {
			char c = lineText.charAt(i);
			if (c != ' ' && c != '\t') {
				break;
			}
			sb.append(c);
		}
		//now append the command as text (command + parameters)
		sb.append(mCommand.toText());
		//update the line
		editor.select(line.getStartOffset(), line.getEndOffset() -1);
		editor.setSelectedText(sb.toString());
	}
	
	/**
	 * Write back all parameters to the corresponding CommandModel and the editor
	 */
	protected void savePressed() {
		//validate parameters value
		validateParameterValues(mCommand.getFrame().getName());
		
		//write back all parameters
		for (int i = 0; i < mParas.size(); i++) {
			String key = mParas.get(i).getId();
			mCommand.setParameter(key, mValues[i].getText());
		}
		
		//write back to editor
		writeToEditor();
		close();
	}
	
	/**
	 * 
	 * @param apiName
	 */
	private void validateParameterValues(String apiName) {
		// currently only process EqfAnalyzeDocEx
		if( !"EqfAnalyzeDocEx".equals(apiName) ) {
			return;
		}
		
		String mtOutputOpts= "";
		for (int i = 0; i < mParas.size(); i++) {
			
			String key = mParas.get(i).getId();
			if( "pszmtoutputoptions".equals(key) )
				 mtOutputOpts = mValues[i].getText();
			
			else if( "loptions".equals(key) ) {
				
				String loptions = mValues[i].getText();
				if( !mtOutputOpts.isEmpty() ) {
					
					if( loptions.isEmpty() )
						mValues[i].setText("SENDTOMT_OPT");
					else if( loptions.indexOf("SENDTOMT_OPT")==-1 ){
						StringBuilder sb = new StringBuilder(loptions);
						if( sb.indexOf(",")==-1 )
					        mValues[i].setText( sb.insert(0,"(SENDTOMT_OPT,").append(")").toString() );
						else
							mValues[i].setText( sb.insert(1, "SENDTOMT_OPT,").toString() );
					}
					
				}
				
			}//end else
			
		}//end for
		
	}

}
