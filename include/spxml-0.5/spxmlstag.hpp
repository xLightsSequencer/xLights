/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#ifndef __spxmlstag_hpp__
#define __spxmlstag_hpp__

class SP_XmlSTagReader;
class SP_XmlStartTagEvent;
class SP_XmlStringBuffer;

class SP_XmlSTagParser {
public:
	SP_XmlSTagParser( const char * encoding );
	~SP_XmlSTagParser();

	void append( const char * source, int len );

	SP_XmlStartTagEvent * takeEvent();
	const char * getError();

	const char * getEncoding();

protected:
	void changeReader( SP_XmlSTagReader * reader );
	void setError( const char * error );

	SP_XmlStartTagEvent * mEvent;

	SP_XmlStringBuffer * mStartTagName;

	friend class SP_XmlSTagReader;

private:
	SP_XmlSTagReader * mReader;
	char * mError;
	char mEncoding[ 32 ];
};

class SP_XmlSTagReader {
public:
	SP_XmlSTagReader();
	virtual ~SP_XmlSTagReader();
	virtual void read( SP_XmlSTagParser * parser, char c ) = 0;

protected:

	/// help to call parser->xxx
	void changeReader( SP_XmlSTagParser * parser, SP_XmlSTagReader * reader );
	void setError( SP_XmlSTagParser * parser, const char * error );

	/// help to call parser->mEvent->xxx
	void setName( SP_XmlSTagParser * parser, const char * name );
	void addAttrName( SP_XmlSTagParser * parser, const char * name );
	void addAttrValue( SP_XmlSTagParser * parser, const char * value );

	SP_XmlStringBuffer * mBuffer;
};

class SP_XmlSTagNameReader : public SP_XmlSTagReader {
public:
	SP_XmlSTagNameReader();
	virtual ~SP_XmlSTagNameReader();
	virtual void read( SP_XmlSTagParser * parser, char c );
};

class SP_XmlSTagAttrNameReader : public SP_XmlSTagReader {
public:
	SP_XmlSTagAttrNameReader();
	virtual ~SP_XmlSTagAttrNameReader();
	virtual void read( SP_XmlSTagParser * parser, char c );

private:
	int mWait4Quot;
};

class SP_XmlSTagEqualMarkReader : public SP_XmlSTagReader {
public:
	SP_XmlSTagEqualMarkReader();
	virtual ~SP_XmlSTagEqualMarkReader();
	virtual void read( SP_XmlSTagParser * parser, char c );
};

class SP_XmlSTagAttrValueReader : public SP_XmlSTagReader {
public:
	SP_XmlSTagAttrValueReader();
	virtual ~SP_XmlSTagAttrValueReader();
	virtual void read( SP_XmlSTagParser * parser, char c );

private:
	int mHasReadQuot;
};

#endif

