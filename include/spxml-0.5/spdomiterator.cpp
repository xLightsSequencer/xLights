/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "spdomiterator.hpp"
#include "spxmlnode.hpp"

SP_DomIterator :: SP_DomIterator( const SP_XmlNode * node )
{
	mRoot = node;
	mCurrent = node;
}

SP_DomIterator :: ~SP_DomIterator()
{
}

const SP_XmlNode * SP_DomIterator :: getNext()
{
	if( NULL == mCurrent ) return NULL;

	const SP_XmlNode * retNode = mCurrent;

	// find first left child
	if( SP_XmlNode::eXMLDOC == mCurrent->getType() ) {
		SP_XmlDocument * document = static_cast<SP_XmlDocument*>((SP_XmlNode*)mCurrent);

		mCurrent = NULL;
		retNode = document->getDocDecl();
		if( NULL == retNode ) retNode = document->getDocType();
		if( NULL == retNode ) {
			retNode = document->getRootElement();
			if( NULL != retNode ) mCurrent = document->getRootElement()->getChildren()->get( 0 );
		}
	} else if( SP_XmlNode::eELEMENT == mCurrent->getType() ) {
		SP_XmlElementNode * element = static_cast<SP_XmlElementNode*>((SP_XmlNode*)mCurrent);
		const SP_XmlNodeList * children = element->getChildren();
		mCurrent = children->get( 0 );
	} else {
		mCurrent = NULL;
	}

	// find next sibling
	if( NULL == mCurrent ) {
		mCurrent = retNode;

		const SP_XmlNode * parent = NULL;
		if( NULL != mCurrent ) parent = mCurrent->getParent();

		for( ; NULL != parent; ) {
			if( SP_XmlNode::eXMLDOC == parent->getType() ) {
				SP_XmlDocument * document = static_cast<SP_XmlDocument*>((SP_XmlNode*)parent);
				if( mCurrent == document->getDocDecl() ) {
					mCurrent = document->getDocType();
					if( NULL == mCurrent ) mCurrent = document->getRootElement();
				} else if( mCurrent == document->getDocType() ) {
					mCurrent = document->getRootElement();
				} else {
					mCurrent = NULL;
				}
			} else if( SP_XmlNode::eELEMENT == parent->getType() ) {
				SP_XmlElementNode * element = static_cast<SP_XmlElementNode*>((SP_XmlNode*)parent);
				const SP_XmlNodeList * children = element->getChildren();

				int index = -1;
				for( int i = 0; i < children->getLength(); i++ ) {
					if( mCurrent == children->get( i ) ) {
						index = i;
						break;
					}
				}

				if( index >= 0 && index < ( children->getLength() - 1 ) ) {
					mCurrent = children->get( index + 1 );
				} else {
					mCurrent = NULL;
				}
			} else {
				mCurrent = NULL;
				assert( 0 ); // should not occur
			}

			if( NULL == mCurrent ) {
				mCurrent = parent;
				parent = mCurrent->getParent();
				if( NULL == parent ) mCurrent = NULL;
				if( mRoot == mCurrent ) mCurrent = NULL;
			} else {
				break;
			}
		}
	}

	return retNode;
}

