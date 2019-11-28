package com.android.systemui2.utils;

import android.util.Log;

import java.util.Map;
import java.util.List;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.HashMap;

import java.io.File;
import java.io.IOException;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;
import org.xmlpull.v1.XmlPullParserException;

import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.nio.charset.StandardCharsets;

public class XMLUtils {
	private static final boolean DEBUG = true;
	private static final String TAG = "XMLUtils";
	
	private static XMLUtils _this = null;
	private boolean isError = false;
	private Map<String, List<String>> mSystemUIPolicy = null;

	private static final String TAG_ROOT_ASSETS   = "xml_assets";
    private static final String TAG_GROUP         = "group";
    private static final String TAG_ASSET         = "asset";

	private static final String ATTR_GROUP_NAME    = "name";
	
	public static final String ATTR_ASSET_CHOOSE   = "choose";
	public static final String ATTR_ASSET_PACKAGE  = "package";
	public static final String ATTR_ASSET_FUNCTION = "function";
	
	private static final String CONFIG = "/system/etc/xml/policySystemUI2.xml";

	public static XMLUtils getInstance() {
		if(null == _this) {
			synchronized(XMLUtils.class) {
				try {
					if(null == _this) {
						_this = new XMLUtils(CONFIG);
					}	
				} catch (Exception ex) {
					Log.e(TAG, "XMLUtils getInstance error.", ex);
				}
			}
		}
		
		return _this;
	}

	public XMLUtils(String sourcePath) {
		File xml = new File(sourcePath);
		if (!xml.exists()) {
			isError = true;
		}
	}

	public static boolean checkMatch(List<String> source, String param) {
	    if (source != null && !source.isEmpty() && param != null) {			
			for (String member : source) {
				if (member.equals("ALL") || param.contains(member))
					return true;
				else
					continue;
			}
	    }

		return false;
	}
	
	public static boolean checkStrictfpMatch(List<String> source, String param) {
		if (source != null && !source.isEmpty() && param != null) { 		
			for (String member : source) {
				if (member.equals("ALL") || param.equals(member))
					return true;
				else
					continue;
			}
		}
		
		return false;
	}
	
	public List<String> getProp(String group, String asset) {
		if (!isError) {
			try {
				XmlPullParser parser = XmlPullParserFactory.newInstance().newPullParser();
				parser.setInput(new FileInputStream(CONFIG), StandardCharsets.UTF_8.name());
			
				int eventType = parser.getEventType();
				do {
					switch (eventType) {
					case XmlPullParser.START_DOCUMENT: 
						dumpi("start parse document!.");
						break;
					case XmlPullParser.START_TAG: 
						String startTagName = parser.getName();
					    switch (startTagName) {
						case TAG_GROUP:
							String groupName = parser.getAttributeValue(null, ATTR_GROUP_NAME);
							if (group.equals(groupName))
								return parserGroupGetAssetList(parser, asset);
							else
							    dumpi("This group("+ groupName + ") is not need parser, jump!.");
						    break;
							
						default:
						    break;
						}
						
						break;
						
		    		case XmlPullParser.END_TAG:
						break;
					}

					eventType = parser.next();
				} while (eventType != XmlPullParser.END_DOCUMENT);
			} catch (XmlPullParserException e) {
				Log.w(TAG, "XML parser exception reading xml assets", e);
			} catch (IOException e) {
				Log.w(TAG, "I/O exception reading xml assets", e);
			}
		}
		
	    dumpe("Abort: maybe group '" + group + "' not exist!.");
		return null;
	}
	
	private List<String> parserGroupGetAssetList(XmlPullParser parser, String AssetName) {
	    try {
			int eventType = parser.getEventType();
			String groupName = parser.getAttributeValue(null, ATTR_GROUP_NAME);
		
			List<String> outList = new ArrayList<String>();
			dumpi("read " + TAG_GROUP + " start.");
			dumpi("---groupName: " + groupName);

			do {
				switch (eventType) {
				case XmlPullParser.START_TAG:
					String asset= parser.getName();
					if (asset.equals(TAG_ASSET)) {
						String attr = parser.getAttributeValue(null, AssetName);
					    if (outList.indexOf(attr) == -1) {
							dumpi("----asset attr: " + attr);
							outList.add(attr);
						}
					}
			        break;
				default:
				    break;
				}
				
				eventType = parser.next();
			} while (eventType != XmlPullParser.END_TAG || !TAG_GROUP.equals(parser.getName()));
			
	        dumpi("read " + TAG_GROUP + " end.");			
			return outList;
		} catch (XmlPullParserException e) {
			Log.w(TAG, "XML parser exception reading xml assets", e);
		} catch (IOException e) {
			Log.w(TAG, "I/O exception reading xml assets", e);
		} 

		return null;
	}

	private static void dumpi(String oops) {
	    if (DEBUG) {
			Log.i(TAG, oops);
		}
	}
	
	private static void dumpe(String oops) {
		Log.i(TAG, oops);
	}
}
