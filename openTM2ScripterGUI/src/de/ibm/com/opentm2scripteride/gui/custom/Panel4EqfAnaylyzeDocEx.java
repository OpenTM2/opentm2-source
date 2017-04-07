/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
 */
package de.ibm.com.opentm2scripteride.gui.custom;

import javax.swing.JPanel;
import javax.swing.border.TitledBorder;
import javax.swing.JTextField;
import javax.swing.UIManager;

import java.awt.Color;

import javax.swing.JRadioButton;
import javax.swing.JButton;

import java.awt.Component;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Map;

import javax.swing.ButtonGroup;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.BoxLayout;
import javax.swing.JOptionPane;
import javax.swing.JSplitPane;


class DependanceMatrix {
	private String []ROWS = new String[]{"Nomatch", "AllSegs","AllWMatch","AllWMatchSource", "Noproposal", "Xliff"};
	private String []COLS = new String[]{"EXP","XML","TMX",
			                     "DUPLICATES","NODUPLICATES",
			                     "NOHAMSTER","HAMSTER",
			                     "NOMACHINEMATCH","MACHINEMATCH",
			                     "NOFUZZYABOVE",
			                     "NOWORDCOUNT","WORDCOUNT"
			                    };
	
	private int [][]mDependance = new int[][]{
			{ 1, 2, 1, 2, 1, 2, 1, 2, 1, 3, 2, 1 },
			{ 1, 2, 1, 2, 1, 0, 0, 0, 0, 0, 2, 1 },
			{ 0, 2, 0, 2, 1, 0, 0, 0, 0, 0, 2, 1 },
			{ 0, 2, 0, 2, 1, 0, 0, 0, 0, 0, 2, 1 },
			{ 1, 2, 1, 2, 1, 2, 1, 2, 1, 3, 2, 1 },
			{ 0, 0, 0, 2, 1, 2, 1, 2, 1, 1, 0, 0 }
			};
	
	
	private boolean isLegalPos(int row, int col) {
		return (row>=0&&row<ROWS.length&&col>=0&&col<COLS.length);
	}
	
	
	public int getIndexByName(String name, String[] names) {
		for(int i=0; i<names.length; i++)
			if(name.equalsIgnoreCase(names[i]))
				return i;
		
		return -1;
	}
	
	
	public int getDependance(int row, int col) {
		if( !isLegalPos(row,col) )
			return -1;
		
		return mDependance[row][col];
	}
	
	
	public int  getDependance(String rowName,String colName) {
		int rowIndex = getIndexByName(rowName,ROWS);
		int colIndex = getIndexByName(colName,COLS);
		
		return getDependance(rowIndex,colIndex);
	}

	
	public Map<String, Integer> getRowDependancesByName(String rowName) {
		HashMap<String,Integer> depmap = new HashMap<String,Integer>();
		
		int rowIndex = getIndexByName(rowName,ROWS);
		if(rowIndex>=0 && rowIndex<ROWS.length) {
			
			int[] temp = mDependance[rowIndex];
			for(int j=0; j<COLS.length; j++) {
				String colName = COLS[j];
				depmap.put(colName, temp[j]);
			}
			
		}//end if
		
		return depmap;
	}
	
	
}//end class 



public class Panel4EqfAnaylyzeDocEx extends JPanel {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private JTextField tfNomatch;
	private JTextField tfAllSegs;
	private JTextField tfAllWMatch;
	private JTextField tfAllWMatchSource;
	private JTextField tfNoproposal;
	private JTextField tfXliff;
	
	JTextField tfNoFuzzyAbove;


	final JPanel formatPanel = new JPanel();
	final JPanel hamsterPanel = new JPanel();
	final JPanel duplicatePanel = new JPanel();
	final JPanel fuzzyPanel = new JPanel();
	final JPanel wordCountPanel = new JPanel();
	final JPanel machinePanel = new JPanel();
	final JPanel mtOutputFileNamesPanel = new JPanel();
	
	JButton btnAdd = new JButton("Apply");
	JButton btnReset = new JButton("Reset");
	
	ArrayList<JPanel> panels = new ArrayList<JPanel>();
	ArrayList<JRadioButton> rdbtns = new ArrayList<JRadioButton>();
	ArrayList<ButtonGroup> btngrps = new ArrayList<ButtonGroup>();
	Hashtable<String,JTextField> rdbtn2tf = new Hashtable<String,JTextField>();
	
	boolean mbSendToMTReminder = true;
	DependanceMatrix mDepMat = new DependanceMatrix();
	
	/**
	 * Create the panel.
	 */
	public Panel4EqfAnaylyzeDocEx() {
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));

		JSplitPane splitPane = new JSplitPane();
		splitPane.setOrientation(JSplitPane.VERTICAL_SPLIT);
		splitPane.setDividerLocation(200);
		add(splitPane);

		//mAllPanels.add(mtOutputFileNamesPanel);
		
		// final JPanel mtOutputFileNamesPanel = new JPanel();
		mtOutputFileNamesPanel.setBorder(new TitledBorder(UIManager
				.getBorder("TitledBorder.border"), "MT output file names",
				TitledBorder.LEADING, TitledBorder.TOP, null,
				new Color(0, 0, 0)));
		splitPane.setLeftComponent(mtOutputFileNamesPanel);

		JRadioButton rdbtnNomatch = new JRadioButton("NOMATCH");

		tfNomatch = new JTextField();
		tfNomatch.setColumns(50);

		JRadioButton rdbtnAllSegs = new JRadioButton("ALLSEGS");

		tfAllSegs = new JTextField();
		tfAllSegs.setColumns(50);

		JRadioButton rdbtnAllWatch = new JRadioButton("ALLWMATCH");

		tfAllWMatch = new JTextField();
		tfAllWMatch.setColumns(50);

		JRadioButton rdbtnAllWMatchSource = new JRadioButton("ALLWMATCHSOURCE");

		tfAllWMatchSource = new JTextField();
		tfAllWMatchSource.setColumns(50);

		JRadioButton rdbtnNoProposal = new JRadioButton("NOPROPOSAL");

		tfNoproposal = new JTextField();
		tfNoproposal.setColumns(50);

		JRadioButton rdbtnXliff = new JRadioButton("XLIFF");

		tfXliff = new JTextField();
		tfXliff.setColumns(50);
		GroupLayout gl_mtOutputFileNamesPanel = new GroupLayout(
				mtOutputFileNamesPanel);
		gl_mtOutputFileNamesPanel.setHorizontalGroup(
			gl_mtOutputFileNamesPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_mtOutputFileNamesPanel.createSequentialGroup()
					.addGap(30)
					.addGroup(gl_mtOutputFileNamesPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(rdbtnAllSegs)
						.addComponent(rdbtnAllWatch)
						.addComponent(rdbtnAllWMatchSource)
						.addComponent(rdbtnNoProposal)
						.addComponent(rdbtnXliff)
						.addComponent(rdbtnNomatch))
					.addGap(35)
					.addGroup(gl_mtOutputFileNamesPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(tfAllSegs, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 416, Short.MAX_VALUE)
						.addComponent(tfNomatch, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 416, Short.MAX_VALUE)
						.addComponent(tfAllWMatch, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 416, Short.MAX_VALUE)
						.addComponent(tfAllWMatchSource, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 416, Short.MAX_VALUE)
						.addComponent(tfNoproposal, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 416, Short.MAX_VALUE)
						.addComponent(tfXliff, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 416, Short.MAX_VALUE))
					.addContainerGap())
		);
		gl_mtOutputFileNamesPanel.setVerticalGroup(
			gl_mtOutputFileNamesPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_mtOutputFileNamesPanel.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_mtOutputFileNamesPanel.createParallelGroup(Alignment.BASELINE)
						.addComponent(rdbtnNomatch)
						.addComponent(tfNomatch, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addGap(5)
					.addGroup(gl_mtOutputFileNamesPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(rdbtnAllSegs)
						.addGroup(gl_mtOutputFileNamesPanel.createSequentialGroup()
							.addGap(1)
							.addComponent(tfAllSegs, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
					.addGap(5)
					.addGroup(gl_mtOutputFileNamesPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(rdbtnAllWatch)
						.addGroup(gl_mtOutputFileNamesPanel.createSequentialGroup()
							.addGap(1)
							.addComponent(tfAllWMatch, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
					.addGap(5)
					.addGroup(gl_mtOutputFileNamesPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(rdbtnAllWMatchSource)
						.addGroup(gl_mtOutputFileNamesPanel.createSequentialGroup()
							.addGap(1)
							.addComponent(tfAllWMatchSource, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
					.addGap(5)
					.addGroup(gl_mtOutputFileNamesPanel.createParallelGroup(Alignment.LEADING)
						.addComponent(rdbtnNoProposal)
						.addGroup(gl_mtOutputFileNamesPanel.createSequentialGroup()
							.addGap(1)
							.addComponent(tfNoproposal, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)))
					.addGap(5)
					.addGroup(gl_mtOutputFileNamesPanel.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_mtOutputFileNamesPanel.createSequentialGroup()
							.addGap(1)
							.addComponent(tfXliff, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
						.addComponent(rdbtnXliff))
					.addContainerGap())
		);
		mtOutputFileNamesPanel.setLayout(gl_mtOutputFileNamesPanel);

		JPanel panel_1 = new JPanel();
		splitPane.setRightComponent(panel_1);
		GridBagLayout gbl_panel_1 = new GridBagLayout();
		gbl_panel_1.columnWidths = new int[] { 0, 95, -26, 0, 0, 0, -77, 0, 0 };
		gbl_panel_1.rowHeights = new int[] { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		gbl_panel_1.columnWeights = new double[] { 0.0, 1.0, 1.0, 1.0, 1.0,
				0.0, 0.0, 0.0, Double.MIN_VALUE };
		gbl_panel_1.rowWeights = new double[] { 1.0, 1.0, 1.0, 1.0, 1.0, 0.0,
				1.0, 0.0, 0.0, 0.0, 0.0, Double.MIN_VALUE };
		panel_1.setLayout(gbl_panel_1);

		// final JPanel formatPanel = new JPanel();
		formatPanel.setBorder(new TitledBorder(UIManager.getBorder("TitledBorder.border"), "Output file types", TitledBorder.LEADING, TitledBorder.TOP, null, new Color(0, 0, 0)));
		GridBagConstraints gbc_formatPanel = new GridBagConstraints();
		gbc_formatPanel.insets = new Insets(0, 0, 5, 5);
		gbc_formatPanel.fill = GridBagConstraints.BOTH;
		gbc_formatPanel.gridx = 1;
		gbc_formatPanel.gridy = 1;
		panel_1.add(formatPanel, gbc_formatPanel);
		GridBagLayout gbl_formatPanel = new GridBagLayout();
		gbl_formatPanel.columnWidths = new int[] { 0, 0 };
		gbl_formatPanel.rowHeights = new int[] { 0, 0, 0, 0 };
		gbl_formatPanel.columnWeights = new double[] { 0.0, Double.MIN_VALUE };
		gbl_formatPanel.rowWeights = new double[] { 0.0, 0.0, 0.0,
				Double.MIN_VALUE };
		formatPanel.setLayout(gbl_formatPanel);

		JRadioButton rdbtnTmx = new JRadioButton("TMX");
		GridBagConstraints gbc_rdbtnTmx = new GridBagConstraints();
		gbc_rdbtnTmx.fill = GridBagConstraints.HORIZONTAL;
		gbc_rdbtnTmx.insets = new Insets(0, 0, 5, 0);
		gbc_rdbtnTmx.gridx = 0;
		gbc_rdbtnTmx.gridy = 0;
		formatPanel.add(rdbtnTmx, gbc_rdbtnTmx);

		JRadioButton rdbtnExp = new JRadioButton("EXP");
		GridBagConstraints gbc_rdbtnExp = new GridBagConstraints();
		gbc_rdbtnExp.anchor = GridBagConstraints.WEST;
		gbc_rdbtnExp.insets = new Insets(0, 0, 5, 0);
		gbc_rdbtnExp.gridx = 0;
		gbc_rdbtnExp.gridy = 1;
		formatPanel.add(rdbtnExp, gbc_rdbtnExp);

		JRadioButton rdbtnXml = new JRadioButton("XML");
		GridBagConstraints gbc_rdbtnXml = new GridBagConstraints();
		gbc_rdbtnXml.gridx = 0;
		gbc_rdbtnXml.gridy = 2;
		formatPanel.add(rdbtnXml, gbc_rdbtnXml);

		// final JPanel hamsterPanel = new JPanel();
		hamsterPanel.setBorder(new TitledBorder(UIManager.getBorder("TitledBorder.border"), "Hamster options", TitledBorder.LEADING, TitledBorder.TOP, null, new Color(0, 0, 0)));
		GridBagConstraints gbc_hamsterPanel = new GridBagConstraints();
		gbc_hamsterPanel.gridwidth = 2;
		gbc_hamsterPanel.insets = new Insets(0, 0, 5, 5);
		gbc_hamsterPanel.fill = GridBagConstraints.BOTH;
		gbc_hamsterPanel.gridx = 2;
		gbc_hamsterPanel.gridy = 1;
		panel_1.add(hamsterPanel, gbc_hamsterPanel);
		GridBagLayout gbl_hamsterPanel = new GridBagLayout();
		gbl_hamsterPanel.columnWidths = new int[] { 0, 0, 0 };
		gbl_hamsterPanel.rowHeights = new int[] { 0, 0, 0, 0, 0, 0 };
		gbl_hamsterPanel.columnWeights = new double[] { 0.0, 1.0,
				Double.MIN_VALUE };
		gbl_hamsterPanel.rowWeights = new double[] { 0.0, 0.0, 0.0, 0.0, 1.0,
				Double.MIN_VALUE };
		hamsterPanel.setLayout(gbl_hamsterPanel);

		JRadioButton rdbtnHamster = new JRadioButton("HAMSTER");
		GridBagConstraints gbc_rdbtnHamster = new GridBagConstraints();
		gbc_rdbtnHamster.anchor = GridBagConstraints.WEST;
		gbc_rdbtnHamster.insets = new Insets(0, 0, 5, 5);
		gbc_rdbtnHamster.gridx = 0;
		gbc_rdbtnHamster.gridy = 1;
		hamsterPanel.add(rdbtnHamster, gbc_rdbtnHamster);

		JRadioButton rdbtnNoHamster = new JRadioButton("NOHAMSTER");
		GridBagConstraints gbc_rdbtnNoHamster = new GridBagConstraints();
		gbc_rdbtnNoHamster.anchor = GridBagConstraints.WEST;
		gbc_rdbtnNoHamster.insets = new Insets(0, 0, 5, 5);
		gbc_rdbtnNoHamster.gridx = 0;
		gbc_rdbtnNoHamster.gridy = 2;
		hamsterPanel.add(rdbtnNoHamster, gbc_rdbtnNoHamster);

		// final JPanel duplicatePanel = new JPanel();
		duplicatePanel.setBorder(new TitledBorder(null, "Duplicate option",
				TitledBorder.LEADING, TitledBorder.TOP, null, null));
		GridBagConstraints gbc_duplicatePanel = new GridBagConstraints();
		gbc_duplicatePanel.insets = new Insets(0, 0, 5, 5);
		gbc_duplicatePanel.fill = GridBagConstraints.BOTH;
		gbc_duplicatePanel.gridx = 4;
		gbc_duplicatePanel.gridy = 1;
		panel_1.add(duplicatePanel, gbc_duplicatePanel);
		GridBagLayout gbl_duplicatePanel = new GridBagLayout();
		gbl_duplicatePanel.columnWidths = new int[] { 0, 0 };
		gbl_duplicatePanel.rowHeights = new int[] { 0, 0, 0 };
		gbl_duplicatePanel.columnWeights = new double[] { 0.0, Double.MIN_VALUE };
		gbl_duplicatePanel.rowWeights = new double[] { 0.0, 0.0,
				Double.MIN_VALUE };
		duplicatePanel.setLayout(gbl_duplicatePanel);

		JRadioButton rdbtnDuplicates = new JRadioButton("DUPLICATES");
		GridBagConstraints gbc_rdbtnDuplicates = new GridBagConstraints();
		gbc_rdbtnDuplicates.anchor = GridBagConstraints.WEST;
		gbc_rdbtnDuplicates.insets = new Insets(0, 0, 5, 0);
		gbc_rdbtnDuplicates.gridx = 0;
		gbc_rdbtnDuplicates.gridy = 0;
		duplicatePanel.add(rdbtnDuplicates, gbc_rdbtnDuplicates);

		JRadioButton rdbtnNoDuplicates = new JRadioButton("NODUPLICATES");
		GridBagConstraints gbc_rdbtnNoDuplicates = new GridBagConstraints();
		gbc_rdbtnNoDuplicates.gridx = 0;
		gbc_rdbtnNoDuplicates.gridy = 1;
		duplicatePanel.add(rdbtnNoDuplicates, gbc_rdbtnNoDuplicates);

		// final JPanel fuzzyPanel = new JPanel();
		fuzzyPanel.setBorder(new TitledBorder(UIManager.getBorder("TitledBorder.border"), "Fuzzy match rate", TitledBorder.LEADING, TitledBorder.TOP, null, new Color(0, 0, 0)));
		GridBagConstraints gbc_fuzzyPanel = new GridBagConstraints();
		gbc_fuzzyPanel.gridwidth = 2;
		gbc_fuzzyPanel.gridheight = 6;
		gbc_fuzzyPanel.insets = new Insets(0, 0, 5, 5);
		gbc_fuzzyPanel.fill = GridBagConstraints.BOTH;
		gbc_fuzzyPanel.gridx = 2;
		gbc_fuzzyPanel.gridy = 2;
		panel_1.add(fuzzyPanel, gbc_fuzzyPanel);
		GridBagLayout gbl_fuzzyPanel = new GridBagLayout();
		gbl_fuzzyPanel.columnWidths = new int[] { 0, 0, 0 };
		gbl_fuzzyPanel.rowHeights = new int[] { 0, 0, 0, 0 };
		gbl_fuzzyPanel.columnWeights = new double[] { 0.0, 1.0,
				Double.MIN_VALUE };
		gbl_fuzzyPanel.rowWeights = new double[] { 0.0, 0.0, 0.0,
				Double.MIN_VALUE };
		fuzzyPanel.setLayout(gbl_fuzzyPanel);

		JRadioButton rdbtnNewNoFuzzyAbove = new JRadioButton("NOFUZZYABOVE");
		GridBagConstraints gbc_rdbtnNewNoFuzzyAbove = new GridBagConstraints();
		gbc_rdbtnNewNoFuzzyAbove.insets = new Insets(0, 0, 5, 5);
		gbc_rdbtnNewNoFuzzyAbove.gridx = 0;
		gbc_rdbtnNewNoFuzzyAbove.gridy = 1;
		fuzzyPanel.add(rdbtnNewNoFuzzyAbove, gbc_rdbtnNewNoFuzzyAbove);

		tfNoFuzzyAbove = new JTextField();
		GridBagConstraints gbc_tfNoFuzzyAbove = new GridBagConstraints();
		gbc_tfNoFuzzyAbove.insets = new Insets(0, 0, 5, 0);
		gbc_tfNoFuzzyAbove.fill = GridBagConstraints.HORIZONTAL;
		gbc_tfNoFuzzyAbove.gridx = 1;
		gbc_tfNoFuzzyAbove.gridy = 1;
		fuzzyPanel.add(tfNoFuzzyAbove, gbc_tfNoFuzzyAbove);
		tfNoFuzzyAbove.setColumns(10);

		// final JPanel wordCountPanel = new JPanel();
		wordCountPanel.setBorder(new TitledBorder(null, "Word count options",
				TitledBorder.LEADING, TitledBorder.TOP, null, null));
		GridBagConstraints gbc_wordCountPanel = new GridBagConstraints();
		gbc_wordCountPanel.gridheight = 7;
		gbc_wordCountPanel.insets = new Insets(0, 0, 5, 5);
		gbc_wordCountPanel.fill = GridBagConstraints.BOTH;
		gbc_wordCountPanel.gridx = 4;
		gbc_wordCountPanel.gridy = 2;
		panel_1.add(wordCountPanel, gbc_wordCountPanel);
		GridBagLayout gbl_wordCountPanel = new GridBagLayout();
		gbl_wordCountPanel.columnWidths = new int[] { 0, 0 };
		gbl_wordCountPanel.rowHeights = new int[] { 0, 0, 0 };
		gbl_wordCountPanel.columnWeights = new double[] { 0.0, Double.MIN_VALUE };
		gbl_wordCountPanel.rowWeights = new double[] { 0.0, 0.0,
				Double.MIN_VALUE };
		wordCountPanel.setLayout(gbl_wordCountPanel);

		JRadioButton rdbtnWordCount = new JRadioButton("WORDCOUNT");
		GridBagConstraints gbc_rdbtnWordCount = new GridBagConstraints();
		gbc_rdbtnWordCount.anchor = GridBagConstraints.WEST;
		gbc_rdbtnWordCount.insets = new Insets(0, 0, 5, 0);
		gbc_rdbtnWordCount.gridx = 0;
		gbc_rdbtnWordCount.gridy = 0;
		wordCountPanel.add(rdbtnWordCount, gbc_rdbtnWordCount);

		JRadioButton rdbtnNoWordCount = new JRadioButton("NOWORDCOUNT");
		GridBagConstraints gbc_rdbtnNoWordCount = new GridBagConstraints();
		gbc_rdbtnNoWordCount.gridx = 0;
		gbc_rdbtnNoWordCount.gridy = 1;
		wordCountPanel.add(rdbtnNoWordCount, gbc_rdbtnNoWordCount);

		// final JPanel machinePanel = new JPanel();
		machinePanel.setBorder(new TitledBorder(UIManager.getBorder("TitledBorder.border"), "Machine translation option", TitledBorder.LEADING, TitledBorder.TOP, null, new Color(0, 0, 0)));
		GridBagConstraints gbc_machinePanel = new GridBagConstraints();
		gbc_machinePanel.gridheight = 7;
		gbc_machinePanel.insets = new Insets(0, 0, 5, 5);
		gbc_machinePanel.fill = GridBagConstraints.BOTH;
		gbc_machinePanel.gridx = 1;
		gbc_machinePanel.gridy = 2;
		panel_1.add(machinePanel, gbc_machinePanel);
		GridBagLayout gbl_machinePanel = new GridBagLayout();
		gbl_machinePanel.columnWidths = new int[] { 0, 0, 0 };
		gbl_machinePanel.rowHeights = new int[] { 0, 0, 0 };
		gbl_machinePanel.columnWeights = new double[] { 0.0, 0.0,
				Double.MIN_VALUE };
		gbl_machinePanel.rowWeights = new double[] { 0.0, 0.0, Double.MIN_VALUE };
		machinePanel.setLayout(gbl_machinePanel);

		JRadioButton rdbtnMachineMatch = new JRadioButton("MACHINEMATCH");
		GridBagConstraints gbc_rdbtnMachineMatch = new GridBagConstraints();
		gbc_rdbtnMachineMatch.gridwidth = 2;
		gbc_rdbtnMachineMatch.anchor = GridBagConstraints.WEST;
		gbc_rdbtnMachineMatch.insets = new Insets(0, 0, 5, 0);
		gbc_rdbtnMachineMatch.gridx = 0;
		gbc_rdbtnMachineMatch.gridy = 0;
		machinePanel.add(rdbtnMachineMatch, gbc_rdbtnMachineMatch);

		JRadioButton rdbtnNoMachineMatch = new JRadioButton("NOMACHINEMATCH");
		GridBagConstraints gbc_rdbtnNoMachineMatch = new GridBagConstraints();
		gbc_rdbtnNoMachineMatch.gridwidth = 2;
		gbc_rdbtnNoMachineMatch.anchor = GridBagConstraints.WEST;
		gbc_rdbtnNoMachineMatch.gridx = 0;
		gbc_rdbtnNoMachineMatch.gridy = 1;
		machinePanel.add(rdbtnNoMachineMatch, gbc_rdbtnNoMachineMatch);

		GridBagConstraints gbc_btnAdd = new GridBagConstraints();
		gbc_btnAdd.insets = new Insets(0, 0, 5, 5);
		gbc_btnAdd.gridx = 2;
		gbc_btnAdd.gridy = 9;
		panel_1.add(btnAdd, gbc_btnAdd);

		GridBagConstraints gbc_btnReset = new GridBagConstraints();
		gbc_btnReset.insets = new Insets(0, 0, 5, 5);
		gbc_btnReset.gridx = 3;
		gbc_btnReset.gridy = 9;
		panel_1.add(btnReset, gbc_btnReset);

		// add manually
		final ButtonGroup bgoutputFileNames = new ButtonGroup();
		groupRadioButtons(bgoutputFileNames,mtOutputFileNamesPanel.getComponents());
        panels.add(mtOutputFileNamesPanel);
        
		// formatPanel hamsterPanel duplicatePanel wordCountPanel,fuzzyPanel,machinePanel
		ButtonGroup bgFormat = new ButtonGroup();
		groupRadioButtons(bgFormat, formatPanel.getComponents());
		panels.add(formatPanel);
		btngrps.add(bgFormat);
		
		ButtonGroup bgHamster = new ButtonGroup();
		groupRadioButtons(bgHamster, hamsterPanel.getComponents());
		panels.add(hamsterPanel);
		btngrps.add(bgHamster);
		
		ButtonGroup bgFuzzy = new ButtonGroup();
		groupRadioButtons(bgFuzzy, fuzzyPanel.getComponents());
		panels.add(fuzzyPanel);
		btngrps.add(bgFuzzy);
		
		ButtonGroup bgMachine = new ButtonGroup();
		groupRadioButtons(bgMachine, machinePanel.getComponents());
		panels.add(machinePanel);
		btngrps.add(bgMachine);
		
		ButtonGroup bgDuplicate = new ButtonGroup();
		groupRadioButtons(bgDuplicate, duplicatePanel.getComponents());
		panels.add(duplicatePanel);
		btngrps.add(bgDuplicate);
		
		ButtonGroup bgWordCount = new ButtonGroup();
		groupRadioButtons(bgWordCount, wordCountPanel.getComponents());
		panels.add(wordCountPanel);
		btngrps.add(bgWordCount);
		
		// keep  controls
		String []btnText = new String[]{"Nomatch", "AllSegs","AllWMatch","AllWMatchSource", "Noproposal", "Xliff"};
		JTextField []tfs = new JTextField[]{tfNomatch,tfAllSegs,tfAllWMatch,tfAllWMatchSource,tfNoproposal,tfXliff};
		for(int i=0; i<btnText.length; i++) {
			rdbtn2tf.put(btnText[i].toUpperCase(), tfs[i]);
		}
		
		for (Component p : mtOutputFileNamesPanel.getComponents()) {
			if (p instanceof JRadioButton) {
				rdbtns.add((JRadioButton) p);
			}
		}
		
	}

	
	private void groupRadioButtons(ButtonGroup bg, Component[] components) {
//		boolean isFirst = true;
		for (Component p : components) {
			if (p instanceof JRadioButton) {
				bg.add((JRadioButton) p);
//				if (isFirst) {
//					isFirst = false;
//					((JRadioButton) p).setSelected(true);
//				}
			}
		}

	}

	private String getSelectedOptions() {
		StringBuilder sb = new StringBuilder();

		// formatPanel filterPanel duplicatePanel wordCountPanel
		for (JPanel panel : panels) {
			
            if(panel == mtOutputFileNamesPanel)
            	continue;
            
			for (Component p : panel.getComponents()) {

				if (p instanceof JRadioButton) {
					JRadioButton rb = (JRadioButton) p;
					if (rb.isSelected()) {
						sb.append(rb.getText());
						if(rb.getText().equalsIgnoreCase("NOFUZZYABOVE")) {
							sb.append("=").append(tfNoFuzzyAbove.getText());
						}
						sb.append(",");
					}
				}

			}// end for

		}//end for

		if(sb.length() == 0)
			return "";
		
		if (sb.charAt(sb.length() - 1) == ',')
			sb.deleteCharAt(sb.length() - 1);

		if(sb.length()>0) {
			sb = sb.insert(0, "(");
			sb.append(")");
		}
		
		return sb.toString();

	}

	private JRadioButton getMtOutputBtnByName(String name) {
		for(JRadioButton rd:rdbtns) {
			if(rd.getText().equals(name))
				return rd;
		}
		return null;
	}
	
	public void addSelectedOptions2MtOutput() {
		for(JRadioButton rd:rdbtns){
			if(!rd.isSelected())
				continue;
			
			String rdname = rd.getText();
			JTextField tf = rdbtn2tf.get(rdname);
			
			StringBuilder sb2Fill = new StringBuilder(rdname).append(getSelectedOptions());
			tf.setText(sb2Fill.toString());
		}
	}
	
	public String getMToutputOptions() {
		StringBuilder sb = new StringBuilder();
		for(JTextField tf:rdbtn2tf.values()) {
			if(tf.getText().isEmpty())
				continue;
			
			sb.append(tf.getText()).append(",");
		}
		
		if(sb.length() == 0)
			return "";
		
		if(sb.charAt(sb.length()-1) == ',')
			sb.deleteCharAt(sb.length()-1);
		
		if(sb.length()>0){
			sb.insert(0, "\"");
			sb.append("\"");
		}
		
		return sb.toString();
	}
	
	
	public void setEnabled(boolean bEnable) {
		for (JPanel p : panels) {
			for (Component comp : p.getComponents())
				comp.setEnabled(bEnable);
		}

        btnAdd.setEnabled(bEnable);
        btnReset.setEnabled(bEnable);
	}
	
	public void setAsInput(String options) {
		if(options.isEmpty())
			return;
		
		if(options.startsWith("\""))
			options = options.substring(1);
		if(options.endsWith("\""))
			options = options.substring(0, options.length()-1);
		
		
		boolean bSelSet = false;
		int beginIdx = 0,i=0;
		boolean bInBrace = false;
		for(; i<options.length();) {
			
			if(options.charAt(i) == ',') {
                if(!bInBrace ) {
                	String temp = options.substring(beginIdx,i);
                	rdbtn2tf.get(temp).setText(temp);
                	i++;
                	beginIdx = i;
                	continue;
                }
                	
			} else if(options.charAt(i)  == '(') {
			    bInBrace = true;
			    
			} else if(options.charAt(i)  == ')') {
				bInBrace = false;
				String temp = options.substring(beginIdx,i+1);
//				System.out.println(temp);
				String []items = temp.split("\\(");
//				System.out.println(items[0]);
//				System.out.println(items[1]);
				rdbtn2tf.get(items[0]).setText(temp);
				if(!bSelSet) {
				    getMtOutputBtnByName(items[0]).setSelected(true);
				    enableOptionsBtnByInput(temp);
				    bSelSet = true;
				}
				i++;
				if(i<options.length() && options.charAt(i) == ',')
					i++;
				beginIdx = i;
				continue;
			} 
			
			i++;
			
		}//end for
		
		if(beginIdx != i) {
			String temp = options.substring(beginIdx,i);
        	rdbtn2tf.get(temp).setText(temp);
		}
	}
	
	
	
	public ArrayList<JRadioButton> getOptionButtons() {
		ArrayList<JRadioButton> res = new ArrayList<JRadioButton>();
		
		// formatPanel filterPanel duplicatePanel wordCountPanel
		for (JPanel panel : panels) {
			
            if(panel == mtOutputFileNamesPanel)
            	continue;
            
			for (Component p : panel.getComponents()) {

				if (p instanceof JRadioButton) {
					JRadioButton rb = (JRadioButton) p;
					res.add(rb);
				}

			}// end for

		}//end for

       return res;
	}
	
	
	
	public void deselectAllPanels() {
		// formatPanel filterPanel duplicatePanel wordCountPanel
		tfNoFuzzyAbove.setText("");
		for (ButtonGroup bg  : btngrps) {
			bg.clearSelection();
		}//end for
	}

	
	public String getSelectedMtOutputBtn() {
		for(JRadioButton rd:rdbtns) {
			if(rd.isSelected())
				return  rd.getText();
		}
		
		return null;
	}
	
	
//	public void showSendToMtReminderDialog() {
//		if(mbSendToMTReminder) {
//			mbSendToMTReminder = false;
//			JOptionPane.showMessageDialog(this, "To make these options works, please be sure to select SENDTOMT_OPT option!");
//		}
//	}
	
	public void enableOptionsBtnByInput(String option) {
		String rdoptions = option.substring(option.indexOf('(')+1,option.length()-1);
		String []items = rdoptions.split(",");
		for(String item:items) {
			if(item.indexOf('=') != -1) {
				tfNoFuzzyAbove.setText(item.substring(item.indexOf('=')+1));
				item = item.substring(0, item.indexOf('='));
			}
			
			for(JRadioButton temp:getOptionButtons()) {
				if(item.equalsIgnoreCase(temp.getText())) {
					temp.setSelected(true);
					break;
				} 
			}
		}
	}
	
	public void configOptionsBtnByDependance(String rdbtnName) {
		Map<String,Integer> dep = mDepMat.getRowDependancesByName(rdbtnName);
		if( !dep.isEmpty() ) {
			ArrayList<JRadioButton> tempBtns = getOptionButtons();
			for(JRadioButton rd:tempBtns) {
				int val = dep.get(rd.getText());
				rd.setEnabled(true);
				if(val == 0) {
					rd.setEnabled(false);
				} else if(val == 1) {
					rd.setEnabled(true);
				} else if(val == 2) {
					rd.setSelected(true);
				} else if(val == 3) {
					rd.setSelected(true);
					tfNoFuzzyAbove.setText("50");
				} else {
					// it should be never here
					System.out.println("illegal value");
				}
			}//end for
			this.invalidate();
		}//end if
	}
	
	
}
