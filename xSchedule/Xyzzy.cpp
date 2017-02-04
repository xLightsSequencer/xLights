#include "Xyzzy.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include "ScheduleOptions.h"
#include "MatrixMapper.h"
#include <wx/config.h>

#ifdef _DEBUG
#include "PlayList/PlayListItemVirtualMatrix.h"
#endif

wxColor __colours[] = { *wxRED, wxColor(255, 0, 255), *wxYELLOW, *wxBLUE, wxColor(64, 255, 64), wxColor(0,255,255), wxColor(40,40,40), *wxWHITE };

wxColor XyzzyPiece::GetColour() const
{
    return __colours[GetType()];
}

void XyzzyPiece::Rotate()
{
    _rotation++;
    if (_rotation > 3) _rotation = 0;
}

Xyzzy::Xyzzy()
{
    wxConfigBase* config = wxConfigBase::Get();
    _highScore = config->ReadLong("XyzzyHighScore", 0);
    _highScoreOwner = config->Read("XyzzyHighScoreOwner", "").ToStdString();
    
    _lastUpdatedMovement = wxDateTime::Now();
    _score = 0;
    _playerName = "";
    memset(_board, 0xFF, sizeof(_board));
    _colsPerSquare = 0;
    _rowsPerSquare = 0;
    _isOk = false;
    _nextPiece = nullptr;
    _currentPiece = nullptr;
    _dropSpeed = STARTSPEED;
    _sideBorder = 0;
    _bottomBorder = 0;
    _matrixMapper = nullptr;
    _gameRunning = false;
    _fullTime = wxDateTime((time_t)0);

    MatrixMapper::Test();
}

Xyzzy::~Xyzzy()
{
    
}

void Xyzzy::DrawNode(int x, int y, wxColour c, wxByte* buffer)
{
    int bl = _matrixMapper->Map(x, y) - 1;
    if (bl >= _matrixMapper->GetChannels())
    {
        // off the top
    }
    else
    {
        wxByte* p = buffer + bl;
        *p = c.Red();
        *(p + 1) = c.Green();
        *(p + 2) = c.Blue();
    }
}

void Xyzzy::DrawPixel(int x, int y, wxColour c, wxByte* buffer)
{
    for (int xout = 0; xout < _colsPerSquare; xout++)
    {
        if (x >= 0 && x < BOARDWIDTH)
        {
            for (int yout = 0; yout < _rowsPerSquare; yout++)
            {
                if (y >= 0 && y < BOARDHEIGHT)
                {
                    DrawNode(_sideBorder + x * _colsPerSquare + xout, _bottomBorder + y * _rowsPerSquare + yout, c, buffer);
                }
            }
        }
    }
}

bool Xyzzy::Frame(wxByte* buffer, size_t size, bool outputframe)
{
    if (_matrixMapper == nullptr) return false;

    // actuall advance the game
    AdvanceGame();

    if (outputframe)
    {
        // draw borders
        // left
        for (int x = 0; x < _sideBorder; x++)
        {
            for (int y = 0; y < _matrixMapper->GetHeight(); y++)
            {
                DrawNode(x, y, *wxWHITE, buffer);
            }
        }
        // right
        for (int x = _sideBorder + BOARDWIDTH * _colsPerSquare; x < _matrixMapper->GetWidth(); x++)
        {
            for (int y = 0; y < _matrixMapper->GetHeight(); y++)
            {
                DrawNode(x, y, *wxWHITE, buffer);
            }
        }
        // bottom
        for (int y = 0; y < _bottomBorder; y++)
        {
            for (int x = 0; x < _matrixMapper->GetWidth(); x++)
            {
                DrawNode(x, y, *wxWHITE, buffer);
            }
        }
        // top
        for (int y = _bottomBorder + BOARDHEIGHT * _rowsPerSquare; y < _matrixMapper->GetHeight(); y++)
        {
            for (int x = 0; x < _matrixMapper->GetWidth(); x++)
            {
                DrawNode(x, y, *wxWHITE, buffer);
            }
        }

        // draw the board
        for (int x = 0; x < BOARDWIDTH; x++)
        {
            for (int y = 0; y < BOARDHEIGHT; y++)
            {
                DrawPixel(x, y, __colours[_board[y*BOARDWIDTH + x]], buffer);
            }
        }

        if (_currentPiece != nullptr)
        {
            auto pixels = _currentPiece->DrawPoints();
            for (auto it = pixels.begin(); it != pixels.end(); ++it)
            {
                DrawPixel(it->x, it->y, _currentPiece->GetColour(), buffer);
            }
        }

#ifdef _DEBUG
        if (_virtualMatrix != nullptr)
        {
            _virtualMatrix->Frame(buffer, size, 0, 0, outputframe);
        }
#endif
    }

    return true;
}

// <matrix name>
void Xyzzy::Initialise(const std::string& parameters, std::string& result)
{
    auto p = wxSplit(parameters, ',');

    if (p.Count() != 2)
    {
        result = "{\"result\":\"failed\",\"message\":\"Two parameters expected ... name of matrix and players name.\",\"highscore\":\""+GetHighScore()+"\"}";
        return;
    }

    auto m = xScheduleFrame::GetScheduleManager()->GetOptions()->GetMatrices();

    for (auto it = m->begin(); it != m->end(); ++it)
    {
        if (wxString((*it)->GetName()).Lower() == p[0].Lower())
        {
            _matrixMapper = *it;
            break;
        }
    }

    if (_matrixMapper == nullptr)
    {
        result = "{\"result\":\"failed\",\"message\":\"Matrix not found.\",\"highscore\":\"" + GetHighScore() + "\"}";
        return;
    }

    _colsPerSquare = _matrixMapper->GetWidth() / BOARDWIDTH;
    _rowsPerSquare = _matrixMapper->GetHeight() / BOARDHEIGHT;

    if (_colsPerSquare > 2 * _rowsPerSquare) _colsPerSquare = 2 * _rowsPerSquare;
    if (_rowsPerSquare > 2 * _colsPerSquare) _rowsPerSquare = 2 * _colsPerSquare;

    _sideBorder = (_matrixMapper->GetWidth() - (BOARDWIDTH * _colsPerSquare)) / 2;
    _bottomBorder = (_matrixMapper->GetHeight() - (BOARDHEIGHT * _rowsPerSquare)) / 2;

#ifdef _DEBUG
    _virtualMatrix = new PlayListItemVirtualMatrix();
    _virtualMatrix->SetWidth(_matrixMapper->GetWidth());
    _virtualMatrix->SetHeight(_matrixMapper->GetHeight());
    _virtualMatrix->SetStartChannel(_matrixMapper->GetStartChannel());
    _virtualMatrix->Start();
#endif

    _playerName = p[1];
    _isOk = true;
    result = "{\"result\":\"ok\",\"message\":\"Initialised.\",\"highscore\":\"" + GetHighScore() + "\"}";
}

void Xyzzy::Close(std::string& result)
{
#ifdef _DEBUG
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
    _gameRunning = false;
}

std::string Xyzzy::GameNotRunningResult()
{
        if (_isOk)
        {
            return "{\"result\":\"gameover\",\"score\":\"" + GetScore() + "\"}";
        }
        else
        {
            return "{\"result\":\"failed\",\"message\":\"Game not initialised.\"}";
        }
}

bool Xyzzy::Action(const std::string& command, const std::string& parameters, std::string& result)
{
    if (command == "q") // query state
    {
        if (_gameRunning)
        {
            result = "{\"result\":\"running\",\"score\":\"" + GetScore() + "\",\"next\":\"" + GetNextPiece() + "\"}";
        }
        else
        {
            result = GameNotRunningResult();
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
                result = "{\"result\":\"ok\",\"score\":\"" + GetScore() + "\",\"next\":\"" + GetNextPiece() + "\"}";
            }
            else
            {
                result = "{\"result\":\"failed\",\"score\":\"" + GetScore() + "\",\"next\":\"" + GetNextPiece() + "\"}";
            }
        }
        else
        {
            result = GameNotRunningResult();
        }
    }
    else if (command == "r") // right
    {
        if (_gameRunning)
        {
            if (TestMoveRight())
            {
                _currentPiece->MoveRight();
                result = "{\"result\":\"ok\",\"score\":\"" + GetScore() + "\",\"next\":\"" + GetNextPiece() + "\"}";
            }
            else
            {
                result = "{\"result\":\"failed\",\"score\":\"" + GetScore() + "\",\"next\":\"" + GetNextPiece() + "\"}";
            }
        }
        else
        {
            result = GameNotRunningResult();
        }
    }
    else if (command == "s") // spin
    {
        if (_gameRunning)
        {
            if (TestSpin())
            {
                _currentPiece->Rotate();
                result = "{\"result\":\"ok\",\"score\":\"" + GetScore() + "\",\"next\":\"" + GetNextPiece() + "\"}";
            }
            else
            {
                result = "{\"result\":\"failed\",\"score\":\"" + GetScore() + "\",\"next\":\"" + GetNextPiece() + "\"}";
            }
        }
        else
        {
            result = GameNotRunningResult();
        }
    }
    else if (command == "d") // drop
    {
        if (_gameRunning)
        {
        Drop();
            result = "{\"result\":\"ok\",\"score\":\"" + GetScore() + "\",\"next\":\"" + GetNextPiece() + "\"}";
        }
        else
        {
            result = GameNotRunningResult();
        }
    }
    else if (command == "c") // quit
    {
        if (_gameRunning)
        {
            Close(result);
        }
        else
        {
            result = GameNotRunningResult();
        }
    }
    else if (command == "g") // start game
    {
        if (_gameRunning)
        {
            result = "{\"result\":\"failed\",\"score\":\"" + GetScore() + "\",\"next\":\"" + GetNextPiece() + "\"}";
        }
        else
        {
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
            _fullTime = wxDateTime((time_t)0);
            _lastUpdatedMovement = wxDateTime::Now();
            memset(_board, 0xFF, sizeof(_board));
            _dropSpeed = STARTSPEED;
            _score = 0;
            if (parameters != "")
            {
                _playerName = parameters;
            }
            _gameRunning = true;
            result = "{\"result\":\"ok\",\"score\":\"" + GetScore() + "\",\"next\":\"" + GetNextPiece() + "\"}";
        }
    }

    return true;
}

std::string Xyzzy::GetHighScore() const
{
    return wxString::Format(wxT("%i %s"), _highScore, _highScoreOwner).ToStdString();
}

std::string Xyzzy::GetScore() const
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

    for (auto it = pts.begin(); it != pts.end(); ++it)
    {
        if (it->y < BOARDHEIGHT)
        {
            if (it->x < 0) return false;
            if (_board[(it->y) * BOARDWIDTH + it->x] != 0xFF) return false;
        }
    }

    return true;
}

bool Xyzzy::TestMoveRight() const
{
    auto pts = _currentPiece->TestRight();

    for (auto it = pts.begin(); it != pts.end(); ++it)
    {
        if (it->y < BOARDHEIGHT)
        {
            if (it->x >= BOARDWIDTH) return false;
            if (_board[(it->y) * BOARDWIDTH + it->x] != 0xFF) return false;
        }
    }

    return true;
}

bool Xyzzy::TestSpin() const
{
    auto pts = _currentPiece->TestRotate();

    for (auto it = pts.begin(); it != pts.end(); ++it)
    {
        if (it->y < BOARDHEIGHT)
        {
            if (_board[(it->y) * BOARDWIDTH + it->x] != 0xFF) return false;
        }
    }

    return true;
}

void Xyzzy::Drop()
{
    int dropdistance = 0;
    while (TestMoveDown())
    {
        _currentPiece->MoveDown();
        dropdistance++;
    }

    _score += dropdistance * DROPBONUSPERROW;

    MakePiecePermanent();
    delete _currentPiece;
    _currentPiece = _nextPiece;
    _nextPiece = XyzzyPiece::CreatePiece();
}

bool Xyzzy::TestMoveDown() const
{
    auto pts = _currentPiece->TestDown();

    for (auto it = pts.begin(); it != pts.end(); ++it)
    {
        if (it->y < BOARDHEIGHT)
        {
            if (it->y < 0) return false;
            if (_board[(it->y) * BOARDWIDTH + it->x] != 0xFF) return false;
        }
    }

    return true;
}

void Xyzzy::MakePiecePermanent()
{
    auto pixels = _currentPiece->DrawPoints();
    for (auto it = pixels.begin(); it != pixels.end(); ++it)
    {
        _board[it->y * BOARDWIDTH + it->x] = _currentPiece->GetType();
    }
}

XyzzyPiece* XyzzyPiece::CreatePiece()
{
    static int hardcount = 0;

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
        res.push_back(wxPoint(position.x + 1, position.y + 1));
        res.push_back(wxPoint(position.x + 1, position.y));
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
    if (!_gameRunning) return false;

    if (_fullTime != wxDateTime((time_t)0) && (wxDateTime::Now() - _fullTime).GetMilliseconds() < PAUSEONCOMPLETEROWTIME) return true;

    if (_fullTime != wxDateTime((time_t)0))
    {
        _fullTime = wxDateTime((time_t)0);
        // clear any white rows
        for (int y = 0; y < BOARDHEIGHT; y++)
        {
            if (_board[y * BOARDWIDTH] == 7)
            {
                if (y < BOARDHEIGHT - 1)
                {
                    // move the rows down
                    memcpy(&_board[y * BOARDWIDTH], &_board[(y + 1)*BOARDWIDTH], (BOARDHEIGHT - y - 1) * BOARDWIDTH);
                }

                // blank top row
                memset(&_board[(BOARDHEIGHT - 1) * BOARDWIDTH], 0xFF, BOARDWIDTH);
                y--;
            }
        }
    }

    if (_currentPiece == nullptr)
    {
        _currentPiece = XyzzyPiece::CreatePiece();
        _lastUpdatedMovement = wxDateTime::Now();
    }

    if (_nextPiece == nullptr)
    {
        _nextPiece = XyzzyPiece::CreatePiece();
    }

    auto elapsed = (wxDateTime::Now() - _lastUpdatedMovement).GetMilliseconds().ToLong();

    if (elapsed > _dropSpeed)
    {
        if (TestMoveDown())
        {
            _currentPiece->MoveDown();
            _lastUpdatedMovement = wxDateTime::Now();
        }
        else
        {
            auto loc = _currentPiece->DrawPoints();
            for (auto it = loc.begin(); it != loc.end(); ++it)
            {
                if (it->y >= BOARDHEIGHT)
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
            _score += PIECESCORE;

            // now check if any row is complete
            int fullcount = 0;
            for (int y = 0; y < BOARDHEIGHT; y++)
            {
                bool full = true;
                for (int x = 0; x < BOARDWIDTH; x++)
                {
                    if (_board[y * BOARDWIDTH + x] == 0xFF)
                    {
                        full = false;
                        break;
                    }
                }

                if (full)
                {
                    fullcount++;
                    for (int x = 0; x < BOARDWIDTH; x++)
                    {
                        _board[y * BOARDWIDTH + x] = 7;
                        _fullTime = wxDateTime::Now();
                    }
                }
            }

            if (fullcount > 0)
            {
                _dropSpeed += ROWADJUSTSPEED;
                if (_dropSpeed < MINDROPSPEED)
                {
                    _dropSpeed = MINDROPSPEED;
                }
            }

            switch (fullcount)
            {
            case 1:
                _score += ROWSCORE1;
                break;
            case 2:
                _score += ROWSCORE2;
                break;
            case 3:
                _score += ROWSCORE3;
                break;
            case 4:
                _score += ROWSCORE4;
                break;
            }
        }
    }

    // do game updates here
    return false;
}

void Xyzzy::SaveHighScore()
{
    if (_score > _highScore)
    {
        _highScore = _score;
        _highScoreOwner = _playerName;

        wxConfigBase* config = wxConfigBase::Get();
        config->Write("XyzzyHighScore", _highScore);
        config->Write("XyzzyHighScoreOwner", wxString(_highScoreOwner));
    }
}