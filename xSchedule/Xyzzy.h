#ifndef XYZZY_H
#define XYZZY_H

#include <wx/wx.h>
#include <list>

//#define SHOWVIRTUALMATRIX

class MatrixMapper;
class OutputManager;

#ifdef SHOWVIRTUALMATRIX
class VirtualMatrix;
#endif

#define BOARDWIDTH 10
#define BOARDHEIGHT 20
#define PAUSEONCOMPLETEROWTIME 200
#define STARTSPEED 800
#define ROWADJUSTSPEED -30
#define MAXHARDPIECES 3
#define DROPBONUSPERROW 10
#define PIECESCORE 1
#define ROWSCORE1 100
#define ROWSCORE2 250
#define ROWSCORE3 500
#define ROWSCORE4 1000
#define MINDROPSPEED 50

enum class XYZZY2
{
    LEFT,
    RIGHT,
    UP,
    DOWN,
    EMPTY,
    HEAD,
    BODY,
    TAIL,
    PILL
};

class XyzzyPiece
{
protected:
    int _rotation;
    wxPoint _position;

public:
    XyzzyPiece() { _rotation = 0; }
    virtual ~XyzzyPiece() {}
    virtual std::list<wxPoint> TestRotate() const;
    virtual std::list<wxPoint> TestDown() const;
    virtual std::list<wxPoint> TestLeft() const;
    virtual std::list<wxPoint> TestRight() const;
    virtual std::list<wxPoint> DrawPoints() const;
    virtual std::list<wxPoint> GetPoints(int rotation, wxPoint position) const = 0;
    void Rotate();
    virtual wxColor GetColour() const;
    virtual void MoveDown() { _position.y--; }
    virtual void MoveLeft() { _position.x--; }
    virtual void MoveRight() { _position.x++; }
    virtual int GetType() const = 0;
    static XyzzyPiece* CreatePiece();
    int GetX() const { return _position.x; }
    int GetY() const { return _position.y; }
};

// xxxx red 0
class LinePiece : public XyzzyPiece
{
public:
    LinePiece() { _rotation = 0; _position = wxPoint(5, 20); }
    virtual ~LinePiece() {}
    virtual std::list<wxPoint> GetPoints(int rotation, wxPoint position) const override;
    virtual int GetType() const override { return 0; }
};

//   x
// xxx  magenta 1
class LPiece : public XyzzyPiece
{
public:
    LPiece() { _rotation = 0; _position = wxPoint(5, 20); }
    virtual ~LPiece() {}
    virtual std::list<wxPoint> GetPoints(int rotation, wxPoint position) const override;
    virtual int GetType() const override { return 1; }
};

// x
// xxx  yellow 2
class JPiece : public XyzzyPiece
{
public:
    JPiece() { _rotation = 0; _position = wxPoint(5, 20); }
    virtual ~JPiece() {}
    virtual std::list<wxPoint> GetPoints(int rotation, wxPoint position) const override;
    virtual int GetType() const override { return 2; }
};

//  xx  Blue 3
// xx
class SPiece : public XyzzyPiece
{
public:
    SPiece() { _rotation = 0; _position = wxPoint(5, 20); }
    virtual ~SPiece() {}
    virtual std::list<wxPoint> GetPoints(int rotation, wxPoint position) const override;
    virtual int GetType() const override { return 3; }
};

// xx   lime green 4
//  xx
class ZPiece : public XyzzyPiece
{
public:
    ZPiece() { _rotation = 0; _position = wxPoint(5, 20); }
    virtual ~ZPiece() {}
    virtual std::list<wxPoint> GetPoints(int rotation, wxPoint position) const override;
    virtual int GetType() const override { return 4; }
};

// xx   cyan 5
// xx
class OPiece : public XyzzyPiece
{
public:
    OPiece() { _rotation = 0; _position = wxPoint(5, 20); }
    virtual ~OPiece() {}
    virtual std::list<wxPoint> GetPoints(int rotation, wxPoint position) const override;
    virtual int GetType() const override { return 5; }
};

//  x   light grey 6
// xxx
class TPiece : public XyzzyPiece
{
public:
    TPiece() { _rotation = 0; _position = wxPoint(5, 20); }
    virtual ~TPiece() {}
    virtual std::list<wxPoint> GetPoints(int rotation, wxPoint position) const override;
    virtual int GetType() const override { return 6; }
};

class XyzzyBase
{
protected:
    bool _isOk;
    MatrixMapper* _matrixMapper = nullptr;
    int _colsPerSquare = 1;
    int _rowsPerSquare = 1;
    int _sideBorder;
    int _bottomBorder;
    int _highScore;
    int _score = 0;
    std::string _playerName;
    std::string _highScoreOwner;
    wxLongLong _lastUpdatedMovement;
    bool _gameRunning = false;
    int _bw = -1;
    int _bh = -1;

#ifdef SHOWVIRTUALMATRIX
    VirtualMatrix* _virtualMatrix;
#endif

    void BaseReset();
    void DrawPixel(int x, int y, wxColour c, uint8_t* buffer, size_t size);
    void DrawNode(int x, int y, wxColour c, uint8_t* buffer, size_t size);
    std::string GetHighScorePlayer() const { return _highScoreOwner; }
    std::string GetHighScore() const;
    std::string GetScore() const;
    std::string GameNotRunningResult(const std::string& reference);
    virtual void Reset() = 0;

public:
    XyzzyBase() {}
    virtual ~XyzzyBase() {}
    virtual bool Frame(uint8_t* buffer, size_t size, bool outputframe) = 0;
    virtual void Close(wxString& result, const wxString& reference) = 0;
    virtual bool Action(const wxString& command, const wxString& parameters, wxString& result, const wxString& reference) = 0;
    void DoInitialise(const wxString& parameters, wxString& result, const wxString& reference, OutputManager* om);
    virtual void Initialise(const wxString& parameters, wxString& result, const wxString& reference, OutputManager* om)=0;
    void DrawBlack(uint8_t* buffer, size_t size);
    bool IsOk() const { return _isOk; }
};

class Xyzzy : public XyzzyBase
{
    uint8_t _board[BOARDWIDTH*BOARDHEIGHT];
    XyzzyPiece* _currentPiece;
    XyzzyPiece* _nextPiece;
    int _dropSpeed;
    wxLongLong _fullTime;

    void SaveHighScore();
    std::string GetNextPiece() const;
    bool TestMoveLeft() const;
    bool TestMoveRight() const;
    bool TestMoveDown() const;
    bool TestSpin() const;
    void Drop();
    bool AdvanceGame();
    void MakePiecePermanent();
    void CheckFullRow();
    virtual void Reset() override;
    void AddToScore(int add);

public:

    static void GetHighScoreJSON(wxString& result, const wxString& reference);
        Xyzzy();
        virtual ~Xyzzy() {}
        virtual bool Frame(uint8_t* buffer, size_t size, bool outputframe) override; 
        virtual void Close(wxString& result, const wxString& reference) override;
        virtual bool Action(const wxString& command, const wxString& parameters, wxString& result, const wxString& reference) override;
        virtual void Initialise(const wxString& parameters, wxString& result, const wxString& reference, OutputManager* om) override;
};

class Xyzzy2 : public XyzzyBase
{
    XYZZY2 _direction;
    std::list<wxPoint> _body;
    wxPoint _pill = wxPoint(-1, -1);
    std::vector<XYZZY2> _board;

    virtual void Reset() override;
    void SaveHighScore();
    bool AdvanceGame();
    wxPoint RandomPill() const;

public:
    static void GetHighScoreJSON(wxString& result, const wxString& reference);
    Xyzzy2();
    virtual ~Xyzzy2() {}
    virtual bool Frame(uint8_t* buffer, size_t size, bool outputframe) override;
    virtual void Close(wxString& result, const wxString& reference) override;
    virtual bool Action(const wxString& command, const wxString& parameters, wxString& result, const wxString& reference) override;
    virtual void Initialise(const wxString& parameters, wxString& result, const wxString& reference, OutputManager* om) override;
};

#endif
