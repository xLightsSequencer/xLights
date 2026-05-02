#include "FseqWriter.h"

#include <QDataStream>
#include <QDateTime>

// ── FSEQ v2 header layout (32 bytes, little-endian) ──────────────────────────
// Offset  Size  Field
//  0      4     Magic "PSEQ"
//  4      2     channel data start offset (= 32)
//  6      1     minor version (0)
//  7      1     major version (2)
//  8      2     fixed header length (32)
// 10      4     channel count
// 14      4     frame count
// 18      1     step time ms
// 19      1     flags (0)
// 20      1     compression type (0 = none)
// 21      1     num compression blocks (0)
// 22      1     num sparse ranges (0)
// 23      1     reserved (0)
// 24      8     unique id (unix timestamp ms)

static constexpr quint16 kDataOffset   = 32;
static constexpr quint16 kHeaderLen    = 32;
static constexpr quint8  kMajorVersion = 2;
static constexpr quint8  kMinorVersion = 0;

FseqWriter::FseqWriter(const QString& path, int channelCount, int frameCount, int stepTimeMs)
    : _channelCount(channelCount), _frameCount(frameCount), _stepTimeMs(stepTimeMs)
{
    _file.setFileName(path);
    if (!_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
    writeHeader();
}

FseqWriter::~FseqWriter() {
    if (_file.isOpen()) finish();
}

void FseqWriter::writeHeader() {
    QDataStream ds(&_file);
    ds.setByteOrder(QDataStream::LittleEndian);

    // Magic
    ds.writeRawData("PSEQ", 4);

    ds << quint16(kDataOffset);       // channel data start
    ds << quint8(kMinorVersion);
    ds << quint8(kMajorVersion);
    ds << quint16(kHeaderLen);        // fixed header length
    ds << quint32(_channelCount);
    ds << quint32(_frameCount);
    ds << quint8(qBound(1, _stepTimeMs, 255));
    ds << quint8(0);                  // flags
    ds << quint8(0);                  // compression type: 0 = uncompressed
    ds << quint8(0);                  // num compression blocks
    ds << quint8(0);                  // num sparse ranges
    ds << quint8(0);                  // reserved

    // Unique id: ms since epoch (8 bytes)
    quint64 uid = quint64(QDateTime::currentMSecsSinceEpoch());
    ds << uid;

    // Header is exactly 32 bytes at this point.
}

void FseqWriter::writeFrame(const QByteArray& data) {
    if (!_file.isOpen()) return;
    int writeLen = qMin(data.size(), _channelCount);
    _file.write(data.constData(), writeLen);
    // Pad with zeros if data is short
    if (writeLen < _channelCount) {
        QByteArray pad(_channelCount - writeLen, 0);
        _file.write(pad);
    }
    ++_framesWritten;
}

void FseqWriter::finish() {
    if (!_file.isOpen()) return;

    // If fewer frames were written than declared, pad with silent frames.
    if (_framesWritten < _frameCount) {
        QByteArray silence(_channelCount, 0);
        while (_framesWritten < _frameCount) {
            _file.write(silence);
            ++_framesWritten;
        }
    }

    // Patch the frame count in the header if it differs.
    if (_framesWritten != _frameCount) {
        _file.seek(14);
        QDataStream ds(&_file);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << quint32(_framesWritten);
    }

    _file.close();
}
