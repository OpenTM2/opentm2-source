//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
package com.otm.util;

public class SimpleBase64 {

	
	static private char[] ENCODETABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=".toCharArray();
	
	static private byte[] DECODETABLE = new byte[256];
	static {
		for (int i = 0; i < 256; i++)
			DECODETABLE[i] = -1;
		for (int i = 'A'; i <= 'Z'; i++)
			DECODETABLE[i] = (byte) (i - 'A');
		for (int i = 'a'; i <= 'z'; i++)
			DECODETABLE[i] = (byte) (26 + i - 'a');
		for (int i = '0'; i <= '9'; i++)
			DECODETABLE[i] = (byte) (52 + i - '0');
		DECODETABLE['+'] = 62;
		DECODETABLE['/'] = 63;
	}
	
	static public char[] encode(byte[] orgStr) {
		
		char[] encodedStr = new char[((orgStr.length + 2) / 3) * 4];

		for (int i = 0, index = 0; i < orgStr.length; i += 3, index += 4) {
			boolean quad = false;
			boolean trip = false;
			int val = (0xFF & (int) orgStr[i]);
			val <<= 8;
			if ((i + 1) < orgStr.length) {
				val |= (0xFF & (int) orgStr[i + 1]);
				trip = true;
			}
			val <<= 8;
			if ((i + 2) < orgStr.length) {
				val |= (0xFF & (int) orgStr[i + 2]);
				quad = true;
			}
			encodedStr[index + 3] = ENCODETABLE[(quad ? (val & 0x3F) : 64)];
			val >>= 6;
			
			encodedStr[index + 2] = ENCODETABLE[(trip ? (val & 0x3F) : 64)];
			val >>= 6;
			
			encodedStr[index + 1] = ENCODETABLE[val & 0x3F];
			val >>= 6;
			
			encodedStr[index + 0] = ENCODETABLE[val & 0x3F];
		}
		return encodedStr;
	}

	
	static public byte[] decode(char[] data) {
		int len = ((data.length + 3) / 4) * 3;
		if (data.length > 0 && data[data.length - 1] == '=')
			--len;
		if (data.length > 1 && data[data.length - 2] == '=')
			--len;
		
		byte[] decodedStr = new byte[len];
		int shift = 0;
		int accum = 0;
		int index = 0;
		for (int ix = 0; ix < data.length; ix++) {
			int value = DECODETABLE[data[ix] & 0xFF];
			if (value >= 0) {
				accum <<= 6;
				shift += 6;
				accum |= value;
				if (shift >= 8) {
					shift -= 8;
					decodedStr[index++] = (byte) ((accum >> shift) & 0xff);
				}
			}
		}
		
		if (index != decodedStr.length)
			throw new Error("length wrong");
		
		return decodedStr;
	}

	

	

	public static void main(String[] args) throws Exception {
		String strSrc = "Cualquier referencia incluida en esta información a sitios web que no sean de IBM sólo se proporciona para su comodidad y en ningún modo constituye una aprobación de dichos sitios web.";
		String strOut = new String(SimpleBase64.encode(strSrc.getBytes()));
		System.out.println(strOut);

		String strOut2 = new String(SimpleBase64.decode(strOut.toCharArray()));
		System.out.println(strOut2);
	}
	
	
}