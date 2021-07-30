/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LORPreview.h"

#include "ModelPreview.h"
#include "UtilFunctions.h"
#include "xLightsMain.h"
#include "models/Model.h"
#include "models/ModelManager.h"

#include <log4cpp/Category.hh>

#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/wx.h>

bool S5Model::ParseXML( wxXmlNode* m ) {
    id                 = m->GetAttribute( "id" );
    name               = m->GetAttribute( "Name" );
    bulbShape          = m->GetAttribute( "BulbShape" );
    deviceType         = m->GetAttribute( "DeviceType" );
    individualChannels = m->GetAttribute( "IndividualChannels" ) == "True";
    previewBulbSize    = wxAtoi( m->GetAttribute( "PreviewBulbSize", "2" ) );
    rgbOrder           = m->GetAttribute( "RgbOrder" );
    separateIds        = m->GetAttribute( "SeparateIds" ) == "True";
    startLocation      = m->GetAttribute( "StartLocation", "n/a" );
    stringType         = m->GetAttribute( "StringType" );
    traditionalColors  = m->GetAttribute( "TraditionalColors" );
    traditionalType    = m->GetAttribute( "TraditionalType" );
    channelGrid        = m->GetAttribute( "ChannelGrid" );
    opacity            = wxAtoi( m->GetAttribute( "Opacity", "0" ) );

    ParseParms( m );

    for( wxXmlNode* shape = m->GetChildren(); shape != nullptr; shape = shape->GetNext() ) {
        if( shape->GetName() == "shape" ) {
            shapeName = shape->GetAttribute( "ShapeName" );
            //custom model
            customWidth  = shape->GetAttribute( "CustomWidth", "5" );
            customHeight = shape->GetAttribute( "CustomHeight", "5" );
            customGrid   = shape->GetAttribute( "CustomGrid" );
            //only non line/point models have these
            offset.x = wxAtof( shape->GetAttribute( "OffsetX", "0.0" ) );
            offset.y = wxAtof( shape->GetAttribute( "OffsetY", "0.0" ) );
            scale.x  = wxAtof( shape->GetAttribute( "ScaleX", "1.0" ) );
            scale.y  = wxAtof( shape->GetAttribute( "ScaleY", "1.0" ) );
            radians  = wxAtof( shape->GetAttribute( "Radians", "0.0" ) );
            ParsePoints( shape );
        }
    }

    return true;
}

void S5Model::ParseParms( wxXmlNode* p ) {
    for( int i = 1; i < 100; i++ ) { //I, Scott, avoid while loops, so just loop 100 times is probably enough
        wxString const parmname = wxString::Format( "Parm%d", i );
        if( p->HasAttribute( parmname ) ) {
            parms.emplace_back( wxAtoi( p->GetAttribute( parmname, "0" ) ) );
        } else {
            break;
        }
    }
}

void S5Model::ParsePoints( wxXmlNode* p ) {
    for( wxXmlNode* pp = p->GetChildren(); pp != nullptr; pp = pp->GetNext() ) {
        if( pp->GetName() == "point" ) {
            points.emplace_back( wxAtof( pp->GetAttribute( "x", "0.0" ) ),
                                 wxAtof( pp->GetAttribute( "y", "0.0" ) ) );
        }
    }
}

bool S5Group::ParseXML( wxXmlNode* g ) {
    id          = g->GetAttribute( "id" );
    name        = g->GetAttribute( "Name" );
    arrangement = g->GetAttribute( "Arrangement" );
    for( wxXmlNode* m = g->GetChildren(); m != nullptr; m = m->GetNext() ) {
        if( m->GetName() == "member" ) {
            modelIds.push_back( m->GetAttribute( "id" ) );
        }
    }
    return true;
}

LORPreview::LORPreview( xLightsFrame* frame, wxString xLigthsPreview ) :
    xlights( frame ),
    xLigths_preview( xLigthsPreview ) {
#ifndef _DEBUG
    DisplayWarning(
        "WARNING: As at this release S5 model import is experimental and its improvement relies on your feedback.\nIf it doesnt do a good job let us know by telling us:\n\
        - which LOR S5 Model type it was\n\
        - what xLights Model was it converted too\n\
        - what you changs you made to it.\n",
        frame );
#else
    RunTests();
#endif
}

bool LORPreview::LoadPreviewFile() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance( std::string( "log_base" ) );

    auto const previewfileName = FindLORPreviewFile();

    if( wxFile::Exists( previewfileName ) ) {
        wxXmlDocument d;
        d.Load( previewfileName );
        if( d.IsOk() ) {
            wxXmlNode* root = d.GetRoot();
            if( root != nullptr ) {
                wxArrayString const previews = GetPreviews( root );
                wxSingleChoiceDialog dlg( xlights, "", "Select Preview", previews );
                if( dlg.ShowModal() == wxID_OK ) {
                    auto previewName = dlg.GetStringSelection();
                    return LoadPreview( root, previewName );
                }
            }
        } else {
            logger_base.warn( "LOR S5 Preview file could not be loaded." );
        }
    } else {
        logger_base.warn( "LOR S5 Preview file not fould." );
    }

    return false;
}

bool LORPreview::LoadPreview( wxXmlNode* root, wxString const& name ) {
    if( root != nullptr ) {
        for( wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext() ) {
            if( n->GetName() == "PreviewClass" ) {
                if( n->GetAttribute( "Name" ) == name ) {
                    return ReadPreview( n );
                }
            }
        }
    }
    return false;
}

bool LORPreview::ReadPreview( wxXmlNode* preview ) {
    int previewWidth  = xlights->AllModels.GetPreviewWidth();
    int previewHeight = xlights->AllModels.GetPreviewHeight();

    std::vector< S5Model > _S5Models;

    for( wxXmlNode* e = preview->GetChildren(); e != nullptr; e = e->GetNext() ) {
        if( e->GetName() == "PropClass" ) {
            std::string startChannel = xlights->AllModels.GenerateNewStartChannel();
            S5Model model;
            bool error;
            model.ParseXML( e );
            _S5Models.push_back( model );
            Model* xModel = CreateModel( model, startChannel, previewWidth, previewHeight, error );
            xlights->AllModels.AddModel( xModel );
        } else if( e->GetName() == "PropGroup" ) {
            S5Group group;
            group.ParseXML( e );
            CreateGroup( group, _S5Models );
        }
    }
    return true;
}

Model* LORPreview::LoadModelFile( wxString const& modelFile, wxString const& startChan, int previewW, int previewH, bool& error ) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance( std::string( "log_base" ) );

    logger_base.debug( "Loading LOR S5 Model file %s.", (const char*)modelFile.c_str() );

    S5Model model;

    wxXmlDocument doc;
    if( wxFile::Exists( modelFile ) && doc.Load( modelFile ) && doc.IsOk() ) {
        for( wxXmlNode* e = doc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext() ) {
            if( e->GetName() == "PropClass" ) {
                model.ParseXML( e );
                break;
            }
        }
    }

    return CreateModel( model, startChan, previewW, previewH, error );
}

Model* LORPreview::CreateModel( S5Model const& model, wxString const& startChan, int previewW, int previewH, bool& error ) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance( std::string( "log_base" ) );

    bool supportsMultiString = false;

    Model* m = nullptr;
    if( model.shapeName.StartsWith( "Arch" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Arches", startChan );
        if( model.stringType.IsSameAs( "Traditional" ) ) {
            m->SetProperty( "parm1", "1" );                                           //number of arches
            m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 0 ) ) ); //sections in LOR
            m->SetProperty( "parm3", wxString::Format( "%d", model.parms.at( 1 ) ) ); //number of lights per section
        } else {
            m->SetProperty( "parm1", "1" );                                                                 //number of arches
            m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 0 ) * model.parms.at( 1 ) ) ); //number of nodes
        }
        SetDirection( model, m );
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Bulb" ) ) {
        //xLights cannot really make this model....TODO convert to custom model??
        m = xlights->AllModels.CreateDefaultModel( "Single Line", startChan );

        m->SetProperty( "parm1", "1" );
        m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 0 ) ) ); //number of nodes

        SetDirection( model, m );
        ScalePointsToSingleLine( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Candycane" ) ) {
        //xLights cannot model multilayer candycans, just make one big one
        m = xlights->AllModels.CreateDefaultModel( "Candy Canes", startChan );
        m->SetProperty( "parm1", "1" );                                                                 //number of canes
        m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 0 ) * model.parms.at( 1 ) ) ); //number of nodes

        if( model.shapeName.Contains( "Left" ) ) {
            m->SetProperty( "CandyCaneReverse", "true" );
        } else {
            m->SetProperty( "CandyCaneReverse", "false" );
        }

        SetDirection( model, m );
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Circles Nested" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Circle", startChan );
        //xLights can not models this Exactly right, we don't support uneven strings....
        int totalCount  = 0;
        int totalLayers = 0;
        wxString layers;
        //skip first parm, its cener hollowness
        for( int i = 1; i < model.parms.size(); i++ ) {
            if( model.parms[ i ] != 0 ) {
                totalCount += model.parms[ i ];
                totalLayers++;
                layers = layers + wxString::Format( "%d", model.parms[ i ] ) + ",";
            }
        }
        layers.RemoveLast(); //remove last ","

        m->SetProperty( "parm1", "1" );                                        //number of strings
        m->SetProperty( "parm2", wxString::Format( "%d", totalCount ) );       //number of nodes
        m->SetProperty( "parm3", wxString::Format( "%d", model.parms[ 0 ] ) ); //center percentage

        m->SetProperty( "LayerSizes", layers );

        m->SetProperty( "StartSide", model.startLocation.Left( 1 ) );
        if( model.startLocation.Contains( "CCW" ) ) {
            m->SetProperty( "Dir", "L" );
        } else {
            m->SetProperty( "Dir", "R" );
        }
        if( model.startLocation.Contains( "Outer" ) ) {
            m->SetProperty( "InsideOut", "0" );
        } else {
            m->SetProperty( "InsideOut", "1" );
        }
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Custom" ) || model.shapeName.StartsWith( "Advance" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Custom", startChan );
        m->SetProperty( "parm1", model.customWidth );  //width
        m->SetProperty( "parm2", model.customHeight ); //height
        m->SetProperty( "CustomModel", model.customGrid );

        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Cylinder" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Tree", startChan );

        m->SetProperty( "parm1", wxString::Format( "%d", model.parms.at( 0 ) ) );     //number of strings
        m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 1 ) ) );     //number of nodes
        m->SetProperty( "parm3", wxString::Format( "%d", model.parms.at( 2 ) + 1 ) ); //number of folds is one less than number of stands per string in xLights

        m->SetProperty( "DisplayAs", wxString::Format( "Tree %d", ( model.parms.at( 4 ) * 90 ) ) );

        m->SetProperty( "TreeBottomTopRatio", "1.0" );

        if( model.shapeName.Contains( "spiral" ) ) {
            float roation = (float)model.parms.at( 2 ) / 10.0; //120 in xml = 12.0 rotaions
            if( model.startLocation.Contains( "CCW" ) ) {
                roation *= -1;
            }
            m->SetProperty( "TreeSpiralRotations", wxString::Format( "%f", roation ) );
        } else {
            m->SetProperty( "parm3", wxString::Format( "%d", model.parms.at( 2 ) + 1 ) ); //number of folds is one less than number of stands per string in xLights
        }

        SetDirection( model, m );
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Fan" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Spinner", startChan );
        
        supportsMultiString = true;
        m->SetProperty( "parm1", wxString::Format( "%d", model.parms.at( 0 ) ) );
        m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 1 ) ) );
        m->SetProperty( "parm3", "1" );

        m->SetProperty( "StartAngle", "90" );
        m->SetProperty( "Arc", "180" );

        if( model.startLocation.Contains( "Top" ) ) {
            m->SetProperty( "StartSide", "T" );
        } else {
            m->SetProperty( "StartSide", "B" );
        }
        if( model.startLocation.Contains( "CCW" ) ) {
            m->SetProperty( "Dir", "L" );
        } else {
            m->SetProperty( "Dir", "R" );
        }

        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Firestick" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Single Line", startChan );
        
        int segments   = model.parms.at( 0 );
        int nodesper = model.parms.at( 1 );

        m->SetProperty( "parm1", wxString::Format( "%d", segments ) );//sections in LOR
        m->SetProperty( "parm2", wxString::Format( "%d", nodesper ) );//number of lights per section

        ScaleModelToSingleLine( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Icicles" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Icicles", startChan );

        int maxdrop   = 0;
        int drops = 0;
        std::vector<int> dropcounts;
        //skip first parm, its drop count
        for( int i = 1; i < model.parms.size(); i++ ) {
            if( model.parms[ i ] != 0 ) {
                maxdrop = std::max( maxdrop, model.parms[ i ] );
                drops++;
                dropcounts.push_back( model.parms.at( i ) );
            }
        }
        int totalDrops  = model.parms[ 0 ];
        int totalNodels = 0;
        for(int i = 0; i < totalDrops; ++i ) {
            int idx = i % drops;
            totalNodels += dropcounts.at( idx );
        }

        wxString dropPattern;
        for( auto const& drp : dropcounts ) {
            dropPattern += wxString::Format( "%d,", drp );
        }
        dropPattern.RemoveLast(); //remove last ","
        m->SetProperty( "parm1", "1" );
        m->SetProperty( "parm2", wxString::Format( "%d", totalNodels ) );
        m->SetProperty( "DropPattern", dropPattern );

        ScaleIcicleToSingleLine( model, maxdrop, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Lines" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Poly Line", startChan );

        //int segments   = model.parms.at( 0 );
        int totalNodes = model.parms.at( 1 );
        m->SetProperty( "parm2", wxString::Format( "%d", totalNodes ) );
        if( model.shapeName.Contains( "-Connected" ) ) {
            wxString point_data;

            S5Point world_pt;
            for( int i = 0; i < model.points.size(); ++i ) {
                if( i == 0 ) {
                    world_pt = ScalePointToXLights( model.points.at( i ), previewW, previewH );
                    point_data += "0.0,0.0,0.0,";
                } else {
                    auto const& npt = ScalePointToXLights( model.points.at( i ), previewW, previewH );
                    point_data += wxString::Format( "%f,%f,0.0,", npt.x - world_pt.x, npt.y - world_pt.y );
                }
            }
            point_data.RemoveLast();

            m->SetProperty( "NumPoints", wxString::Format( "%d", (int)model.points.size() ) );
            m->SetProperty( "PointData", point_data );
            m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", world_pt.x ) );
            m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", world_pt.y ) );
            m->SetProperty( "WorldPosZ", "0.0" );
            m->SetProperty( "versionNumber", "5" );
        } else if( model.shapeName.Contains( "-Unconnected" ) ) {
            //TODO skip middle sections
            wxString point_data;
            int points = 0;
            S5Point world_pt;
            for( int i = 0; i < model.points.size()-1; i+=2 ) {//skip every other point
                if( i == 0 ) {
                    world_pt = ScalePointToXLights( model.points.at( i ), previewW, previewH );
                    point_data += "0.0,0.0,0.0,";
                } else {
                    auto const& npt = ScalePointToXLights( model.points.at( i ), previewW, previewH );
                    point_data += wxString::Format( "%f,%f,0.0,", npt.x - world_pt.x, npt.y - world_pt.y );
                }
                points++;
            }
            auto const& lastpt = ScalePointToXLights( model.points.back(), previewW, previewH );
            point_data += wxString::Format( "%f,%f,0.0", lastpt.x - world_pt.x, lastpt.y - world_pt.y );
            points++;
            m->SetProperty( "NumPoints", wxString::Format( "%d", points ) );
            m->SetProperty( "PointData", point_data );
            m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", world_pt.x ) );
            m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", world_pt.y ) );
            m->SetProperty( "WorldPosZ", "0.0" );
            m->SetProperty( "versionNumber", "5" );
        } else if( model.shapeName.Contains( "-Closed Shape" ) ) {
            wxString point_data;

            S5Point world_pt;
            for( int i = 0; i < model.points.size(); ++i ) {
                if( i == 0 ) {
                    world_pt = ScalePointToXLights( model.points.at( i ), previewW, previewH );
                    point_data += "0.0,0.0,0.0,";
                } else {
                    auto const& npt = ScalePointToXLights( model.points.at( i ), previewW, previewH );
                    point_data += wxString::Format( "%f,%f,0.0,", npt.x - world_pt.x, npt.y - world_pt.y );
                }
            }

            //loop back to the first point AKA world point
            point_data += "0.0,0.0,0.0";

            m->SetProperty( "NumPoints", wxString::Format( "%d", (int)model.points.size() + 1 ) );
            m->SetProperty( "PointData", point_data );
            m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", world_pt.x ) );
            m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", world_pt.y ) );
            m->SetProperty( "WorldPosZ", "0.0" );
            m->SetProperty( "versionNumber", "5" );
        }

        m->SetProperty( "ScaleX", "1.0000" );
        m->SetProperty( "ScaleY", "1.0000" );
        m->SetProperty( "ScaleZ", "1.0000" );

    } else if( model.shapeName.StartsWith( "Matrix" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Matrix", startChan );
        
        supportsMultiString = true;

        m->SetProperty( "parm1", wxString::Format( "%d", model.parms.at( 0 ) ) );     //number of strings
        m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 1 ) ) );     //number of nodes
        m->SetProperty( "parm3", wxString::Format( "%d", model.parms.at( 2 ) + 1 ) ); //number of folds is one less than number of stands per string in xLights

        if( model.shapeName.Contains( "Vertical" ) ) {
            m->SetProperty( "DisplayAs", "Vert Matrix" );
        } else {
            m->SetProperty( "DisplayAs", "Horiz Matrix" );
        }

        //Decode Direction and Start Location
        SetDirection( model, m );
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Sphere" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Sphere", startChan );

        m->SetProperty( "parm1", wxString::Format( "%d", model.parms.at( 0 ) ) );     //number of strings
        m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 1 ) ) );     //number of nodes
        m->SetProperty( "parm3", wxString::Format( "%d", model.parms.at( 2 ) + 1 ) ); //number of folds is one less than number of stands per string in xLights

        m->SetProperty( "Degrees", wxString::Format( "%d", ( model.parms.at( 4 ) * 90 ) ) );

        m->SetProperty( "StartLatitude", wxString::Format( "%d", ( (float)model.parms.at( 5 ) / 100.0 * -86.0 ) ) );
        m->SetProperty( "EndLatitude", wxString::Format( "%d", ( (float)model.parms.at( 5 ) / 100.0 * 86.0 ) ) );

        SetDirection( model, m );
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Spokes" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Spinner", startChan );

        supportsMultiString = true;
        m->SetProperty( "parm1", wxString::Format( "%d", model.parms.at( 0 ) ) );
        m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 1 ) ) );
        m->SetProperty( "parm3", "1" );

        if( model.startLocation.Contains( "Top" ) ) {
            m->SetProperty( "StartSide", "T" );
        } else {
            m->SetProperty( "StartSide", "B" );
        }
        if( model.startLocation.Contains( "Counter" ) ) {
            m->SetProperty( "Dir", "L" );
        } else {
            m->SetProperty( "Dir", "R" );
        }

        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Star" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Star", startChan );

        if( model.shapeName.Contains( "Nested" ) ) {
            //xLights can not models this Exactly right, we don't support uneven strings....
            int totalCount  = 0;
            int totalLayers = 0;
            wxString layers;
            for( auto layer : model.parms ) {
                if( layer != 0 ) {
                    totalCount += layer;
                    totalLayers++;
                    layers = layers + wxString::Format( "%d", layer ) + ",";
                }
            }
            layers.RemoveLast(); //remove last ","

            m->SetProperty( "parm1", "1" );                                  //number of strings
            m->SetProperty( "parm2", wxString::Format( "%d", totalCount ) ); //number of nodes
            m->SetProperty( "parm3", "5" );                                  //number of points
            m->SetProperty( "LayerSizes", layers );
        } else { //Regular Star
            supportsMultiString = true;
            m->SetProperty( "parm1", wxString::Format( "%d", model.parms.at( 0 ) ) ); //number of strings
            m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 1 ) ) ); //number of nodes
            m->SetProperty( "parm3", wxString::Format( "%d", model.parms.at( 2 ) ) ); //number of points

            //try to conver LOR ratio to xLights
            float radio = 2.618034 * ( (float)model.parms.at( 3 ) / 10.0 );
            m->SetProperty( "starRatio", wxString::Format( "%lf", radio ) );
        }

        //Convert Start Location and Dir, CW to Ctr-CCW, CCW to Ctr-CW, this is super confussing.....
        wxString startLoc = model.startLocation;
        if( model.startLocation.EndsWith( "-CW" ) ) {
            startLoc.Replace( "-CW", " Ctr-CCW" );
        }
        if( model.startLocation.EndsWith( "-CCW" ) ) {
            startLoc.Replace( "-CCW", " Ctr-CW" );
        }
        m->SetProperty( "StarStartLocation", startLoc );

        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Tree" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Tree", startChan );
        supportsMultiString = true;

        m->SetProperty( "parm1", wxString::Format( "%d", model.parms.at( 0 ) ) ); //number of strings
        m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 1 ) ) ); //number of nodes

        m->SetProperty( "DisplayAs", DecodeTreeType( model.shapeName ) );

        if( model.shapeName.Contains( "spiral" ) ) {
            float roation = (float)model.parms.at( 2 ) / 10.0; //120 in xml = 12.0 rotaions
            if( model.startLocation.Contains( "CCW" ) ) {
                roation *= -1;
            }
            m->SetProperty( "TreeSpiralRotations", wxString::Format( "%f", roation ) );
        } else {
            //normal tree
            m->SetProperty( "parm3", wxString::Format( "%d", model.parms.at( 2 ) + 1 ) ); //number of folds is one less than number of stands per string in xLights
        }
        //Decode Direction and Start Location
        SetDirection( model, m );
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Window Frame" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Window Frame", startChan );
        m->SetProperty( "parm1", wxString::Format( "%d", model.parms.at( 0 ) ) ); //sections in LOR
        m->SetProperty( "parm2", wxString::Format( "%d", model.parms.at( 1 ) ) ); //number of lights per section
        m->SetProperty( "parm3", wxString::Format( "%d", model.parms.at( 2 ) ) ); //number of lights per section

        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Wreath" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Circle", startChan );
        m->SetProperty( "parm1", wxString::Format( "%d", model.parms[ 0 ] ) ); //number of "sections"
        m->SetProperty( "parm2", wxString::Format( "%d", model.parms[ 1 ] ) ); //number of nodes

        if( model.startLocation.Contains( "Top" ) || model.startLocation.Contains( "Bottom" ) ) {
            m->SetProperty( "StartSide", model.startLocation.Left( 1 ) );
        }
        if( model.startLocation.Contains( "CCW" ) ) {
            m->SetProperty( "Dir", "L" );
        } else {
            m->SetProperty( "Dir", "R" );
        }
        ScaleToPreview( model, m, previewW, previewH );
    } else {
        m = xlights->AllModels.CreateDefaultModel( "Single Line" );
        ScaleToPreview( model, m, previewW, previewH );
        error = true;
        logger_base.debug( "Unknown LOR S5 Model type %s.", (const char*)model.shapeName.c_str() );
    }

    //Decode Type, "Traditional" vs "RGB"
    //Decode RGB Node Type
    //Decode Traditional Colors
    SetStringType( model, m );
    SetBulbTypeSize( model, m );

    //Decode Start Channel
    SetStartChannel( model, m, supportsMultiString );

    m->SetProperty( "LayoutGroup", xLigths_preview );

    //rename
    auto newName = xlights->AllModels.GenerateModelName( Model::SafeModelName(model.name ));
    m->SetProperty( "name", newName, true );

    return m;
}

wxString LORPreview::DecodeTreeType( wxString const& value ) const {
    auto const parts = wxSplit( value, ' ' );
    if( parts.size() > 1 ) {
        return parts[ 0 ] + " " + parts[ 1 ];
    }
    return "Tree 360";
}

void LORPreview::SetStartChannel( S5Model const& model, Model* xModel, bool doMultiString ) {
    if( model.deviceType.IsSameAs( "DMX" ) ) {                     //only set start channel if type is DMX(E131), LOR type is on their own
        if( model.channelGrid.Contains( ";" ) && doMultiString ) { //multistring start channel
            auto const multaddress = wxSplit( model.channelGrid, ';' );
            xModel->SetProperty( "Advanced", "1" );
            int i = 0;
            for( auto const& address : multaddress ) {
                int universe;
                int chan;
                if( GetStartUniverseChan( address, universe, chan ) ) {
                    xModel->SetProperty( Model::StartChanAttrName( i ), "#" + std::to_string( universe ) + ":" + std::to_string( chan ) );
                }
                i++;
            }
        } else { //single string start channel
            int universe;
            int chan;
            if( GetStartUniverseChan( model.channelGrid, universe, chan ) ) {
                xModel->SetProperty( "StartChannel", "#" + std::to_string( universe ) + ":" + std::to_string( chan ) );
            }
        }
    }
}

void LORPreview::SetDirection( S5Model const& model, Model* xModel ) {
    if( model.startLocation.Contains( "Left" ) ) {
        xModel->SetProperty( "Dir", "L" );
    }
    if( model.startLocation.Contains( "Right" ) ) {
        xModel->SetProperty( "Dir", "R" );
    }
    if( model.startLocation.Contains( "Bottom" ) ) {
        xModel->SetProperty( "StartSide", "B" );
    }
    if( model.startLocation.Contains( "Top" ) ) {
        xModel->SetProperty( "StartSide", "T" );
    }
}

void LORPreview::SetStringType( S5Model const& model, Model* xModel ) {
    if( model.stringType.IsSameAs( "Traditional" ) ) {
        if( model.traditionalType.IsSameAs( "Multicolor_string_1_ch" ) ) {
            if( model.traditionalColors.Contains( "," ) ) { //multicolor strings
                xModel->SetProperty( "StringType", "Single Color Intensity" );
            } else {
                //Single Color
                if( model.traditionalColors == "Red" || model.traditionalColors == "Blue" || 
                    model.traditionalColors == "Green" ||model.traditionalColors == "White" ) {
                    wxString const color = wxString::Format( "Single Color %s", model.traditionalColors );
                    xModel->SetProperty( "StringType", color );
                } else {
                    xlColor colors( model.traditionalColors );
                    xModel->SetProperty( "StringType", "Single Color Custom" );
                    xModel->SetProperty( "CustomColor", colors );
                }
            }
        } else if( model.traditionalType.IsSameAs( "Channel_per_color" ) ) {
            if( !model.traditionalColors.Contains( "," ) ) { //single color
                wxString const color = wxString::Format( "Single Color %s", model.traditionalColors );
                xModel->SetProperty( "StringType", color );
            } else { //multi color
                xModel->SetProperty( "StringType", "Superstring" );
                auto const colors = wxSplit( model.traditionalColors, ',' );
                //superstrings
                for( auto const& color : colors ) {
                    xModel->AddSuperStringColour( wxColor( color ), false );
                }
                xModel->SaveSuperStringColours();
            }
        }
    } else if( model.stringType.IsSameAs( "DumbRGB" ) ) {
        if( model.rgbOrder.Contains( "RGB" ) ) {
            xModel->SetProperty( "StringType", "3 Channel RGB" );
        } else {
            //superstrings
            xModel->SetProperty( "StringType", "Superstring" );
            for( int i = 0; i < 3; i++ ) {
                wxString color( model.rgbOrder[ i ] ); //take char
                color.Replace( "R", "Red" );           //convert to full color
                color.Replace( "G", "Green" );
                color.Replace( "B", "Blue" );
                xModel->AddSuperStringColour( wxColor( color ), false );
            }
            xModel->SaveSuperStringColours();
        }
    } else if( model.stringType.IsSameAs( "RGB" ) ) { //RGB nodes
        auto order = model.rgbOrder;
        order.Replace( "order", "Nodes" );
        xModel->SetProperty( "StringType", order );
    }
}

void LORPreview::SetBulbTypeSize( S5Model const& model, Model* xModel ) {
    if( model.bulbShape == "Square" ) {
        xModel->SetProperty( "Antialias", "0" );
    }
    xModel->SetProperty( "PixelSize", std::to_string( model.previewBulbSize ) );

    // Opacity="255" //full dark 255-0
    //"Transparency" 0-100
    xModel->SetProperty( "Transparency", std::to_string( int( ( 255.0 - model.opacity ) / 2.55 ) ) );
}

bool LORPreview::GetStartUniverseChan( wxString const& value, int& unv, int& chan ) const {
    auto const parts = wxSplit( value, ',' );
    if( parts.size() == 5 || parts.size() == 6 ) {
        unv  = wxAtoi( parts.at( 1 ) );
        chan = wxAtoi( parts.at( 2 ) );
        return true;
    }
    return false;
}

void LORPreview::ScaleToPreview( S5Model const& model, Model* m, int pvwW, int pvwH ) const {
    m->SetProperty( "versionNumber", "5", true );
    int const bwidth  = m->GetWidth();
    int const bheight = m->GetHeight();

    auto xModelCenter = ScalePointToXLights( model.offset, pvwW, pvwH );
    auto xSize        = GetXLightsSizeFromScale( model.scale, pvwW, pvwH );

    float worldPos_x = xModelCenter.x;
    float worldPos_y = xModelCenter.y;
    float worldPos_z = 0.0f;
    float scalex     = xSize.x / bwidth;
    float scaley     = xSize.y / bheight;
    float scalez     = scalex;
    float rotatex    = 0.0f;
    float rotatey    = 0.0f;
    float rotatez    = 0.0f;
    if( model.radians != 0.0f ) {
        rotatez = ( model.radians ) * 180.0f / M_PI;
    }

    m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", worldPos_x ) );
    m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", worldPos_y ) );
    m->SetProperty( "WorldPosZ", wxString::Format( "%6.4f", worldPos_z ) );
    m->SetProperty( "ScaleX", wxString::Format( "%6.4f", scalex ) );
    m->SetProperty( "ScaleY", wxString::Format( "%6.4f", scaley ) );
    m->SetProperty( "ScaleZ", wxString::Format( "%6.4f", scalez ) );

    m->SetProperty( "RotateX", wxString::Format( "%4.8f", rotatex ) );
    m->SetProperty( "RotateY", wxString::Format( "%4.8f", rotatey ) );
    m->SetProperty( "RotateZ", wxString::Format( "%4.8f", rotatez ) );
}

void LORPreview::ScalePointsToSingleLine( S5Model const& model, Model* m, int pvwW, int pvwH ) const {
    if( model.points.size() == 0 ) {
        return;
    }
    m->SetProperty( "versionNumber", "5", true );

    auto xModelFirst = ScalePointToXLights( model.points.front(), pvwW, pvwH );

    if( model.points.size() > 1 ) {
        auto xModelLast = ScalePointToXLights( model.points.back(), pvwW, pvwH );
        m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", xModelFirst.x ) );
        m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", xModelFirst.y ) );
        m->SetProperty( "WorldPosZ", "0.0000" );

        m->SetProperty( "X2", wxString::Format( "%6.4f", xModelLast.x - xModelFirst.x ) );
        m->SetProperty( "Y2", wxString::Format( "%6.4f", xModelLast.y - xModelFirst.y ) );
        m->SetProperty( "Z2", "0.0000" );
    } else {
        m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", xModelFirst.x - 5) );
        m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", xModelFirst.y ) );
        m->SetProperty( "WorldPosZ", "0.0000" );

        m->SetProperty( "X2", "10.0000" );
        m->SetProperty( "Y2", "0.0000" );
        m->SetProperty( "Z2", "0.0000" );
    }
}

void LORPreview::ScaleModelToSingleLine( S5Model const& model, Model* m, int pvwW, int pvwH ) const {

    auto xModelCenter = ScalePointToXLights( model.offset, pvwW, pvwH );
    auto xSize        = GetXLightsSizeFromScale( model.scale, pvwW, pvwH );
    if( model.startLocation == "Top" ) {
        m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", xModelCenter.x ) );
        m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", xModelCenter.y + ( xSize.y / 2.0f) ) );
        m->SetProperty( "X2", "0.0000" );
        m->SetProperty( "Y2", wxString::Format( "%6.4f", - ( xSize.y / 2.0f ) ) );
    } else if( model.startLocation == "Bottom" ) {
        m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", xModelCenter.x ) );
        m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", xModelCenter.y - ( xSize.y / 2.0f ) ) );
        m->SetProperty( "X2", "0.0000" );
        m->SetProperty( "Y2", wxString::Format( "%6.4f",  ( xSize.y / 2.0f ) ) );
    } else if( model.startLocation == "Left" ) {
        m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", xModelCenter.x + ( xSize.x / 2.0f ) ) );
        m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", xModelCenter.y ) );
        m->SetProperty( "X2", wxString::Format( "%6.4f",  - ( xSize.x / 2.0f ) ) );
        m->SetProperty( "Y2", "0.0000" );
    } else if( model.startLocation == "Right" ) {
        m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", xModelCenter.x - ( xSize.x / 2.0f ) ) );
        m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", xModelCenter.y ) );
        m->SetProperty( "X2", wxString::Format( "%6.4f", ( xSize.x / 2.0f ) ) );
        m->SetProperty( "Y2", "0.0000" );
    } else  {//should not happen.....
        m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", xModelCenter.x ) );
        m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", xModelCenter.y ) );
        m->SetProperty( "X2", wxString::Format( "%6.4f", xSize.x ) );
        m->SetProperty( "Y2", wxString::Format( "%6.4f", xSize.y ) );
    }

    m->SetProperty( "WorldPosZ", "0.0000" );
    m->SetProperty( "Z2", "0.0000" );

    m->SetProperty( "ScaleX", "1.0000" );
    m->SetProperty( "ScaleY", "1.0000" );
    m->SetProperty( "ScaleZ", "1.0000" );
    m->SetProperty( "versionNumber", "5", true );
}

void LORPreview::ScaleIcicleToSingleLine( S5Model const& model, int maxdrop, Model* m, int pvwW, int pvwH ) const {

    if( model.points.size() < 4 ) {
        return;
    }
    /*
     *   LOR Screen Point locations
     * 
     *    0              1
     *
     *    3              2
     */

    m->SetProperty( "versionNumber", "5", true );

    auto xModelFirst = ScalePointToXLights( model.points.at( 0 ), pvwW, pvwH );
    auto xModelSecond = ScalePointToXLights( model.points.at( 1 ), pvwW, pvwH );
    auto xModelFour = ScalePointToXLights( model.points.at( 3 ), pvwW, pvwH );

    m->SetProperty( "WorldPosX", wxString::Format( "%6.4f", xModelFirst.x ) );
    m->SetProperty( "WorldPosY", wxString::Format( "%6.4f", xModelFirst.y ) );
    m->SetProperty( "WorldPosZ", "0.0000" );

    m->SetProperty( "X2", wxString::Format( "%6.4f", xModelSecond.x - xModelFirst.x ) );
    m->SetProperty( "Y2", wxString::Format( "%6.4f", xModelSecond.y - xModelFirst.y ) );
    m->SetProperty( "Z2", "0.0000" );
    //kinda a guess
    float new_Height = (( xModelFirst.y - xModelFour.y ) / (float)maxdrop) / 50.0F;
    m->SetProperty( "Height", wxString::Format( "%6.4f", new_Height * -1.0f ) );

    m->SetProperty( "ScaleX", "1.0000" );
    m->SetProperty( "ScaleY", "1.0000" );
    m->SetProperty( "ScaleZ", "1.0000" );
}

S5Point LORPreview::ScalePointToXLights( S5Point const& pt, int pvwW, int pvwH ) const
{
    /*
     *   LOR Screen
     *   absolute
     *    _________
     * 1 |         |
     *   |         |
     * 0 |         |
     *   |         |
     *-1 |_________|
     *   -1   0    1
     *
     *   xLights Screen
     * pvwW = 10, pvwH = 10
     *    _________
     *10 |         |
     *   |         |
     * 5 |         |
     *   |         |
     * 0 |_________|
     *   0    5   10
     * 
     */

    int xLights_x = ( (pvwW / 2) * ( pt.x + 1 ) ) ;
    int xLights_y = ( (pvwH / 2) * ( pt.y + 1 ) ) ;
    return S5Point( xLights_x, xLights_y );
}

S5Point LORPreview::GetXLightsSizeFromScale( S5Point const& scale, int pvwW, int pvwH ) const
{
    float xLightsWidth  = 1.0f;
    float xLightsHeight = 1.0f;

    if( scale.x < 5.0f ) { //models will have crazy high scale values on the axes that doesn't matter
        xLightsWidth = ( (float)pvwW / 2.0f ) * scale.x;
    } else {
        xLightsWidth = ( (float)pvwW / 2.0f ) * scale.y;
    }

    if( scale.y < 5.0f ) {
        xLightsHeight = ( (float)pvwH / 2.0f ) * scale.y;
    } else {
        xLightsHeight = ( (float)pvwH / 2.0f ) * scale.x;
    }
    return S5Point( xLightsWidth, xLightsHeight );
}

void LORPreview::CreateGroup( S5Group const& grp, std::vector< S5Model > const& models )
{
    auto newName    = xlights->AllModels.GenerateModelName( Model::SafeModelName(grp.name ));
    wxXmlNode* node = new wxXmlNode( wxXML_ELEMENT_NODE, "modelGroup" );
    xlights->ModelGroupsNode->AddChild( node );
    node->AddAttribute( "selected", "0" );
    node->AddAttribute( "name", newName );
    node->AddAttribute( "layout", "minimalGrid" );
    node->AddAttribute( "GridSize", "400" );
    node->AddAttribute( "LayoutGroup", xLigths_preview );

    // create group and reload before adding selected models. prior models were added before create and I was seeing frequent
    // crashes in Render() with invalid model pointers especially with mixed selections (groups, submodels & models)
    xlights->AllModels.AddModel( xlights->AllModels.CreateModel( node ) );

    wxArrayString newGroupModels;
    for( auto const& id : grp.modelIds ) {
        auto const& index = std::find_if( models.cbegin(), models.cend(), [ id ]( auto const& model ) {
            return model.id == id;
        } );
        if( index != models.cend() ) {
            newGroupModels.push_back( Model::SafeModelName(( *index ).name) );
        }
    }
    // now add the group models to already created group
    node->DeleteAttribute( "models" );
    wxString groups = wxJoin( newGroupModels, ',' );
    node->AddAttribute( "models", groups );
}

wxString LORPreview::FindLORPreviewFile()
{
    //Default LOR S5 Prieview File Location On Windows
    //C:\Users\scoot\Documents\Light-O-Rama\CommonData\LORPreviews.xml

    wxString const filePath = wxStandardPaths::Get().GetDocumentsDir() + wxFileName::GetPathSeparator() + "Light-O-Rama" + wxFileName::GetPathSeparator() + "CommonData" + wxFileName::GetPathSeparator() + "LORPreviews.xml";
    if( wxFile::Exists( filePath ) ) {
        return filePath;
    }

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
#ifdef __WXOSX__
    wxString const wildcard = "*.xml";
#else
    wxString const wildcard = "LORPreviews.xml";
#endif
    return wxFileSelector( _( "Choose LOR S5 Preview File to Import" ), wxEmptyString,
                            XLIGHTS_RGBEFFECTS_FILE, wxEmptyString,
                            "LOR S5 Preview File (LORPreviews.xml)|" + wildcard,
                            wxFD_FILE_MUST_EXIST | wxFD_OPEN );
}

wxArrayString LORPreview::GetPreviews( wxXmlNode* root ) const
{
    wxArrayString previews;
    if( root != nullptr ) {
        for( wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext() ) {
            if( n->GetName() == "PreviewClass" ) {
                previews.emplace_back( n->GetAttribute( "Name" ) );
            }
        }
    }
    previews.Sort();
    return previews;
}

#ifdef _DEBUG
void LORPreview::RunTests() {
    //built in testing, until I add google test some day
    auto pt = ScalePointToXLights( S5Point( 0.0f, 0.0f ), 10, 10 );
    wxASSERT( pt.x == 5.0f );
    wxASSERT( pt.y == 5.0f );

    pt = ScalePointToXLights( S5Point( -1.0f, -1.0f ), 10, 10 );
    wxASSERT( pt.x == 0.0f );
    wxASSERT( pt.y == 0.0f );

    pt = ScalePointToXLights( S5Point( 1.0f, 1.0f ), 10, 10 );
    wxASSERT( pt.x == 10.0f );
    wxASSERT( pt.y == 10.0f );

    pt = ScalePointToXLights( S5Point( -0.373974591f, -0.431859612f ), 1280, 720 );
    wxASSERT( pt.x == 400.0f );
    wxASSERT( pt.y == 204.0f );

    pt = ScalePointToXLights( S5Point( -0.07563785f, -0.26f ), 1280, 720 );
    wxASSERT( pt.x == 591.0f );
    wxASSERT( pt.y == 266.0f );

    pt = ScalePointToXLights( S5Point( 0.1659517f, -0.4756281f ), 1280, 720 );
    wxASSERT( pt.x == 746.0f );
    wxASSERT( pt.y == 188.0f );

    pt = GetXLightsSizeFromScale( S5Point( 1.0f, 1.0f ), 10, 10 );
    wxASSERT( pt.x == 5.0f );
    wxASSERT( pt.y == 5.0f );

    pt = GetXLightsSizeFromScale( S5Point( 2.0f, 2.0f ), 10, 10 );
    wxASSERT( pt.x == 10.0f );
    wxASSERT( pt.y == 10.0f );

    pt = GetXLightsSizeFromScale( S5Point( 0.5f, 0.5f ), 10, 10 );
    wxASSERT( pt.x == 2.5f );
    wxASSERT( pt.y == 2.5f );

    int unv, chan;
    wxASSERT( GetStartUniverseChan( "Regular,40,1,300,0,", unv, chan ) );
    wxASSERT( unv == 40 );
    wxASSERT( chan == 1 );

    wxASSERT( GetStartUniverseChan( "Regular,1,509,300,0,", unv, chan ) );
    wxASSERT( unv == 1 );
    wxASSERT( chan == 509 );

    wxASSERT( "Tree 360" == DecodeTreeType( "ARRRR" ) );
    wxASSERT( "Tree 90" == DecodeTreeType( "Tree 90" ) );
    wxASSERT( "Tree 180" == DecodeTreeType( "Tree 180" ) );
    wxASSERT( "Tree 270" == DecodeTreeType( "Tree 270" ) );
    wxASSERT( "Tree 360" == DecodeTreeType( "Tree 360 spirals" ) );
    wxASSERT( "Tree 360" == DecodeTreeType( "Tree 360 up&over" ) );
    wxASSERT( "Tree 360" == DecodeTreeType( "Tree 360 panels" ) );
    wxASSERT( "Tree 360" == DecodeTreeType( "Tree 360 tiers" ) );
    wxASSERT( "Tree 360" == DecodeTreeType( "Tree 360 wedges" ) );
}
#endif