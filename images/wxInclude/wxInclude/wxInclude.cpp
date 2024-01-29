/*
	wxInclude version 1.0
	Kim De Deyn
*/

#include "stdafx.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

#define WXINCLUDE_INFO	"wxInclude by Kim De Deyn, use --help for more information.\n"

#define WXINCLUDE_HELP	"This tool can be used to convert binary files into a useful C/C++ header.\n" \
						"The primary goal is to provide wxWidgets users an easy way of integrating images in their programs. " \
						"The addition of extra helper macros for wxWidgets can be disabled.\n\n" \
						"It is able to convert multiple files into one header. " \
						"Input can be defined by passing files or the extension masks you want to convert.\n\n" \
						"Example of use:\n\n" \
						"	wxInclude.exe --const --input-file=mydata1.bin\n" \
						"	--input-type=.png --input-type=.bmp\n" \
						"	--output-file=myheader.h mydata2.bin myimage.png\n"

#define WXINCLUDE_VERSION "Version 1.0, compiled at " __DATE__ " " __TIME__

#define BUFFER_SIZE 4096

void defineheader_start( std::ostringstream& data, std::string& headername, bool usemacro = true, bool useconst = false )
{
	/* Write info header */
	data << "/*" << std::endl;
	data << "	Automatic generated header by:" << std::endl << std::endl;
	data << "		" << WXINCLUDE_INFO;
	data << "		" << WXINCLUDE_VERSION << std::endl << std::endl;
	data << "	Header: " << headername << std::endl;
	data << "	Macros: " << ( usemacro ? "yes" : "no" ) << std::endl;
	data << "	Const: " << ( useconst ? "yes" : "no" ) << std::endl;
	data << "*/" << std::endl << std::endl;

	/* Prevent multiple defines */
	std::string temp( headername );
	boost::to_upper( temp );
	data << "#ifndef _WXINCLUDE_" << temp << "_H_" << std::endl;
	data << "#define _WXINCLUDE_" << temp << "_H_" << std::endl << std::endl;
}

void defineheader_end( std::ostringstream& data, std::string& name )
{
	/* Prevent multiple defines */
	data << "#endif" << std::endl << std::endl;
}

void definemacros( std::ostringstream& data, std::string& includename, bool definestream = true )
{
	/* Include wxWidgets header */
	data << "#include \"" << includename << "\"" << std::endl;

	/* When using default header, include memory stream header!*/
	if ( definestream )
		data << "#include \"wx/mstream.h\"" << std::endl;

	data << std::endl;

	/* Define some useful macros */
	//data << "#define wxMEMORY_IMAGE( name ) _wxConvertMemoryToImage( name, sizeof( name ) )" << std::endl;
	//data << "#define wxMEMORY_IMAGEEX( name, type ) _wxConvertMemoryToImage( name, sizeof( name ), type )" << std::endl;
	//data << "#define wxMEMORY_BITMAP( name ) _wxConvertMemoryToBitmap( name, sizeof( name ) )" << std::endl;
	//data << "#define wxMEMORY_BITMAPEX( name, type ) _wxConvertMemoryToBitmap( name, sizeof( name ), type )" << std::endl << std::endl;

	//data << "inline wxImage _wxConvertMemoryToImage(const unsigned char* data, int length, long type = wxBITMAP_TYPE_ANY )" << std::endl;
	//data << "{" << std::endl;
	//data << "	wxMemoryInputStream stream( data, length );" << std::endl;
	//data << "	return wxImage( stream, type, -1 );" << std::endl;
	//data << "}" << std::endl << std::endl;

	//data << "inline wxBitmap _wxConvertMemoryToBitmap(const unsigned char* data, int length, long type = wxBITMAP_TYPE_ANY )" << std::endl;
	//data << "{" << std::endl;
	//data << "	wxMemoryInputStream stream( data, length );" << std::endl;
	//data << "	return wxBitmap( wxImage( stream, type, -1 ), -1 );" << std::endl;
	//data << "}" << std::endl << std::endl;
}

static std::vector<std::string> list;

void definefile( std::ostringstream& data, fs::ifstream& input, std::string& name, bool useconst = false )
{
	/* Check if already defined */
	std::vector<std::string>::iterator search = std::find( list.begin(), list.end(), name );
	if ( search == list.end() )
	{
		list.push_back( name );
	}
	else
	{
		/* Show warning, object of this name is already processed! */
		std::cout << "Warning: '" << name << "' already defined, processing of new one stopped." << std::endl;
		return;
	}

	/* Define array */
	data << "static" << ( useconst ? " const " : " " ) << "unsigned char " << name << "_png[] = {" << std::endl;
	
	int size = input.tellg();
	input.seekg( 0, std::ios::beg );

	int c = 0;
	int col = 0;

	for ( int i = 1; i <= size; ++i )
	{
		/* Get character and add to array */
		c = input.get();

		/*
			Using a static copy of the boost::format string gives a nice performance boost!
			Boost help says using const boost::format fmter(fstring);
			But static is faster and using the object without copy constructor is even faster!
		*/
		//static boost::format fmt( "0x%02X" );
		//data << fmt % c;

		/*
			Fast option then... this code is executed allot!
			Still faster then the optimized boost::format use, but not that much!
		*/
		static char temp[5];
		_snprintf( temp, 5, "0x%02X", c );
		data << temp;

		if ( i >= size )
		{
			/* Last character */
			data << std::endl;
		}
		else
		{
			/* Next */
			data << ", ";
		}

		/* New colume? */
		int curcol = ( i / 10 );
		if ( col < curcol )
		{
			col = curcol;
			data << std::endl;
		}
	}

	data << "};" << std::endl << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
    try
	{
        po::options_description desc( "Options" );
        desc.add_options()
            ( "help,h", "Show detailed help." )
			( "options,p", "Show parameter information." )
			( "version,p", "Show version information." )
			( "input-file,i", po::value<std::vector<std::string>>(), "Define file(s) for the convertion input." )
			( "input-type,I", po::value<std::vector<std::string>>(), "Define file type(s) for automatic conversion of files in the working directory." )
			( "output-file,o", po::value<std::string>(), "Define file for the convertion output." )
			( "noheader,h", "Disable adding of header support defines." )
            ( "const,C", "Define array as const." )
			( "respectcase,r", "Disable converting file types into lower case." )
			( "wxnone,w", "Disable adding of wxWidgets support macro's." )
			( "wxheader,W", po::value<std::string>()->default_value( "wx/wx.h" ), "Select the header that includes wxWidgets (precompiled header?)." )
			( "appendtype,t", "Add the file type at the end of the identifier (myimage_png)." )
			( "text,T", "Disable binary output and use text output, converts feed codes to systems defaults." )
		;

        po::positional_options_description posdesc;
        posdesc.add( "input-file", -1 );

        po::variables_map opt;
        po::store( po::command_line_parser( argc, argv ).options( desc ).positional( posdesc ).run(), opt );
		po::store( po::parse_config_file( fs::ifstream( fs::path( "default.cfg" ) ), desc ), opt );
        po::notify( opt );

		std::cout << WXINCLUDE_INFO << std::endl;

		/* Show options when requested */
        if ( opt.count( "options" ) )
		{
			std::cout << desc << std::endl << std::endl;
            exit( 0 );
        }

		/* Show help when requested */
        if ( opt.count( "help" ) )
		{
			std::cout << WXINCLUDE_HELP;
			std::cout << std::endl << desc << std::endl << std::endl;
            exit( 0 );
        }

		/* Show version when requested */
        if ( opt.count( "version" ) )
		{
			std::cout << WXINCLUDE_VERSION << std::endl;
            exit( 0 );
        }

		/* Process */
        if ( opt.count( "input-file" ) || opt.count( "input-type" ) )
		{
			if ( opt.count( "output-file" ) )
			{	
				/* Create timer */
				boost::timer timer;

				/* Create output file */
				std::string headername( opt[ "output-file" ].as<std::string>() );

				fs::path outputpath( headername );
				fs::ofstream output( outputpath, std::ios::out | std::ios::trunc );

				/* Use buffer */
				char outbuffer[BUFFER_SIZE];
				output.rdbuf()->pubsetbuf( outbuffer, BUFFER_SIZE );

				if ( !opt.count( "text" ) )
					output.setf( std::ios::binary );

				if ( !output )
					throw std::runtime_error( "Failed to create output file!" );

				/* Show status */
				std::cout << "Build  : file '" << outputpath.leaf() << "'..." << std::endl;

				/* Get base name of file */
				headername = fs::basename( outputpath );

				/* Data string stream */
				std::ostringstream data;

				/* Write header start when wanted */
				if ( !opt.count( "noheader" ) )
					defineheader_start( data, headername, opt.count( "wxnone" ) ? false : true, opt.count( "const" ) ? true : false );

				/* Get defined or else default wx header */
				std::string includename( opt[ "wxheader" ].as<std::string>() );

				/* Write macros when wanted */
				if ( !opt.count( "wxnone" ) )
					definemacros( data, includename, opt[ "wxheader" ].defaulted() );

				/* Common input buffer */
				char inbuffer[BUFFER_SIZE];

				/* Process input files based on provided list */
				if ( opt.count( "input-file" ) )
				{
					std::vector<std::string> files( opt[ "input-file" ].as<std::vector<std::string>>() );

					BOOST_FOREACH( std::string& file, files )
					{
						fs::path inputpath( file );
						std::string fileext( fs::extension( inputpath ) );

						fs::ifstream input( inputpath, std::ios::in | std::ios::binary | std::ios::ate );
						input.rdbuf()->pubsetbuf( inbuffer, BUFFER_SIZE ); 

						if ( input.is_open() )
						{
							/* Show status */
							std::cout << "Process: file '" << file << "'..." << std::endl;

							/* Remove extension */
							boost::erase_last( file, fileext );

							if ( !opt.count( "respectcase" ) )
								boost::to_lower( fileext );

							/* Append type */
							if ( opt.count( "appendtype" ) )
							{
								boost::erase_first( fileext, "." );

								/* Static and NO copy constructor for speed */
								static boost::format fmt( "%1%_%2%" );
								file = boost::str( fmt % file % fileext );
							}

							/* Lower case names when wanted */
							if ( !opt.count( "respectcase" ) )
								boost::to_lower( file );

							/* Process file */
							definefile( data, input, file, opt.count( "const" ) ? true : false );
						}
						else
						{
							/* Only show warning, other files need to be processed */
							std::cout << "Warning: input file '" << file << "' failed to open." << std::endl;
						}
					}
				}

				/* Process input files based on provided type */
				if ( opt.count( "input-type" ) )
				{
					std::vector<std::string> types( opt[ "input-type" ].as<std::vector<std::string>>() );

					for ( fs::directory_iterator dir_itr( fs::initial_path() ); dir_itr != fs::directory_iterator(); ++dir_itr )
					{
						BOOST_FOREACH( std::string& type, types )
						{
							/* Normal file? */
							if ( fs::is_regular( dir_itr->status() ) )
							{
								/* Wanted type? */
								std::string fileext( fs::extension( dir_itr->path() ));

								bool equals = false;

								if ( opt.count( "respectcase" ) )
									equals = boost::equals( fileext, type );
								else
									equals = boost::iequals( fileext, type );

								if ( equals )
								{
									fs::ifstream input( dir_itr->path(), std::ios::in | std::ios::binary | std::ios::ate );
									input.rdbuf()->pubsetbuf( inbuffer, BUFFER_SIZE ); 

									std::string file( dir_itr->path().leaf() );

									if ( input.is_open() )
									{
										/* Show status */
										std::cout << "Process: file '" << file << "'..." << std::endl;

										/* Remove extension */
										boost::erase_last( file, fileext );

										/* Append type */
										if ( opt.count( "appendtype" ) )
										{
											boost::erase_first( fileext, "." );

											/* Static and NO copy constructor for speed */
											static boost::format fmt( "%1%_%2%" );
											file = boost::str( fmt % file % fileext );
										}

										/* Lower case names when wanted */
										if ( !opt.count( "respectcase" ) )
											boost::to_lower( file );

										/* Process file */
										definefile( data, input, file, opt.count( "const" ) ? true : false );
									}
									else
									{
										/* Only show warning, other files need to be processed */
										std::cout << "Warning: input file '" << file << "' failed to open." << std::endl;
									}
								}
							}
						}
					}
				}

				/* Write header end when wanted */
				if ( !opt.count( "noheader" ) )
					defineheader_end( data, headername );
				
				/* Write data to output file */
				output.seekp( 0, std::ios::beg );
				output << data.str();

				/* Show status */
				std::cout << "Build  : " << timer.elapsed() << "s needed for conversion of " << list.size() << " files." << std::endl;
			}
			else
			{
				throw std::invalid_argument( "No output defined!" );
			}
		}
		else
		{
			throw std::invalid_argument( "No input defined!" );
		}
    }
	catch( std::exception& e )
	{
		std::cerr << "Error: " << e.what() << std::endl;
    }
    catch( ... )
	{
		std::cerr << "Error: Exception of unknown type!" << std::endl;
    }

    return 0;
}
