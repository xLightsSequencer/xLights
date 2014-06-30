/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#ifndef __spxmlnode_hpp__
#define __spxmlnode_hpp__

class SP_XmlArrayList;

class SP_XmlNode {
public:
	enum { eXMLDOC, eDOCDECL, ePI, eDOCTYPE, eELEMENT, eCDATA, eCOMMENT  };

	SP_XmlNode( int type );
	virtual ~SP_XmlNode();

	void setParent( SP_XmlNode * parent );
	const SP_XmlNode * getParent() const;
	int getType() const;

protected:
	SP_XmlNode( SP_XmlNode & );
	SP_XmlNode & operator=( SP_XmlNode & );

private:
	SP_XmlNode * mParent;
	const int mType;
};

class SP_XmlNodeList {
public:
	SP_XmlNodeList();
	~SP_XmlNodeList();

	int getLength() const;
	void append( SP_XmlNode * node );
	SP_XmlNode * get( int index ) const;
	SP_XmlNode * take( int index ) const;

private:
	SP_XmlNodeList( SP_XmlNodeList & );
	SP_XmlNodeList & operator=( SP_XmlNodeList & );

	SP_XmlArrayList * mList;
};

class SP_XmlPIEvent;
class SP_XmlDocDeclEvent;
class SP_XmlDocTypeEvent;
class SP_XmlStartTagEvent;
class SP_XmlCDataEvent;
class SP_XmlCommentEvent;

class SP_XmlElementNode;
class SP_XmlDocDeclNode;
class SP_XmlDocTypeNode;

class SP_XmlDocument : public SP_XmlNode {
public:
	SP_XmlDocument();
	virtual ~SP_XmlDocument();

	void setDocDecl( SP_XmlDocDeclNode * docDecl );
	SP_XmlDocDeclNode * getDocDecl() const;
	void setDocType( SP_XmlDocTypeNode * docType );
	SP_XmlDocTypeNode * getDocType() const;
	void setRootElement( SP_XmlElementNode * rootElement );
	SP_XmlElementNode * getRootElement() const;
	SP_XmlNodeList * getChildren() const;

private:
	SP_XmlDocDeclNode * mDocDecl;
	SP_XmlDocTypeNode * mDocType;
	SP_XmlNodeList * mChildren;
};

class SP_XmlPINode : public SP_XmlNode {
public:
	SP_XmlPINode();
	SP_XmlPINode( SP_XmlPIEvent * event );
	virtual ~SP_XmlPINode();

	void setTarget( const char * target );
	const char * getTarget();

	void setData( const char * data );
	const char * getData();

private:
	SP_XmlPIEvent * mEvent;
};

class SP_XmlDocDeclNode : public SP_XmlNode {
public:
	SP_XmlDocDeclNode();
	SP_XmlDocDeclNode( SP_XmlDocDeclEvent * event );
	virtual ~SP_XmlDocDeclNode();

	void setVersion( const char * version );
	const char * getVersion() const;
	void setEncoding( const char * encoding );
	const char * getEncoding() const;
	void setStandalone( int standalone );
	int getStandalone() const;

private:
	SP_XmlDocDeclEvent * mEvent;
};

class SP_XmlDocTypeNode : public SP_XmlNode {
public:
	SP_XmlDocTypeNode();
	SP_XmlDocTypeNode( SP_XmlDocTypeEvent * event );
	virtual ~SP_XmlDocTypeNode();

	void setName( const char * name );
	const char * getName() const;
	void setSystemID( const char * systemID );
	const char * getSystemID() const;
	void setPublicID( const char * publicID );
	const char * getPublicID() const;
	void setDTD( const char * dtd );
	const char * getDTD() const;

private:
	SP_XmlDocTypeEvent * mEvent;
};

class SP_XmlElementNode : public SP_XmlNode {
public:
	SP_XmlElementNode();
	SP_XmlElementNode( SP_XmlStartTagEvent * event );
	virtual ~SP_XmlElementNode();

	void setName( const char * name );
	const char * getName() const;
	void addChild( SP_XmlNode * node );
	const SP_XmlNodeList * getChildren() const;

	void addAttr( const char * name, const char * value );
	const char * getAttrValue( const char * name ) const;
	int getAttrCount() const;
	const char * getAttr( int index, const char ** value ) const;

	void removeAttr( const char * name );

protected:
	SP_XmlStartTagEvent * mEvent;
	SP_XmlNodeList * mChildren;
};

class SP_XmlCDataNode : public SP_XmlNode {
public:
	SP_XmlCDataNode();
	SP_XmlCDataNode( SP_XmlCDataEvent * event );
	virtual ~SP_XmlCDataNode();

	void setText( const char * content );
	const char * getText() const;

protected:
	SP_XmlCDataEvent * mEvent;
};

class SP_XmlCommentNode : public SP_XmlNode {
public:
	SP_XmlCommentNode();
	SP_XmlCommentNode( SP_XmlCommentEvent * event );
	virtual ~SP_XmlCommentNode();

	void setText( const char * comment );
	const char * getText() const;

protected:
	SP_XmlCommentEvent * mEvent;
};

#endif

