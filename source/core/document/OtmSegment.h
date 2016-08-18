/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMSEGMENT_H_
#define _OTMSEGMENT_H_

/*! \brief Abstract base-class for segment objects */
class __declspec(dllexport) OtmSegment
{

public:

/*! \brief Constructor */
	OtmSegment(int iID) {iSegmentID = iID};

/*! \brief Destructor */
	virtual ~OtmSegment() {};

/*! \brief Get the segment ID
	\returns ID of the segment. */
	virtual int getSegmentID() {return iSegmentID;};

/*! \brief Get the source segment
	\returns pointer to source segment. */
	virtual char *getSource() {return pszSource;};

/*! \brief Get the target segment
	\returns pointer to target segment. */
	virtual char *getTarget() {return pszTarget;};

private:

/*! \brief ID of the document-object. */
	int iSegmentID;

	char *pszSource;
	char *pszTarget;

};

#endif // #ifndef _OTMSEGMENT_H_
 