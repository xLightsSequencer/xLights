/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <string.h>

#include "spcanonxml.hpp"

#include "spxmlnode.hpp"
#include "spxmlutils.hpp"
#include "spxmlcodec.hpp"

SP_CanonXmlBuffer :: SP_CanonXmlBuffer( const SP_XmlNode * node )
{
	mBuffer = new SP_XmlStringBuffer();
	dump( node, mBuffer );
}

SP_CanonXmlBuffer :: ~SP_CanonXmlBuffer()
{
	if( NULL != mBuffer ) delete mBuffer;
	mBuffer = NULL;
}

const char * SP_CanonXmlBuffer :: getBuffer() const
{
	return mBuffer->getBuffer();
}

int SP_CanonXmlBuffer :: getSize() const
{
	return mBuffer->getSize();
}

void SP_CanonXmlBuffer :: canonEncode( const char * value,
		SP_XmlStringBuffer * buffer )
{
	SP_XmlStringBuffer temp;
	SP_XmlStringCodec::encode( "", value, &temp );

	for( const char * pos = temp.getBuffer(); '\0' != *pos; pos++ ) {
		if( '\r' == *pos ) {
		} else if( '\n' == *pos ) {
			buffer->append( "&#10;" );
		} else {
			buffer->append( *pos );
		}
	}
}

void SP_CanonXmlBuffer :: dump(
		const SP_XmlNode * node, SP_XmlStringBuffer * buffer )
{
	if( NULL == node ) return;

	if( SP_XmlNode::eXMLDOC == node->getType() ) {
		SP_XmlDocument * document = static_cast<SP_XmlDocument*>((SP_XmlNode*)node);
		const SP_XmlNodeList * children = document->getChildren();
		for( int j = 0; j < children->getLength(); j++ ) {
			dump( children->get( j ), buffer );
		}
	} else if( SP_XmlNode::eCDATA == node->getType() ) {
		SP_XmlCDataNode * cdata = static_cast<SP_XmlCDataNode*>((SP_XmlNode*)node);

		canonEncode( cdata->getText(), buffer );
	} else if( SP_XmlNode::ePI == node->getType() ) {
		SP_XmlPINode * piNode = static_cast<SP_XmlPINode*>((SP_XmlNode*)node);

		buffer->append( "<?" );
		buffer->append( piNode->getTarget() );
		if( '\0' != *( piNode->getTarget() ) ) buffer->append( ' ' );
		buffer->append( piNode->getData() );
		buffer->append( "?>" );
	} else if( SP_XmlNode::eCOMMENT == node->getType() ) {
		// ignore
	} else if( SP_XmlNode::eELEMENT == node->getType() ) {
		dumpElement( node, buffer );
	} else if( SP_XmlNode::eDOCDECL == node->getType() ) {
		// ignore
	} else if( SP_XmlNode::eDOCTYPE == node->getType() ) {
		// ignore
	} else {
		// ignore
	}
}

void SP_CanonXmlBuffer :: dumpElement(
		const SP_XmlNode * node, SP_XmlStringBuffer * buffer )
{
	if( NULL == node ) return;

	if( SP_XmlNode::eELEMENT == node->getType() ) {
		SP_XmlElementNode * element = static_cast<SP_XmlElementNode*>((SP_XmlNode*)node);
		buffer->append( "<" );
		buffer->append( element->getName() );

		int i = 0;

		SP_XmlArrayList attrList;
		for( i = 0; i < element->getAttrCount(); i++ ) {
			attrList.append( (void*)element->getAttr( i, NULL ) );
		}
		attrList.sort( reinterpret_cast<int(*)(const void*, const void*)>(strcmp) );

		const char * name = NULL, * value = NULL;
		for( i = 0; i < attrList.getCount(); i++ ) {
			name = (char*)attrList.getItem( i );
			value = element->getAttrValue( name );
			if( NULL != name && NULL != value ) {
				buffer->append( ' ' );
				buffer->append( name );
				buffer->append( "=\"" );
				canonEncode( value, buffer );
				buffer->append( "\"" );
			}
		}

		const SP_XmlNodeList * children = element->getChildren();

		buffer->append( ">" );

		for( int j = 0; j < children->getLength(); j++ ) {
			dump( children->get( j ), buffer );
		}

		buffer->append( "</" );
		buffer->append( element->getName() );
		buffer->append( ">" );
	} else {
		dump( node, buffer );
	}
}

