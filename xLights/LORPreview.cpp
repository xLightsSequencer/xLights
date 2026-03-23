/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LORPreview.h"

#include "ModelPreview.h"
#include "UtilFunctions.h"
#include "xLightsMain.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/ModelGroup.h"
#include "models/ArchesModel.h"
#include "models/CandyCaneModel.h"
#include "models/CircleModel.h"
#include "models/CustomModel.h"
#include "models/IciclesModel.h"
#include "models/MatrixModel.h"
#include "models/SphereModel.h"
#include "models/SpinnerModel.h"
#include "models/StarModel.h"
#include "models/TreeModel.h"
#include "models/SingleLineModel.h"
#include "models/WindowFrameModel.h"
#include "models/PolyLineModel.h"
#include "models/BoxedScreenLocation.h"
#include "models/TwoPointScreenLocation.h"
#include "models/ThreePointScreenLocation.h"
#include "models/PolyPointScreenLocation.h"
#include "ExternalHooks.h"
#include "XmlSerializer/XmlSerializeFunctions.h"

#include <log4cpp/Category.hh>

#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/wx.h>
#include <wx/gdicmn.h>

bool S5Model::ParseXML( pugi::xml_node m ) {
    id                 = m.attribute( "id" ).as_string();
    name               = m.attribute( "Name" ).as_string();
    bulbShape          = m.attribute( "BulbShape" ).as_string();
    deviceType         = m.attribute( "DeviceType" ).as_string();
    individualChannels = std::string_view(m.attribute( "IndividualChannels" ).as_string()) == "True";
    previewBulbSize    = m.attribute( "PreviewBulbSize" ).as_int(2);
    rgbOrder           = m.attribute( "RgbOrder" ).as_string();
    separateIds        = std::string_view(m.attribute( "SeparateIds" ).as_string()) == "True";
    startLocation      = m.attribute( "StartLocation" ).as_string("n/a");
    stringType         = m.attribute( "StringType" ).as_string();
    traditionalColors  = m.attribute( "TraditionalColors" ).as_string();
    traditionalType    = m.attribute( "TraditionalType" ).as_string();
    channelGrid        = m.attribute( "ChannelGrid" ).as_string();
    opacity            = m.attribute( "Opacity" ).as_int(0);

    ParseParms( m );

    for( pugi::xml_node shape = m.first_child(); shape; shape = shape.next_sibling() ) {
        if( std::string_view(shape.name()) == "shape" ) {
            shapeName = shape.attribute( "ShapeName" ).as_string();
            //custom model
            customWidth  = shape.attribute( "CustomWidth" ).as_string("5");
            customHeight = shape.attribute( "CustomHeight" ).as_string("5");
            customGrid   = shape.attribute( "CustomGrid" ).as_string();
            //only non line/point models have these
            offset.x = shape.attribute( "OffsetX" ).as_float(0.0f);
            offset.y = shape.attribute( "OffsetY" ).as_float(0.0f);
            scale.x  = shape.attribute( "ScaleX" ).as_float(1.0f);
            scale.y  = shape.attribute( "ScaleY" ).as_float(1.0f);
            radians  = shape.attribute( "Radians" ).as_float(0.0f);
            ParsePoints( shape );
        }
    }

    return true;
}

void S5Model::ParseParms( pugi::xml_node p ) {
    for( int i = 1; i < 100; i++ ) { //I, Scott, avoid while loops, so just loop 100 times is probably enough
        std::string parmname = "Parm" + std::to_string(i);
        if( p.attribute( parmname.c_str() ) ) {
            parms.emplace_back( p.attribute( parmname.c_str() ).as_int(0) );
        } else {
            break;
        }
    }
}

void S5Model::ParsePoints( pugi::xml_node p ) {
    for( pugi::xml_node pp = p.first_child(); pp; pp = pp.next_sibling() ) {
        if( std::string_view(pp.name()) == "point" ) {
            points.emplace_back( pp.attribute( "x" ).as_float(0.0f),
                                 pp.attribute( "y" ).as_float(0.0f) );
        }
    }
}

bool S5Group::ParseXML( pugi::xml_node g ) {
    id          = g.attribute( "id" ).as_string();
    name        = g.attribute( "Name" ).as_string();
    arrangement = g.attribute( "Arrangement" ).as_string();
    for( pugi::xml_node m = g.first_child(); m; m = m.next_sibling() ) {
        if( std::string_view(m.name()) == "member" ) {
            modelIds.push_back( m.attribute( "id" ).as_string() );
        }
    }
    return true;
}

LORPreview::LORPreview( xLightsFrame* frame, wxString xLightsPreview ) :
    xlights( frame ),
    xLights_preview( xLightsPreview ) {
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

    if (FileExists( previewfileName )) {
        pugi::xml_document d;
        if (!d.load_file( previewfileName.mb_str() )) {
            logger_base.warn( "LOR S5 Preview file could not be loaded." );
            return false;
        }
        pugi::xml_node root = d.document_element();
        if( root ) {
            if (std::string_view(root.name()) == "PreviewClass") {
                return ReadPreview(root);
            }
            wxArrayString const previews = GetPreviews( root );
            wxSingleChoiceDialog dlg( xlights, "", "Select Preview", previews );
            if( dlg.ShowModal() == wxID_OK ) {
                auto previewName = dlg.GetStringSelection();
                return LoadPreview( root, previewName );
            }
        }
    } else {
        logger_base.warn( "LOR S5 Preview file not fould." );
    }

    return false;
}

bool LORPreview::LoadPreview( pugi::xml_node root, wxString const& name ) {
    if( root ) {
        for( pugi::xml_node n = root.first_child(); n; n = n.next_sibling() ) {
            if( std::string_view(n.name()) == "PreviewClass" ) {
                if( wxString(n.attribute( "Name" ).as_string()) == name ) {
                    return ReadPreview( n );
                }
            }
        }
    }
    return false;
}

bool LORPreview::ReadPreview( pugi::xml_node preview ) {
    int previewWidth  = xlights->AllModels.GetPreviewWidth();
    int previewHeight = xlights->AllModels.GetPreviewHeight();

    std::vector< S5Model > _S5Models;

    for( pugi::xml_node e = preview.first_child(); e; e = e.next_sibling() ) {
        if( std::string_view(e.name()) == "PropClass" ) {
            std::string startChannel = xlights->AllModels.GenerateNewStartChannel();
            S5Model model;
            bool error;
            model.ParseXML( e );
            _S5Models.push_back( model );
            Model* xModel = CreateModel( model, startChannel, previewWidth, previewHeight, error );
            xlights->AllModels.AddModel( xModel );
        } else if( std::string_view(e.name()) == "PropGroup" ) {
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

    pugi::xml_document doc;
    if (wxFileExists(modelFile) && doc.load_file(modelFile.mb_str())) {
        for( pugi::xml_node e = doc.document_element().first_child(); e; e = e.next_sibling() ) {
            if( std::string_view(e.name()) == "PropClass" ) {
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
        auto* archesModel = dynamic_cast<ArchesModel*>( m );
        if( archesModel != nullptr ) {
            if( model.stringType.IsSameAs( "Traditional" ) ) {
                archesModel->SetNumArches( 1 );                    //number of arches
                archesModel->SetNodesPerArch( model.parms.at( 0 ) );  //sections in LOR
                archesModel->SetLightsPerNode( model.parms.at( 1 ) );  //number of lights per section
            } else {
                archesModel->SetNumArches( 1 );                                      //number of arches
                archesModel->SetNodesPerArch( model.parms.at( 0 ) * model.parms.at( 1 ) ); //number of nodes
            }
        }
        SetDirection( model, m );
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Bulb" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Custom", startChan );
        BulbToCustomModel(model, m, previewW, previewH);
    } else if( model.shapeName.StartsWith( "Candycane" ) ) {
        //xLights cannot model multilayer candycans, just make one big one
        m = xlights->AllModels.CreateDefaultModel( "Candy Canes", startChan );
        auto* candyModel = dynamic_cast<CandyCaneModel*>( m );
        if( candyModel != nullptr ) {
            candyModel->SetNumCanes( 1 );                                       //number of canes
            candyModel->SetNodesPerCane( model.parms.at( 0 ) * model.parms.at( 1 ) ); //number of nodes
            candyModel->SetReverse( model.shapeName.Contains( "Left" ) );
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

        auto* circleModel = dynamic_cast<CircleModel*>( m );
        if( circleModel != nullptr ) {
            circleModel->SetNumCircleStrings( 1 );                  //number of strings
            circleModel->SetCircleNodesPerString( totalCount );         //number of nodes
            circleModel->SetCenterPercent( model.parms[ 0 ] );   //center percentage
            circleModel->DeserializeLayerSizes( layers.ToStdString(), false );
            circleModel->SetStartSide( model.startLocation.Left( 1 ).ToStdString() );
            if( model.startLocation.Contains( "CCW" ) ) {
                circleModel->SetDirection( "L" );
            } else {
                circleModel->SetDirection( "R" );
            }
            circleModel->SetInsideOut( !model.startLocation.Contains( "Outer" ) );
        }
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Custom" ) || model.shapeName.StartsWith( "Advance" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Custom", startChan );
        auto* customModel = dynamic_cast<CustomModel*>( m );
        if( customModel != nullptr ) {
            customModel->SetCustomWidth( wxAtoi( model.customWidth ) );   //width
            customModel->SetCustomHeight( wxAtoi( model.customHeight ) );  //height
            std::vector<std::vector<std::vector<int>>>& locations = customModel->GetData();
            locations = XmlSerialize::ParseCustomModel(model.customGrid);
        }

        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Cylinder" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Tree", startChan );
        auto* treeModel = dynamic_cast<TreeModel*>( m );
        if( treeModel != nullptr ) {
            treeModel->SetNumMatrixStrings( model.parms.at( 0 ) );       //number of strings
            treeModel->SetNodesPerString( model.parms.at( 1 ) );       //number of nodes
            treeModel->SetStrandsPerString( model.parms.at( 2 ) + 1 );   //number of folds is one less than number of stands per string in xLights
            treeModel->SetTreeDegrees( model.parms.at( 4 ) * 90 );
            treeModel->SetTreeBottomTopRatio( 1.0 );

            if( model.shapeName.Contains( "spiral" ) ) {
                float roation = (float)model.parms.at( 2 ) / 10.0F; //120 in xml = 12.0 rotaions
                if( model.startLocation.Contains( "CCW" ) ) {
                    roation *= -1;
                }
                treeModel->SetTreeSpiralRotations( roation );
            } else {
                treeModel->SetStrandsPerString( model.parms.at( 2 ) + 1 ); //number of folds is one less than number of stands per string in xLights
            }
        }

        SetDirection( model, m );
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Fan" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Spinner", startChan );
        auto* spinnerModel = dynamic_cast<SpinnerModel*>( m );
        if( spinnerModel != nullptr ) {
            supportsMultiString = true;
            spinnerModel->SetNumSpinnerStrings( model.parms.at( 0 ) );
            spinnerModel->SetNodesPerArm( model.parms.at( 1 ) );
            spinnerModel->SetArmsPerString( 1 );
            spinnerModel->SetStartAngle( 90 );
            spinnerModel->SetArc( 180 );

            if( model.startLocation.Contains( "Top" ) ) {
                spinnerModel->SetStartSide( "T" );
            } else {
                spinnerModel->SetStartSide( "B" );
            }
            if( model.startLocation.Contains( "CCW" ) ) {
                spinnerModel->SetDirection( "L" );
            } else {
                spinnerModel->SetDirection( "R" );
            }
        }

        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Firestick" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Single Line", startChan );

        int segments   = model.parms.at( 0 );
        int nodesper = model.parms.at( 1 );

        auto* singleLineModel = dynamic_cast<SingleLineModel*>( m );
        if( singleLineModel != nullptr ) {
            singleLineModel->SetNumLines( segments ); //sections in LOR
            singleLineModel->SetNodesPerLine( nodesper ); //number of lights per section
        }

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
        auto* iciclesModel = dynamic_cast<IciclesModel*>( m );
        if( iciclesModel != nullptr ) {
            iciclesModel->SetNumIcicleStrings( 1 );
            iciclesModel->SetLightsPerString( totalNodels );
            iciclesModel->SetDropPattern( dropPattern.ToStdString() );
        }

        ScaleIcicleToSingleLine( model, maxdrop, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Lines" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Poly Line", startChan );

        //int segments   = model.parms.at( 0 );
        int totalNodes = model.parms.at( 1 );
        auto* polyLineModel = dynamic_cast<PolyLineModel*>( m );
        if( polyLineModel != nullptr ) {
            polyLineModel->SetTotalLightCount( totalNodes );
        }
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

            // Set PolyLine screen location properties using direct setters
            auto& screenLoc = m->GetModelScreenLocation();
            auto* polyPointLoc = dynamic_cast<PolyPointScreenLocation*>(&screenLoc);
            if (polyPointLoc != nullptr) {
                polyPointLoc->SetNumPoints((int)model.points.size());
                polyPointLoc->SetDataFromString(point_data.ToStdString());
            }
            screenLoc.SetWorldPosition(glm::vec3(world_pt.x, world_pt.y, 0.0f));
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
            
            // Set PolyLine screen location properties using direct setters
            auto& screenLoc = m->GetModelScreenLocation();
            auto* polyPointLoc = dynamic_cast<PolyPointScreenLocation*>(&screenLoc);
            if (polyPointLoc != nullptr) {
                polyPointLoc->SetNumPoints(points);
                polyPointLoc->SetDataFromString(point_data.ToStdString());
            }
            screenLoc.SetWorldPosition(glm::vec3(world_pt.x, world_pt.y, 0.0f));
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

            // Set PolyLine screen location properties using direct setters
            auto& screenLoc = m->GetModelScreenLocation();
            auto* polyPointLoc = dynamic_cast<PolyPointScreenLocation*>(&screenLoc);
            if (polyPointLoc != nullptr) {
                polyPointLoc->SetNumPoints((int)model.points.size() + 1);
                polyPointLoc->SetDataFromString(point_data.ToStdString());
            }
            screenLoc.SetWorldPosition(glm::vec3(world_pt.x, world_pt.y, 0.0f));
        }

        // Set scale properties using direct setter
        auto& screenLoc = m->GetModelScreenLocation();
        screenLoc.SetScaleMatrix(glm::vec3(1.0f, 1.0f, 1.0f));

    } else if( model.shapeName.StartsWith( "Matrix" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Matrix", startChan );
        auto* matrixModel = dynamic_cast<MatrixModel*>( m );
        if( matrixModel != nullptr ) {
            supportsMultiString = true;
            matrixModel->SetNumMatrixStrings( model.parms.at( 0 ) );       //number of strings
            matrixModel->SetNodesPerString( model.parms.at( 1 ) );       //number of nodes
            matrixModel->SetStrandsPerString( model.parms.at( 2 ) + 1 );   //number of folds is one less than number of stands per string in xLights
            matrixModel->SetVertical( model.shapeName.Contains( "Vertical" ) );
        }

        //Decode Direction and Start Location
        SetDirection( model, m );
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Sphere" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Sphere", startChan );
        auto* sphereModel = dynamic_cast<SphereModel*>( m );
        if( sphereModel != nullptr ) {
            sphereModel->SetNumMatrixStrings( model.parms.at( 0 ) );       //number of strings
            sphereModel->SetNodesPerString( model.parms.at( 1 ) );       //number of nodes
            sphereModel->SetStrandsPerString( model.parms.at( 2 ) + 1 );   //number of folds is one less than number of stands per string in xLights
            sphereModel->SetDegrees( model.parms.at( 4 ) * 90 );
            sphereModel->SetStartLatitude( (float)model.parms.at( 5 ) / 100.0 * -86.0 );
            sphereModel->SetEndLatitude( (float)model.parms.at( 5 ) / 100.0 * 86.0 );
        }

        SetDirection( model, m );
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Spokes" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Spinner", startChan );
        auto* spinnerModel = dynamic_cast<SpinnerModel*>( m );
        if( spinnerModel != nullptr ) {
            supportsMultiString = true;
            spinnerModel->SetNumSpinnerStrings( model.parms.at( 0 ) );
            spinnerModel->SetNodesPerArm( model.parms.at( 1 ) );
            spinnerModel->SetArmsPerString( 1 );

            if( model.startLocation.Contains( "Top" ) ) {
                spinnerModel->SetStartSide( "T" );
            } else {
                spinnerModel->SetStartSide( "B" );
            }
            if( model.startLocation.Contains( "Counter" ) ) {
                spinnerModel->SetDirection( "L" );
            } else {
                spinnerModel->SetDirection( "R" );
            }
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
                    layers = "," + wxString::Format("%d", layer) + layers;
                }
            }
            layers.Remove(0, 1); //remove first ","

            auto* starModel = dynamic_cast<StarModel*>( m );
            if( starModel != nullptr ) {
                starModel->SetNumStarStrings( 1 );           //number of strings
                starModel->SetStarNodesPerString( totalCount );  //number of nodes
                starModel->SetStarPoints( 5 );           //number of points
                starModel->DeserializeLayerSizes( layers.ToStdString(), false );
            }
        } else { //Regular Star
            supportsMultiString = true;
            auto* starModel = dynamic_cast<StarModel*>( m );
            if( starModel != nullptr ) {
                starModel->SetNumStarStrings( model.parms.at( 0 ) ); //number of strings
                starModel->SetStarNodesPerString( model.parms.at( 1 ) ); //number of nodes
                starModel->SetStarPoints( model.parms.at( 2 ) ); //number of points

                //try to convert LOR ratio to xLights
                float radio = 2.618034F * ( (float)model.parms.at( 3 ) / 10.0 );
                starModel->SetStarRatio( radio );
            }
        }

        //Convert Start Location and Dir, CW to Ctr-CCW, CCW to Ctr-CW, this is super confusing.....
        wxString startLoc = model.startLocation;
        if( model.startLocation.EndsWith( "-CW" ) ) {
            startLoc.Replace( "-CW", " Ctr-CCW" );
        }
        if( model.startLocation.EndsWith( "-CCW" ) ) {
            startLoc.Replace( "-CCW", " Ctr-CW" );
        }
        auto* starModel = dynamic_cast<StarModel*>( m );
        if( starModel != nullptr ) {
            starModel->SetStarStartLocation( startLoc.ToStdString() );
        }

        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Tree" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Tree", startChan );
        auto* treeModel = dynamic_cast<TreeModel*>( m );
        if( treeModel != nullptr ) {
            supportsMultiString = true;
            treeModel->SetNumMatrixStrings( model.parms.at( 0 ) ); //number of strings
            treeModel->SetNodesPerString( model.parms.at( 1 ) ); //number of nodes

            // DecodeTreeType returns string like "Tree 360" - extract the degrees
            wxString treeTypeStr = DecodeTreeType( model.shapeName );
            long degrees = 360;
            if( treeTypeStr.AfterFirst(' ').ToLong(&degrees) ) {
                treeModel->SetTreeDegrees( degrees );
            }

            if( model.shapeName.Contains( "spiral" ) ) {
                float roation = (float)model.parms.at( 2 ) / 10.0F; //120 in xml = 12.0 rotations
                if( model.startLocation.Contains( "CCW" ) ) {
                    roation *= -1;
                }
                treeModel->SetTreeSpiralRotations( roation );
            } else {
                //normal tree
                treeModel->SetStrandsPerString( model.parms.at( 2 ) + 1 ); //number of folds is one less than number of stands per string in xLights
            }
        }
        //Decode Direction and Start Location
        SetDirection( model, m );
        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Window Frame" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Window Frame", startChan );
        auto* windowFrameModel = dynamic_cast<WindowFrameModel*>( m );
        if( windowFrameModel != nullptr ) {
            windowFrameModel->SetTopNodes( model.parms.at( 0 ) ); //sections in LOR
            windowFrameModel->SetSideNodes( model.parms.at( 1 ) ); //number of lights per section
            windowFrameModel->SetBottomNodes( model.parms.at( 2 ) ); //number of lights per section
        }

        ScaleToPreview( model, m, previewW, previewH );
    } else if( model.shapeName.StartsWith( "Wreath" ) ) {
        m = xlights->AllModels.CreateDefaultModel( "Circle", startChan );
        auto* circleModel = dynamic_cast<CircleModel*>( m );
        if( circleModel != nullptr ) {
            circleModel->SetNumCircleStrings( model.parms[ 0 ] ); //number of "sections"
            circleModel->SetCircleNodesPerString( model.parms[ 1 ] ); //number of nodes

            if( model.startLocation.Contains( "Top" ) || model.startLocation.Contains( "Bottom" ) ) {
                circleModel->SetStartSide( model.startLocation.Left( 1 ).ToStdString() );
            }
            if( model.startLocation.Contains( "CCW" ) ) {
                circleModel->SetDirection( "L" );
            } else {
                circleModel->SetDirection( "R" );
            }
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

    m->SetLayoutGroup( xLights_preview );

    //rename
    auto newName = xlights->AllModels.GenerateModelName( Model::SafeModelName(model.name ));
    m->SetName(newName);

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
            xModel->SetHasIndividualStartChannels(true);
            xModel->SetIndivStartChannelCount(multaddress.GetCount());
            int i = 0;
            for( auto const& address : multaddress ) {
                int universe;
                int chan;
                if( GetStartUniverseChan( address, universe, chan ) ) {
                    xModel->SetIndividualStartChannel(i, "#" + std::to_string( universe ) + ":" + std::to_string( chan ));
                }
                i++;
            }
        } else { //single string start channel
            int universe;
            int chan;
            if( GetStartUniverseChan( model.channelGrid, universe, chan ) ) {
                xModel->SetStartChannel("#" + std::to_string( universe ) + ":" + std::to_string( chan ));
            }
        }
    }
}

void LORPreview::SetDirection( S5Model const& model, Model* xModel ) {
    if( model.startLocation.Contains( "Left" ) ) {
        xModel->SetDirection( "L" );
    }
    if( model.startLocation.Contains( "Right" ) ) {
        xModel->SetDirection( "R" );
    }
    if( model.startLocation.Contains( "Bottom" ) ) {
        xModel->SetStartSide( "B" );
    }
    if( model.startLocation.Contains( "Top" ) ) {
        xModel->SetStartSide( "T" );
    }
}

void LORPreview::SetStringType( S5Model const& model, Model* xModel ) {
    if( model.stringType.IsSameAs( "Traditional" ) ) {
        if( model.traditionalType.IsSameAs( "Multicolor_string_1_ch" ) ) {
            if( model.traditionalColors.Contains( "," ) ) { //multicolor strings
                xModel->SetStringType( "Single Color Intensity" );
            } else {
                //Single Color
                if( model.traditionalColors == "Red" || model.traditionalColors == "Blue" ||
                    model.traditionalColors == "Green" ||model.traditionalColors == "White" ) {
                    wxString const color = wxString::Format( "Single Color %s", model.traditionalColors );
                    xModel->SetStringType( color.ToStdString() );
                } else {
                    xlColor colors( model.traditionalColors );
                    xModel->SetStringType( "Single Color Custom" );
                    xModel->SetCustomColor( colors );
                }
            }
        } else if( model.traditionalType.IsSameAs( "Channel_per_color" ) ) {
            if( !model.traditionalColors.Contains( "," ) ) { //single color
                wxString const color = wxString::Format( "Single Color %s", model.traditionalColors );
                xModel->SetStringType( color.ToStdString() );
            } else { //multi color
                xModel->SetStringType( "Superstring" );
                auto const colors = wxSplit( model.traditionalColors, ',' );
                //superstrings
                for( auto const& color : colors ) {
                    xlColor c; c.SetFromString(color.ToStdString());
                    xModel->AddSuperStringColour( c );
                }
            }
        }
    } else if( model.stringType.IsSameAs( "DumbRGB" ) ) {
        if( model.rgbOrder.Contains( "RGB" ) ) {
            xModel->SetStringType( "3 Channel RGB" );
        } else {
            //superstrings
            xModel->SetStringType( "Superstring" );
            for( int i = 0; i < 3; i++ ) {
                wxString color( model.rgbOrder[ i ] ); //take char
                color.Replace( "R", "Red" );           //convert to full color
                color.Replace( "G", "Green" );
                color.Replace( "B", "Blue" );
                xlColor c; c.SetFromString(color.ToStdString());
                xModel->AddSuperStringColour( c );
            }
        }
    } else if( model.stringType.IsSameAs( "RGB" ) ) { //RGB nodes
        auto order = model.rgbOrder;
        order.Replace( "order", "Nodes" );
        xModel->SetStringType( order.ToStdString() );
    }
}

void LORPreview::SetBulbTypeSize( S5Model const& model, Model* xModel ) {
    if( model.bulbShape == "Square" ) {
        xModel->SetPixelStyle( Model::PIXEL_STYLE::PIXEL_STYLE_SQUARE );
    }
    xModel->SetPixelSize( model.previewBulbSize );

    // Opacity="255" //full dark 255-0
    //"Transparency" 0-100
    xModel->SetTransparency( int( ( 255.0 - model.opacity ) / 2.55 ) );
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
    int const bwidth  = m->GetWidth();
    int const bheight = m->GetHeight();

    auto xModelCenter = ScalePointToXLights( model.offset, pvwW, pvwH );
    auto xSize        = GetXLightsSizeFromScale( model.scale, pvwW, pvwH );

    float worldPos_x = xModelCenter.x;
    float worldPos_y = xModelCenter.y;
    float worldPos_z = 0.0F;
    float scalex     = xSize.x / bwidth;
    float scaley     = xSize.y / bheight;
    float scalez     = scalex;
    float rotatex    = 0.0F;
    float rotatey    = 0.0F;
    float rotatez    = 0.0F;
    if( model.radians != 0.0F ) {
        rotatez = ( model.radians ) * 180.0f / M_PI;
    }

    // Set screen location properties using direct setters
    auto& screenLoc = m->GetModelScreenLocation();
    screenLoc.SetWorldPosition(glm::vec3(worldPos_x, worldPos_y, worldPos_z));
    
    auto* boxedLoc = dynamic_cast<BoxedScreenLocation*>(&screenLoc);
    if (boxedLoc != nullptr) {
        boxedLoc->SetScale(scalex, scaley);
        boxedLoc->SetScaleZ(scalez);
    }
    
    screenLoc.SetRotation(glm::vec3(rotatex, rotatey, rotatez));
}

void LORPreview::ScalePointsToSingleLine( S5Model const& model, Model* m, int pvwW, int pvwH ) const {
    if( model.points.empty() ) {
        return;
    }

    auto xModelFirst = ScalePointToXLights( model.points.front(), pvwW, pvwH );

    // Set screen location properties using direct setters
    auto& screenLoc = m->GetModelScreenLocation();
    
    if( model.points.size() > 1 ) {
        auto xModelLast = ScalePointToXLights( model.points.back(), pvwW, pvwH );
        screenLoc.SetWorldPosition(glm::vec3(xModelFirst.x, xModelFirst.y, 0.0f));
        
        auto* twoPointLoc = dynamic_cast<TwoPointScreenLocation*>(&screenLoc);
        if (twoPointLoc != nullptr) {
            twoPointLoc->SetX2(xModelLast.x - xModelFirst.x);
            twoPointLoc->SetY2(xModelLast.y - xModelFirst.y);
            twoPointLoc->SetZ2(0.0f);
        }
    } else {
        screenLoc.SetWorldPosition(glm::vec3(xModelFirst.x - 5, xModelFirst.y, 0.0f));
        
        auto* twoPointLoc = dynamic_cast<TwoPointScreenLocation*>(&screenLoc);
        if (twoPointLoc != nullptr) {
            twoPointLoc->SetX2(10.0f);
            twoPointLoc->SetY2(0.0f);
            twoPointLoc->SetZ2(0.0f);
        }
    }
}

void LORPreview::ScaleModelToSingleLine( S5Model const& model, Model* m, int pvwW, int pvwH ) const {

    auto xModelCenter = ScalePointToXLights( model.offset, pvwW, pvwH );
    auto xSize        = GetXLightsSizeFromScale( model.scale, pvwW, pvwH );
    
    // Set screen location properties using direct setters
    auto& screenLoc = m->GetModelScreenLocation();
    auto* twoPointLoc = dynamic_cast<TwoPointScreenLocation*>(&screenLoc);
    
    if( model.startLocation == "Top" ) {
        screenLoc.SetWorldPosition(glm::vec3(xModelCenter.x, xModelCenter.y + (xSize.y / 2.0F), 0.0f));
        if (twoPointLoc != nullptr) {
            twoPointLoc->SetX2(0.0f);
            twoPointLoc->SetY2(-(xSize.y / 2.0F));
            twoPointLoc->SetZ2(0.0f);
        }
    } else if( model.startLocation == "Bottom" ) {
        screenLoc.SetWorldPosition(glm::vec3(xModelCenter.x, xModelCenter.y - (xSize.y / 2.0F), 0.0f));
        if (twoPointLoc != nullptr) {
            twoPointLoc->SetX2(0.0f);
            twoPointLoc->SetY2(xSize.y / 2.0F);
            twoPointLoc->SetZ2(0.0f);
        }
    } else if( model.startLocation == "Left" ) {
        screenLoc.SetWorldPosition(glm::vec3(xModelCenter.x + (xSize.x / 2.0F), xModelCenter.y, 0.0f));
        if (twoPointLoc != nullptr) {
            twoPointLoc->SetX2(-(xSize.x / 2.0F));
            twoPointLoc->SetY2(0.0f);
            twoPointLoc->SetZ2(0.0f);
        }
    } else if( model.startLocation == "Right" ) {
        screenLoc.SetWorldPosition(glm::vec3(xModelCenter.x - (xSize.x / 2.0F), xModelCenter.y, 0.0f));
        if (twoPointLoc != nullptr) {
            twoPointLoc->SetX2(xSize.x / 2.0F);
            twoPointLoc->SetY2(0.0f);
            twoPointLoc->SetZ2(0.0f);
        }
    } else  {//should not happen.....
        screenLoc.SetWorldPosition(glm::vec3(xModelCenter.x, xModelCenter.y, 0.0f));
        if (twoPointLoc != nullptr) {
            twoPointLoc->SetX2(xSize.x);
            twoPointLoc->SetY2(xSize.y);
            twoPointLoc->SetZ2(0.0f);
        }
    }

    // Set scale properties using direct setter
    m->GetModelScreenLocation().SetScaleMatrix(glm::vec3(1.0f, 1.0f, 1.0f));
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

    auto xModelFirst = ScalePointToXLights( model.points.at( 0 ), pvwW, pvwH );
    auto xModelSecond = ScalePointToXLights( model.points.at( 1 ), pvwW, pvwH );
    auto xModelFour = ScalePointToXLights( model.points.at( 3 ), pvwW, pvwH );

    // Set screen location properties using direct setters
    auto& screenLoc = m->GetModelScreenLocation();
    screenLoc.SetWorldPosition(glm::vec3(xModelFirst.x, xModelFirst.y, 0.0f));
    
    auto* threePointLoc = dynamic_cast<ThreePointScreenLocation*>(&screenLoc);
    if (threePointLoc != nullptr) {
        threePointLoc->SetX2(xModelSecond.x - xModelFirst.x);
        threePointLoc->SetY2(xModelSecond.y - xModelFirst.y);
        threePointLoc->SetZ2(0.0f);
        
        //kinda a guess
        float new_Height = (( xModelFirst.y - xModelFour.y ) / (float)maxdrop) / 50.0F;
        threePointLoc->SetMHeight(new_Height * -1.0F);
    }
    
    // Set scale properties using direct setter
    screenLoc.SetScaleMatrix(glm::vec3(1.0f, 1.0f, 1.0f));
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

    float xLights_x = ( (pvwW / 2) * ( pt.x + 1 ) ) ;
    float xLights_y = ((pvwH / 2) * (pt.y + 1));
    return { xLights_x, xLights_y };
}

S5Point LORPreview::GetXLightsSizeFromScale( S5Point const& scale, int pvwW, int pvwH ) const
{
    float xLightsWidth  = 1.0F;
    float xLightsHeight = 1.0F;

    if( scale.x < 5.0F ) { //models will have crazy high scale values on the axes that doesn't matter
        xLightsWidth = ( (float)pvwW / 2.0F ) * scale.x;
    } else {
        xLightsWidth = ( (float)pvwW / 2.0F ) * scale.y;
    }

    if( scale.y < 5.0F ) {
        xLightsHeight = ( (float)pvwH / 2.0F ) * scale.y;
    } else {
        xLightsHeight = ( (float)pvwH / 2.0F ) * scale.x;
    }
    return { xLightsWidth, xLightsHeight };
}

void LORPreview::CreateGroup( S5Group const& grp, std::vector< S5Model > const& models )
{
    auto newName    = xlights->AllModels.GenerateModelName( Model::SafeModelName(grp.name ));

    // Create the model group directly using setters
    ModelGroup* newGroup = new ModelGroup(xlights->AllModels);
    newGroup->SetName(newName);
    newGroup->SetLayout("minimalGrid");
    newGroup->SetGridSize(400);
    newGroup->SetLayoutGroup(xLights_preview);

    // create group before adding selected models to avoid crashes with invalid model pointers
    xlights->AllModels.AddModel( newGroup );

    wxArrayString newGroupModels;
    for( auto const& id : grp.modelIds ) {
        auto const& index = std::find_if( models.cbegin(), models.cend(), [ id ]( auto const& model ) {
            return model.id == id;
        } );
        if( index != models.cend() ) {
            newGroupModels.push_back( Model::SafeModelName(( *index ).name) );
        }
    }
    // now add the group models to the already created group
    std::vector<std::string> modelsList;
    for (const auto& m : newGroupModels) {
        modelsList.push_back(m.ToStdString());
    }
    newGroup->SetModels(modelsList);
}

wxString LORPreview::FindLORPreviewFile()
{
    //Default LOR S5 Preview File Location On Windows
    //C:\Users\scoot\Documents\Light-O-Rama\CommonData\LORPreviews.xml

    wxString const filePath = wxStandardPaths::Get().GetDocumentsDir() + wxFileName::GetPathSeparator() + "Light-O-Rama" + wxFileName::GetPathSeparator() + "CommonData" + wxFileName::GetPathSeparator();

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
#ifdef __WXOSX__
    wxString const wildcard = "*.xml";
#else
    wxString const wildcard = "LORPreviews.xml";
#endif
    return wxFileSelector(_("Choose LOR S5 Preview File to Import"), filePath, wxEmptyString,
                          wxEmptyString,
                          "LOR S5 Preview Files (LORPreviews.xml or .lorprev)|" + wildcard + ";*.lorprev;",
                            wxFD_FILE_MUST_EXIST | wxFD_OPEN );
}

wxArrayString LORPreview::GetPreviews( pugi::xml_node root ) const
{
    wxArrayString previews;
    if( root ) {
        for( pugi::xml_node n = root.first_child(); n; n = n.next_sibling() ) {
            if( std::string_view(n.name()) == "PreviewClass" ) {
                previews.emplace_back( n.attribute( "Name" ).as_string() );
            }
        }
    }
    previews.Sort();
    return previews;
}

void LORPreview::BulbToCustomModel(S5Model const& model, Model* m, int pvwW, int pvwH) const
{
    auto [min, max] = GetMinMax(model.points);
    S5Point size{(max.x - min.x), (max.y - min.y)};
    S5Point center{ min.x + size.x / 2, min.y + size.y / 2 };

    int scale = 10;
    while (!FindBulbModelScale(scale, model.points)) {
        scale += 10;
        if (scale > 101) { // I(Scott) am afraid of infinite while loops
            scale = 100;
            break;
        }
    }

    wxPoint scalemin{ (int)(min.x * scale), (int)(min.y * scale) };
    wxPoint scalesize{ (int)(size.x * scale), (int)(size.y * scale) };
    auto scaleBulbs = ScaleBulbs(model.points, scale, scalemin);

    std::string cm_data;
    for (int y = 0; y < scalesize.y + 1; y++) {
        for (int x = 0; x < scalesize.x + 1; x++) {
            std::string cell;
            if (std::find_if(scaleBulbs.cbegin(), scaleBulbs.cend(), [x, y](auto const& b) {
                    return b.x == x && b.y == y;
                }) != scaleBulbs.cend()) {
                cell = "1" ;
            }
            cm_data += cell + ",";
        }
        cm_data += ";";
    }
    if (!cm_data.empty()) {
        cm_data.pop_back(); // remove last semicolen
    }

    auto cm = dynamic_cast<CustomModel*>(m);
    if( cm != nullptr ) {
        cm->SetCustomWidth(scalesize.x+1); // width
        cm->SetCustomHeight(scalesize.y+1); // height
        std::vector<std::vector<std::vector<int>>>& locations = cm->GetData();
        locations = XmlSerialize::ParseCustomModel(cm_data);
    }

    ScaleBulbToXLights(center, size, scale, m, pvwW, pvwH);
}

void LORPreview::ScaleBulbToXLights(S5Point center, S5Point size, int scale, Model* m, int pvwW, int pvwH) const
{
    auto xModelCenter = ScalePointToXLights(center, pvwW, pvwH);
    auto xSize = GetXLightsSizeFromScale(size, pvwW, pvwH);

    float worldPos_x = xModelCenter.x;
    float worldPos_y = xModelCenter.y;
    float worldPos_z = 0.0F;
    float scalex = xSize.x * (1.0 / scale);
    float scaley = xSize.y * (1.0 / scale);
    float scalez = scalex;

    // Set screen location properties using direct setters
    auto& screenLoc = m->GetModelScreenLocation();
    screenLoc.SetWorldPosition(glm::vec3(worldPos_x, worldPos_y, worldPos_z));
    
    auto* boxedLoc = dynamic_cast<BoxedScreenLocation*>(&screenLoc);
    if (boxedLoc != nullptr) {
        boxedLoc->SetScale(scalex, scaley);
        boxedLoc->SetScaleZ(scalez);
    }
    
    screenLoc.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
}

bool LORPreview::FindBulbModelScale(int scale, std::vector<S5Point> const& bulbs) const
{
    if (bulbs.size() <= 1) {
        return true;
    }
    for (int i = 0; i < bulbs.size(); i++) {
        for (int j = i + 1; j < bulbs.size(); j++) {
            int x1 = (bulbs[i].x * scale);
            int y1 = (bulbs[i].y * scale);
            int x2 = (bulbs[j].x * scale);
            int y2 = (bulbs[j].y * scale);
            if (x1 == x2 && y1 == y2) {
                return false;
            }
        }
    }
    return true;
}

std::pair<S5Point, S5Point> LORPreview::GetMinMax(std::vector<S5Point> const& bulbs) const
{
    S5Point min;
    S5Point max;
    for (auto const& b:bulbs) {
        min.x = std::min(min.x, b.x);
        min.y = std::min(min.y, b.y);
        max.x = std::max(max.x, b.x);
        max.y = std::max(max.y, b.y);
    }
    return { min, max };
}

std::vector<wxPoint> LORPreview::ScaleBulbs(std::vector<S5Point> const& bulbs, int scale, wxPoint offset) const
{
    std::vector<wxPoint> points;
    std::transform(bulbs.begin(), bulbs.end(), std::back_inserter(points),
                   [scale, &offset](S5Point const& b) -> wxPoint { return { (int)(b.x * scale) - offset.x, (int)(b.y * scale) - offset.y }; });
    return points;
}

#ifdef _DEBUG
void LORPreview::RunTests() {
    //built in testing, until I add google test some day
    auto pt = ScalePointToXLights( S5Point( 0.0F, 0.0F ), 10, 10 );
    wxASSERT( pt.x == 5.0F );
    wxASSERT( pt.y == 5.0F );

    pt = ScalePointToXLights( S5Point( -1.0F, -1.0F ), 10, 10 );
    wxASSERT( pt.x == 0.0F );
    wxASSERT( pt.y == 0.0F );

    pt = ScalePointToXLights( S5Point( 1.0F, 1.0F ), 10, 10 );
    wxASSERT( pt.x == 10.0F );
    wxASSERT( pt.y == 10.0F );

    pt = ScalePointToXLights( S5Point( -0.373974591F, -0.431859612F ), 1280, 720 );
    wxASSERT( pt.x == 400.656281F );
    wxASSERT( pt.y == 204.530533F );

    pt = ScalePointToXLights( S5Point( -0.07563785F, -0.26F ), 1280, 720 );
    wxASSERT( pt.x == 591.591797F );
    wxASSERT( pt.y == 266.399994F );

    pt = ScalePointToXLights( S5Point( 0.1659517F, -0.4756281F ), 1280, 720 );
    wxASSERT( pt.x == 746.209106F );
    wxASSERT( pt.y == 188.773865F );

    pt = GetXLightsSizeFromScale( S5Point( 1.0F, 1.0F ), 10, 10 );
    wxASSERT( pt.x == 5.0F );
    wxASSERT( pt.y == 5.0F );

    pt = GetXLightsSizeFromScale( S5Point( 2.0F, 2.0F ), 10, 10 );
    wxASSERT( pt.x == 10.0F );
    wxASSERT( pt.y == 10.0F );

    pt = GetXLightsSizeFromScale( S5Point( 0.5F, 0.5F ), 10, 10 );
    wxASSERT( pt.x == 2.5F );
    wxASSERT( pt.y == 2.5F );

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
