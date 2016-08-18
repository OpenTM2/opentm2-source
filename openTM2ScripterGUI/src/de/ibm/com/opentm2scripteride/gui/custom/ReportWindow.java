/*! \file
	Description:
	
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/
package de.ibm.com.opentm2scripteride.gui.custom;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Scanner;

import javax.swing.ButtonGroup;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.JToolBar;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableColumn;

import net.miginfocom.swing.MigLayout;

public class ReportWindow{

	private JTable mTable = null;
	private Object[][] mData = null;
	private JFrame mFrame = new JFrame("OpenTM2ScripterGUI-Report");
	private JSplitPane mSp = new JSplitPane();
	private JTextField mTotalCases = new JTextField(10);
	private JTextField mPassCases = new JTextField(10);
	private JTextField mFailCases = new JTextField(10);
	private boolean    mOnlyErrors = false;
	private JRadioButton mErrorRb = null;
	private String[] mHeaders = null;
	
	public static final int MAXCOLUMNWIDTH = 6;

	public ReportWindow() {
		mFrame.setLayout(new BorderLayout(2,2));
		// split it into left and right
		mFrame.add(mSp, BorderLayout.CENTER);
		
		//add panel for General Report
		JPanel generalPanel = new JPanel();
		generalPanel.setLayout(new MigLayout("", "[][]", "[30][][][][30][]"));
		mSp.setLeftComponent(generalPanel);
		
		JLabel totalLb = new JLabel("Total Cases");
		
		mTotalCases.setEditable(false);
		generalPanel.add(totalLb,"cell 0 1");
		generalPanel.add(mTotalCases,"cell 1 1");
		
		JLabel passLb = new JLabel("Pass Cases");
		mPassCases.setEditable(false);
		generalPanel.add(passLb,"cell 0 2");
		generalPanel.add(mPassCases,"cell 1 2");
		
		JLabel failLb = new JLabel("Fail Cases");
		mFailCases.setEditable(false);
		generalPanel.add(failLb,"cell 0 3");
		generalPanel.add(mFailCases,"cell 1 3");
		
		ButtonGroup cbg = new ButtonGroup();
		
		final JRadioButton checkAllBtn = new JRadioButton("List all cases");
		checkAllBtn.setSelected(!mOnlyErrors);
		checkAllBtn.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				if(mOnlyErrors && checkAllBtn.isSelected()){	
					mOnlyErrors = false;
					redrawReportTable();
				}
			}
			
		});
		generalPanel.add(checkAllBtn,"cell 0 5");
		
	    mErrorRb = new JRadioButton("List only error cases");
		mErrorRb.setSelected(mOnlyErrors);
		mErrorRb.addActionListener(new ActionListener(){

			@Override
			public void actionPerformed(ActionEvent e) {
			    if(!mOnlyErrors && mErrorRb.isSelected()) {
			    	mOnlyErrors = true;
			    	redrawReportTable();
			    }
			}
			
		});
		generalPanel.add(mErrorRb,"cell 1 5");
		
		cbg.add(checkAllBtn);
		cbg.add(mErrorRb);
			
		// detail panel
		mTable = new JTable();
		JScrollPane detailPanel = new JScrollPane(mTable);
		mSp.setRightComponent(detailPanel);
		
		// add bottom bar
		JToolBar bottomBar = new JToolBar();
		mFrame.add(bottomBar, BorderLayout.SOUTH);
		bottomBar.setEnabled(false);
		bottomBar.add(new JLabel(" "));

		//
		mFrame.addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				//System.exit(0);
				mFrame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
			}
		});
		
	}
    
	public void generateReport(String logs) {
		LogAnalyzer la = new LogAnalyzer(logs);
		LinkedList<String[]> datas = la.analyze();
		mHeaders = la.getHeaders();
		
		mData = new Object[datas.size()][mHeaders.length];
		int index = 0;
		for( String[] data: datas) {
			if(mOnlyErrors){
				if(data[2].equals("FAIL"))
					mData[index++] = data;
			}
			else
			    mData[index++] = data;
		}
		
		if(la.getErrorCnt()==0)
			mErrorRb.setEnabled(false);
		
		//
		mTotalCases.setText(Integer.toString(la.getTotalCnt()) );
		mFailCases.setText(Integer.toString(la.getErrorCnt()));
		mPassCases.setText(Integer.toString(la.getTotalCnt()-la.getErrorCnt()));
		
		//
		Dimension screenSize =Toolkit.getDefaultToolkit().getScreenSize();
		mTable.setModel( new ReportDataModel(mData,mHeaders) );
        mTable.setPreferredScrollableViewportSize(new Dimension(screenSize.width-500, screenSize.height-300));
		
        setTableColumWidth();
    	mFrame.setLocation(100, 100);
		mFrame.pack();
		mFrame.setVisible(true);
		
	}
	
	public  void generateHtmlReport(String logs,String outPath, String nameNoExt) {
		LogAnalyzer la = new LogAnalyzer(logs);
		LinkedList<String[]> datas = la.analyze();
	
		// string to output as html
		StringBuilder sbHtml = new StringBuilder();
		
		sbHtml.append("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
		sbHtml.append("<head>\n");
		sbHtml.append("</head>\n");
		
		sbHtml.append("<body>\n");
		sbHtml.append("<div>\n");
		sbHtml.append("<br/>\n");
		sbHtml.append("<table id=\"resultTable\" border='1' cellspacing=\"0\">\n");
		
		sbHtml.append("<tr>\n");
		sbHtml.append("<td>&nbsp;&nbsp; Total Cases &nbsp;&nbsp;</td>\n");
		sbHtml.append("<td>");
		sbHtml.append("&nbsp;&nbsp;&nbsp;&nbsp;");
		sbHtml.append(la.getTotalCnt());
		sbHtml.append("&nbsp;&nbsp;&nbsp;&nbsp;");
		sbHtml.append("</td>\n");
		sbHtml.append("</tr>\n");
		
		sbHtml.append("<tr>\n");
		sbHtml.append("<td>&nbsp;&nbsp; Pass Cases &nbsp;&nbsp;</td>\n");
		sbHtml.append("<td>");
		sbHtml.append("&nbsp;&nbsp;&nbsp;&nbsp;");
		sbHtml.append(la.getTotalCnt()-la.getErrorCnt());
		sbHtml.append("&nbsp;&nbsp;&nbsp;&nbsp;");
		sbHtml.append("</td>\n");
		sbHtml.append("</tr>\n");
		
		sbHtml.append("<tr>\n");
		sbHtml.append("<td>&nbsp;&nbsp; Fail Cases &nbsp;&nbsp;</td>\n");
		sbHtml.append("<td>");
		sbHtml.append("&nbsp;&nbsp;&nbsp;&nbsp;");
		sbHtml.append(la.getErrorCnt());
		sbHtml.append("&nbsp;&nbsp;&nbsp;&nbsp;");
		sbHtml.append("</td>\n");
		sbHtml.append("</tr>\n");
		
		sbHtml.append("</table>\n");
		sbHtml.append("</div>\n");
		sbHtml.append("<br/><br/>\n");
		
		// detail table
		sbHtml.append("<div>\n");
		sbHtml.append("<table id=\"detailTable\" border='1' cellspacing=\"0\">\n");
		
		sbHtml.append("<tr>");
		String[] headers = la.getHeaders();
		for(String thd:headers) {
			sbHtml.append("<th>&nbsp;&nbsp;");
			sbHtml.append(thd);
			sbHtml.append("&nbsp;&nbsp;</th>\n");
		}
		sbHtml.append("</tr>");
		
		//append columns
		for( String[] data: datas) {
		    sbHtml.append("<tr>\n");
		        for(String cell:data){
		        	sbHtml.append("<td>&nbsp;&nbsp;");
		        	sbHtml.append(cell);
		        	sbHtml.append("&nbsp;&nbsp;</td>\n");
		        }
		    sbHtml.append("</tr>\n");
		}
				
		sbHtml.append("</table>\n");
		sbHtml.append("</div>\n");

		sbHtml.append("</body>\n");
		
		//output to a file
		try {
			File fout = new File(outPath+nameNoExt+".html");
			if(fout.exists())
				fout.delete();
			
			boolean bSuc = fout.createNewFile();
			if(bSuc) {
			    FileWriter fw = new FileWriter(fout);
			    fw.write(sbHtml.toString());
			    fw.close();
			}
		} 
		catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	private int getColumnIndexByName(String columnName) {
	    for(int i=0; i<mHeaders.length; i++) {
	    	if(columnName.equals(mHeaders[i]))
	    		return i;
	    }
	    return -1;
	}
	
	/**
	 * 
	 */
	private void redrawReportTable() {
		
		Object[][] datas = new Object[mData.length][mHeaders.length];
		int failColIndex = getColumnIndexByName("Status");
		int index = 0;
		for( Object[] data: mData) {
			if(mOnlyErrors){
				if(data[failColIndex].equals("FAIL"))
					datas[index++] = data;
			}
			else
				datas[index++] = data;
		}
		
		if(index == 0)
			return;
		
	    mTable.setModel( new ReportDataModel(datas,mHeaders) );
	
		setTableColumWidth();
	}
	
	private void setTableColumWidth() {
        int[] columnWidth = {50,10,80,300,10,250};
        int startIndex = MAXCOLUMNWIDTH-mHeaders.length;
        for(int i=0; i<mHeaders.length; i++) {
        	TableColumn nameColumn = mTable.getColumnModel().getColumn(i);
    		nameColumn.setPreferredWidth(columnWidth[i+startIndex]);
        }
	}
	

	public static void createReportWindow(String fileName, boolean isHtml) {
		
		File fobj = new File(fileName);
		if(!fobj.exists())
			return;
		
		StringBuilder sbCons = new StringBuilder();
		BufferedReader br = null;
		try {
			br = new BufferedReader(new FileReader(fobj));
			String line = null;
			
			while((line=br.readLine())!=null) {
				sbCons.append(line);
				sbCons.append("\n");
			}
			
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} finally{
			if(br!=null)
				try {
					br.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
		}
		
		if(sbCons.length()!=0) {
			ReportWindow rw = new ReportWindow();
			if(!isHtml)
				 rw.generateReport(sbCons.toString());
		    else { 
		    	 int lastSepIndex = fileName.lastIndexOf(File.separator);
		    	 int extIdx = fileName.indexOf('.', lastSepIndex);
		    	 String nameNoExt = "";
		    	 if(extIdx != -1)
		    		 nameNoExt = fileName.substring(lastSepIndex+1,extIdx);
		    	 else
		    		 nameNoExt = fileName.substring(lastSepIndex+1);
		    	 if(lastSepIndex != -1) {
		    		 rw.generateHtmlReport(sbCons.toString(), fileName.substring(0, lastSepIndex+1),nameNoExt);
		    	 } 
		    }
		}
		
	}//end
	
}

/**
 * 
 * @author wliping
 *
 */
class ReportDataModel extends AbstractTableModel {
	private static final long serialVersionUID = 1L;
	private Object[][] mData = null;
	public static String[] mHeader = null;
	
	public ReportDataModel(Object[][] data, String[] header) {
		mData =  data;
		mHeader = header;
	}
	
	public int getColumnCount() {
		if(mHeader == null)
			return 0;
		return mHeader.length;
	}

	public int getRowCount() {
		if(mData != null)
		  return mData.length;
		else
		  return 0;
	}

	public String getColumnName(int col) {
		if(mHeader==null)
			return "";
		return mHeader[col];
	}

	public Object getValueAt(int row, int col) {
		if(mData != null)
		    return mData[row][col];
		else
			return null;
	}

	public Class<? extends Object> getColumnClass(int c) {
		if(getValueAt(0, c) != null)
		    return getValueAt(0, c).getClass();
		else
			return null;
	}
	
}

class LogAnalyzer {
	
	private Scanner mScan =null;
	
	private LinkedList< String[] > mReportDatas = new LinkedList< String[] >();
	private int mErrorCases = 0;
	private int mTotalCases = 0;
	private ArrayList<String> mHeaders = new ArrayList<String>();
	
	public LogAnalyzer(String loginfo) {
		mScan = new Scanner(loginfo);
	}
	
	public int getTotalCnt() {
		return mTotalCases;
	}
	
	public int getErrorCnt() {
		return mErrorCases;
	}
	
	public String[]  getHeaders() {
		String[] objs = mHeaders.toArray( new String[mHeaders.size()]);
		return objs;
	}
	
	public LinkedList< String[] > analyze() {
		boolean bFirst = true;
		while(mScan.hasNext()) {
			String tLine = mScan.nextLine().trim();
			
			if(tLine.endsWith("=>Begin")) {
				
				// get DateTime and PID if have
				int pos = getPos4DatePID(tLine);
				int colWidth = 4;
				// Date/Time row[-1]
				String dateTime = null;
				// ProcID row[-2]
				String pid = null;
				if(pos != 0) {
					String[] dateTimePID = tLine.substring(0,pos).trim().split("  ");
					colWidth += dateTimePID.length;
					
					if(dateTimePID[0].indexOf("/")!=-1)
						dateTime = dateTimePID[0];
					else
						pid = dateTimePID[0];
					
					if(dateTimePID.length==2) 
						pid = dateTimePID[1];
					
					tLine = tLine.substring(pos);
				}//end if
				
				mTotalCases++;
				
				int idxLeft = tLine.indexOf('[');	
				int idxRight = tLine.indexOf(']');
				
				//caseName row[0]
				String caseName = tLine.substring(0,idxLeft);
				//desc row[1]
				String description = tLine.substring(idxLeft+1,idxRight);
				// status row[2]
				String status = "PASS";
				//Message row[3]
                String message = "OK";

				while(mScan.hasNext()) {
				    String infoLine = mScan.nextLine().trim();
				    if(pos!=0)
				    	infoLine = infoLine.substring(pos);
				    
				    if(infoLine.indexOf("TESTRESULT=>")!=-1) {
				    	message = infoLine.substring("TESTRESULT=>".length(), infoLine.length());
				    	status = "FAIL";
				    }
				    else if(infoLine.indexOf("=>Finish")!=-1)
				    	break;
				}//end while
				
				if(status.equals("FAIL")) {
					mErrorCases++;
				}
				
				// append row data
				String[] row = new String[colWidth];
				int colIndex = 0;
				if(dateTime != null) {
					row[colIndex++] = dateTime;
					if(bFirst)
						mHeaders.add("Date/Time");
				}
				
				if(pid != null) {
					row[colIndex++] = pid;
					if(bFirst)
					   mHeaders.add("Proc-ID");
				}
				
				row[colIndex++] = caseName;
				row[colIndex++] = description;
				row[colIndex++] = status;
				row[colIndex++] = message;
				
				if(bFirst) {
					mHeaders.add("Case Name");
					mHeaders.add("Description");
					mHeaders.add("Status");
					mHeaders.add("Message");
					bFirst = false;
				}
				
				mReportDatas.add(row);

			}//end if 
			
		}//end while
		
		return mReportDatas;
	}
	
	private int getPos4DatePID(String line) {
		int pos = 0;
		// xx/xx/xx xx:xx:xx
		if(line.indexOf("/")!=-1 && line.indexOf(":")!=-1) 
			pos = 17;
		
		// NNNNN or space
		while( Character.isSpaceChar(line.charAt(pos)) || Character.isDigit(line.charAt(pos)))
			pos++;
		
	    return pos;
	}
	
}

