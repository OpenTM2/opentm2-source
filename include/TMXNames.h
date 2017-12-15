// Copyright (c) 2017, International Business Machines
// Corporation and others.  All rights reserved.
//
/*! \file
	Description: tables to map TMX names to OpenTM2 names

	Copyright Notice:

	Copyright (C) 2017, International Business Machines
	Corporation and others. All rights reserved
*/

// structure 
typedef struct _NAMETABLE
{
  char *pszName;
  char *pszValue;
} NAMETABLE, *PNAMETABLE;

// Table for the conversion of the TranslationManager language name to the language name used in TMX
NAMETABLE Language_Tmgr2TMX[] =
{
  { "English(U.S.)","en-US" },
  { "English(U.K.)","en-UK" },
  { "German(national)","de-DE" },
  { "German(newnat)","de-DE" },
  { "German(DPAnat)","de-DE" },
  { "German(reform)","de-DE" },
  { "German(Swiss)","de-CH" },
  { "French(national)","fr-FR" },
  { "French(Canadian)","fr-CA" },
  { "Italian","it" },
  { "Spanish","es" },
  { "Dutch(restrictive)","nl" },
  { "Dutch(permissive)","nl" },
  { "Portuguese(Brasil)","pt-BR" },
  { "Portuguese(nat.)","pt-PT" },
  { "Catalan","ca" },
  { "Danish","da" },
  { "Icelandic","is" },
  { "Norwegian(bokmal)","no_BO" },
  { "Norwegian(nynorsk)","no" },
  { "Swedish","sv" },
  { "Greek","el" },
  { "Afrikaans","af" },
  { "Finnish","fi" },
  { "Polish","pl" },
  { "Hungarian","hu" },
  { "Czech","cs" },
  { "Turkish","tr" },
  { "Russian","ru" },
  { "Japanese","ja" },
  { "Chinese(simpl.)","zh" },
  { "Chinese(trad.)","zh-tw" },
  { "Korean","ko" },
  { "Australian","en-AU" },
  { "Arabic","ar" },
  { "Hebrew","he" },
  { "Romanian","ro" },
  { "Slovene","sl" },
  { "Croatian","hr" },
  { "Slovakian","sk" },
  { "Lithuanian","lt" },
  { "Latvian","lv" },
  { "Estonian","et" },
  { "Thai","th" },
  { "Bulgarian","bg" },
  { "Macedonian","mk" },
  { "Serbian(Cyrillic)","sr" },
  { "Belarusian","br" },
  { "Ukrainian","uk" },
  { "Indonesian","id" },
  { "Malay","ms" },
  { "Vietnamese","vi-VN" },
  { "Basque","eu" },
  { "Azerbaijani(Latin)","az-La" },
  { "Welsh","cy-GB" },
  { NULL, NULL } };


// TMX language to translationmanager language
//
// first the complete langguage string is checked against the list
// if there is no match the langage part is checked against the list
// if there is still no match "Other Languages" is used as language name
NAMETABLE Language_TMX2Tmgr[] = {
  { "en-US","English(U.S.)" },
  { "en-UK","English(U.K.)" },
  { "en-GB","English(U.K.)" },
  { "de-DE","German(reform)" },
  { "de-CH","German(Swiss)" },
  { "fr-FR","French(national)" },
  { "fr-CA","French(Canadian)" },
  { "pt-BR","Portuguese(Brasil)" },
  { "pt-PT","Portuguese(nat.)" },
  { "en-AU","Australian" },
  { "en","English(U.S.)" },
  { "de","German(reform)" },
  { "fr","French(national)" },
  { "pt","Portuguese(nat.)" },
  { "it","Italian" },
  { "es","Spanish" },
  { "nl","Dutch(restrictive)" },
  { "nl","Dutch(permissive)" },
  { "ca","Catalan" },
  { "da","Danish" },
  { "is","Icelandic" },
  { "nb-NO","Norwegian(bokmal)" },
  { "no","Norwegian(bokmal)" },
  { "no","Norwegian(nynorsk)" },
  { "sv","Swedish" },
  { "el","Greek" },
  { "af","Afrikaans" },
  { "fi","Finnish" },
  { "pl","Polish" },
  { "hu","Hungarian" },
  { "cs","Czech" },
  { "tr","Turkish" },
  { "ru","Russian" },
  { "ja","Japanese" },
  { "zh","Chinese(simpl.)" },
  { "zh-tw","Chinese(trad.)" },
  { "ko","Korean" },
  { "ar","Arabic" },
  { "he","Hebrew" },
  { "ro","Romanian" },
  { "sl","Slovene" },
  { "hr","Croatian" },
  { "sk","Slovakian" },
  { "lt","Lithuanian" },
  { "lv","Latvian" },
  { "et","Estonian" },
  { "th","Thai" },
  { "bg","Bulgarian" },
  { "mk","Macedonian" },
  { "sr","Serbian(Cyrillic)" },
  { "br","Belarusian" },
  { "uk","Ukrainian" },
  { "id","Indonesian" },
  { "ms","Malay" },
  { "vi-VN","Vietnamese" },
  { "eu","Basque" },
  { "az-LA","Azerbaijani(Latin)" },
  { "az-AZ","Azerbaijani(Latin)" },
  { "cy-GB","Welsh" },
  { NULL, NULL } };


NAMETABLE Markup2Type[] = {
  { "OTMHTM32","html" },
  { "OTMANSI","plaintext" },
  { "OTMASCII","plaintext" },
  { "OTMRTF","rtf" },
  { "OTMXML","xml" },
// "alptext" = WinJoust data.
// "cdf" = Channel Definition Format.
// "cmx" = Corel CMX Format.
// "cpp" = C and C++ style text.
// "hptag" = HP-Tag.
// "java" = Java, source and property files.
// "javascript" = JavaScript, ECMAScript scripts.
// "lisp" = Lisp.
// "opentag" = OpenTag data.
// "pascal" = Pascal, Delphi style text.
// "pm" = PageMaker.
// "sgml" = SGML.
// "stf-f" = S-Tagger for FrameMaker.
// "stf-i" = S-Tagger for Interleaf.
// "transit" = Transit data.
// "vbscript" = Visual Basic scripts.
// "winres" = Windows resources from RC, DLL, EXE.
// "xptag" = Quark XPressTag.
  { NULL, NULL } };

NAMETABLE Type2Markup[] = {
  { "html","OTMHTM32" },
  { "plaintext","OTMANSI" },
  { "rtf","OTMRTF" },
  { "xml","OTMXML" },
  { NULL, NULL } };
