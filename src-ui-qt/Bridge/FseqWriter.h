#pragma once
#include <QFile>
#include <QString>

// Writes an uncompressed FSEQ v2 file suitable for Falcon Player.
// Usage: construct → isOpen() check → writeFrame() per frame → finish().
class FseqWriter {
public:
    FseqWriter(const QString& path, int channelCount, int frameCount, int stepTimeMs);
    ~FseqWriter();

    bool isOpen()     const { return _file.isOpen(); }
    int  channelCount() const { return _channelCount; }

    // Append one frame of channel data. data.size() should equal channelCount.
    void writeFrame(const QByteArray& data);

    // Flush and close.  Call once after the last writeFrame().
    void finish();

    QString errorString() const { return _file.errorString(); }

private:
    void writeHeader();

    QFile _file;
    int   _channelCount;
    int   _frameCount;
    int   _stepTimeMs;
    int   _framesWritten = 0;
};
