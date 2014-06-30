/*
 * Copyright 2008 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#ifndef __spxmlhandle_hpp__
#define __spxmlhandle_hpp__

class SP_XmlNode;

class SP_XmlElementNode;
class SP_XmlCDataNode;
class SP_XmlCommentNode;

/**
 *  This class is a clone of TinyXML's TiXmlHandle class.
 *
 *
 *  A SP_XmlHandle is a class that wraps a node pointer with null checks; this is
 *  an incredibly useful thing. Note that SP_XmlHandle is not part of the SPXml
 *  DOM structure. It is a separate utility class.
 *

	Take an example:
	@verbatim
	<Document>
		<Element attributeA = "valueA">
			<Child attributeB = "value1" />
			<Child attributeB = "value2" />
		</Element>
	<Document>
	@endverbatim

	Assuming you want the value of "attributeB" in the 2nd "Child" element, a
	SP_XmlHandle checks for null pointers so it is perfectly safe and correct to use:

	@verbatim
	SP_XmlHandle rootHandle( parser.getDocument()->getRootElement() );
	SP_XmlElementNode * child2 = rootHandle.getChild( "Element" )
			.getChild( "Child", 1 ).toElement();

	if( child2 ) {
		// do something
	}
	@endverbatim
 *
 */

class SP_XmlHandle {
public:
	SP_XmlHandle( SP_XmlNode * node );
	SP_XmlHandle( const SP_XmlHandle & ref );
	SP_XmlHandle & operator=( const SP_XmlHandle & ref );

	~SP_XmlHandle();

	SP_XmlHandle getChild( const char * name, int index = 0 ) const;

	SP_XmlHandle getChild( int index ) const;

	SP_XmlHandle getElement( int index ) const;

	SP_XmlNode * toNode();

	SP_XmlElementNode * toElement();

	SP_XmlCDataNode * toCData();

private:
	SP_XmlNode * mNode;
};

#endif

