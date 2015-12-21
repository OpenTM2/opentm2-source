//+----------------------------------------------------------------------------+
//| OtmMemoryTool.cpp                                                          |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: liping                                                             |
//|(based on OtmCmm,OtmDmm, OtmTmCl, OtmRecM, OtmTmT by G.Queck)               |
//+----------------------------------------------------------------------------+
//|  This file contains the code for the memory related tools                  |
//+----------------------------------------------------------------------------+
#include "OtmMemoryTool.h"
#include "eqftm.h"
#include "OTMFUNC.H"
#include "core\memory\MemoryFactory.h"
#include "OtmProposal.h"
#include <eqfserno.h>             // version number
#include <algorithm>
#include <time.h>


/*******************************************************************************************************/
/*                             function decalare                                                       */
/*******************************************************************************************************/
static bool CheckCmdLine (int iArgc,char** ppArgv, OtmMemoryTool** ppMemTool);
static bool getTask(char * pCh, std::string& strTaskName);
static void showHelp();
static int MTListCompare( const void *p1, const void *p2 );
static bool isFileExisted(std::string dir, std::string fileName);

/*******************************************************************************************************/
/*                             bein main                                                               */
/*******************************************************************************************************/

int main( int argc, char *argv[], char *envp[] )
{
  envp;
  
  if ( argc==1 || (argc==2 && stricmp(argv[1],"-h")==0) )
  {
      showHelp();
      return 0;
  }
  
  // show logo
  printf("\nOtmMemoryTool - The OpenTM2 memory value change utility\n\n");

  // skip program name
  argc--;
  argv++;

  // start the batch session
  HSESSION hSession = 0L;
  unsigned short usRC = 0;
  
  usRC = EqfStartSession( &hSession );
  if ( usRC )
  {
      printf( "Error: Could not start TranslationManager session, rc = %u\n", usRC );
	  return usRC;
  }

 
  OtmMemoryTool *pMemTool = NULL;
  bool fOK = true;

  do
  {     
       // check commandline
       fOK = CheckCmdLine(argc,argv, &pMemTool);
       if(!fOK)
       {	
            if( pMemTool!=NULL && !pMemTool->getConfirmFlag() )
            {  
                if(pMemTool->getLastErrorMsg().empty())
                {
                    showHelp();
                    MessageBox(NULL,"Command line error, please check the help information","Error",MB_OK);
                }
            }
            else 
            {    
                if(pMemTool->getLastErrorMsg().empty())
                {
                    printf("\nCommand line error, please check the help information:\n\n");
                    showHelp();
                }
            }

            continue;
       }

       // open memory
      fOK = !pMemTool->open();
	  if (!fOK)
	      continue;
		  
	  // process for every segment
	  while(fOK)
	  {
	      fOK = pMemTool->executeTask();
	  }
	  
	  // report
	  std::vector<std::string> vec;
      pMemTool->getInfoDisplay(vec);
      std::reverse(vec.begin(),vec.end());
      printf("==================Result report==========================================\n");
	  for(std::size_t i=0; i<vec.size(); i++)
	  {
	      printf("%s\n\n",vec[i].c_str());
	  }
      printf("=========================================================================\n");
	  
  }while(fOK);
  
  if(!fOK)
  {    
      usRC = 1;
      std::string errorMsg = pMemTool->getLastErrorMsg();
      if(!errorMsg.empty())
      {   
          if( pMemTool!=NULL && !pMemTool->getConfirmFlag() )
              MessageBox(NULL,(char*)errorMsg.c_str(), "Error", MB_OK);
          else
              printf("%s\n",(char*)errorMsg.c_str());
      }
  }

  if (pMemTool != NULL)
  {
      pMemTool->close();
	  pMemTool = NULL;
  }
  
  if ( hSession != 0L )
  {
    EqfEndSession( hSession );
  }

  return( usRC );
} /* end of function main */

// Non-member functions
bool getTask(char * pCh, std::string& strTaskName)
{
    std::string strTemp(pCh);
    std::transform(strTemp.begin(), 
                   strTemp.end(), 
                   strTemp.begin(),
                   ::tolower);

    std::string strToFind("/task=");
    std::string::size_type idx = strTemp.find(strToFind);
    if(idx != std::string::npos)
    {
        strTaskName = strTemp.substr(idx+strToFind.length(),
                                     strTemp.length()-strToFind.length());
        return true;
    }
    return false;
}

bool CheckCmdLine
(
  int             iArgc,
  char**          ppArgv,
  OtmMemoryTool** ppMemTool
)
{
    std::vector<std::string> commandVec;
    bool fOK = false;

    if ( iArgc > 0 )
    {
        fOK = true;
        std::string strTaskName;
        while ( iArgc )
        {
            bool isTask = false;
            if(!isTask && strTaskName.empty())
                isTask = getTask(*(ppArgv),strTaskName);

            if(!isTask)
                commandVec.push_back(*(ppArgv));
            ppArgv++;
            iArgc--;
        } 

        // create specific task accroding to task name
        int i;
        for(i=0; i<TASK_NUM; i++)
        {
            if(stricmp(strTaskName.c_str(),Tasks[i].c_str())==0)
                break;
        }
        
        switch(i)
        {
        case 0:
            *ppMemTool = new DeleteMtProposal();
            break;
        case 1:
            *ppMemTool = new ReverseMemory();
            break;
        case 2:
            *ppMemTool = new DeleteIdenticalProposal();
            break;
        case 3:
            *ppMemTool = new ChgProposalMeta();
            break;
        default:
            fOK = false;
            break;
        }

        // call sub-class implementation
        if( *ppMemTool != NULL && fOK )
        {
            fOK = (*ppMemTool)->checkCommandLine(commandVec);
        }
    }//end if
   
    return fOK;
}


void showHelp()
{
    std::string strHelpInfo;
    strHelpInfo += "\n";
    strHelpInfo += "OtmMemoryTool.EXE : The OpenTM2 memory value change utility.\n";
    strHelpInfo += "Version           : "; strHelpInfo += STR_DRIVER_LEVEL_NUMBER; strHelpInfo += "\n";
    strHelpInfo += "Copyright         : ";strHelpInfo += STR_COPYRIGHT; strHelpInfo+="\n"; 
    strHelpInfo += "                    All rights reserved.\n";
    strHelpInfo += "Purpose           : Change parameters in OpenTM2 internal translation memories.\n";
    strHelpInfo += "Syntax format     : OtmMemoryTool /TASK=task [/OPTIONS]\n\n";

    strHelpInfo += "Operations:\n";
    strHelpInfo += "[1] Deleting MT proposals from a translation memory:\n";
    strHelpInfo += "    OtmMemoryTool /TASK=deleteMtProposal /MEM=memoryName [/TYPE=noconf]\n\n";

    strHelpInfo += "[2] Reversing segments in a translation memory:\n";
    strHelpInfo += "    OtmMemoryTool /TASK=reverseMemory /MEM=memoryName /REv=revMemoryName [/TYPE=noconf]\n\n";

    strHelpInfo += "[3] Deleting identical segments from a translation memory:\n";
    strHelpInfo += "    OtmMemoryTool /TASK=deleteIdentical /MEM=inputMemoryName /OUT=outputMemoryName [/TYPE=noconf]\n\n";

    strHelpInfo += "[4] Changing translation memory attributes:\n";
    strHelpInfo += "    OtmMemoryTool /TASK=ChgProposalMeta /MEM=memoryName \n";
    strHelpInfo += "    [/FROMMARKUP=oldMarkupTableName][/TOMARKUP=newMarkupTableName]\n";
    strHelpInfo += "    [/FROMLANG=oldLanguage][/TOLANG=newLanguage]\n";
    strHelpInfo += "    [/DATE=yyyy-mm-dd[hh:mm[:ss]]][/DOC=documentName][/TYPE=noconf]\n";
    strHelpInfo += "    [/CLEAR][/SET]\n\n";

    strHelpInfo += "Description of general options and parameters:\n";
    strHelpInfo += "    /MEM       |/ME  The name of the internal OpenTM2 translation memory.\n";
    strHelpInfo += "    /OUT       |/OU  Memory only containing segments with no duplicates.\n";
    strHelpInfo += "    /REV       |/RE  Memory containing the reversed segments.\n";
    strHelpInfo += "    /FROMMARKUP|/FM  The name of the markup table that must be changed.\n";
    strHelpInfo += "    /TOMARKUP  |/TM  The name of the new markup table.\n";
    strHelpInfo += "    /FROMLANG  |/FL  The name of the language that must be changed.\n";
    strHelpInfo += "    /TOLANG    |/TL  The name of the new language.\n";
    strHelpInfo += "    /DATE      |/DA  The new date, and the new time.\n";
    strHelpInfo += "    /DOC       |/DO  The document name of the segments to be changed.\n";
    strHelpInfo += "    /TYPE      |/TY  If set to \"noconf\", messages are supressed.\n";
    strHelpInfo += "    /CLEAR     |/CL  Clear the MT-flag in the memory.\n";
    strHelpInfo += "    /SET       |/SE  Set the MT-flag in the memory.\n";

    printf("========================Help Information=================================\n");
    printf("%s\n",strHelpInfo.c_str());
    printf("=========================================================================\n");
}


int MTListCompare( const void *p1, const void *p2 )
{
  PSZ psz1 = *((PSZ *)p1);
  PSZ psz2 = *((PSZ *)p2);

  return( strcmp( psz1, psz2 ) );
}


bool isFileExisted(std::string dir, std::string fileName)
{
    HDIR hDir = HDIR_CREATE;
    USHORT usCount = 1;	
    FILEFINDBUF ffb;
    std::string curDir = dir +"\\"+fileName;
    USHORT usDosRc = UtlFindFirst((PSZ) curDir.c_str(), 
                                  &hDir, 
                                  FILE_NORMAL, 
                                  &ffb,
                                  sizeof(ffb), 
                                  &usCount, 
                                  0L, 
                                  FALSE);
    if (hDir != 0)
    {
        UtlFindClose(hDir, FALSE);
    }
    
    if(usCount==1)
    {
        return true;
    }    
    
    // recursivly to search in sub-directory
    curDir = dir + "\\*";
    usCount = 1;
    usDosRc = UtlFindFirst((PSZ) curDir.c_str(), 
                           &hDir, 
                           FILE_DIRECTORY, 
                           &ffb,
                           sizeof(ffb), 
                           &usCount, 
                           0L, 
                           FALSE);

    while ( usCount == 1 )
    {
        if (ffb.cFileName[0] != '.')
        {
            std::string strSubdir(dir+"\\"+ ffb.cFileName);
            bool existed = isFileExisted(strSubdir,fileName);
            if(existed) return true;
        }
        usDosRc = UtlFindNext(hDir, &ffb, sizeof(ffb), &usCount, FALSE);
    }

    return false;
}

/*******************************************************************************************************/
/*                             member functions in OtmMemoryTool                                       */
/*******************************************************************************************************/

bool OtmMemoryTool::executeTask()
{
    bool fOK = true;
    OtmProposal prop;

    if(m_bFirstCall)
    {
        fOK = (m_pMem->getFirstProposal( prop ) == 0);
        m_bFirstCall = false;
    }
    else
    {
       fOK = (m_pMem->getNextProposal( prop ) == 0);
    }

    if ( fOK )
    {
        m_ulSegNo++;
        // call sub-class implementation
        fOK = doExecute(prop);
    }

    return fOK;
}

int OtmMemoryTool::open()
{
    int iRC = 0;
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    OtmMemory *pTempMem = pFactory->openMemory( NULL, (char*)m_strMemName.c_str(), NONEXCLUSIVE, &iRC );
    if(iRC != 0)
    {
        int iLastError = 0;
        pFactory->getLastError(NULL,iLastError,m_strLastError);
    }
    else
    {
        m_bFirstCall = true;
        m_pMem = std::shared_ptr<OtmMemory>(pTempMem);
    }

    // call subclass implementation
    if(iRC == 0) 
    {
        iRC = openTask(); 
    }

    return iRC;
}

bool OtmMemoryTool::close()
{
    if (m_pMem != NULL )
    {
        MemoryFactory *pFactory = MemoryFactory::getInstance();
        pFactory->closeMemory( m_pMem.get() );
    }
    // call subclass implementaion
    closeTask();

    return true;
}

BATCHCMD OtmMemoryTool::validateToken(const std::string& token, std::string& cmdParam)
{
    PCMDLIST pCmdList = getCommandList();//m_pCmdList;
    while (pCmdList->BatchCmd != BATCH_END)
    {
        int idx = token.find(pCmdList->szDesc);
        if(idx == 0)
        {
            cmdParam = token.substr(strlen(pCmdList->szDesc),
                                    token.length()-strlen(pCmdList->szDesc));
            break;
        }   
        else
        {
            idx = token.find(pCmdList->szShortCut);
            if(idx == 0)
            {
                 cmdParam = token.substr(strlen(pCmdList->szShortCut),
                                    token.length()-strlen(pCmdList->szShortCut));
                 break;
            }
        }
        pCmdList++;
    }

    return( pCmdList->BatchCmd );
}


bool OtmMemoryTool::checkCommandLine(std::vector<std::string>& commandVec)
{

    for(std::vector<std::string>::iterator iter = commandVec.begin();
        iter!= commandVec.end();
        iter++)
    {
        std::string strTemp(*iter);
        std::transform(strTemp.begin(), 
                strTemp.end(), 
                strTemp.begin(),
                ::toupper); 

        std::string commandPara;
        BATCHCMD cmd = validateToken(strTemp,commandPara);

        switch(cmd)
        {
        case BATCH_MEM:
            m_strMemName = commandPara;
            break;

        case BATCH_TYPE:
            {
             std::transform(commandPara.begin(), 
                           commandPara.end(), 
                           commandPara.begin(),
                           ::tolower); 

            if(commandPara == "noconf")
                m_bNoConfirm = true;
            }
            break;

        default:
            bool bEnd = (iter+1)==commandVec.end()?true:false;
            if(!checkParameter(commandPara, cmd, bEnd))
                return false;
            break;
        }
    }//end for
    return true;
}//end checkCommandLine

/*******************************************************************************************************/
/*                             member functions in ReverseMemory                                       */
/*******************************************************************************************************/

int ReverseMemory::openTask()
{
    int iRC = 0;
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    OtmMemory *pTempMem = pFactory->openMemory( NULL, (char*)m_strTgtMemName.c_str(), EXCLUSIVE, &iRC );
    if ( iRC != 0 )
    {
        char *pCh = (char*)m_strTgtMemName.c_str();
        int iLastError = 0;
        pFactory->getLastError(NULL,iLastError,m_strLastError);
    }
    else
    {
        m_pTgtMem  = std::shared_ptr<OtmMemory>(pTempMem);
    }

    return iRC;
}

bool ReverseMemory::closeTask()
{
    if ( m_pTgtMem != NULL ) 
    {   
        MemoryFactory *pFactory = MemoryFactory::getInstance();
        pFactory->closeMemory( m_pTgtMem.get());
    }
    return true;
}


bool ReverseMemory::checkParameter(std::string& param, const BATCHCMD& cmd, bool  bEnd)
{
    switch(cmd)
    {
    case BATCH_MEM:
    case BATCH_TYPE:
        break;

    case BATCH_REVMEM:
        m_bFlag = true;
        m_strTgtMemName = param;
        break;

    default:
        return false;
        break;
    }

    if(bEnd && (!m_bFlag || m_strMemName ==m_strTgtMemName) )
    {
        return false;
    }
    return true;
}

bool ReverseMemory::doExecute(OtmProposal& proposal)
{
    CHAR_W    szSource[MAX_SEGMENT_SIZE+1] = {0};  // proposal source
    CHAR_W    szTarget[MAX_SEGMENT_SIZE+1] = {0};  // proposal target
    CHAR      szSourceLang[256] = {0};             // proposal source language
    CHAR      szTargetLang[256] = {0};             // proposal target la

    proposal.getSource( szSource, sizeof(szSource) / sizeof(CHAR_W) );
    proposal.getTarget( szTarget, sizeof(szTarget) / sizeof(CHAR_W) );

    proposal.getSourceLanguage( szSourceLang, sizeof(szSourceLang) );
    proposal.getTargetLanguage( szTargetLang, sizeof(szTargetLang) );

    proposal.setSource( szTarget );
    proposal.setTarget( szSource );

    proposal.setSourceLanguage(szTargetLang );
    proposal.setTargetLanguage(szSourceLang );

    int iRC = m_pTgtMem->putProposal( proposal );

    if ( iRC==0 )
    {
         m_ulSuccSegNo++;
    }

    return iRC==0?true:false;
}

void ReverseMemory::getInfoDisplay(std::vector<std::string> & vec)
{
    std::string  strRevMemName("Reversed Memory   :  ");
    strRevMemName += m_strTgtMemName;
    strRevMemName += OTMMEMORYTOOL_SPACESTRING;
    vec.push_back(strRevMemName); 
    
    std::string  strSrcMemName("Translation Memory:  ");
    strSrcMemName += m_strMemName;
    strSrcMemName += OTMMEMORYTOOL_SPACESTRING;
    vec.push_back(strSrcMemName);
}

void ReverseMemory::initCommandList()
{
    const CMDLIST temp[] = {
        { BATCH_MEM,       "/MEM=",       "/ME="      },
        { BATCH_REVMEM,    "/REV=",       "/RE="      },
        { BATCH_TYPE,      "/TYPE=",      "/TY="      },
        { BATCH_END,       "",            "",         }
    };

    for(int i=0; i<sizeof(temp)/sizeof(temp[0]); i++)
    {
        m_pCmdList[i] = temp[i];
    }
}



/*******************************************************************************************************/
/*                             member functions in DeleteMtProposal                                    */
/*******************************************************************************************************/
bool DeleteMtProposal::doExecute(OtmProposal& proposal)
{
    if ( proposal.getType() == OtmProposal::eptMachine )
    {
        bool fOK = m_pMem->deleteProposal( proposal );
      
        if ( fOK )
        {
            (m_ulDelSegNo)++;
        }
        return fOK;
    }

    return true;
}

bool DeleteMtProposal::checkParameter(std::string& param, const BATCHCMD& cmd, bool  bEnd)
{
    return false;
}

void DeleteMtProposal::getInfoDisplay(std::vector<std::string> & vec)
{
    char szBuf[256] = {0};
    ltoa ( m_ulDelSegNo, szBuf, 10 );
    std::string strDelSegNo("Delete Segments     :  ");
    strDelSegNo += szBuf;
    vec.push_back(strDelSegNo);

    std::string  strSrcMemName("Translation Memory:  ");
    strSrcMemName += m_strMemName;
    strSrcMemName += OTMMEMORYTOOL_SPACESTRING;
    vec.push_back(strSrcMemName);
}


void DeleteMtProposal::initCommandList()
{
    const CMDLIST temp[]=
    {
       { BATCH_MEM,       "/MEM=",    "/ME="      },
       { BATCH_TYPE,      "/TYPE=",   "/TY="      },
       { BATCH_END,       "",         ""          }
    };

    for(int i=0; i<sizeof(m_pCmdList)/sizeof(m_pCmdList[0]); i++)
    {
        m_pCmdList[i] = temp[i];
    }
}

/*******************************************************************************************************/
/*                             member functions in DeleteIdentical                                     */
/*******************************************************************************************************/
int DeleteIdenticalProposal::openTask()
{
    int iRC = 0;
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    OtmMemory *pTempMem = pFactory->openMemory( NULL, (char*)m_strTgtMemName.c_str(), EXCLUSIVE, &iRC );
    if ( iRC != 0 )
    {
        char *pCh = (char*)m_strTgtMemName.c_str();
        int iLastError = 0;
        pFactory->getLastError(NULL,iLastError,m_strLastError);
    }
    else
    {
        m_pTgtMem  = std::shared_ptr<OtmMemory>(pTempMem);
    }

    return iRC;
}

bool DeleteIdenticalProposal::closeTask()
{
    if ( m_pTgtMem != NULL ) 
    {   
        MemoryFactory *pFactory = MemoryFactory::getInstance();
        pFactory->closeMemory( m_pTgtMem.get());
    }
    return true;
}

bool DeleteIdenticalProposal::doExecute(OtmProposal& proposal)
{   
    CHAR_W    szSource[MAX_SEGMENT_SIZE*2] = {0};
    CHAR_W    szTarget[MAX_SEGMENT_SIZE*2] = {0};

    CHAR_W    szNormSource[MAX_SEGMENT_SIZE*2] = {0};
    CHAR_W    szNormTarget[MAX_SEGMENT_SIZE*2] = {0};

    // get source and target strings
    proposal.getSource( szSource, sizeof(szSource) / sizeof(CHAR_W) );
    proposal.getTarget( szTarget, sizeof(szTarget) / sizeof(CHAR_W) );

    // Normalize strings 
    TmClNormalize( szNormSource, szSource );
    TmClNormalize( szNormTarget, szTarget );

    bool fSourceTargetIdentical = false;
    if ( UTF16strcmp( szNormSource, szNormTarget ) == 0 )
    {
       m_ulSkippedSegNo++;
       fSourceTargetIdentical = true;
    } /* endif */

    // put segment if it is not to be deleted
    if ( !fSourceTargetIdentical )
    {
      // Write the segment to the output translation memory
       m_pTgtMem->putProposal( proposal );
       m_ulSuccSegNo++;
    } 

    return true;
}

bool DeleteIdenticalProposal::checkParameter(std::string& param, const BATCHCMD& cmd, bool  bEnd)
{
    switch(cmd)
    {
    case BATCH_MEM:
    case BATCH_TYPE:
        break;

    case TMT_TOMU:
        m_strTgtMemName = param;
        break;

    default:
        return false;
        break;
    }

    return true;
}

void DeleteIdenticalProposal::getInfoDisplay(std::vector<std::string> & vec)
{
    char szBuf[256] = {0};
    
    ltoa ( m_ulSkippedSegNo, szBuf, 10 );
    std::string strSkipSegNo("Skipped Segments     :  ");
    strSkipSegNo += szBuf;
    vec.push_back(strSkipSegNo);

    ltoa ( m_ulSuccSegNo, szBuf, 10 );
    std::string strSuccSegNo("Written out Segments :  ");
    strSuccSegNo += szBuf;
    vec.push_back(strSuccSegNo);

    std::string  strSrcMemName("Translation Memory:  ");
    strSrcMemName += m_strMemName;
    strSrcMemName += OTMMEMORYTOOL_SPACESTRING;
    vec.push_back(strSrcMemName);
}

void DeleteIdenticalProposal::initCommandList()
{
    const CMDLIST temp[]=
    {
        { BATCH_MEM,       "/MEM=",        "/ME="      },
        { TMT_TOMU,        "/OUT=",        "/OU="      },
        { BATCH_TYPE,      "/TYPE=",   "/TY="          },
        { BATCH_END,       "",           ""            }
    };

    for(int i=0; i<sizeof(m_pCmdList)/sizeof(m_pCmdList[0]); i++)
    {
        m_pCmdList[i] = temp[i];
    }
}


void DeleteIdenticalProposal::TmClNormalize( wchar_t *pszNormString, wchar_t *pszSource )
{
  while ( *pszSource )
  {
    if ( (*pszSource == LF) || (*pszSource == CR) || (*pszSource == SPACE) )
    {
      *pszNormString++ = SPACE;
      while ( (*pszSource == LF) || (*pszSource == CR) || (*pszSource == SPACE) )
      {
        pszSource++;
      }
    }
    else
    {
      *pszNormString++ = *pszSource++;
    }
  }
  *pszNormString = EOS;
}

/*******************************************************************************************************/
/*                             member functions in ChgProposalMeta                                     */
/*******************************************************************************************************/

bool ChgProposalMeta::closeTask()
{
    return true;
}

bool ChgProposalMeta::doExecute(OtmProposal& proposal)
{
    bool fChangeMarkup = false;
    bool fChangeLang   = false;
    bool fChangeDate   = false;
    bool fDocMatch     = false;
    bool fMarkupMatch  = false;
    
    char  szBuf[MAX_SEGMENT_SIZE];
    bool fOK = true;

     // check if document name matches
    if ( !m_strDoc.empty() )
    {
     
      proposal.getDocName(szBuf, sizeof(szBuf) );
      if ( szBuf[0] == EOS)
      {
        proposal.getDocShortName( szBuf, sizeof(szBuf) );
      }
      UtlMatchStrings( szBuf, (char*)m_strDoc.c_str(), (PBOOL)&fDocMatch );
    }
    else
    {
      fDocMatch = true;
    } /* endif */

    // check if markup name matches
    proposal.getMarkup( szBuf, sizeof(szBuf) );
    if ( (!m_strFromMarkup.empty()) && (m_strToMarkup.empty()) )
    {
      UtlMatchStrings( szBuf, (char*)m_strFromMarkup.c_str(), (PBOOL)&fMarkupMatch );
    }
    else
    {
      fMarkupMatch = true;
    }

    // check for markup changes
    if ( (!m_strToMarkup.empty()) &&      // to markup specified and
         (m_strToMarkup!=szBuf) )         // not the current one
    {
      if ( m_strFromMarkup.empty())
      {
        // no specific from markup, so change markup anyway
        fChangeMarkup = true;
      }
      else
      {
        // change segment if current markup matches the search markup
        UtlMatchStrings( szBuf, (char*)m_strFromMarkup.c_str(), (PBOOL)&fChangeMarkup );
      }
    }

    // check for date changes
    if (!m_strDate.empty())
    {
      fChangeDate = true;
    } 

    // check for target language changes
    proposal.getTargetLanguage(szBuf, sizeof(szBuf) );
    if ( (!m_strToLang.empty()) &&                // to language specified and current
         (m_strToLang!=szBuf) )
    {
        if ( m_strFromLang.empty() )
        {
            // no specific from language, so change language anyway
            fChangeLang = TRUE;
        }
        else
        {
            // change segment if current lang matches the tolang
			fChangeLang = (m_strToLang!=m_strFromLang);
            //UtlMatchStrings( m_strToLang, (char*)m_strFromLang.c_str(), (PBOOL)&fChangeLang );
        }
    }

    // change segment if necessary
    if ( fDocMatch && fMarkupMatch && (fChangeLang || fChangeMarkup || fChangeDate || m_bClearMTFlag || m_bSetMTFlag) )
    {
        USHORT usUpdFlags = 0;

        if ( fChangeLang )
        {
           proposal.setTargetLanguage( (char*)m_strToLang.c_str() );
           m_ulLangSegs++;
           usUpdFlags |= OtmMemory::UPDATE_TARGLANG;
        } 

        if ( fChangeDate )
        {
           proposal.setUpdateTime( m_lNewDate);
           m_ulDateSegs++;
           usUpdFlags |= OtmMemory::UPDATE_DATE;
        }

        if ( fChangeMarkup )
        {
            proposal.setMarkup( (char*)m_strToMarkup.c_str() );
            m_ulMarkupSegs++;
            usUpdFlags |= OtmMemory::UPDATE_MARKUP;
        }

        if ( m_bSetMTFlag  && proposal.getType() == OtmProposal::eptManual )
        {
            proposal.setType( OtmProposal::eptMachine );
            (m_ulSetSegNo)++;
            usUpdFlags |= OtmMemory::UPDATE_MTFLAG;
        }

        if( m_bClearMTFlag  && proposal.getType() == OtmProposal::eptMachine )
        {
            proposal.setType( OtmProposal::eptManual );
            (m_ulClearSegNo)++;
            usUpdFlags |= OtmMemory::UPDATE_MTFLAG;
        }

        fOK = (m_pMem->updateProposal( proposal, usUpdFlags ) == 0 );

    }//end if
     
    return fOK;
}


bool ChgProposalMeta::checkParameter(std::string& param, const BATCHCMD& cmd, bool  bEnd)
{
    bool fOK = true;

    switch(cmd)
    {
    case BATCH_MEM:
    case BATCH_TYPE:
        break;

    case TMT_FROMMU:
        m_strFromMarkup = param;
        break;

    case TMT_TOMU:
        m_strToMarkup = param;
        if(!m_strToMarkup.empty())
        {
            // check if markup table exists
            CHAR      chPathBuffer[MAX_EQF_PATH];
            UtlQueryString( QST_PLUGINPATH, chPathBuffer, sizeof( chPathBuffer ));
            UtlMakeEQFPath( chPathBuffer, NULC, PLUGIN_PATH, NULL );

            std::string strMarkupPath(chPathBuffer);
            std::string strFileName(m_strToMarkup);
            strFileName += EXT_OF_FORMAT;
            if(!isFileExisted(strMarkupPath,strFileName))
            {
                PSZ pszParm = (char*)m_strToMarkup.c_str();
                m_strLastError = std::string("The specified markup table ")+
                                 std::string(pszParm)+
                                 std::string(" is not available");
                fOK = false;
            }
        }
        break;

    case  TMT_FROMLANG:
        m_strFromLang = param;
        break;

    case  TMT_TOLANG:
        m_strToLang = param;
        if(!m_strToLang.empty())
        {
            // check if target language is valid
            if ( !isValidLanguage( (PSZ)m_strToLang.c_str(), FALSE ) )
            {
                PSZ pszErrParm = (char*)m_strToLang.c_str();
                m_strLastError = std::string("The specified language ")+
                                 std::string(pszErrParm)+
                                 std::string(" is not valid or the associated language file is missing or corrupted");

                fOK = false;
            }
        } 
                        
        break;

    case  TMT_DATE:
        m_strDate = param;
        // check entered date 
        if (m_strDate=="*")
        {
            UtlTime(&m_lNewDate );
        }
        else
        {
            struct tm tm_timedate;
            PSZ pszTemp = (char*)m_strDate.c_str(); 
            memset(&tm_timedate, 0, sizeof(tm_timedate));
                           
            if ( isdigit( *pszTemp ) )
            {
                int iYear = atoi( pszTemp );
                while ( isdigit(*pszTemp) ) pszTemp++;
                tm_timedate.tm_year = iYear-1900;
            }
            else
            {
                fOK = FALSE;
            } /* endif */

            if ( (*pszTemp == '-') || (*pszTemp == '/') )
            {
                pszTemp++;
            }
            else
            {
                fOK = FALSE;
            } /* endif */

            if ( isdigit( *pszTemp ) )
            {
                int iMonth = atoi( pszTemp );
                while ( isdigit(*pszTemp) ) pszTemp++;
                tm_timedate.tm_mon  = iMonth-1;
            }
            else
            {
                fOK = FALSE;
            } /* endif */
            if ( (*pszTemp == '-') || (*pszTemp == '/') )
            {
                pszTemp++;
            }
            else
            {
                fOK = FALSE;
            } /* endif */

            if ( isdigit( *pszTemp ) )
            {
                int iDay = atoi( pszTemp );
                while ( isdigit(*pszTemp) ) pszTemp++;
                tm_timedate.tm_mday = iDay;
            }
            else
            {
                fOK = FALSE;
            } /* endif */

            while ( *pszTemp == ' ' ) pszTemp++;

            if ( *pszTemp )
            {
                if ( isdigit( *pszTemp ) )
                {
                    int iHour = atoi( pszTemp );
                    while ( isdigit(*pszTemp) ) pszTemp++;
                    tm_timedate.tm_hour = iHour;
                }
                else
                {
                    fOK = FALSE;
                } /* endif */

                if ( *pszTemp == ':' )
                {
                    pszTemp++;
                }
                else
                {
                    fOK = FALSE;
                } /* endif */

                if ( isdigit( *pszTemp ) )
                {
                    int iMinutes = atoi( pszTemp );
                    while ( isdigit(*pszTemp) ) pszTemp++;
                    tm_timedate.tm_min = iMinutes;
                }
                else
                {
                    fOK = FALSE;
                } /* endif */

                if ( *pszTemp == ':' )
                {
                    pszTemp++;
                    if ( isdigit( *pszTemp ) )
                    {
                    int iSecs = atoi( pszTemp );
                    while ( isdigit(*pszTemp) ) pszTemp++;
                    tm_timedate.tm_sec = iSecs;
                    }
                    else
                    {
                    fOK = FALSE;
                    } /* endif */
                } /* endif */
            } /* endif */

            if ( fOK )
            {
                m_lNewDate = (long) mktime(&tm_timedate);
                m_lNewDate -= 10800; // To become compliant with OS/2 time format
            }
            else
            {
                return false;
            }
        }
        break;

    case  TMT_DOC:
        m_strDoc = param;
        break;

    case  BATCH_CLEAR:
        m_bClearMTFlag = true;
        if(m_bSetMTFlag)
            fOK = FALSE;
        break;

    case  BATCH_SET:
        m_bSetMTFlag = true;
        if(m_bClearMTFlag)
            fOK = FALSE;
        break;

    default:
        fOK = FALSE;
        break;
    }

    return fOK;
}


void ChgProposalMeta::getInfoDisplay(std::vector<std::string> & vec)
{
    char szBuf[256] = {0};
    ltoa ( m_ulSetSegNo, szBuf, 10 );
    std::string strSetSegNo(    "Set Segments             :  ");
    strSetSegNo += szBuf;
    vec.push_back(strSetSegNo);

    memset(szBuf, 0, sizeof(szBuf));
    ltoa(m_ulClearSegNo, szBuf,10);
    std::string strClearSegNo(  "Clear Segments           :  ");
    strClearSegNo += szBuf;
    vec.push_back(strClearSegNo);

    ltoa ( m_ulMarkupSegs, szBuf, 10 );
    std::string strMarkupSegNo( "Changed markups          :  ");
    strMarkupSegNo += szBuf;
    vec.push_back(strMarkupSegNo);

    ltoa ( m_ulLangSegs, szBuf, 10 );
    std::string strLangSegNo(   "Changed Languages        :  ");
    strLangSegNo += szBuf;
    vec.push_back(strLangSegNo);

    ltoa ( m_ulDateSegs, szBuf, 10 );
    std::string strChangedSegNo("Changed Segment dates    :  ");
    strChangedSegNo += szBuf;
    vec.push_back(strChangedSegNo);

    std::string  strSrcMemName( "Translation Memory       :  ");
    strSrcMemName += m_strMemName;
    strSrcMemName += OTMMEMORYTOOL_SPACESTRING;
    vec.push_back(strSrcMemName);
}

void ChgProposalMeta::initCommandList()
{
    const CMDLIST temp[]=
    {
      { BATCH_MEM,       "/MEM=",        "/ME="      },
      { TMT_FROMMU,      "/FROMMARKUP=", "/FM="      },
      { TMT_TOMU,        "/TOMARKUP=",   "/TM="      },
      { TMT_FROMLANG,    "/FROMLANG=",   "/FL="      },
      { TMT_TOLANG,      "/TOLANG=",     "/TL="      },
      { TMT_DOC,         "/DOC=",        "/DO="      },
      { TMT_DATE,        "/DATE=",       "/DA="      },   
      { BATCH_TYPE,      "/TYPE=",       "/TY="      },
      {BATCH_CLEAR,      "/CLEAR",       "/CL"       },
      {BATCH_SET,        "/SET",         "/SE"       },
      { BATCH_END,       "",           ""            }
    };

    for(int i=0; i<sizeof(m_pCmdList)/sizeof(m_pCmdList[0]); i++)
    {
        m_pCmdList[i] = temp[i];
    }
}
