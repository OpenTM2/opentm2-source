/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

package de.ibm.com.opentm2scripteride.gui.custom;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

import javax.swing.JCheckBox;
import javax.swing.JRadioButton;
import javax.swing.JToggleButton;

/**
 * Selectable group for parameter
 * @author wliping
 *
 */
public class ScriptSelectableGroup {
    
    private ArrayList<ToggleBttonGroup> mGroups = new ArrayList<ToggleBttonGroup>();	
	private String mDescription = null;
	
    public void parser(String strOptions) {
    	//
    	String[] items = strOptions.split("@");
		if(items.length<2)
			return;
		
		HashMap<String,JToggleButton> btnsMap = new HashMap<String,JToggleButton>();
		HashMap<String,String[]> childrenMap = new HashMap<String,String[]>();
		ArrayList<String> XOR = new ArrayList<String>();
		
		mDescription = items[0];		
        // process all items here
		for(int i=1; i<items.length; i++) {
			String[] keyvalues = items[i].split(":");
			if(keyvalues.length!=2)
				continue;
			
			String key = keyvalues[0].trim();	
			String value = keyvalues[1].trim();
			
			//  @GROUP
			if("GROUP".equals(key)) {
				//[,]
				int begIdx = value.indexOf('[');
				int endIdx = value.indexOf(']');
				if( begIdx!=-1 && endIdx!=-1 ) {
					String[] vals= value.substring(begIdx+1, endIdx).split(",");
					
					for(int iL=0; iL<vals.length; iL++)
						vals[iL] = vals[iL].trim();
					Arrays.sort(vals);

					for(String v:vals)
						XOR.add(v);
				}
			}//end if GROUP
			else {
				
				boolean bSingle = true;
				if(value.indexOf('{')==0 && value.charAt(value.length()-1)=='}') {
					value = value.substring(1, value.length()-1);
					bSingle = false;
				}
				
				ToggleBttonGroup tbg = null;
				if(bSingle)
					tbg = new SingleToggleBttonGroup(key);
				else
					tbg = new MultiToggleBttonGroup(key);
				
				// Other normal Options
				String[] vals = value.split(",");
				
				for(int iL=0; iL<vals.length; iL++)
					vals[iL] = vals[iL].trim();
				Arrays.sort(vals);

				for(String val:vals) {
					//val = val.trim();
					// 1. default
					int defaultIndex = val.indexOf("(default)");
					boolean bDefault = false;
					String rdName = val;
					if(defaultIndex!=-1 ) {
						rdName = val.substring(0,defaultIndex);
						bDefault = true;
					} 
		
					// 2.child
					int fromIndex = defaultIndex==-1?0:defaultIndex+1;
				    int childBegIndex = val.indexOf("{",fromIndex);
			    	int childEndIndex = val.indexOf("}", fromIndex);
			    	
			    	if(childBegIndex!=-1 && childEndIndex!=-1) {
			    		if(!bDefault)
			    			rdName = rdName.substring(0, childBegIndex);
			    		String[] children = val.substring(childBegIndex+1, childEndIndex).split(" ");
			    		
			    		if(children.length != 0){
				    		String keyname = key+","+rdName;
				    		childrenMap.put(keyname, children);
			    		}
			    	}//end if
			    	
			    	// 3. add 
			    	addToGroup(btnsMap, tbg, rdName, bDefault);
				   
				}//end for 
				
				// add XOR
				if(!XOR.isEmpty()) {
					for(ToggleBttonGroup btng:mGroups) {
						if(tbg.getGroupName()!=null && btng.getButtons()!=null &&
						   XOR.indexOf(tbg.getGroupName())!=-1 && XOR.indexOf(btng.getGroupName())!=-1) {
							
							btng.addXOR(tbg);
							tbg.addXOR(btng);
						}
					}
				}//end if
				
				// add to groups
				mGroups.add(tbg);
				
			}//end else

		}//end for
		
		// add children
		addGroupsChildren(btnsMap,childrenMap);
    }//end parser
    
    
    public ToggleBttonGroup getGroupByName(String groupName) {
    	if(groupName==null || groupName.isEmpty())
    		return null;
    	
    	for(ToggleBttonGroup btg:mGroups) {
    		if(!btg.getGroupName().isEmpty() && btg.getGroupName().equals(groupName))
    			return btg;
    	}
    	
    	return null;
    }
    
    
    public ArrayList<ToggleBttonGroup> getGroups() {
    	return mGroups;
    }
    
    
    public String getDescription() {
    	return mDescription;
    }
   
   
	public String formatToText() {
		StringBuilder sb = new StringBuilder();
		// format string
		for(ToggleBttonGroup tbg:mGroups) {
			if(!tbg.getSelectedText().isEmpty()){
				sb.append(tbg.getSelectedText());
				sb.append(",");
			}
		}
        
        if(sb.length()>0)
        	sb.deleteCharAt(sb.length()-1);
        
        // use "()" to protect more options
        if(sb.indexOf(",")!=-1) {
        	sb.insert(0, '(');
        	sb.append(")");
        }
        
        return sb.toString();
	}
	
	public void enableGroups(boolean bEnabled) {
		for(ToggleBttonGroup tbg:mGroups) {
			tbg.enableAll(bEnabled);
		}
		
		for(ToggleBttonGroup tbg:mGroups) {
			tbg.adjustChildrenState();
		}
	}
	
	private void addToGroup(HashMap<String,JToggleButton> btnsMap,ToggleBttonGroup toGroup, String key, boolean bDefault) {
		if(btnsMap.containsKey(key)) {
			toGroup.add(btnsMap.get(key), bDefault);
    	    
    	} else {
    		JToggleButton btn = toGroup.add(key, bDefault);
    		btnsMap.put(key, btn);
    	}
	}
	
    private void addGroupsChildren(HashMap<String,JToggleButton> btnsMap,HashMap<String,String[]> children) {
    	if(children==null || children.isEmpty())
    		return;
    	
    	Iterator<Entry<String,String[]>> iter = children.entrySet().iterator();
    	while(iter.hasNext()) {
    		Entry<String,String[]> entry = iter.next();
    		String keyname = entry.getKey();
    		
    		String[] names = keyname.split(",");
    		if(names.length!=2)
    			continue;
    		
    		String groupName = names[0];
    		String buttonName = names[1];
    		
    		ToggleBttonGroup btg = getGroupByName(groupName);
    		if(btg == null)
    			continue;
    		
    		String[] values = entry.getValue();
    		if(values.length==0)
    			continue;
    		
    		for(String val:values) {
    			JToggleButton btn = btnsMap.get(val);
    			if(btn!=null) {
    				btn.setEnabled(false);
    			    btg.addChild(buttonName, btn);
    			}
    		}
    	}//end while
    }
	
}//end class


/**
 * 
 * @author wliping
 *
 */
abstract class ToggleBttonGroup {
	
	protected ArrayList<JToggleButton> mBtns = new ArrayList<JToggleButton>();
	protected HashMap<String,ArrayList<JToggleButton> > mChildrenTable = new HashMap<String,ArrayList<JToggleButton> >();
	protected ArrayList<ToggleBttonGroup> mXOR = new ArrayList<ToggleBttonGroup>();
	
	protected String  mGroupName = null;
	
    public ToggleBttonGroup(String name) {
    	mGroupName = name;
    }
    
	public String getGroupName() {
		return mGroupName;
	}

	public void setGroupName(String mGroupName) {
		this.mGroupName = mGroupName;
	}
	
	protected  boolean isInGroup(JToggleButton btn) {
		for(JToggleButton button:mBtns) {
			if(button == btn)
				return true;
		}
		return false;
	}
	
	protected boolean isInChildGroup(JToggleButton btn) {
		return false;
		
	}
	
	protected void add(JToggleButton btn) {
		if(isInGroup(btn))
			return;
		mBtns.add(btn);
	}

	
	public void adjustChildrenState() {
		ArrayList<JToggleButton> vistedChildren = new ArrayList<JToggleButton>();
		for(JToggleButton button:mBtns) {
			
			boolean bSelected = button.isSelected() && button.isEnabled();
			
			if( mChildrenTable.containsKey(button.getText()) ) {
				ArrayList<JToggleButton> children = mChildrenTable.get(button.getText());
				for(JToggleButton child:children) {
					if(vistedChildren.indexOf(child)!=-1)
						continue;
					
					if(bSelected) {
						vistedChildren.add(child);
						child.setEnabled(true);
					} else {
						child.setEnabled(false);
						// dangerous point,can't cancel selected button in groups
					    child.setSelected(false);
					}
				}
			}//end if
			
		}//end for
	}
	
	
	public String getSelectedText() {
		StringBuilder sb = new StringBuilder("");
		for(JToggleButton button:mBtns) {
			if(button.isEnabled() && button.isSelected()) {
				sb.append(button.getText());
				sb.append(',');
			}
		}
		if(sb.length()>1)
			sb.deleteCharAt(sb.length()-1);
		return sb.toString();
	}
	
	public void enableAll(boolean bEnabled) {
		if(bEnabled && !isNoneXORSelected() )
			return;
		
		for(JToggleButton button:mBtns) {
			button.setEnabled(bEnabled);
		}//end for

	}
	
	
	public ArrayList<JToggleButton> getButtons() {
		return mBtns;
	}
	
	
	public void addXOR(ToggleBttonGroup tbg) {
		if(mXOR.indexOf(tbg)==-1)
			mXOR.add(tbg);
		
	}
	
	public void addChild(String key,JToggleButton child) {
		ArrayList<JToggleButton> children = mChildrenTable.get(key);
		if(children==null){
			children = new ArrayList<JToggleButton>();
			mChildrenTable.put(key,children);
		}
		
		if(children.indexOf(child) == -1) {
			children.add(child);
		}
	}

	public boolean isNoneXORSelected () {
		if(mXOR.isEmpty())
			return true;
		
		for(ToggleBttonGroup btg:mXOR) {
			if(!btg.isNoneSeleted())
				return false;
		}

		return true;
	}
	
	
	public void enableAllXOR (boolean bEnable) {
		if(mXOR.isEmpty())
			return;
		
		for(ToggleBttonGroup btg:mXOR) {
			btg.enableAll(bEnable);
		}
	}
	
	
	/**
	 * 
	 */
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append(mGroupName);
		sb.append("=>");
		
		for(JToggleButton btn:mBtns) {
			sb.append(btn.getText());
			if(mChildrenTable.containsKey(btn.getText())) {
				sb.append("{");
				sb.append(mChildrenTable.get(btn.getText()).toString());
				sb.append("}");
			}
			sb.append(",");
		}
		sb.deleteCharAt(sb.length()-1);
		return sb.toString();
	}

	
	/**
	 * 
	 */
	public abstract void enableDefault();
	
	/**
	 * 
	 * @param btn
	 * @param bDefault
	 */
	public abstract void add(JToggleButton btn, boolean bDefault);

	/**
	 * 
	 * @param btnName
	 * @param bDefault
	 */
	public abstract JToggleButton add(String btnName, boolean bDefault);


	/**
	 * 
	 * @param btn
	 */
	public abstract void setSelected(JToggleButton btn);
	
	/**
	 * 
	 * @return
	 */
	public abstract boolean isNoneSeleted();
	
	
}

/**
 * own toggle button to used to display for options
 * @author wliping
 *
 */
class SingleToggleBttonGroup extends ToggleBttonGroup{
	
	private JToggleButton mSeletedBtn = null;
	private JToggleButton mDefaultBtn = null;
	
	public SingleToggleBttonGroup(String name) {
		super(name);
	}
	
	@Override
	public void add(JToggleButton btn, boolean bDefault) {
		add(btn);
		if(bDefault)
			mDefaultBtn = btn;
	}

	@Override
	public JToggleButton add(String btnName, boolean bDefault) {
		JToggleButton  btn = new JRadioButton(btnName);
		add(btn,bDefault);
		return btn;
	}


	@Override
	public void enableDefault() {
		if(mDefaultBtn!=null && mSeletedBtn==null) {
			setSelected(mDefaultBtn);
		}
	}
	

	@Override
	public void setSelected(JToggleButton btn) {
		if(!isInGroup(btn))
			return;
		
		for(JToggleButton button:mBtns) {
			button.setSelected(false);
		}
		
		//setSelectedChildrenEnabled(false);
		
		if(!isNoneXORSelected()) {
			return;
		}
		
		if(mSeletedBtn == null || btn!=mSeletedBtn) {
			btn.setSelected(true);
			mSeletedBtn = btn;
			//setSelectedChildrenEnabled(true);
			enableAllXOR(false);
			
		}else {
			mSeletedBtn = null;
			enableAllXOR(true);
		}
		
		adjustChildrenState();
	}

	@Override
	public boolean isNoneSeleted() {
		if(mSeletedBtn==null || !mSeletedBtn.isEnabled())
			return true;
		return false;
	}
	

}//end class


/**
 * Multiple Selected button groups
 * @author wliping
 *
 */
class MultiToggleBttonGroup extends ToggleBttonGroup{
	
	private ArrayList<JToggleButton> mDefaultBtns = new ArrayList<JToggleButton>();
	private ArrayList<JToggleButton> mSeletedBtns = new ArrayList<JToggleButton>();
	
	public MultiToggleBttonGroup(String name) {
		super(name);
	}

	@Override
	public void add(JToggleButton btn, boolean bDefault) {
		add(btn);
		if(bDefault && mDefaultBtns.indexOf(btn)==-1)
			mDefaultBtns.add(btn);
	}

	@Override
	public JToggleButton add(String btnName, boolean bDefault) {
		JToggleButton  btn = new JCheckBox(btnName);
		add(btn,bDefault);
		return btn;
	}

	@Override
	public void enableDefault() {
		if(!mSeletedBtns.isEmpty())
			return;
		
		for(JToggleButton btn: mDefaultBtns) {
			setSelected(btn);	
		}
		
	}

	@Override
	public void setSelected(JToggleButton btn) {
		if(!isInGroup(btn) || !isNoneXORSelected())
			return;
		
		if( mSeletedBtns.indexOf(btn)!=-1 ) {
			btn.setSelected(false);
			
			if(isNoneXORSelected())
				enableAllXOR(true);
			
			//setSelectedChildrenEnabled(false);
			mSeletedBtns.remove(btn);
		}
		else {
			btn.setSelected(true);
			enableAllXOR(false);
			mSeletedBtns.add(btn);
			//setSelectedChildrenEnabled(true);
		}
		
		adjustChildrenState();
	}

	@Override
	public boolean isNoneSeleted() {
		if(mSeletedBtns.isEmpty())
			return true;
		
		for(JToggleButton btn:mSeletedBtns) {
			if(btn.isEnabled())
				return true;
		}
		
		return false;
	}
	
}

