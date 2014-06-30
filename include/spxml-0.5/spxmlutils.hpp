/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#ifndef __spxmlutils_hpp__
#define __spxmlutils_hpp__

#include <stdio.h>

typedef struct tagSP_XmlArrayListNode SP_XmlArrayListNode_t;

class SP_XmlArrayList {
public:
	static const int LAST_INDEX;

	SP_XmlArrayList( int initCount = 2 );
	virtual ~SP_XmlArrayList();

	int getCount() const;
	int append( void * value );
	const void * getItem( int index ) const;
	void * takeItem( int index );
	void sort( int ( * cmpFunc )( const void *, const void * ) );

private:
	SP_XmlArrayList( SP_XmlArrayList & );
	SP_XmlArrayList & operator=( SP_XmlArrayList & );

	int mMaxCount;
	int mCount;
	void ** mFirst;
};

class SP_XmlQueue {
public:
	SP_XmlQueue();
	virtual ~SP_XmlQueue();

	void push( void * item );
	void * pop();
	void * top();

private:
	void ** mEntries;
	unsigned int mHead;
	unsigned int mTail;
	unsigned int mCount;
	unsigned int mMaxCount;
};

class SP_XmlStringBuffer {
public:
	SP_XmlStringBuffer();
	virtual ~SP_XmlStringBuffer();
	int append( char c );
	int append( const char * value, int size = 0 );
	int getSize() const;
	const char * getBuffer() const;
	char * takeBuffer();
	void clean();

private:
	SP_XmlStringBuffer( SP_XmlStringBuffer & );
	SP_XmlStringBuffer & operator=( SP_XmlStringBuffer & );

	void init();

	char * mBuffer;
	int mMaxSize;
	int mSize;	
};

#ifdef WIN32

#define snprintf _snprintf
#define strncasecmp strnicmp
#define strcasecmp  stricmp
#endif

#endif

