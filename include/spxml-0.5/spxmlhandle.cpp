/*
 * Copyright 2008 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <stdio.h>
#include <string.h>

#include "spxmlhandle.hpp"
#include "spxmlnode.hpp"


SP_XmlHandle :: SP_XmlHandle( SP_XmlNode * node )
{
	mNode = node;
}

SP_XmlHandle :: SP_XmlHandle( const SP_XmlHandle & ref )
{
	mNode = ref.mNode;
}

SP_XmlHandle & SP_XmlHandle :: operator=( const SP_XmlHandle & ref )
{
	mNode = ref.mNode;
	return *this;
}

SP_XmlHandle :: ~SP_XmlHandle()
{
}

SP_XmlHandle SP_XmlHandle :: getChild( const char * name, int index ) const
{
	SP_XmlNode * ret = NULL;

	if( NULL != mNode ) {
		if( SP_XmlNode::eELEMENT == mNode->getType() ) {
			SP_XmlElementNode * element = (SP_XmlElementNode*)mNode;
			const SP_XmlNodeList * children = element->getChildren();

			int tmpIndex = index;
			for( int i = 0; i < children->getLength(); i++ ) {
				if( SP_XmlNode::eELEMENT == children->get(i)->getType() ) {
					SP_XmlElementNode * iter = (SP_XmlElementNode*)children->get(i);
					if( 0 == strcmp( name, iter->getName() ) ) {
						if( 0 == tmpIndex ) {
							ret = iter;
							break;
						}
						tmpIndex--;
					}
				}
			}
		}
	}

	return SP_XmlHandle( ret );
}

SP_XmlHandle SP_XmlHandle :: getChild( int index ) const
{
	SP_XmlNode * ret = NULL;

	if( NULL != mNode ) {
		if( SP_XmlNode::eELEMENT == mNode->getType() ) {
			SP_XmlElementNode * element = (SP_XmlElementNode*)mNode;
			ret = (SP_XmlNode*)element->getChildren()->get( index );
		}
	}

	return SP_XmlHandle( ret );
}

SP_XmlHandle SP_XmlHandle :: getElement( int index ) const
{
	SP_XmlNode * ret = NULL;

	if( NULL != mNode ) {
		if( SP_XmlNode::eELEMENT == mNode->getType() ) {
			SP_XmlElementNode * element = (SP_XmlElementNode*)mNode;
			const SP_XmlNodeList * children = element->getChildren();

			int tmpIndex = index;
			for( int i = 0; i < children->getLength(); i++ ) {
				if( SP_XmlNode::eELEMENT == children->get(i)->getType() ) {
					SP_XmlElementNode * iter = (SP_XmlElementNode*)children->get(i);

					if( 0 == tmpIndex ) {
						ret = iter;
						break;
					}
					tmpIndex--;
				}
			}
		}
	}

	return SP_XmlHandle( ret );
}

SP_XmlNode * SP_XmlHandle :: toNode()
{
	return mNode;
}

SP_XmlElementNode * SP_XmlHandle :: toElement()
{
	if( NULL != mNode && SP_XmlNode::eELEMENT == mNode->getType() ) {
		return (SP_XmlElementNode*)mNode;
	}

	return NULL;
}

SP_XmlCDataNode * SP_XmlHandle :: toCData()
{
	if( NULL != mNode && SP_XmlNode::eCDATA == mNode->getType() ) {
		return (SP_XmlCDataNode*)mNode;
	}

	return NULL;
}

