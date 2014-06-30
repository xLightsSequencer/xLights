/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#ifndef __spxmlreader_hpp__
#define __spxmlreader_hpp__

class SP_XmlPullParser;
class SP_XmlPullEvent;
class SP_XmlStringBuffer;

class SP_XmlReader {
public:
	enum { MAX_READER = 16 };
	enum { ePI, eDocType, eSTag, eETag, ePCData,
		eCDataSection, eComment, eLBracket, eSign };

	/**
	 * @param  parser : act as reader's context
	 * @param  c : a char in xml stream
	 */
	virtual void read( SP_XmlPullParser * parser, char c ) = 0;

	/**
	 * reset reader state
	 */
	virtual void reset();

	/**
	 * convert internal xml string to event
	 * @return NULL : this reader don't generate any event or error occured
	 */
	virtual SP_XmlPullEvent * getEvent( SP_XmlPullParser * parser ) = 0;

protected:
	SP_XmlStringBuffer * mBuffer;

	friend class SP_XmlReaderPool;

	SP_XmlReader();
	virtual ~SP_XmlReader();

	/// help to call parser->changeReader
	void changeReader( SP_XmlPullParser * parser, SP_XmlReader * reader );

	/// help to call parser->getReader
	SP_XmlReader * getReader( SP_XmlPullParser * parser, int type );

	/// help to call parser->setError
	static void setError( SP_XmlPullParser * parser, const char * error );

private:
	SP_XmlReader( SP_XmlReader & );
	SP_XmlReader & operator=( SP_XmlReader & );
};

class SP_XmlPIReader : public SP_XmlReader {
public:
	SP_XmlPIReader();
	virtual ~SP_XmlPIReader();
	virtual void read( SP_XmlPullParser * parser, char c );
	virtual SP_XmlPullEvent * getEvent( SP_XmlPullParser * parser );

private:
	static SP_XmlPullEvent * parseDocDeclEvent( SP_XmlPullParser * parser,
			SP_XmlStringBuffer * buffer );
};

class SP_XmlStartTagReader : public SP_XmlReader {
public:
	SP_XmlStartTagReader();
	virtual ~SP_XmlStartTagReader();
	virtual void read( SP_XmlPullParser * parser, char c );
	virtual SP_XmlPullEvent * getEvent( SP_XmlPullParser * parser );
	virtual void reset();

private:
	int mIsQuot;
};

class SP_XmlEndTagReader : public SP_XmlReader {
public:
	SP_XmlEndTagReader();
	virtual ~SP_XmlEndTagReader();
	virtual void read( SP_XmlPullParser * parser, char c );
	virtual SP_XmlPullEvent * getEvent( SP_XmlPullParser * parser );
};

class SP_XmlPCDataReader : public SP_XmlReader {
public:
	SP_XmlPCDataReader();
	virtual ~SP_XmlPCDataReader();
	virtual void read( SP_XmlPullParser * parser, char c );
	virtual SP_XmlPullEvent * getEvent( SP_XmlPullParser * parser );
};

class SP_XmlCDataSectionReader : public SP_XmlReader {
public:
	SP_XmlCDataSectionReader();
	virtual ~SP_XmlCDataSectionReader();
	virtual void read( SP_XmlPullParser * parser, char c );
	virtual SP_XmlPullEvent * getEvent( SP_XmlPullParser * parser );
};

class SP_XmlCommentReader : public SP_XmlReader {
public:
	SP_XmlCommentReader();
	virtual ~SP_XmlCommentReader();
	virtual void read( SP_XmlPullParser * parser, char c );
	virtual SP_XmlPullEvent * getEvent( SP_XmlPullParser * parser );
};

class SP_XmlDocTypeReader : public SP_XmlReader {
public:
	SP_XmlDocTypeReader();
	virtual ~SP_XmlDocTypeReader();
	virtual void read( SP_XmlPullParser * parser, char c );
	virtual SP_XmlPullEvent * getEvent( SP_XmlPullParser * parser );
};

class SP_XmlLeftBracketReader : public SP_XmlReader {
public:
	SP_XmlLeftBracketReader();
	virtual ~SP_XmlLeftBracketReader();
	virtual void read( SP_XmlPullParser * parser, char c );
	virtual SP_XmlPullEvent * getEvent( SP_XmlPullParser * parser );
	virtual void reset();

private:
	int mHasReadBracket;
};

class SP_XmlSignReader : public SP_XmlReader {
public:
	SP_XmlSignReader();
	virtual ~SP_XmlSignReader();
	virtual void read( SP_XmlPullParser * parser, char c );
	virtual SP_XmlPullEvent * getEvent( SP_XmlPullParser * parser );
};

class SP_XmlReaderPool {
public:
	SP_XmlReaderPool();
	~SP_XmlReaderPool();
	SP_XmlReader * borrow( int type );
	void save( SP_XmlReader * reader );

private:
	SP_XmlReader ** mReaderList;
};

#endif

