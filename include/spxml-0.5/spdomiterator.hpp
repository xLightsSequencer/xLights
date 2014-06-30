/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#ifndef __spdomiterator_hpp__
#define __spdomiterator_hpp__

class SP_XmlNode;

/// DFS iterator -- Depth First Search
class SP_DomIterator {
public:
	/// node as tree node, iterator this tree by DFS
	SP_DomIterator( const SP_XmlNode * node );
	~SP_DomIterator();

	/// @return NULL : reach the end
	const SP_XmlNode * getNext();

private:

	SP_DomIterator( SP_DomIterator & );
	SP_DomIterator & operator=( SP_DomIterator & );

	const SP_XmlNode * mRoot;
	const SP_XmlNode * mCurrent;
};

#endif

