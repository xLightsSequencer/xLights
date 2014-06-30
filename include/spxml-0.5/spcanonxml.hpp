/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#ifndef __spcanonxml_hpp__
#define __spcanonxml_hpp__

class SP_XmlNode;
class SP_XmlStringBuffer;
class SP_XmlDocDeclNode;
class SP_XmlDocTypeNode;

/// XML Canonical, defined by James Clark.
class SP_CanonXmlBuffer {
public:
	SP_CanonXmlBuffer( const SP_XmlNode * node );
	~SP_CanonXmlBuffer();

	const char * getBuffer() const;
	int getSize() const;

private:
	SP_CanonXmlBuffer( SP_CanonXmlBuffer & );
	SP_CanonXmlBuffer & operator=( SP_CanonXmlBuffer & );

	static void dump( const SP_XmlNode * node,
			SP_XmlStringBuffer * buffer );
	static void dumpElement( const SP_XmlNode * node,
			SP_XmlStringBuffer * buffer );

	static void canonEncode( const char * value,
			SP_XmlStringBuffer * buffer );

	SP_XmlStringBuffer * mBuffer;
};

#endif

