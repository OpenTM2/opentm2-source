/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef AUTOLOCK_H
#define AUTOLOCK_H

#include <windows.h>

/*! \brief a thread lock which is easier to use.
 */
class ThreadLock{

public:

	/*! \brief Constructor */
	ThreadLock() { InitializeCriticalSection(&m_cs); }

	/*! \brief Destructor */
	~ThreadLock() { DeleteCriticalSection(&m_cs); }

	/*! \brief lock the resource.
	 */
	void lock() { EnterCriticalSection(&m_cs); }

	/*! \brief unlock the resource.
	 */
	void unlock() { LeaveCriticalSection(&m_cs); }

private:

	/*! \brief a critical section that is used to perform lock and unlock operation	 */
	CRITICAL_SECTION m_cs;
};

/*! \brief make the lock above easier to use in a function.
 */
template <typename T>
class Autolock {

public:

		/*! \brief Constructor to perform lock operation */
	Autolock(T& vLockable) : m_lock(vLockable) { m_lock.lock(); }

		/*! \brief Desstructor to perform unlock operation */
	~Autolock() { m_lock.unlock(); }

private:

	/*! \brief a lock that is used to implement lock and unlock operation.	 */
	T& m_lock;
};
#endif