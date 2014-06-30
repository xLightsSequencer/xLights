/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>

#include "spxmlutils.hpp"

//=========================================================

const int SP_XmlArrayList::LAST_INDEX = -1;

SP_XmlArrayList :: SP_XmlArrayList( int initCount )
{
	mMaxCount = initCount <= 0 ? 2 : initCount;
	mCount = 0;
	mFirst = (void**)malloc( sizeof( void * ) * mMaxCount );
}

SP_XmlArrayList :: ~SP_XmlArrayList()
{
	free( mFirst );
	mFirst = NULL;
}

int SP_XmlArrayList :: getCount() const
{
	return mCount;
}

int SP_XmlArrayList :: append( void * value )
{
	if( NULL == value ) return -1;

	if( mCount >= mMaxCount ) {
		mMaxCount = ( mMaxCount * 3 ) / 2 + 1;
		mFirst = (void**)realloc( mFirst, sizeof( void * ) * mMaxCount );
		assert( NULL != mFirst );
		memset( mFirst + mCount, 0, ( mMaxCount - mCount ) * sizeof( void * ) );
	}

	mFirst[ mCount++ ] = value;

	return 0;
}

void * SP_XmlArrayList :: takeItem( int index )
{
	void * ret = NULL;

	if( LAST_INDEX == index ) index = mCount -1;
	if( index < 0 || index >= mCount ) return ret;

	ret = mFirst[ index ];

	mCount--;

	if( ( index + 1 ) < mMaxCount ) {
		memmove( mFirst + index, mFirst + index + 1,
			( mMaxCount - index - 1 ) * sizeof( void * ) );
	} else {
		mFirst[ index ] = NULL;
	}

	return ret;
}

const void * SP_XmlArrayList :: getItem( int index ) const
{
	const void * ret = NULL;

	if( LAST_INDEX == index ) index = mCount - 1;
	if( index < 0 || index >= mCount ) return ret;

	ret = mFirst[ index ];

	return ret;
}

void SP_XmlArrayList :: sort( int ( * cmpFunc )( const void *, const void * ) )
{
	for( int i = 0; i < mCount - 1; i++ ) {
		int min = i;
		for( int j = i + 1; j < mCount; j++ ) {
			if( cmpFunc( mFirst[ min ], mFirst[ j ] ) > 0 ) {
				min = j;
			}
		}

		if( min != i ) {
			void * temp = mFirst[ i ];
			mFirst[ i ] = mFirst[ min ];
			mFirst[ min ] = temp;
		}
	}
}

//=========================================================

SP_XmlQueue :: SP_XmlQueue()
{
	mMaxCount = 8;
	mEntries = (void**)malloc( sizeof( void * ) * mMaxCount );

	mHead = mTail = mCount = 0;
}

SP_XmlQueue :: ~SP_XmlQueue()
{
	free( mEntries );
	mEntries = NULL;
}

void SP_XmlQueue :: push( void * item )
{
	if( mCount >= mMaxCount ) {
		mMaxCount = ( mMaxCount * 3 ) / 2 + 1;
		void ** newEntries = (void**)malloc( sizeof( void * ) * mMaxCount );

		unsigned int headLen = 0, tailLen = 0;
		if( mHead < mTail ) {
			headLen = mTail - mHead;
		} else {
			headLen = mCount - mTail;
			tailLen = mTail;
		}

		memcpy( newEntries, &( mEntries[ mHead ] ), sizeof( void * ) * headLen );
		if( tailLen ) {
			memcpy( &( newEntries[ headLen ] ), mEntries, sizeof( void * ) * tailLen );
		}

		mHead = 0;
		mTail = headLen + tailLen;

		free( mEntries );
		mEntries = newEntries;
	}

	mEntries[ mTail++ ] = item;
	mTail = mTail % mMaxCount;
	mCount++;
}

void * SP_XmlQueue :: pop()
{
	void * ret = NULL;

	if( mCount > 0 ) {
		ret = mEntries[ mHead++ ];
		mHead = mHead % mMaxCount;
		mCount--;
	}

	return ret;
}

void * SP_XmlQueue :: top()
{
	return mCount > 0 ? mEntries[ mHead ] : NULL;
}

//=========================================================

SP_XmlStringBuffer :: SP_XmlStringBuffer()
{
	init();
}

void SP_XmlStringBuffer :: init()
{
	mSize = 0;
	mMaxSize = 8;
	mBuffer = (char*)malloc( mMaxSize );
	memset( mBuffer, 0, mMaxSize );
}

SP_XmlStringBuffer :: ~SP_XmlStringBuffer()
{
	free( mBuffer );
}

int SP_XmlStringBuffer :: append( char c )
{
	if( mSize >= ( mMaxSize - 1 ) ) {
		mMaxSize += ( mMaxSize * 3 ) / 2 + 1;
		mBuffer = (char*)realloc( mBuffer, mMaxSize );
		assert( NULL != mBuffer );
		memset( mBuffer + mSize, 0, mMaxSize - mSize );
	}
	mBuffer[ mSize++ ] = c;

	return 0;
}

int SP_XmlStringBuffer :: append( const char * value, int size )
{
	if( NULL == value ) return -1;

	size = ( size <= 0 ? strlen( value ) : size );
	if( size <= 0 ) return -1;

	if( ( size + mSize ) > ( mMaxSize - 1 ) ) {
		mMaxSize += size;
		mBuffer = (char*)realloc( mBuffer, mMaxSize );
		assert( NULL != mBuffer );
		memset( mBuffer + mSize, 0, mMaxSize - mSize );
	}

	memcpy( mBuffer + mSize, value, size );
	mSize += size;

	return 0;
}

int SP_XmlStringBuffer :: getSize() const
{
	return mSize;
}

const char * SP_XmlStringBuffer :: getBuffer() const
{
	return mBuffer;
}

char * SP_XmlStringBuffer :: takeBuffer()
{
	char * ret = mBuffer;

	mBuffer = NULL;
	init();

	return ret;
}

void SP_XmlStringBuffer :: clean()
{
	memset( mBuffer, 0, mMaxSize );
	mSize = 0;
}

