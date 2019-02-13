#ifndef XYZZY_H
#define XYZZY_H

#include <wx/wx.h>
#include <list>

//#define SHOWVIRTUALMATRIX

class MatrixMapper;

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

class Xyzzy 
{
    wxLongLong _lastUpdatedMovement;
    int _highScore;
    int _score;
    std::string _playerName;
    std::string _highScoreOwner;
    uint8_t _board[BOARDWIDTH*BOARDHEIGHT];
    int _colsPerSquare;
    int _rowsPerSquare;
    bool _isOk;
    XyzzyPiece* _currentPiece;
    XyzzyPiece* _nextPiece;
    int _dropSpeed;
    int _sideBorder;
    int _bottomBorder;
    MatrixMapper* _matrixMapper;
    bool _gameRunning;
    wxLongLong _fullTime;

#ifdef SHOWVIRTUALMATRIX
    VirtualMatrix* _virtualMatrix;
#endif

    void SaveHighScore();
    std::string GetHighScore() const;
    std::string GetHighScorePlayer() const { return _highScoreOwner; }
    std::string GetScore() const;
    std::string GetNextPiece() const;
    bool TestMoveLeft() const;
    bool TestMoveRight() const;
    bool TestMoveDown() const;
    bool TestSpin() const;
    void Drop();
    void DrawPixel(int x, int y, wxColour c, uint8_t* buffer, size_t size);
    bool AdvanceGame();
    void MakePiecePermanent();
    std::string GameNotRunningResult(const std::string& reference);
    void DrawNode(int x, int y, wxColour c, uint8_t* buffer, size_t size);
    void CheckFullRow();
    void Reset();
    void AddToScore(int add);

public:

        Xyzzy();
        virtual ~Xyzzy();
        bool Frame(uint8_t* buffer, size_t size, bool outputframe); 
        void DrawBlack(uint8_t* buffer, size_t size);
        void Initialise(const wxString& parameters, wxString& result, const wxString& reference);
        void Close(wxString& result, const wxString& reference);
        bool Action(const wxString& command, const wxString& parameters, wxString& result, const wxString& reference);
        bool IsOk() const { return _isOk; }
};

#endif
