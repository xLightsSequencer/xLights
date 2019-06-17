/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <typeinfo>

#include "spxmlstag.hpp"
#include "spxmlutils.hpp"
#include "spxmlevent.hpp"
#include "spxmlcodec.hpp"

SP_XmlSTagParser :: SP_XmlSTagParser( const char * encoding )
{
	mEvent = new SP_XmlStartTagEvent();
	mReader = new SP_XmlSTagNameReader();
	mStartTagName = new SP_XmlStringBuffer();
	mError = NULL;

	snprintf( mEncoding, sizeof( mEncoding ), "%s", encoding );
}

SP_XmlSTagParser :: ~SP_XmlSTagParser()
{
	if( NULL != mEvent ) delete mEvent;
	mEvent = NULL;

	if( NULL != mReader ) delete mReader;
	mReader = NULL;

	if( NULL != mStartTagName ) delete mStartTagName;
	mStartTagName = NULL;

	if( NULL != mError ) free( mError );
	mError = NULL;
}

const char * SP_XmlSTagParser :: getEncoding()
{
	return mEncoding;
}

SP_XmlStartTagEvent * SP_XmlSTagParser :: takeEvent()
{
	SP_XmlStartTagEvent * event = mEvent;

	mEvent = NULL;

	return event;
}

const char * SP_XmlSTagParser :: getError()
{
	return mError;
}

void SP_XmlSTagParser :: changeReader( SP_XmlSTagReader * reader )
{
	delete mReader;
	mReader = reader;
}

void SP_XmlSTagParser :: setError( const char * error )
{
	if( NULL != error ) {
		if( NULL != mError ) free( mError );
		mError = strdup( error );
	}
}

void SP_XmlSTagParser :: append( const char * source, int len )
{
	for( int i = 0; i < len && NULL == mError; i++ ) {
		mReader->read( this, source[ i ] );
	}
}

//=========================================================

SP_XmlSTagReader :: SP_XmlSTagReader()
{
	mBuffer = new SP_XmlStringBuffer();
}

SP_XmlSTagReader :: ~SP_XmlSTagReader()
{
	delete mBuffer;
	mBuffer = NULL;
}

void SP_XmlSTagReader :: changeReader( SP_XmlSTagParser * parser,
		SP_XmlSTagReader * reader )
{
	//printf( "\nchange: %s\n", typeid( *reader ).name() );
	parser->changeReader( reader );
}

void SP_XmlSTagReader :: setError( SP_XmlSTagParser * parser, const char * error )
{
	parser->setError( error );
}

void SP_XmlSTagReader :: setName( SP_XmlSTagParser * parser, const char * name )
{
	parser->mEvent->setName( name );
}

void SP_XmlSTagReader :: addAttrName( SP_XmlSTagParser * parser, const char * name )
{
	parser->mStartTagName->append( name );
}

void SP_XmlSTagReader :: addAttrValue( SP_XmlSTagParser * parser, const char * value )
{
	SP_XmlStringBuffer decodeValue;
	SP_XmlStringCodec::decode( parser->getEncoding(), value, &decodeValue );

	parser->mEvent->addAttr( parser->mStartTagName->getBuffer(), decodeValue.getBuffer() );
	parser->mStartTagName->clean();
}

//=========================================================

SP_XmlSTagNameReader :: SP_XmlSTagNameReader()
{
}

SP_XmlSTagNameReader :: ~SP_XmlSTagNameReader()
{
}

void SP_XmlSTagNameReader :: read( SP_XmlSTagParser * parser, char c )
{
	if( isspace( c ) ) {
		if( 0 == mBuffer->getSize() ) {
			//leading space, skip
		} else {
			setName( parser, mBuffer->getBuffer() );
			changeReader( parser, new SP_XmlSTagAttrNameReader() );
		}
	} else {
		mBuffer->append( c );
	}
}

//=========================================================

SP_XmlSTagAttrNameReader :: SP_XmlSTagAttrNameReader()
{
	mWait4Quot = 0;
}

SP_XmlSTagAttrNameReader :: ~SP_XmlSTagAttrNameReader()
{
}

void SP_XmlSTagAttrNameReader :: read( SP_XmlSTagParser * parser, char c )
{
	if( 1 == mWait4Quot ) {
		if( '"' == c ) {
			addAttrName( parser, mBuffer->getBuffer() );
			changeReader( parser, new SP_XmlSTagEqualMarkReader() );
		} else {
			mBuffer->append( c );
		}
	} else {
		if( isspace( c ) ) {
			if( 0 == mBuffer->getSize() ) {
				//leading space, skip
			} else {
				addAttrName( parser, mBuffer->getBuffer() );
				changeReader( parser, new SP_XmlSTagEqualMarkReader() );
			}
		} else {
			if( '"' == c && 0 == mBuffer->getSize() ) {
				mWait4Quot = 1;
			} else if( '=' == c ) {
				addAttrName( parser, mBuffer->getBuffer() );
				SP_XmlSTagReader * reader = new SP_XmlSTagEqualMarkReader();
				changeReader( parser, reader );
				reader->read( parser, c );
			} else {
				mBuffer->append( c );
			}
		}
	}
}

//=========================================================

SP_XmlSTagEqualMarkReader :: SP_XmlSTagEqualMarkReader()
{
}

SP_XmlSTagEqualMarkReader :: ~SP_XmlSTagEqualMarkReader()
{
}

void SP_XmlSTagEqualMarkReader :: read( SP_XmlSTagParser * parser, char c )
{
	if( isspace( c ) ) {
		//skip
	} else if( '=' == c ) {
		changeReader( parser, new SP_XmlSTagAttrValueReader() );
	} else {
		addAttrValue( parser, "" );
		SP_XmlSTagReader * reader = new SP_XmlSTagAttrNameReader();
		changeReader( parser, reader );
		reader->read( parser, c );

		//setError( parser, "miss '=' between name & value" );
	}
}

//=========================================================

SP_XmlSTagAttrValueReader :: SP_XmlSTagAttrValueReader()
{
	mHasReadQuot = 0;
}

SP_XmlSTagAttrValueReader :: ~SP_XmlSTagAttrValueReader()
{
}

void SP_XmlSTagAttrValueReader :: read( SP_XmlSTagParser * parser, char c )
{
	if( 0 == mHasReadQuot ) {
		if( isspace( c ) ) {
			//skip  
		} else if( '"' == c ) {
			mHasReadQuot = 1;
		} else if( '\'' == c ) {
			mHasReadQuot = 2;
		} else {
			setError( parser, "unknown attribute value start" );
		}
	} else {
		if( ( 1 == mHasReadQuot && '"' == c ) 
				|| ( 2 == mHasReadQuot && '\'' == c ) ) {
			addAttrValue( parser, mBuffer->getBuffer() );
			changeReader( parser, new SP_XmlSTagAttrNameReader() );
		} else {
			mBuffer->append( c );
		}
	}
}

