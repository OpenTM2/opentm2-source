/*! \brief FifoQueue.H - Include file for the FifoQueue class
	Copyright (c) 1999-2012, International Business Machines Corporation and others. All rights reserved.
	Description: This file contains the class definition for the JSONize helper class
*/

#ifndef _FIFOQUEUE_H_
#define _FIFOQUEUE_H_

#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
//                    LOCK CLASS UTILITY                                      //
////////////////////////////////////////////////////////////////////////////////
class CSyncLock
{
public:
    virtual void lock() = 0;
	virtual void unlock() = 0;
	virtual bool tryLock() = 0;
};

class CThreadLock : public CSyncLock
{
public:
    CThreadLock()
	{
		InitializeCriticalSection(&cs);
	}

	~CThreadLock()
	{
        DeleteCriticalSection(&cs);
	}

	void lock()
	{
		EnterCriticalSection(&cs);
	}

	void unlock()
	{
        LeaveCriticalSection(&cs);
	}

    bool tryLock()
	{
        return TryEnterCriticalSection(&cs);		
	}


private:
    CRITICAL_SECTION cs;   
};


class CProcessLock : public CSyncLock
{
public:
	CProcessLock(std::string &name=std::string("")):mutexName(name+".lock")
	{
		hMutexSem = OpenMutex( MUTEX_ALL_ACCESS, TRUE, mutexName.c_str() );

		if ( hMutexSem == NULL) 
			hMutexSem = CreateMutex( NULL, FALSE, mutexName.c_str() );
	}

	~CProcessLock()
	{
		if ( hMutexSem )
        {  
			ReleaseMutex( hMutexSem ); 
			CloseHandle( hMutexSem ); 
		}
	}

	void lock()
    {
		if ( hMutexSem ) 
			WaitForSingleObject( hMutexSem, INFINITE ); 
    }

	bool tryLock()
	{
		if(hMutexSem)
		{
			if(WaitForSingleObject( hMutexSem, 100 ) == WAIT_OBJECT_0)
				return true;
		}
		return false;
	}

	void unlock()
	{
		if ( hMutexSem )
		{  
			ReleaseMutex( hMutexSem ); 
		}
	}

private:
	HANDLE hMutexSem;
	const std::string mutexName;
};


////////////////////////////////////////////////////////////////////////////////
//                 DISK   SHARE FILE CLASS UTILITY                            //
////////////////////////////////////////////////////////////////////////////////

class CSharedFiles
{
public:
	CSharedFiles(std::string &queName=std::string(""));
	~CSharedFiles(){}
	void write(const std::string &);
	void read(std::string &);
    bool isEmpty();
protected:
	std::string queuePath;
	std::string queueName;
	std::string fpath;

    void getMetas(const std::string &fpath,int &header, int &tail);
    void updateMetas(const std::string &fpath, const int &header, const int &tail);

	void init(std::string &);
};

class COutQueue : public CSharedFiles
{
public:
	COutQueue(std::string &queName=std::string("")):CSharedFiles(queName)
	{
		init(queuePath+".OUT");
	}
};

class CInQueue : public CSharedFiles
{
public:
	CInQueue(std::string &queName=std::string("")):CSharedFiles(queName)
	{
		init(queuePath+".IN");
	}

};

////////////////////////////////////////////////////////////////////////////////
//                 MEMORY   SHARE FILE CLASS UTILITY                          //
////////////////////////////////////////////////////////////////////////////////
class CSharedBuffer
{
public:
    const static int BUFFERSIZE=1024*1024;//1M
    virtual void write(std::string &) = 0;
	virtual void read(std::string &)  = 0;
	virtual ~CSharedBuffer(){}
};

class CSharedBuffer4Thread : public CSharedBuffer
{
public:
     CSharedBuffer4Thread( std::string &name=std::string("") ):syncFile(name){}

    void write(std::string &);
	void read(std::string &);
	
private:
    CThreadLock   Lock;
	COutQueue     syncFile;	
	std::string   cache; 
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//                    PROCESS COMMUNICATION                                                        //
////////////////////////////////////////////////////////////////////////////////////////////////////

class CUploadBuffer4Process
{
public:
	const static int BUFFERSIZE=1024*1024;//1M
	CUploadBuffer4Process(std::string &mutexName=std::string("")):Lock(mutexName),syncFile(mutexName),m_hFileHandle(NULL),m_pBuffer(NULL)
	{
	}
	
	virtual ~CUploadBuffer4Process()
	{
		if(m_pBuffer != NULL)
		    UnmapViewOfFile(m_pBuffer);

		if(m_hFileHandle != NULL)
            CloseHandle(m_hFileHandle);

	}
	
protected:
	CProcessLock  Lock;
	COutQueue     syncFile;
    HANDLE        m_hFileHandle;
    char*         m_pBuffer;
};


class CWriteToSharedBuffer:public CUploadBuffer4Process
{
public:

	CWriteToSharedBuffer(LPCTSTR lpszFileName):CUploadBuffer4Process(std::string(lpszFileName))
	{
		m_hFileHandle = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,BUFFERSIZE,lpszFileName);
		if(m_hFileHandle != NULL)
		{
		    m_pBuffer=(char*)MapViewOfFile(m_hFileHandle,FILE_MAP_ALL_ACCESS,0,0,BUFFERSIZE);
			if( m_pBuffer != NULL )
				*m_pBuffer = '\0';
		}
	}


	bool write(const std::string &inStr);

};

class CReadFromSharedBuffer:public CUploadBuffer4Process
{
public:

	CReadFromSharedBuffer(LPCTSTR lpszFileName):CUploadBuffer4Process(std::string(lpszFileName))
	{
		m_hFileHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,lpszFileName);
		if(m_hFileHandle != NULL)
		{
		    m_pBuffer=(char*)MapViewOfFile(m_hFileHandle,FILE_MAP_ALL_ACCESS,0,0,BUFFERSIZE);
		}
	}

	void read(std::string &outStr);
};
#endif // ifndef _FIFOQUEUE_H_
