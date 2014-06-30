/*
 * Copyright 2007 Stephen Liu
 * For license terms, see the file COPYING along with this library.
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "spxmlcodec.hpp"
#include "spxmlutils.hpp"

const char * SP_XmlStringCodec :: DEFAULT_ENCODING = "utf-8";

const char SP_XmlStringCodec :: XML_CHARS [] =
		{ '<', '>', '&', '\'', '"' };
const char * SP_XmlStringCodec :: ESC_CHARS [] =
		{ "&lt;", "&gt;", "&amp;", "&apos;", "&quot;" };

int SP_XmlStringCodec :: decode( const char * encoding, const char * encodeValue,
		SP_XmlStringBuffer * outBuffer )
{
	int isUtf8 = ( 0 == strcasecmp( encoding, "utf-8" ) );

	const char * pos = encodeValue;
	for( ; '\0' != *pos; ) {
		if( '&' == *pos ) {
			int index = -1;
			int len = 0;
			for( int i = 0; i < (int)( sizeof( ESC_CHARS ) / sizeof( ESC_CHARS[0] ) ); i++ ) {
				len = strlen( ESC_CHARS[ i ] );
				if( 0 == strncmp( pos, ESC_CHARS[i], len ) ) {
					index = i;
					break;
				}
			}
			if( index >= 0 ) {
				outBuffer->append( XML_CHARS[ index ] );
				pos += len;
			} else {
				char * next = "";
				int ch = 0;
				if( '#' == *( pos + 1 ) ) {
					if( 'x' == *( pos + 2 ) ) {
						ch = strtol( pos + 3, &next, 16 );
					} else {
						ch = strtol( pos + 2, &next, 10 );
					}
				}

				// TODO: fully support xml entity, currently only support unicode entity
				if( ';' == *next && 0 != ch ) {
					if( isUtf8 ) {
						SP_XmlUtf8Codec::uni2utf8( ch, outBuffer );
					} else {
						outBuffer->append( ch );
					}
					pos = next + 1;
				} else {
					outBuffer->append( *pos++ );
				}
			}
		} else {
			outBuffer->append( *pos++ );
		}
	}

	return 0;
}

int SP_XmlStringCodec :: encode( const char * encoding, const char * decodeValue,
		SP_XmlStringBuffer * outBuffer )
{
	int isUtf8 = ( 0 == strcasecmp( encoding, "utf-8" ) );

	const unsigned char * pos = (unsigned char *)decodeValue;
	for( ; '\0' != *pos; pos++ ) {
		int index = -1;
		for( int i = 0; i < (int)( sizeof( XML_CHARS ) / sizeof( XML_CHARS[0] ) ); i++ ) {
			if( XML_CHARS[i] == *pos ) {
				index = i;
				break;
			}
		}
		if( index >= 0 && '\'' != *pos ) {
			outBuffer->append( ESC_CHARS[ index ] );
		} else {
			if( isUtf8 ) {
				int ch = 0;
				int len = SP_XmlUtf8Codec::utf82uni( (unsigned char*)pos, &ch );

				if( len > 0 ) {
					pos += len - 1;

					char temp[ 32 ] = { 0 };
					snprintf( temp, sizeof( temp ), "&#%d;", ch );
					outBuffer->append( temp );
				} else {
					outBuffer->append( *pos );
				}
			} else {
				if( *pos < 32 ) {
					char temp[ 32 ] = { 0 };
					snprintf( temp, sizeof( temp ), "&#%d;", *pos );
					outBuffer->append( temp );
				} else {
					outBuffer->append( *pos );
				}
			}
		}	
	}

	return 0;
}

int SP_XmlStringCodec :: isNameChar( const char * encoding, char c )
{
	if( 0 == strcasecmp( encoding, "utf-8" ) ) {
		return 1;
	} else {
		return isalnum(c) || c == ':' || c == '-' || c == '.' || c == '_';
	}
}

//=========================================================

int SP_XmlUtf8Codec :: utf82uni( const unsigned char * utf8, int * ch )
{
	int len = 0;

	unsigned char c1 = 0, c2 = 0, c3 = 0, c4 = 0;

	if( *utf8 >= 0x80 ) {
		c1 = *utf8++;

		if( c1 < 0xE0 ) {         // 2 bytes
			if( '\0' != ( c2 = *utf8 ) ) {
				*ch = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
				len = 2;
			}
		} else if( c1 < 0xF0 ) {  // 3 bytes
			if( '\0' != ( c2 = *utf8++ ) && '\0' != ( c3 = *utf8 ) ) {
				*ch = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6)| (c3 & 0x3F);
				len = 3;
			}
		} else {                     // 4 bytes
			if( '\0' != ( c2 = *utf8++ ) && '\0' != ( c3 = *utf8++ )
					&& '\0' != ( c4 = *utf8 ) ) {
				*ch = ((c1 & 0x07) << 16) | ((c2 & 0x3F) << 12)
						| ((c3 & 0x3F) << 6) | (c4 & 0x3F);
				len = 4;
			}
		}
	}

	return len;
}

void SP_XmlUtf8Codec :: uni2utf8( int ch, SP_XmlStringBuffer * outBuffer )
{
	if( ch < 0x80 ) outBuffer->append( ch );
	else if( ch < 0x800 ) {
		outBuffer->append( 0xC0 | ( ch >> 6 ) );
		outBuffer->append( 0x80 | ( ch & 0x3F ) );
	} else if( ch < 0x10000 ) {
		outBuffer->append( 0xE0 | ( ch >> 12 ) );
		outBuffer->append( 0x80 | ( ( ch >> 6 ) & 0x3F ) );
		outBuffer->append( 0x80 | ( ch & 0x3F ) );
	} else if( ch < 0x200000 ) {
		outBuffer->append( 0xF0 | ( ch >> 18 ) );
		outBuffer->append( 0x80 | ( ( ch >> 12 ) & 0x3F ) );
		outBuffer->append( 0x80 | ( ( ch >> 6 ) & 0x3F ) );
		outBuffer->append( 0x80 | ( ch & 0x3F ) );
	}
}

