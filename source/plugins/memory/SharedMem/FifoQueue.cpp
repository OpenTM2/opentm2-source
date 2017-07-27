/*! \brief FifoQueue.CPP - Simple First-In-First-Out queue implementation
	Copyright (c) 1999-2017, International Business Machines Corporation and others. All rights reserved.
	Description: This module contains functions to wrap parameters in the JSON format and to retrieve parameters from JSON strings
*/

#include <shlobj.h>
#include <io.h>
#include <direct.h>
#include "FifoQueue.h"

// path within application data for queue data
#define APPLDATA_PATH "\\OpenTM2\\QueueData\\"

//
static int MkMultDirHwnd( char *pszPath );
static std::string& getQueuePath(std::string &queuePath);

///////////////////////////////////////////////////////////////////////////////////////////////////
//                       CSharedFiles                                                            //
//////////////////////////////////////////////////////////////////////////////////////////////////
CSharedFiles::CSharedFiles(std::string &queName):queueName(queName),fpath("")
{
	std::string rootPath;
	rootPath = getQueuePath(rootPath);
	queuePath = rootPath+queueName;
}

void CSharedFiles::init(std::string &path)
{
	fpath = path+"\\";
	if ( _access( fpath.c_str(), 0 ) == -1 )
	{
		MkMultDirHwnd( const_cast<char*>(fpath.c_str()) );
		updateMetas(fpath+"metas.DAT",1,1);
	}
	else
	{
		// need to delete the already existed folder?
	}
}

void CSharedFiles::write(const std::string &outStr)
{
	int header,tail;
	//std::string fpath = createFilePathName();
	getMetas(fpath+std::string("metas.DAT"),header,tail);

	char tailBuf[10+1]={'\0'};
	itoa(tail,tailBuf,10);
	
	std::string outname(fpath+tailBuf+".DAT");
    FILE *fout = fopen( outname.c_str(), "w" );
	if(fout)
	{
		fputs( outStr.c_str(), fout );
		fflush(fout);
		fclose(fout);
		updateMetas(fpath+"metas.DAT",header,++tail);
	}
}

void CSharedFiles::read(std::string &inStr)
{
	int header,tail;
	getMetas(fpath+"metas.DAT",header,tail);
	if(header == tail)
		return;

	char headerBuf[10+1]={'\0'};
	itoa(header,headerBuf,10);

	std::string inName(fpath+headerBuf+".DAT");
	FILE *fin = fopen( inName.c_str(), "r" );
	if(fin)
	{
		//
		fseek(fin,0,SEEK_END);
		int len=ftell(fin);
		rewind(fin);
		//
		std::string line(len+1,' ');
		fread(&(line[0]),1,len,fin);
		inStr += line;
		fclose(fin);
		//
		DeleteFile(inName.c_str());
		updateMetas(fpath+"metas.DAT",++header,tail);
	}
}

bool CSharedFiles::isEmpty()
{
    int header,tail;
    getMetas(fpath+"metas.DAT",header,tail);
    return (header==tail);
}

void CSharedFiles::getMetas(const std::string &fpath, int &header, int &tail)
{
	header = 0,tail = 0;

	FILE *fin = fopen( fpath.c_str(), "r" );
	if(fin)
	{
		std::string headerLine(100+1,' ');
		if( fgets( &(headerLine[0]),100,fin) )
		{
			header = atoi(headerLine.c_str());
		}

		std::string tailLine(100+1,' ');
		if( fgets( &(tailLine[0]),100,fin) )
		{
			tail = atoi(tailLine.c_str());
		}
		fclose(fin);
	}
}

void CSharedFiles::updateMetas(const std::string &fpath, const int &header, const int &tail)
{
	FILE *fout = fopen( fpath.c_str(), "w" );
	if(fout)
	{
		char temp[100+1] = {'\0'};
		itoa(header,temp,10);
		fputs( temp, fout );
		fputs( "\n", fout );

		*temp = '\0';
		itoa(tail,temp,10);
		fputs( temp, fout );
		fputs( "\n", fout );

		fflush(fout);
		fclose(fout);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//        CSharedBuffer4Thread                                                                   //
//////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedBuffer4Thread::write(std::string &strIn)
{
    Lock.lock();
      
	cache += strIn;
    if( cache.size() > BUFFERSIZE)
	{
        syncFile.write(cache);
		cache.clear();
	}

	Lock.unlock();
}

void CSharedBuffer4Thread::read(std::string &strOut)
{
	Lock.lock();

	syncFile.read(strOut);
	if( strOut.empty() )
	{
	    strOut.assign(cache);
	    cache.clear();
	}
	
	Lock.unlock();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//        CWriteToSharedBuffer                                                                   //
//////////////////////////////////////////////////////////////////////////////////////////////////
bool CWriteToSharedBuffer::write(const std::string &inStr)
{
	Lock.lock();

	
	if(m_pBuffer != NULL)
	{
		bool bFull = BUFFERSIZE <= (strlen(m_pBuffer)+inStr.size()+1)*sizeof(char);
		if(bFull)
		{
			syncFile.write(std::string(m_pBuffer)+inStr);
			*m_pBuffer = '\0';
		}
		else
		{
		    strncat(m_pBuffer,inStr.c_str(),BUFFERSIZE-(strlen(m_pBuffer)+1)*sizeof(char));
			m_pBuffer[BUFFERSIZE-1] = '\0';
		}
	}

	Lock.unlock();
	return true;
}

void CReadFromSharedBuffer::read(std::string &outStr)
{
	Lock.lock();

	std::string resStr;
	syncFile.read(resStr);
	if( !resStr.empty() )
	{
		outStr += resStr;
	}
	else 
	{
		if(m_pBuffer!=NULL)
		{
			outStr += std::string(m_pBuffer);
			*m_pBuffer = '\0';
		}
	}
	Lock.unlock();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  NON MEMBER FUNCTION                                                                 //
/////////////////////////////////////////////////////////////////////////////////////////

std::string& getQueuePath(std::string &queuePath)
{
	queuePath = "";
    char path[ MAX_PATH ];
    if ( SHGetFolderPathA( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path ) == S_OK )
    {
      queuePath = path;
      queuePath += APPLDATA_PATH;
    }
    else
    {
      return queuePath;
    }
    return queuePath;
}

int MkMultDirHwnd( char *pszPath )
{
   PSZ   pszPathEnd;                   // ptr to current end of path
   int iRC = 0;                        // function return value
   char szPath[512];

   strcpy( szPath, pszPath );
   pszPathEnd = strchr( szPath, '\\' );
   while ( pszPathEnd && !iRC )
   {
      if ( *(pszPathEnd+1) == 0  )     // at terminating backslash ...
      {
         pszPathEnd = NULL;            // ... force end of loop
      }
      else                             // else create directory
      {
         pszPathEnd = strchr( pszPathEnd+1, '\\' );
         if ( pszPathEnd ) *pszPathEnd = 0;
          
         _mkdir( szPath );

         if ( pszPathEnd ) *pszPathEnd = '\\';
      } 
   }
   return( iRC );
}

