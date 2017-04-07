/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom;

import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JTextField;
import javax.swing.JToggleButton;

import net.miginfocom.swing.MigLayout;
import de.ibm.com.opentm2scripteride.models.ParameterFrame;
import de.ibm.com.opentm2scripteride.utility.OpenTM2Datas;

/**
 * This is a simple panel with the name and attributes of a Parameter
 */
public class ParameterFramePanel {

	private ParameterFrame mFrame;
	private Font mNameFont;
	private Font mDescriptionFont;
	private JCheckBox mChecked        = null;
	private JPanel mNamePanel         = new JPanel();
	private String mTextValue         = null;
	private ParameterTextSource mPts  = null;
	
	private static String[]  COMBOBOXSELECTABLE  = {"chTargetDrive","chToDrive","chFromDrive","chDriveLetter",
		                                            "pszMarkup","pszEditor","pszSourceLanguage","pszTargetLanguage","pszLanguage"};
	

	/**
	 * Creates the panel
	 * @param frame The parameter this panel should represent
	 */
	public ParameterFramePanel(ParameterFrame frame, String textValue) {
		//super();
		mFrame = frame;
		mTextValue = textValue;
		mNameFont = new Font(Font.SANS_SERIF, Font.PLAIN, 13);
		mDescriptionFont = new Font(Font.SANS_SERIF, Font.ITALIC, 11);
		init();
	}
	
	/**
	 * Initializes the graphical elements
	 */
	private void init() {
		mNamePanel.setLayout(new MigLayout("", "0 [100px:200px] 10px [] 30px", "0 [] 3px [] 0"));

		String nameText = "<html><b>" + mFrame.getName() + "</b> ";
		JLabel name = new JLabel(nameText);
		name.setFont(mNameFont);
		mNamePanel.add(name, "cell 0 0,alignx left,aligny top");
		     
		// add check box for optional or mandatory
		if(mFrame.isOptional()) {
			mChecked = new JCheckBox("Optional");
		}
		else {
			mChecked = new JCheckBox("Mandatory");
			mChecked.setSelected(true);
			mChecked.setEnabled(false);
		}
		
		mChecked.addActionListener(new ActionListener(){
			@Override
			public void actionPerformed(ActionEvent e) {
				mPts.enable(mChecked.isSelected());
				mPts.fillText();
			}//end
			
		});
		mNamePanel.add(mChecked, "cell 1 0,alignx left,aligny top");
		
		String description =  mFrame.getDescription();//"<html>" + mFrame.getDescription().replace("\n", "<br>") + "</html>";
	
		// make decision here about which select able to use
		if( isGroupSelectable() ) {
			String tempDescription = mFrame.getDescription();
			mPts = new GroupSelectableTextSource(mChecked, mTextValue, tempDescription);
			
			int atIdx = tempDescription.indexOf("@");
			if(atIdx!=-1) {
				description = tempDescription.substring(0,atIdx);
			}
			
		} else if( isComboBoxSelectable() ) {
			
			mPts = new ComboBoxSelectableTextSource(mChecked, mTextValue, mFrame.getName());
				
		} else if(mFrame.getName().equals("pszMTOutputOptions")) {
			// hard code for EqfAnalyzeDocEx
			mPts =  new MTOutputOption4EqfAanalyzeDocEx(mChecked, mTextValue);
			
		} else {
			
			mPts = new ParameterTextSource(mChecked, mTextValue);
		}
			
		
        StringBuilder descSb = new StringBuilder("<html>");
        descSb.append(description);
        descSb.append("</html>");
		JLabel descr = new JLabel(descSb.toString());
		descr.setFont(mDescriptionFont);
		mNamePanel.add(descr, "cell 0 1,growx,aligny top");
		mNamePanel.setMinimumSize(new Dimension(name.getWidth(), 50));
		mNamePanel.setSize(mNamePanel.getMinimumSize());
	}
	
	
	public boolean isChecked() {
		return mChecked.isSelected();
	}
	
	
	public JCheckBox getCheckedBox() {
		return mChecked;
	}
	
	public boolean isGroupSelectable() {
		return mFrame.getDescription().indexOf("@")!=-1?true:false;
	}
	
	public JTextField getTextField() {
		if(mPts == null)
			return null;
		return mPts.getTextField();
	}
	
	public JPanel getNamePanel() {
		return mNamePanel;
	}
	
	public boolean isComboBoxSelectable() {
		String paraName = mFrame.getName();
		if(paraName == null || paraName.isEmpty())
			return false;
		
		for(String name:COMBOBOXSELECTABLE) {
			if(paraName.equals(name))
				return true;
		}
		return false;
	}

	public JComponent getValueComponent() {
		if(mPts == null)
			return null;
		return mPts.getTextSourceComponent();	
	}

}

/**
 * base class fro selectable parameter
 * @author wliping
 *
 */
class ParameterTextSource {
	protected JTextField mText = new JTextField();
	private JCheckBox  mChecked = null;

	public ParameterTextSource(JCheckBox checkBox,String textValue) {
		mChecked = checkBox;
		
		if(textValue!=null && !textValue.isEmpty()) {
			mChecked.setSelected(true);
			enable(true);
		} else if(!isSelected()){
			enable(false);
		}
			
		mText.setText(textValue);
	}
	
	public void enable(boolean bEnable) {
		mText.setEnabled(bEnable);
	}
	
	public void fillText() {
		if( !isSelected() )
			mText.setText("");
	}
	
	public JTextField getTextField() {
		return mText;
	}
	
	public JComponent getTextSourceComponent() {
		return mText;
	}
	
	protected boolean isSelected() {
		if(mChecked == null)
			return false;
		
		return mChecked.isSelected();
	}

	
 	protected String[] textTokens() {
		String text = mText.getText();
		if(text==null || text.isEmpty())
			return null;
		
		if(text.charAt(0)=='(')
			text = text.substring(1, text.length()-1);
		
		String[] vals = text.split(",");
		return vals;
	}
	
	protected boolean isValueIn(String val, String[] tokens) {
		if(val==null || val.isEmpty() || tokens==null || tokens.length==0)
			return false;

		for(String token:tokens) {
			if(token==null || token.isEmpty())
				continue;
			if(token.equals(val))
				return true;
		}
		return false;
	}
}

/**
 * For user select group parameter from group panel
 * @author wliping
 *
 */
class GroupSelectableTextSource extends ParameterTextSource {

	private ScriptSelectableGroup mScriptOptions = null;
	private JPanel mValuePanel = null;
	
	public GroupSelectableTextSource(JCheckBox checkBox, String textValue,String description) {
		super(checkBox,textValue);
		init(description);
	}
	
	public void enable(boolean bEnable) {
		super.enable(bEnable);
		if(mScriptOptions == null)
			return;
		
		mScriptOptions.enableGroups(bEnable);
	}
	
	public void fillText() {
		if(mScriptOptions == null)
			return;
		
		mText.setText(mScriptOptions.formatToText());
	}
	
	public JComponent getTextSourceComponent() {
		if(mValuePanel == null)
			mValuePanel = buildGroupPanel();
		
		return mValuePanel;
	}
	
	private void init(String description) {
		mScriptOptions = new ScriptSelectableGroup();
		mScriptOptions.parser(description);
	}
	
	private JPanel buildGroupPanel() {
		if(mScriptOptions==null)
			return null;
		
		JPanel panel = new JPanel();
		ArrayList<ToggleBttonGroup> groups = mScriptOptions.getGroups();
		if(!groups.isEmpty()) {
			
			boolean bStartup = mText.getText().isEmpty()?true:false;
			if(bStartup) {
				for(ToggleBttonGroup tbg:groups)
					tbg.enableDefault();
			}

			for(ToggleBttonGroup tbg:groups) {
				JPanel groupPanel = createGroupPanel(tbg);
				panel.add(groupPanel);
			}//end for
						
			mText.setText( mScriptOptions.formatToText() );
		}//end if
		
		return panel;
	}
	
	private JPanel createGroupPanel(final ToggleBttonGroup tbg) {
		if(tbg==null)
			return null;
		
		ArrayList<JToggleButton> btns = tbg.getButtons();
		if(btns == null || btns.size()==0)
			return null;
		
		String groupName = tbg.getGroupName();
		// create a group box
		JPanel groupPanel = new JPanel(); 
		BoxLayout boxLayout =  new BoxLayout(groupPanel, BoxLayout.Y_AXIS);
		groupPanel.setLayout(boxLayout);
		groupPanel.setBorder(BorderFactory.createTitledBorder(groupName));

		String[] textTokens = textTokens();
		for(final JToggleButton btn:btns) {
			groupPanel.add(btn);
			
			//in the mText text, enabled
			if( isValueIn(btn.getText(),textTokens) )
				tbg.setSelected(btn);	

			// action listener
			btn.addActionListener(new ActionListener(){
				@Override
				public void actionPerformed(ActionEvent e) {
					tbg.setSelected(btn);
	                mText.setText( mScriptOptions.formatToText() );
				}
			});
		}//end for
		
		if(!isSelected())
			tbg.enableAll(false);
		
		return groupPanel;
	}
	
}

/**
 * For user select parameter from JComboBox
 * @author wliping
 *
 */
class ComboBoxSelectableTextSource extends ParameterTextSource {

	private JComboBox<String> mComboBox  = null;
    
	public ComboBoxSelectableTextSource(JCheckBox checkBox, String textValue, String paraName) {
		super(checkBox,textValue);
		init(paraName);
	}
	
	public void enable(boolean bEnable) {
		super.enable(bEnable);
		if(mComboBox == null)
			return;
		
		mComboBox.setEnabled(bEnable);
	}
	
	public void fillText() {
		if(mComboBox == null)
			return;
		
		if(isSelected())
		    mText.setText((String) mComboBox.getSelectedItem());
		else 
			mText.setText("");
	}
	
	public JComponent getTextSourceComponent() {
		return mComboBox;
	}
	
	private void init(String paraName) {
		ArrayList<String> comboStrings = get2FillComboBox(paraName);
		if(comboStrings == null)
			return;

		String[] contents = comboStrings.toArray(new String[comboStrings.size()]);
		mComboBox = new JComboBox<String>(contents);
		mComboBox.setSelectedIndex(-1);
		mComboBox.setEnabled(isSelected());
		
		mComboBox.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				fillText();
			}
		});
		
		buildComboBoxComponent();
	}
	
	private ArrayList<String> get2FillComboBox(String parameterName) {
		String paraName = parameterName;
		if(paraName == null || paraName.isEmpty())
			return null;
		
		OpenTM2Datas otmdatas = OpenTM2Datas.getInstance();
		
		if(paraName.equals("chTargetDrive") || paraName.equals("chFromDrive") ||
				paraName.equals("chDriveLetter") || paraName.equals("chToDrive") )
			return otmdatas.getDrivers();
		else if(paraName.equals("pszMarkup"))
			return otmdatas.getMarkupTables();
		else if(paraName.equals("pszEditor"))
			return otmdatas.getEditors();
		else if(paraName.equals("pszSourceLanguage") || paraName.equals("pszLanguage"))
			return otmdatas.getSourceLanguages();
		else if(paraName.equals("pszTargetLanguage"))
			return otmdatas.getTargetLanguages();
		else
			return null;
	}
	
	private void buildComboBoxComponent() {
		String[] valItems = mText.getText().split(",");
		for(int idx=0; idx<mComboBox.getItemCount(); idx++) {
			String item = (String)mComboBox.getItemAt(idx);
			if(item==null || item.isEmpty()) 
				continue;
			boolean bFound = false;
			for(String vi:valItems) {
				if(vi==null || vi.isEmpty())
					continue;
				if(vi.equals(item)) {
					bFound = true;
					mComboBox.setSelectedIndex(idx);
					mComboBox.setEnabled(true);
					break;
				}
			}//end for
			if(bFound)
				break;
		}//end for
	}
	
}

class MTOutputOption4EqfAanalyzeDocEx extends ParameterTextSource {
	
    private Panel4EqfAnaylyzeDocEx mMTOptPanel = new Panel4EqfAnaylyzeDocEx();
	public MTOutputOption4EqfAanalyzeDocEx(JCheckBox checkBox, String textValue) {
		super(checkBox, textValue);
		mMTOptPanel.setEnabled(isSelected());
		
		mMTOptPanel.setAsInput(mText.getText());
		
		mMTOptPanel.btnAdd.addActionListener(new ActionListener(){

			@Override
			public void actionPerformed(ActionEvent arg0) {
				mMTOptPanel.addSelectedOptions2MtOutput();
				mText.setText(mMTOptPanel.getMToutputOptions());
				//mMTOptPanel.showSendToMtReminderDialog();
			}
			
		});
		
	
		mMTOptPanel.btnReset.addActionListener(new ActionListener(){

			@Override
			public void actionPerformed(ActionEvent arg0) {
				String selRdbtnName = mMTOptPanel.getSelectedMtOutputBtn();
				if(selRdbtnName == null)
					return;
				
				mMTOptPanel.rdbtn2tf.get(selRdbtnName).setText("");
				mText.setText(mMTOptPanel.getMToutputOptions());
				mMTOptPanel.deselectAllPanels();
			}
			
		});
		
		for(final JRadioButton rd:mMTOptPanel.rdbtns) {
			rd.addActionListener(new ActionListener(){

				@Override
				public void actionPerformed(ActionEvent arg0) {
					mMTOptPanel.deselectAllPanels();
					mMTOptPanel.configOptionsBtnByDependance(rd.getText());
					//System.out.println(rd.getText());
					String option = mMTOptPanel.rdbtn2tf.get(rd.getText()).getText();
					if(option.indexOf('(') == -1) {
						return;
					}
					
					mMTOptPanel.enableOptionsBtnByInput(option);
					/*String rdoptions = option.substring(option.indexOf('(')+1,option.length()-1);
					String []items = rdoptions.split(",");
					for(String item:items) {
						if(item.indexOf('=') != -1) {
							mMTOptPanel.tfNoFuzzyAbove.setText(item.substring(item.indexOf('=')+1));
							item = item.substring(0, item.indexOf('='));
						}
						//System.out.println(item);
						for(JRadioButton temp:mMTOptPanel.getOptionButtons()) {
							if(item.equalsIgnoreCase(temp.getText())) {
								temp.setSelected(true);
								break;
							} 
						}
					}*/
				}//end
				
			});
		}
	}
	
	public void enable(boolean bEnable) {
		super.enable(bEnable);
		if(mMTOptPanel == null)
			return;
		
		mMTOptPanel.setEnabled(bEnable);
	}
	
	
	public JComponent getTextSourceComponent() {
		return mMTOptPanel;
	}
	
	public void fillText() {
		if(isSelected())
		    mText.setText(mMTOptPanel.getMToutputOptions());
		else 
			mText.setText("");

	}
}