/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <string.h>
#include <stdlib.h>

#include "spxmlevent.hpp"
#include "spxmlutils.hpp"

SP_XmlPullEvent :: SP_XmlPullEvent( int eventType )
	: mEventType( eventType )
{
}

SP_XmlPullEvent :: ~SP_XmlPullEvent()
{
}

int SP_XmlPullEvent :: getEventType()
{
	return mEventType;
}

//=========================================================

SP_XmlPullEventQueue :: SP_XmlPullEventQueue()
{
	mQueue = new SP_XmlQueue();
}

SP_XmlPullEventQueue :: ~SP_XmlPullEventQueue()
{
	for( void * item = mQueue->pop(); NULL != item; item = mQueue->pop() ) {
		delete (SP_XmlPullEvent*)item;
	}

	delete mQueue;
}

void SP_XmlPullEventQueue :: enqueue( SP_XmlPullEvent * event )
{
	mQueue->push( event );
}

SP_XmlPullEvent * SP_XmlPullEventQueue :: dequeue()
{
	SP_XmlPullEvent * event = (SP_XmlPullEvent*)mQueue->pop();
	return event;
}

//=========================================================

SP_XmlStartDocEvent :: SP_XmlStartDocEvent()
	: SP_XmlPullEvent( eStartDocument )
{
}

SP_XmlStartDocEvent :: ~SP_XmlStartDocEvent()
{
}

//=========================================================

SP_XmlEndDocEvent :: SP_XmlEndDocEvent()
	: SP_XmlPullEvent( eEndDocument )
{
}

SP_XmlEndDocEvent :: ~SP_XmlEndDocEvent()
{
}

//=========================================================

SP_XmlDocTypeEvent :: SP_XmlDocTypeEvent()
	: SP_XmlPullEvent( eDocType )
{
	memset( mName, 0, sizeof( mName ) );
	memset( mSystemID, 0, sizeof( mSystemID ) );
	memset( mPublicID, 0, sizeof( mPublicID ) );
	memset( mDTD, 0, sizeof( mDTD ) );
}

SP_XmlDocTypeEvent :: ~SP_XmlDocTypeEvent()
{
}

void SP_XmlDocTypeEvent :: setName( const char * name )
{
	strncpy( mName, name, sizeof( mName ) - 1 );
}

const char * SP_XmlDocTypeEvent :: getName() const
{
	return mName;
}

void SP_XmlDocTypeEvent :: setSystemID( const char * systemID )
{
	strncpy( mSystemID, systemID, sizeof( mSystemID ) - 1 );
}

const char * SP_XmlDocTypeEvent :: getSystemID() const
{
	return mSystemID;
}

void SP_XmlDocTypeEvent :: setPublicID( const char * publicID )
{
	strncpy( mPublicID, publicID, sizeof( mPublicID ) - 1 );
}

const char * SP_XmlDocTypeEvent :: getPublicID() const
{
	return mPublicID;
}

void SP_XmlDocTypeEvent :: setDTD( const char * dtd )
{
	strncpy( mDTD, dtd, sizeof( mDTD ) - 1 );
}

const char * SP_XmlDocTypeEvent :: getDTD() const
{
	return mDTD;
}

//=========================================================

SP_XmlPIEvent :: SP_XmlPIEvent()
	: SP_XmlPullEvent( ePI )
{
	memset( mTarget, 0, sizeof( mTarget ) );
	mData = NULL;
}

SP_XmlPIEvent :: ~SP_XmlPIEvent()
{
	if( NULL != mData ) free( mData );
	mData = NULL;
}

void SP_XmlPIEvent :: setTarget( const char * target )
{
	snprintf( mTarget, sizeof( mTarget ), "%s", target );
}

const char * SP_XmlPIEvent :: getTarget()
{
	return mTarget;
}

void SP_XmlPIEvent :: setData( const char * data, int len )
{
	if( NULL != data ) {
		if( NULL != mData ) free( mData );
		mData = (char*)malloc( len + 1 );
		memcpy( mData, data, len );
		mData[ len ] = '\0';
	}
}

const char * SP_XmlPIEvent :: getData()
{
	return mData;
}

//=========================================================

SP_XmlDocDeclEvent :: SP_XmlDocDeclEvent()
	: SP_XmlPullEvent( eDocDecl )
{
	memset( mVersion, 0, sizeof( mVersion ) );
	memset( mEncoding, 0, sizeof( mEncoding ) );
	mStandalone = -1;
}

SP_XmlDocDeclEvent :: ~SP_XmlDocDeclEvent()
{
}

void SP_XmlDocDeclEvent :: setVersion( const char * version )
{
	strncpy( mVersion, version, sizeof( mVersion ) -1 );
}

const char * SP_XmlDocDeclEvent :: getVersion() const
{
	return mVersion;
}

void SP_XmlDocDeclEvent :: setEncoding( const char * encoding )
{
	strncpy( mEncoding, encoding, sizeof( mEncoding ) -1 );
}

const char * SP_XmlDocDeclEvent :: getEncoding() const
{
	return mEncoding;
}

void SP_XmlDocDeclEvent :: setStandalone( int standalone )
{
	mStandalone = standalone;
}

int SP_XmlDocDeclEvent :: getStandalone() const
{
	return mStandalone;
}

//=========================================================

SP_XmlStartTagEvent :: SP_XmlStartTagEvent()
	: SP_XmlPullEvent( eStartTag )
{
	mName = NULL;
	mAttrNameList = new SP_XmlArrayList();
	mAttrValueList = new SP_XmlArrayList();
}

SP_XmlStartTagEvent :: ~SP_XmlStartTagEvent()
{
	if( NULL != mName ) free( mName );
	mName = NULL;

	int i = 0;

	for( i = 0; i < mAttrNameList->getCount(); i++ ) {
		free( (char*)mAttrNameList->getItem( i ) );
	}

	delete mAttrNameList;
	mAttrNameList = NULL;

	for( i = 0; i < mAttrValueList->getCount(); i++ ) {
		free( (char*)mAttrValueList->getItem( i ) );
	}
	delete mAttrValueList;
	mAttrValueList = NULL;
}

void SP_XmlStartTagEvent :: setName( const char * name )
{
	if( NULL != name ) {
		if( NULL != mName ) free( mName );
		mName = strdup( name );
	}
}

const char * SP_XmlStartTagEvent :: getName() const
{
	return mName;
}

void SP_XmlStartTagEvent :: addAttr( const char * name, const char * value )
{
	if( NULL != name ) mAttrNameList->append( strdup( name ) );
	if( NULL != value ) mAttrValueList->append( strdup( value ) );
}

const char * SP_XmlStartTagEvent :: getAttrValue( const char * name ) const
{
	const char * ret = NULL;

	for( int i = 0; i < mAttrNameList->getCount(); i++ ) {
		if( 0 == strcmp( name, (char*)mAttrNameList->getItem( i ) ) ) {
			ret = (char*)mAttrValueList->getItem( i );
			break;
		}
	}

	return ret;
}

int SP_XmlStartTagEvent :: getAttrCount() const
{
	return mAttrNameList->getCount();
}

const char * SP_XmlStartTagEvent :: getAttr( int index, const char ** value ) const
{
	const char * name = (char*)mAttrNameList->getItem( index );
	if( NULL != name && NULL != value ) *value = (char*)mAttrValueList->getItem( index );

	return name;
}

void SP_XmlStartTagEvent :: removeAttr( const char * name )
{
	int index = -1;

	for( int i = 0; i < mAttrNameList->getCount(); i++ ) {
		if( 0 == strcmp( name, (char*)mAttrNameList->getItem( i ) ) ) {
			index = i;
			break;
		}
	}

	if( index >= 0 ) {
		free( mAttrNameList->takeItem( index ) );
		free( mAttrValueList->takeItem( index ) );
	}
}

//=========================================================

SP_XmlTextEvent :: SP_XmlTextEvent( int eventType )
	: SP_XmlPullEvent( eventType )
{
	mText = NULL;
}

SP_XmlTextEvent :: ~SP_XmlTextEvent()
{
	if( NULL != mText ) free( mText );
	mText = NULL;
}

void SP_XmlTextEvent :: setText( const char * text, int len )
{
	if( NULL != text ) {
		if( NULL != mText ) free( mText );
		mText = (char*)malloc( len + 1 );
		memcpy( mText, text, len );
		mText[ len ] = '\0';
	}
}

const char * SP_XmlTextEvent :: getText() const
{
	return mText;
}

//=========================================================

SP_XmlEndTagEvent :: SP_XmlEndTagEvent()
	: SP_XmlTextEvent( eEndTag )
{
}

SP_XmlEndTagEvent :: ~SP_XmlEndTagEvent()
{
}

//=========================================================

SP_XmlCDataEvent :: SP_XmlCDataEvent()
	: SP_XmlTextEvent( eCData )
{
}

SP_XmlCDataEvent :: ~SP_XmlCDataEvent()
{
}

//=========================================================

SP_XmlCommentEvent :: SP_XmlCommentEvent()
	: SP_XmlTextEvent( eComment )
{
}

SP_XmlCommentEvent :: ~SP_XmlCommentEvent()
{
}

