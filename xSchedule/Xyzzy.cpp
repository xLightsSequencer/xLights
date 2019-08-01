#include "Xyzzy.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include "ScheduleOptions.h"
#include "MatrixMapper.h"
#include <wx/config.h>
#include <log4cpp/Category.hh>
#include "xScheduleApp.h"
#include "../xLights/UtilFunctions.h"

#ifdef SHOWVIRTUALMATRIX
#include "VirtualMatrix.h"
#endif

wxColor __colours[] = { *wxRED, wxColor(255, 0, 255), *wxYELLOW, *wxBLUE, wxColor(64, 255, 64), wxColor(0,255,255), wxColor(255,80,0), *wxWHITE };

wxColor XyzzyPiece::GetColour() const
{
    return __colours[GetType()];
}

void XyzzyPiece::Rotate()
{
    _rotation++;
    if (_rotation > 3) _rotation = 0;
}

void XyzzyBase::BaseReset()
{
    _lastUpdatedMovement = wxGetUTCTimeMillis();
    _gameRunning = false;
    _score = 0;
    _playerName = "";
    _colsPerSquare = 0;
    _rowsPerSquare = 0;
    _sideBorder = 0;
    _bottomBorder = 0;
    _matrixMapper = nullptr;
    _isOk = false;
}

void Xyzzy2::Reset()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Xyzzy2 reset.");
    BaseReset();
    _board.clear();
    _body.clear();
    _pill = wxPoint(-1, -1);
    _direction = XYZZY2::RIGHT;
#ifdef SHOWVIRTUALMATRIX
    if (_virtualMatrix != nullptr)
    {
        _virtualMatrix->Stop();
        delete _virtualMatrix;
        _virtualMatrix = nullptr;
    }
#endif
    wxCommandEvent event(EVT_XYZZY);
    wxPostEvent(wxGetApp().GetTopWindow(), event);
}

void Xyzzy::Reset()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Xyzzy reset.");
    BaseReset();
    memset(_board, 0xFF, sizeof(_board));
    if (_nextPiece != nullptr)
    {
        delete _nextPiece;
        _nextPiece = nullptr;
    }
    if (_currentPiece != nullptr)
    {
        delete _currentPiece;
        _currentPiece = nullptr;
    }
    _dropSpeed = STARTSPEED;
    _gameRunning = false;
    _fullTime = 0;
#ifdef SHOWVIRTUALMATRIX
    if (_virtualMatrix != nullptr)
    {
        _virtualMatrix->Stop();
        delete _virtualMatrix;
        _virtualMatrix = nullptr;
    }
#endif
    wxCommandEvent event(EVT_XYZZY);
    wxPostEvent(wxGetApp().GetTopWindow(), event);
}

Xyzzy::Xyzzy()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Xyzzy created.");

    wxConfigBase* config = wxConfigBase::Get();
    _highScore = config->ReadLong(_("XyzzyHighScore"), 0);
    _highScoreOwner = config->Read(_("XyzzyHighScoreOwner"), "").ToStdString();

    logger_base.debug("High score %d %s.", _highScore, (const char *)_highScoreOwner.c_str());

    _nextPiece = nullptr;
    _currentPiece = nullptr;
    _matrixMapper = nullptr;
#ifdef SHOWVIRTUALMATRIX
    _virtualMatrix = nullptr;
#endif

    Reset();

    //MatrixMapper::Test();
    wxCommandEvent event(EVT_XYZZY);
    wxPostEvent(wxGetApp().GetTopWindow(), event);
}

Xyzzy2::Xyzzy2()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Xyzzy2 created.");

    wxConfigBase* config = wxConfigBase::Get();
    _highScore = config->ReadLong(_("Xyzzy2HighScore"), 0);
    _highScoreOwner = config->Read(_("Xyzzy2HighScoreOwner"), "").ToStdString();

    logger_base.debug("High score %d %s.", _highScore, (const char*)_highScoreOwner.c_str());

    _matrixMapper = nullptr;
#ifdef SHOWVIRTUALMATRIX
    _virtualMatrix = nullptr;
#endif

    Reset();

    //MatrixMapper::Test();
    wxCommandEvent event(EVT_XYZZY);
    wxPostEvent(wxGetApp().GetTopWindow(), event);
}

void XyzzyBase::DrawNode(int x, int y, wxColour c, uint8_t* buffer, size_t size)
{
    if (x < 0 || x >= _matrixMapper->GetWidth()) return;
    if (y < 0 || y >= _matrixMapper->GetHeight()) return;

    size_t bl = _matrixMapper->Map(x, y) - 1;

    if (bl > size) return; // outside bounds ... this would crash

    if (bl >= _matrixMapper->GetChannels() + _matrixMapper->GetStartChannelAsNumber())
    {
        // off the top
    }
    else
    {
        uint8_t* p = buffer + bl;
        *p = c.Red();
        *(p + 1) = c.Green();
        *(p + 2) = c.Blue();
    }
}

void XyzzyBase::DrawPixel(int x, int y, wxColour c, uint8_t* buffer, size_t size)
{
    for (int xout = 0; xout < _colsPerSquare; xout++)
    {
        if (x >= 0 && x < _bw)
        {
            for (int yout = 0; yout < _rowsPerSquare; yout++)
            {
                if (y >= 0 && y < _bh)
                {
                    DrawNode(_sideBorder + x * _colsPerSquare + xout, _bottomBorder + y * _rowsPerSquare + yout, c, buffer, size);
                }
            }
        }
    }
}

void XyzzyBase::DrawBlack(uint8_t* buffer, size_t size)
{
    if (_matrixMapper == nullptr) return;

    for (int x = 0; x < _matrixMapper->GetWidth(); x++)
    {
        for (int y = 0; y < _matrixMapper->GetHeight(); y++)
        {
            DrawNode(x, y, *wxBLACK, buffer, size);
        }
    }
}

bool Xyzzy::Frame(uint8_t* buffer, size_t size, bool outputframe)
{
    if (_matrixMapper == nullptr) return false;

    // actuall advance the game
    AdvanceGame();

    if (outputframe)
    {
        wxColor colour = *wxLIGHT_GREY;

        if (!_gameRunning)
        {
            colour = *wxRED;
        }

        // Erase the board background
        for (int x = _sideBorder; x < _sideBorder + _bw * _colsPerSquare; x++)
        {
            for (int y = _bottomBorder; y < _bottomBorder + _bh * _rowsPerSquare; y++)
            {
                DrawNode(x, y, *wxBLACK, buffer, size);
            }
        }

        // draw borders
        // left
        for (int x = 0; x < _sideBorder; x++)
        {
            for (int y = 0; y < _matrixMapper->GetHeight(); y++)
            {
                DrawNode(x, y, colour, buffer, size);
            }
        }
        // right
        for (int x = _sideBorder + _bw * _colsPerSquare; x < _matrixMapper->GetWidth(); x++)
        {
            for (int y = 0; y < _matrixMapper->GetHeight(); y++)
            {
                DrawNode(x, y, colour, buffer, size);
            }
        }
        // bottom
        for (int y = 0; y < _bottomBorder; y++)
        {
            for (int x = 0; x < _matrixMapper->GetWidth(); x++)
            {
                DrawNode(x, y, colour, buffer, size);
            }
        }
        // top
        for (int y = _bottomBorder + _bh * _rowsPerSquare; y < _matrixMapper->GetHeight(); y++)
        {
            for (int x = 0; x < _matrixMapper->GetWidth(); x++)
            {
                DrawNode(x, y, colour, buffer, size);
            }
        }

        // draw the board
        for (int x = 0; x < _bw; x++)
        {
            for (int y = 0; y < _bh; y++)
            {
                if (_board[y*_bw + x] != 0xFF)
                {
                    DrawPixel(x, y, __colours[_board[y*_bw + x]], buffer, size);
                }
            }
        }

        if (_currentPiece != nullptr)
        {
            auto pixels = _currentPiece->DrawPoints();
            for (auto it = pixels.begin(); it != pixels.end(); ++it)
            {
                DrawPixel(it->x, it->y, _currentPiece->GetColour(), buffer, size);
            }

            for (int i = 0; i < _currentPiece->GetY(); ++i)
            {
                if (_board[i * _bw + _currentPiece->GetX()] == 0xFF)
                {
                    bool found = false;
                    for (auto it = pixels.begin(); it != pixels.end(); ++it)
                    {
                        if (it->x == _currentPiece->GetX() && it->y == i)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        DrawPixel(_currentPiece->GetX(), i, wxColour(40, 40, 40), buffer, size);
                    }
                }
            }
        }

#ifdef SHOWVIRTUALMATRIX
        if (_virtualMatrix != nullptr)
        {
            _virtualMatrix->Frame(buffer, size);
        }
#endif
    }

    return true;
}

bool Xyzzy2::Frame(uint8_t* buffer, size_t size, bool outputframe)
{
    if (_matrixMapper == nullptr) return false;

    // actuall advance the game
    AdvanceGame();

    if (outputframe)
    {
        wxColor colour = *wxLIGHT_GREY;

        if (!_gameRunning)
        {
            colour = *wxRED;
        }

        // draw borders
        // left
        for (int x = 0; x < _sideBorder; x++)
        {
            for (int y = 0; y < _matrixMapper->GetHeight(); y++)
            {
                DrawNode(x, y, colour, buffer, size);
            }
        }
        // right
        for (int x = _sideBorder + _bw * _colsPerSquare; x < _matrixMapper->GetWidth(); x++)
        {
            for (int y = 0; y < _matrixMapper->GetHeight(); y++)
            {
                DrawNode(x, y, colour, buffer, size);
            }
        }
        // bottom
        for (int y = 0; y < _bottomBorder; y++)
        {
            for (int x = 0; x < _matrixMapper->GetWidth(); x++)
            {
                DrawNode(x, y, colour, buffer, size);
            }
        }
        // top
        for (int y = _bottomBorder + _bh * _rowsPerSquare; y < _matrixMapper->GetHeight(); y++)
        {
            for (int x = 0; x < _matrixMapper->GetWidth(); x++)
            {
                DrawNode(x, y, colour, buffer, size);
            }
        }

        // draw the board
        for (int x = 0; x < _bw; x++)
        {
            for (int y = 0; y < _bh; y++)
            {
                if (_board[y * _bw + x] == XYZZY2::PILL)
                {
                    DrawPixel(x, y, *wxBLUE, buffer, size);
                }
                else if (_board[y * _bw + x] == XYZZY2::HEAD)
                {
                    DrawPixel(x, y, *wxYELLOW, buffer, size);
                }
                else if (_board[y * _bw + x] == XYZZY2::TAIL || _board[y * _bw + x] == XYZZY2::BODY)
                {
                    DrawPixel(x, y, *wxGREEN, buffer, size);
                }
                else
                {
                    DrawPixel(x, y, *wxBLACK, buffer, size);
                }
            }
        }

#ifdef SHOWVIRTUALMATRIX
        if (_virtualMatrix != nullptr)
        {
            _virtualMatrix->Frame(buffer, size);
        }
#endif
    }

    return true;
}

// <matrix name>
void XyzzyBase::DoInitialise(const wxString& parameters, wxString& result, const wxString& reference, OutputManager* om)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Xyzzy initialising.");

    Close(result, reference);

    Reset();

    auto p = wxSplit(parameters, ',');

    if (p.Count() < 2)
    {
        result = "{\"result\":\"failed\",\"message\":\"Two parameters expected ... name of matrix and players name.\",\"highscore\":\""+GetHighScore()+
            "\",\"r\":\"" + reference +
            "\",\"highscoreplayer\":\"" + GetHighScorePlayer() +
                 "\"}";
        return;
    }

    auto m = xScheduleFrame::GetScheduleManager()->GetOptions()->GetMatrices();
    for (auto it : *m)
    {
        if (wxString(it->GetName()).Lower() == p[0].Lower())
        {
            _matrixMapper = it;
            break;
        }
    }

    if (_matrixMapper != nullptr)
    {
        if (_bw == -1) {
            _bw = _matrixMapper->GetWidth();
            _bh = _matrixMapper->GetHeight();
        }

        _colsPerSquare = _matrixMapper->GetWidth() / _bw;
        _rowsPerSquare = _matrixMapper->GetHeight() / _bh;

        if (_colsPerSquare > 2 * _rowsPerSquare) _colsPerSquare = 2 * _rowsPerSquare;
        if (_rowsPerSquare > 2 * _colsPerSquare) _rowsPerSquare = 2 * _colsPerSquare;

        _sideBorder = (_matrixMapper->GetWidth() - (_bw * _colsPerSquare)) / 2;
        _bottomBorder = (_matrixMapper->GetHeight() - (_bh * _rowsPerSquare)) / 2;
    }

    if (_matrixMapper == nullptr)
    {
        result = "{\"result\":\"failed\",\"message\":\"Matrix '"+p[0]+"' not found.\",\"highscore\":\"" + GetHighScore() +
            "\",\"r\":\"" + reference +
            "\",\"highscoreplayer\":\"" + GetHighScorePlayer() +
            "\"}";
        return;
    }

#ifdef SHOWVIRTUALMATRIX
    if (_virtualMatrix == nullptr)
    {
        _virtualMatrix = new VirtualMatrix(om);
        _virtualMatrix->SetWidth(_matrixMapper->GetWidth());
        _virtualMatrix->SetHeight(_matrixMapper->GetHeight());
        _virtualMatrix->SetStartChannel(_matrixMapper->GetStartChannel());
        _virtualMatrix->SetTopMost(true);
        _virtualMatrix->SetScalingQuality(wxIMAGE_QUALITY_NORMAL, -1);
        if (p.Count() == 3 && p[2] == "90")
        {
            _virtualMatrix->SetRotation(VMROTATION::VM_90);
            _virtualMatrix->SetLocation(wxPoint(0, 0));
            _virtualMatrix->SetSize(wxSize(_matrixMapper->GetHeight(), _matrixMapper->GetWidth()));
        }
        else if (p.Count() == 3 && p[3] == "270")
        {
            _virtualMatrix->SetRotation(VMROTATION::VM_270);
            _virtualMatrix->SetLocation(wxPoint(0, 0));
            _virtualMatrix->SetSize(wxSize(_matrixMapper->GetHeight(), _matrixMapper->GetWidth()));
        }
        else
        {
            _virtualMatrix->SetLocation(wxPoint(0, 0));
            _virtualMatrix->SetSize(wxSize(_matrixMapper->GetWidth(), _matrixMapper->GetHeight()));
        }
        _virtualMatrix->Start();
    }
#endif

    _playerName = p[1];
    _isOk = true;
    result = "{\"result\":\"ok\",\"message\":\"Initialised.\",\"highscore\":\"" + GetHighScore() +
        "\",\"r\":\"" + reference +
        "\",\"highscoreplayer\":\"" + GetHighScorePlayer() +
        "\"}";

    wxCommandEvent event(EVT_XYZZY);
    wxPostEvent(wxGetApp().GetTopWindow(), event);

    wxCommandEvent event2(EVT_XYZZYEVENT);
    event2.SetString("initialised");
    wxPostEvent(wxGetApp().GetTopWindow(), event2);
}

void Xyzzy::Close(wxString& result, const wxString& reference)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Xyzzy closing.");

#ifdef SHOWVIRTUALMATRIX
    if (_virtualMatrix != nullptr)
    {
        _virtualMatrix->Stop();
        delete _virtualMatrix;
        _virtualMatrix = nullptr;
    }
#endif

    if (_currentPiece != nullptr)
    {
        delete _currentPiece;
        _currentPiece = nullptr;
    }

    if (_nextPiece != nullptr)
    {
        delete _nextPiece;
        _nextPiece = nullptr;
    }
    SaveHighScore();
    _matrixMapper = nullptr;
    _gameRunning = false;
    result = "{\"result\":\"ok\",\"message\":\"Initialised.\",\"highscore\":\"" + GetHighScore() +
        "\",\"r\":\"" + reference +
        "\",\"highscoreplayer\":\"" + GetHighScorePlayer() +
        "\"}";

    wxCommandEvent event(EVT_XYZZY);
    wxPostEvent(wxGetApp().GetTopWindow(), event);
}

void Xyzzy2::Close(wxString& result, const wxString& reference)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Xyzzy2 closing.");

#ifdef SHOWVIRTUALMATRIX
    if (_virtualMatrix != nullptr)
    {
        _virtualMatrix->Stop();
        delete _virtualMatrix;
        _virtualMatrix = nullptr;
    }
#endif

    SaveHighScore();

    _matrixMapper = nullptr;
    _gameRunning = false;
    result = "{\"result\":\"ok\",\"message\":\"Initialised.\",\"highscore\":\"" + GetHighScore() +
        "\",\"r\":\"" + reference +
        "\",\"highscoreplayer\":\"" + GetHighScorePlayer() +
        "\"}";

    wxCommandEvent event(EVT_XYZZY);
    wxPostEvent(wxGetApp().GetTopWindow(), event);
}

std::string XyzzyBase::GameNotRunningResult(const std::string& reference)
{
    if (_isOk)
    {
        return "{\"result\":\"gameover\",\"score\":\"" + GetScore() +
            "\",\"playername\":\"" + _playerName +
            "\",\"highscore\":\"" + GetHighScore() +
            "\",\"highscoreplayer\":\"" + GetHighScorePlayer() +
            "\",\"r\":\"" + reference +
            "\"}";

    }
    else
    {
        return "{\"result\":\"failed\",\"message\":\"Game not initialised.\",\"highscore\":\"" + GetHighScore() +
            "\",\"r\":\"" + reference +
            "\",\"highscoreplayer\":\"" + GetHighScorePlayer() +
            "\"}";
    }
}

bool Xyzzy::Action(const wxString& command, const wxString& parameters, wxString& result, const wxString& reference)
{
    if (command == "q") // query state
    {
        if (_gameRunning)
        {
            result = "{\"result\":\"running\",\"score\":\"" + 
                GetScore() + "\",\"r\":\"" +
                reference + "\",\"next\":\"" +
                GetNextPiece() + "\"}";
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
        return true;
    }
    else if (command == "l") // left
    {
        if (_gameRunning)
        {
            if (TestMoveLeft())
            {
                _currentPiece->MoveLeft();
                result = "{\"result\":\"ok\",\"score\":\"" + 
                    GetScore() + "\",\"r\":\"" +
                    reference + "\",\"next\":\"" +
                    GetNextPiece() + "\"}";
                wxCommandEvent event(EVT_XYZZYEVENT);
                event.SetString("movedleft");
                wxPostEvent(wxGetApp().GetTopWindow(), event);
            }
            else
            {
                result = "{\"result\":\"failed\",\"score\":\"" + 
                    GetScore() + "\",\"r\":\"" +
                    reference + "\",\"next\":\"" +
                    GetNextPiece() + "\"}";
            }
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
    }
    else if (command == "r") // right
    {
        if (_gameRunning)
        {
            if (TestMoveRight())
            {
                _currentPiece->MoveRight();
                result = "{\"result\":\"ok\",\"score\":\"" + 
                    GetScore() + "\",\"r\":\"" +
                    reference + "\",\"next\":\"" +
                    GetNextPiece() + "\"}";
                wxCommandEvent event(EVT_XYZZYEVENT);
                event.SetString("movedright");
                wxPostEvent(wxGetApp().GetTopWindow(), event);
            }
            else
            {
                result = "{\"result\":\"failed\",\"score\":\"" + 
                    GetScore() + "\",\"r\":\"" +
                    reference + "\",\"next\":\"" +
                    GetNextPiece() + "\"}";
            }
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
    }
    else if (command == "s") // spin
    {
        if (_gameRunning)
        {
            if (TestSpin())
            {
                _currentPiece->Rotate();
                result = "{\"result\":\"ok\",\"score\":\"" + 
                    GetScore() + "\",\"r\":\"" +
                    reference + "\",\"next\":\"" +
                    GetNextPiece() + "\"}";
                wxCommandEvent event(EVT_XYZZYEVENT);
                event.SetString("spun");
                wxPostEvent(wxGetApp().GetTopWindow(), event);
            }
            else
            {
                result = "{\"result\":\"failed\",\"score\":\"" + 
                    GetScore() + "\",\"r\":\"" +
                    reference + "\",\"next\":\"" +
                    GetNextPiece() + "\"}";
            }
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
    }
    else if (command == "d") // drop
    {
        if (_gameRunning)
        {
            Drop();
            result = "{\"result\":\"ok\",\"score\":\"" + 
                GetScore() + "\",\"r\":\"" +
                reference + "\",\"next\":\"" +
                GetNextPiece() + "\"}";
            wxCommandEvent event(EVT_XYZZYEVENT);
            event.SetString("dropped");
            wxPostEvent(wxGetApp().GetTopWindow(), event);
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
    }
    else if (command == "c") // quit game ... but dont close it
    {
        if (_gameRunning)
        {
            _gameRunning = false;
            SaveHighScore();
            result = "{\"result\":\"ok\",\"score\":\"" + GetScore() + 
                "\",\"playername\":\"" + _playerName +
                "\",\"r\":\"" + reference +
                "\",\"highscore\":\"" + GetHighScore() +
                "\",\"highscoreplayer\":\"" + GetHighScorePlayer() +
                "\"}";
            wxCommandEvent event(EVT_XYZZYEVENT);
            event.SetString("gamestopped");
            wxPostEvent(wxGetApp().GetTopWindow(), event);
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
    }
    else if (command == "g") // start game
    {
        if (_gameRunning)
        {
            result = "{\"result\":\"failed\",\"score\":\"" + 
                GetScore() + "\",\"r\":\"" +
                reference + "\",\"next\":\"" +
                GetNextPiece() + "\"}";
        }
        else
        {
            if (parameters != "")
            {
                _playerName = parameters;
            }

            if (_currentPiece != nullptr)
            {
                delete _currentPiece;
            }
            _currentPiece = XyzzyPiece::CreatePiece();
            if (_nextPiece != nullptr)
            {
                delete _nextPiece;
            }
            _nextPiece = XyzzyPiece::CreatePiece();
            _fullTime = 0;
            _lastUpdatedMovement = wxGetUTCTimeMillis();
            memset(_board, 0xFF, sizeof(_board));
            _dropSpeed = STARTSPEED;
            _score = 0;
            if (parameters != "")
            {
                _playerName = parameters;
            }
            _gameRunning = true;
            result = "{\"result\":\"ok\",\"score\":\"" + 
                GetScore() + "\",\"r\":\"" +
                reference + "\",\"next\":\"" +
                GetNextPiece() + "\"}";
            wxCommandEvent event(EVT_XYZZYEVENT);
            event.SetString("gamestarted");
            wxPostEvent(wxGetApp().GetTopWindow(), event);
        }
    }
    else if (command == "reset") // reset to pristine state
    {
        Reset();
        result = "{\"result\":\"ok\",\"r\":\""+reference+"\"}";
        wxCommandEvent event(EVT_XYZZYEVENT);
        event.SetString("gamereset");
        wxPostEvent(wxGetApp().GetTopWindow(), event);
    }
    else
    {
        result = "{\"result\":\"failed\",\"r\":\"" +
            reference + "\",\"message\":\"Unknown command '"+command+"'.\"}";
    }

    wxCommandEvent event(EVT_XYZZY);
    wxPostEvent(wxGetApp().GetTopWindow(), event);

    return true;
}

void Xyzzy::Initialise(const wxString& parameters, wxString& result, const wxString& reference, OutputManager* om)
{
    _bw = BOARDWIDTH;
    _bh = BOARDHEIGHT;
    DoInitialise(parameters, result, reference, om);
}

void Xyzzy2::Initialise(const wxString& parameters, wxString& result, const wxString& reference, OutputManager* om)
{
    auto p = wxSplit(parameters, ',');

    if (p.Count() < 2)
    {
    }
    else
    {
        auto m = xScheduleFrame::GetScheduleManager()->GetOptions()->GetMatrices();
        for (auto it : *m)
        {
            if (wxString(it->GetName()).Lower() == p[0].Lower())
            {
                _bw = it->GetWidth();
                _bh = it->GetHeight();

                if (_bw >= _bh)
                {
                    if (_bw > 21) _bw = 21;
                    if (_bh > 14) _bh = 14;

                    if (_bw < 21 || _bh < 14)
                    {
                        _bw = it->GetWidth();
                        _bh = it->GetHeight();
                    }
                }
                else
                {
                    if (_bh > 21) _bh = 21;
                    if (_bw > 14) _bw = 14;

                    if (_bh < 21 || _bw < 14)
                    {
                        _bw = it->GetWidth();
                        _bh = it->GetHeight();
                    }
                }
            }
        }
    }
    DoInitialise(parameters, result, reference, om);
    _board.resize(_bw * _bh);
    for (auto& it : _board)
    {
        it = XYZZY2::EMPTY;
    }
}

wxPoint Xyzzy2::RandomPill() const
{
    if (_body.size() >= _bw * _bh)
    {
        // full grid
        return wxPoint(-1, -1);
    }
    wxPoint p;
    do
    {
        p = wxPoint(rand01() * _bw, rand01() * _bh);
    } while (std::find(_body.begin(), _body.end(), p) != _body.end());
    return p;
}

bool Xyzzy2::Action(const wxString& command, const wxString& parameters, wxString& result, const wxString& reference)
{
    if (command == "q") // query state
    {
        if (_gameRunning)
        {
            result = "{\"result\":\"running\",\"score\":\"" +
                GetScore() + "\",\"r\":\"" +
                reference + "\"}";
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
        return true;
    }
    else if (command == "l") // left
    {
        if (_gameRunning)
        {
            _direction = XYZZY2::LEFT;
            result = "{\"result\":\"ok\",\"score\":\"" +
                GetScore() + "\",\"r\":\"" +
                reference + "\"}";
            wxCommandEvent event(EVT_XYZZYEVENT);
            event.SetString("movingleft");
            wxPostEvent(wxGetApp().GetTopWindow(), event);
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
    }
    else if (command == "r") // right
    {
        if (_gameRunning)
        {
                _direction = XYZZY2::RIGHT;
                result = "{\"result\":\"ok\",\"score\":\"" +
                    GetScore() + "\",\"r\":\"" +
                    reference + "\"}";
                wxCommandEvent event(EVT_XYZZYEVENT);
                event.SetString("movingright");
                wxPostEvent(wxGetApp().GetTopWindow(), event);
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
    }
    else if (command == "u") // up
    {
        if (_gameRunning)
        {
                _direction = XYZZY2::UP;
                result = "{\"result\":\"ok\",\"score\":\"" +
                    GetScore() + "\",\"r\":\"" +
                    reference + "\"}";
                wxCommandEvent event(EVT_XYZZYEVENT);
                event.SetString("movingup");
                wxPostEvent(wxGetApp().GetTopWindow(), event);
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
    }
    else if (command == "d") // down
    {
        if (_gameRunning)
        {
            _direction = XYZZY2::DOWN;
            result = "{\"result\":\"ok\",\"score\":\"" +
                GetScore() + "\",\"r\":\"" +
                reference + "\"}";
            wxCommandEvent event(EVT_XYZZYEVENT);
            event.SetString("movingdown");
            wxPostEvent(wxGetApp().GetTopWindow(), event);
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
    }
    else if (command == "c") // quit game ... but dont close it
    {
        if (_gameRunning)
        {
            _gameRunning = false;
            SaveHighScore();
            result = "{\"result\":\"ok\",\"score\":\"" + GetScore() +
                "\",\"playername\":\"" + _playerName +
                "\",\"r\":\"" + reference +
                "\",\"highscore\":\"" + GetHighScore() +
                "\",\"highscoreplayer\":\"" + GetHighScorePlayer() +
                "\"}";
            wxCommandEvent event(EVT_XYZZYEVENT);
            event.SetString("gamestopped");
            wxPostEvent(wxGetApp().GetTopWindow(), event);
        }
        else
        {
            result = GameNotRunningResult(reference);
        }
    }
    else if (command == "g") // start game
    {
        if (_gameRunning)
        {
            result = "{\"result\":\"failed\",\"score\":\"" +
                GetScore() + "\",\"r\":\"" +
                reference + "\"}";
        }
        else
        {
            if (parameters != "")
            {
                _playerName = parameters;
            }

            _direction = XYZZY2::RIGHT;
            _body.clear();
            _body.push_back(wxPoint(1, rand01() * _bh - 1));
            _body.push_back(wxPoint(0, _body.front().y));
            _lastUpdatedMovement = wxGetUTCTimeMillis();
            _board.clear();
            _board.resize(_bw * _bh);
            for (auto& it : _board)
            {
                it = XYZZY2::EMPTY;
            }
            for (auto it : _body)
            {
                if (it == _body.front())
                {
                    _board[it.x + it.y * _bw] = XYZZY2::HEAD;
                }
                else if (it == _body.back())
                {
                    _board[it.x + it.y * _bw] = XYZZY2::TAIL;
                }
                else
                {
                    _board[it.x + it.y * _bw] = XYZZY2::BODY;
                }
            }
            _pill = wxPoint(rand01() * _bw / 2 + _bw / 2 - 1, rand01() * _bh - 1);
            _board[_pill.x + _pill.y * _bw] = XYZZY2::PILL;
            _score = 0;
            if (parameters != "")
            {
                _playerName = parameters;
            }
            _gameRunning = true;
            result = "{\"result\":\"ok\",\"score\":\"" +
                GetScore() + "\",\"r\":\"" +
                reference + "\"}";
            wxCommandEvent event(EVT_XYZZYEVENT);
            event.SetString("gamestarted");
            wxPostEvent(wxGetApp().GetTopWindow(), event);
        }
    }
    else if (command == "reset") // reset to pristine state
    {
        Reset();
        result = "{\"result\":\"ok\",\"r\":\"" + reference + "\"}";
        wxCommandEvent event(EVT_XYZZYEVENT);
        event.SetString("gamereset");
        wxPostEvent(wxGetApp().GetTopWindow(), event);
    }
    else
    {
        result = "{\"result\":\"failed\",\"r\":\"" +
            reference + "\",\"message\":\"Unknown command '" + command + "'.\"}";
    }

    wxCommandEvent event(EVT_XYZZY);
    wxPostEvent(wxGetApp().GetTopWindow(), event);

    return true;
}

std::string XyzzyBase::GetHighScore() const
{
    return wxString::Format(wxT("%i"), _highScore).ToStdString();
}

std::string XyzzyBase::GetScore() const
{
    return wxString::Format(wxT("%i"), _score).ToStdString();
}

std::string Xyzzy::GetNextPiece() const
{
    return wxString::Format(wxT("%i"), _nextPiece->GetType()).ToStdString();
}

bool Xyzzy::TestMoveLeft() const
{
    auto pts = _currentPiece->TestLeft();

    for (auto it : pts)
    {
        if (it.y < _bh)
        {
            if (it.x < 0) return false;
            if (_board[(it.y) * _bw + it.x] != 0xFF) return false;
        }
    }

    return true;
}

bool Xyzzy::TestMoveRight() const
{
    auto pts = _currentPiece->TestRight();

    for (auto it : pts)
    {
        if (it.y < _bh)
        {
            if (it.x >= _bw) return false;
            if (_board[(it.y) * _bw + it.x] != 0xFF) return false;
        }
    }

    return true;
}

bool Xyzzy::TestSpin() const
{
    auto pts = _currentPiece->TestRotate();

    for (auto it : pts)
    {
        if (it.x < 0) return false;
        if (it.x >= _bw) return false;

        if (it.y < _bh)
        {
            if (_board[(it.y) * _bw + it.x] != 0xFF) return false;
        }
    }

    return true;
}

void Xyzzy::Drop()
{
    if (!_gameRunning) return;

    int dropdistance = 0;
    while (TestMoveDown())
    {
        _currentPiece->MoveDown();
        dropdistance++;
    }

    auto loc = _currentPiece->DrawPoints();
    for (auto it = loc.begin(); it != loc.end(); ++it)
    {
        if (it->y >= _bh)
        {
            SaveHighScore();
            _gameRunning = false;
            wxCommandEvent event(EVT_XYZZYEVENT);
            event.SetString("gameover");
            wxPostEvent(wxGetApp().GetTopWindow(), event);
            return;
        }
    }

    AddToScore(dropdistance * DROPBONUSPERROW);

    MakePiecePermanent();

    delete _currentPiece;
    _currentPiece = _nextPiece;
    _nextPiece = XyzzyPiece::CreatePiece();

    CheckFullRow();
}

void Xyzzy::AddToScore(int add)
{
    _score += add;
    wxCommandEvent event(EVT_XYZZY);
    wxPostEvent(wxGetApp().GetTopWindow(), event);
}

bool Xyzzy::TestMoveDown() const
{
    auto pts = _currentPiece->TestDown();

    for (auto it = pts.begin(); it != pts.end(); ++it)
    {
        if (it->y < _bh)
        {
            if (it->y < 0) return false;
            if (_board[(it->y) * _bw + it->x] != 0xFF) return false;
        }
    }

    return true;
}

void Xyzzy::MakePiecePermanent()
{
    auto pixels = _currentPiece->DrawPoints();
    for (auto it = pixels.begin(); it != pixels.end(); ++it)
    {
        _board[it->y * _bw + it->x] = _currentPiece->GetType();
    }
}

XyzzyPiece* XyzzyPiece::CreatePiece()
{
    static int hardcount = 0;

    wxCommandEvent event(EVT_XYZZY);
    wxPostEvent(wxGetApp().GetTopWindow(), event);

    if (hardcount < MAXHARDPIECES)
    {
        int r = rand() % 7;

        switch (r)
        {
        case 0:
            hardcount = 0;
            return new LinePiece();
        case 1:
            hardcount = 0;
            return new LPiece();
        case 2:
            hardcount = 0;
            return new JPiece();
        case 3:
            hardcount++;
            return new SPiece();
        case 4:
            hardcount++;
            return new ZPiece();
        case 5:
            hardcount = 0;
            return new OPiece();
        case 6:
            hardcount++;
            return new TPiece();
        default:
            break;
        }
    }
    else
    {
        hardcount = 0;

        int r = rand() % 4;

        switch (r)
        {
        case 0:
            return new LinePiece();
        case 1:
            return new LPiece();
        case 2:
            return new JPiece();
        case 3:
            return new OPiece();
        default:
            break;
        }
    }

    wxASSERT(false);
    return nullptr;
}

std::list<wxPoint> LinePiece::GetPoints(int rotation, wxPoint position) const
{
    std::list<wxPoint> res;

    if (rotation % 2 == 0)
    {
        // what would it look like vertical
        res.push_back(wxPoint(position.x, position.y - 1));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x, position.y + 1));
        res.push_back(wxPoint(position.x, position.y + 2));
    }
    else
    {
        res.push_back(wxPoint(position.x - 1, position.y));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x + 1, position.y));
        res.push_back(wxPoint(position.x + 2, position.y));
    }

    return res;
}

std::list<wxPoint> LPiece::GetPoints(int rotation, wxPoint position) const
{
    std::list<wxPoint> res;

    switch(rotation)
    {
    case 0:
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x - 1, position.y));
        res.push_back(wxPoint(position.x + 1, position.y));
        res.push_back(wxPoint(position.x + 1, position.y + 1));
        break;
    case 1:
        // what would it look like vertical
        res.push_back(wxPoint(position.x, position.y - 1));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x, position.y + 1));
        res.push_back(wxPoint(position.x+1, position.y - 1));
        break;
    case 2:
        res.push_back(wxPoint(position.x - 1, position.y));
        res.push_back(wxPoint(position.x - 1, position.y - 1));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x + 1, position.y));
        break;
    case 3:
        res.push_back(wxPoint(position.x, position.y - 1));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x, position.y + 1));
        res.push_back(wxPoint(position.x - 1, position.y + 1));
        break;
    default:
        break;
    }

    return res;
}

std::list<wxPoint> JPiece::GetPoints(int rotation, wxPoint position) const
{
    std::list<wxPoint> res;

    switch (rotation)
    {
    case 0:
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x - 1, position.y));
        res.push_back(wxPoint(position.x + 1, position.y));
        res.push_back(wxPoint(position.x - 1, position.y + 1));
        break;
    case 1:
        // what would it look like vertical
        res.push_back(wxPoint(position.x, position.y - 1));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x, position.y + 1));
        res.push_back(wxPoint(position.x + 1, position.y + 1));
        break;
    case 2:
        res.push_back(wxPoint(position.x - 1, position.y));
        res.push_back(wxPoint(position.x + 1, position.y - 1));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x + 1, position.y));
        break;
    case 3:
        res.push_back(wxPoint(position.x, position.y - 1));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x, position.y + 1));
        res.push_back(wxPoint(position.x - 1, position.y - 1));
        break;
    default:
        break;
    }

    return res;
}

std::list<wxPoint> SPiece::GetPoints(int rotation, wxPoint position) const
{
    std::list<wxPoint> res;

    switch (rotation % 2)
    {
    case 0:
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x - 1, position.y));
        res.push_back(wxPoint(position.x + 1, position.y + 1));
        res.push_back(wxPoint(position.x, position.y + 1));
        break;
    case 1:
        // what would it look like vertical
        res.push_back(wxPoint(position.x, position.y + 1));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x + 1, position.y - 1));
        res.push_back(wxPoint(position.x + 1, position.y));
        break;
    default:
        break;
    }

    return res;
}

std::list<wxPoint> ZPiece::GetPoints(int rotation, wxPoint position) const
{
    std::list<wxPoint> res;

    switch (rotation % 2)
    {
    case 0:
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x - 1, position.y + 1));
        res.push_back(wxPoint(position.x + 1, position.y));
        res.push_back(wxPoint(position.x, position.y + 1));
        break;
    case 1:
        // what would it look like vertical
        res.push_back(wxPoint(position.x, position.y - 1));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x + 1, position.y + 1));
        res.push_back(wxPoint(position.x + 1, position.y));
        break;
    default:
        break;
    }

    return res;
}

std::list<wxPoint> OPiece::GetPoints(int rotation, wxPoint position) const
{
    std::list<wxPoint> res;

    res.push_back(wxPoint(position.x, position.y));
    res.push_back(wxPoint(position.x + 1, position.y + 1));
    res.push_back(wxPoint(position.x + 1, position.y));
    res.push_back(wxPoint(position.x, position.y + 1));

    return res;
}

std::list<wxPoint> TPiece::GetPoints(int rotation, wxPoint position) const
{
    std::list<wxPoint> res;

    switch (rotation)
    {
    case 0:
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x - 1, position.y));
        res.push_back(wxPoint(position.x + 1, position.y));
        res.push_back(wxPoint(position.x, position.y + 1));
        break;
    case 1:
        // what would it look like vertical
        res.push_back(wxPoint(position.x, position.y - 1));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x, position.y + 1));
        res.push_back(wxPoint(position.x + 1, position.y));
        break;
    case 2:
        res.push_back(wxPoint(position.x - 1, position.y));
        res.push_back(wxPoint(position.x + 1, position.y));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x, position.y - 1));
        break;
    case 3:
        res.push_back(wxPoint(position.x, position.y - 1));
        res.push_back(wxPoint(position.x, position.y));
        res.push_back(wxPoint(position.x, position.y + 1));
        res.push_back(wxPoint(position.x - 1, position.y));
        break;
    default:
        break;
    }

    return res;
}

std::list<wxPoint> XyzzyPiece::TestRotate() const
{
    int r = _rotation;
    r++;
    if (r > 3) r = 0;
    return GetPoints(r, _position);
}

std::list<wxPoint> XyzzyPiece::TestLeft() const
{
    wxPoint p = _position;
    p.x--;
    return GetPoints(_rotation, p);
}

std::list<wxPoint> XyzzyPiece::TestRight() const
{
    wxPoint p = _position;
    p.x++;
    return GetPoints(_rotation, p);
}

std::list<wxPoint> XyzzyPiece::TestDown() const
{
    wxPoint p = _position;
    p.y--;
    return GetPoints(_rotation, p);
}

std::list<wxPoint> XyzzyPiece::DrawPoints() const
{
    return GetPoints(_rotation, _position);
}

// true when game over
bool Xyzzy::AdvanceGame()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_gameRunning) return false;

    if (_fullTime != 0 && (wxGetUTCTimeMillis() - _fullTime) < PAUSEONCOMPLETEROWTIME)
    {
        _lastUpdatedMovement = wxGetUTCTimeMillis();
        return true;
    }

    if (_fullTime != 0)
    {
        _fullTime = 0;
        // clear any white rows
        for (int y = 0; y < _bh; y++)
        {
            if (_board[y * _bw] == 7)
            {
                if (y < _bh - 1)
                {
                    // move the rows down
                    memcpy(&_board[y * _bw], &_board[(y + 1)*_bw], (_bh - y - 1) * _bw);
                }

                // blank top row
                memset(&_board[(_bh - 1) * _bw], 0xFF, _bw);
                y--;
            }
        }
    }

    if (_currentPiece == nullptr)
    {
        _currentPiece = XyzzyPiece::CreatePiece();
        _lastUpdatedMovement = wxGetUTCTimeMillis();
    }

    if (_nextPiece == nullptr)
    {
        _nextPiece = XyzzyPiece::CreatePiece();
    }

    auto elapsed = (wxGetUTCTimeMillis() - _lastUpdatedMovement).ToLong();

    if (elapsed > _dropSpeed)
    {
        if (TestMoveDown())
        {
            _currentPiece->MoveDown();
            _lastUpdatedMovement = wxGetUTCTimeMillis();
            wxCommandEvent event(EVT_XYZZYEVENT);
            event.SetString("piecemoved");
            wxPostEvent(wxGetApp().GetTopWindow(), event);

            // check a move ahead to see if we will land
            if (!TestMoveDown())
            {
                // THIS MAY BE WRONG!!!! THERE IS A CHANCE THE USER COULD MOVE THE PIECE AFTER THIS AND NOT LAND
                // BUT TO PROVIDE THE BEST EXPERIENCE THIS SEEMS THE BEST APPROACH
                wxCommandEvent event2(EVT_XYZZYEVENT);
                event2.SetString("piecelanded");
                wxPostEvent(wxGetApp().GetTopWindow(), event2);
            }
        }
        else
        {
            logger_base.debug("Xyzzy piece landed.");

            auto loc = _currentPiece->DrawPoints();
            for (auto it = loc.begin(); it != loc.end(); ++it)
            {
                if (it->y >= _bh)
                {
                    SaveHighScore();
                    _gameRunning = false;
                    return true;
                }
            }

            MakePiecePermanent();
            delete _currentPiece;
            _currentPiece = _nextPiece;
            _nextPiece = XyzzyPiece::CreatePiece();
            AddToScore(PIECESCORE);

            CheckFullRow();
        }
    }

    // do game updates here
    return false;
}

// true when game over
bool Xyzzy2::AdvanceGame()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_gameRunning) return false;

    auto elapsed = (wxGetUTCTimeMillis() - _lastUpdatedMovement).ToLong();

    int frametime = 300;
    frametime -= _score * 5;
    if (frametime < 100) frametime = 100;

    if (elapsed > frametime)
    {
        _lastUpdatedMovement = wxGetUTCTimeMillis();

        wxPoint np = _body.front();
        switch (_direction)
        {
        case XYZZY2::LEFT:
            np = wxPoint(np.x - 1, np.y);
            break;
        case XYZZY2::RIGHT:
            np = wxPoint(np.x + 1, np.y);
            break;
        case XYZZY2::UP:
            np = wxPoint(np.x, np.y + 1);
            break;
        case XYZZY2::DOWN:
            np = wxPoint(np.x, np.y - 1);
            break;
        }

        if (np.x < 0 || np.x >= _bw || np.y < 0 || np.y >= _bh || std::find(_body.begin(), _body.end(), np) != _body.end())
        {
            SaveHighScore();
            _gameRunning = false;
            return true;
        }

        _body.push_front(np);
        if (np != _pill)
        {
            _body.pop_back();
        }
        else
        {
            _score++;
            _pill = RandomPill();
        }

        for (auto& it : _board)
        {
            it = XYZZY2::EMPTY;
        }
        for (auto it : _body)
        {
            if (it == _body.front())
            {
                _board[it.x + it.y * _bw] = XYZZY2::HEAD;
            }
            else if (it == _body.back())
            {
                _board[it.x + it.y * _bw] = XYZZY2::TAIL;
            }
            else
            {
                _board[it.x + it.y * _bw] = XYZZY2::BODY;
            }
        }
        _board[_pill.x + _pill.y * _bw] = XYZZY2::PILL;
    }

    // do game updates here
    return false;
}

void Xyzzy::CheckFullRow()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // now check if any row is complete
    int fullcount = 0;
    for (int y = 0; y < _bh; y++)
    {
        bool full = true;
        for (int x = 0; x < _bw; x++)
        {
            if (_board[y * _bw + x] == 0xFF)
            {
                full = false;
                break;
            }
        }

        if (full)
        {
            fullcount++;
            for (int x = 0; x < _bw; x++)
            {
                _board[y * _bw + x] = 7;
                _fullTime = wxGetUTCTimeMillis();
            }
        }
    }

    if (fullcount > 0)
    {
        logger_base.debug("Xyzzy %d full rows completed.");
        _dropSpeed += ROWADJUSTSPEED;
        if (_dropSpeed < MINDROPSPEED)
        {
            _dropSpeed = MINDROPSPEED;
        }

        wxCommandEvent event(EVT_XYZZYEVENT);
        event.SetString("rowcomplete");
        wxPostEvent(wxGetApp().GetTopWindow(), event);
    }

    switch (fullcount)
    {
    case 1:
        AddToScore(ROWSCORE1);
        break;
    case 2:
        AddToScore(ROWSCORE2);
        break;
    case 3:
        AddToScore(ROWSCORE3);
        break;
    case 4:
        AddToScore(ROWSCORE4);
        break;
    default:
        break;
    }
}

void Xyzzy::SaveHighScore()
{
    if (_score > _highScore)
    {
        _highScore = _score;
        _highScoreOwner = _playerName;

        wxConfigBase* config = wxConfigBase::Get();
        config->Write(_("XyzzyHighScore"), _highScore);
        config->Write(_("XyzzyHighScoreOwner"), wxString(_highScoreOwner));
        config->Flush();

        wxCommandEvent event(EVT_XYZZY);
        wxPostEvent(wxGetApp().GetTopWindow(), event);
    }
}

void Xyzzy2::SaveHighScore()
{
    if (_score > _highScore)
    {
        _highScore = _score;
        _highScoreOwner = _playerName;

        wxConfigBase* config = wxConfigBase::Get();
        config->Write(_("Xyzzy2HighScore"), _highScore);
        config->Write(_("Xyzzy2HighScoreOwner"), wxString(_highScoreOwner));
        config->Flush();

        wxCommandEvent event(EVT_XYZZY);
        wxPostEvent(wxGetApp().GetTopWindow(), event);
    }
}
