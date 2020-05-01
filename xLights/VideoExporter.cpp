/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "VideoExporter.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <algorithm>
#include <cstring>
#include <stdexcept>

#ifdef min
#undef min
#endif

namespace
{
   // initialize to solid color (varies with each frame)
   bool getVideo( uint8_t* buf, int bufSize, unsigned frameIndex )
   {
      uint8_t* ptr = buf;
      enum Color { Red, Green, Blue } color = Color( frameIndex % 3 );
      int n = bufSize / 3;
      for ( int i = 0; i < n; ++i )
      {
         if ( color == Red )
         {
            *ptr++ = 0xff;
            *ptr++ = 0x00;
            *ptr++ = 0x00;
         }
         else if ( color == Green )
         {
            *ptr++ = 0x00;
            *ptr++ = 0xff;
            *ptr++ = 0x00;
         }
         else
         {
            *ptr++ = 0x00;
            *ptr++ = 0x00;
            *ptr++ = 0xff;
         }
      }
      return true;
   }

   // initialize to silence
   bool getAudio( float* leftCh, float *rightCh, int frameSize )
   {
      std::memset( leftCh, 0, frameSize * sizeof( float ) );
      std::memset( rightCh, 0, frameSize * sizeof( float ) );

      return true;
   }

   bool queryForCancel()
   {
      return false;
   }

   void progressReporter( int )
   {

   }

   //void my_av_log_callback( void* ptr, int level, const char*fmt, va_list vargs )
   //{
   //   char message[2048];
   //   if ( level <= 16 )
   //   {
   //      ::vsnprintf( message, 2048, fmt, vargs );
   //   }
   //}
}

GenericVideoExporter::GenericVideoExporter( const std::string& outPath, const Params& inParams, bool videoOnly/*=false*/ )
   : _path( outPath )
   , _inParams( inParams )
   , _videoOnly( videoOnly )
{
   if ( inParams.pfmt != AV_PIX_FMT_RGB24 )
      throw std::runtime_error( "VideoExporter - expecting RGB24 input!" );

   _outParams = inParams;

   // MP4/MOV has some restrictions on width... apparently it's common
   // with FFmpeg to just enforce even-number width and height
   if ( _outParams.width % 2 )
      ++_outParams.width;
   if ( _outParams.height % 2 )
      ++_outParams.height;

   // We're outputing an H.264 / AAC MP4 file; most players only support profiles with 4:2:0 compression
   _outParams.pfmt = AV_PIX_FMT_YUV420P;

   _getVideo = getVideo;
   _getAudio = getAudio;
   _queryForCancel = queryForCancel;
   _progressReporter = progressReporter;

   //::av_log_set_callback( my_av_log_callback );
}

GenericVideoExporter::~GenericVideoExporter()
{
   cleanup();

   //::av_log_set_callback( nullptr );
}

void GenericVideoExporter::initialize()
{
   // Initialize color-converter
   int sws_flags = SWS_FAST_BILINEAR; // usually doing just a colorspace conversion, so not too critical

   AVPixelFormat inPfmt = static_cast<AVPixelFormat>( _inParams.pfmt );
   AVPixelFormat outPfmt = static_cast<AVPixelFormat>( _outParams.pfmt );
   SwsContext* sws_ctx = ::sws_getContext( _inParams.width, _inParams.height, inPfmt,
                                           _outParams.width, _outParams.height, outPfmt,
                                           sws_flags, nullptr, nullptr, nullptr );
   if ( sws_ctx == nullptr )
      throw std::runtime_error( "VideoExporter - error setting up video format conversion!" );

   // Initialize video & audio
   AVOutputFormat* fmt = ::av_guess_format( nullptr, _path.c_str(), nullptr );
   const AVCodec* videoCodec = ::avcodec_find_encoder( fmt->video_codec );
   const AVCodec* audioCodec = ::avcodec_find_encoder( fmt->audio_codec );

   int status = ::avformat_alloc_output_context2( &_formatContext, fmt, nullptr, _path.c_str() );
   if ( _formatContext == nullptr )
      throw std::runtime_error( "VideoExporter - Error allocating output-context" );

   initializeVideo( videoCodec );
   initializeAudio( audioCodec );

   // Initialize frames and packets
   initializeFrames();
   initializePackets();

   // Open file for output and write header
   status = ::avio_open( &_formatContext->pb, _path.c_str(), AVIO_FLAG_WRITE );
   if ( status < 0 )
      throw std::runtime_error( "VideoExporter - Error opening output file" );

   // prepare to write... don't trust ::avformat_init_output() telling you that
   // a call to ::avformat_write_header() is unnecessary. If you don't call it,
   // the stream(s) won't be packaged in an MP4 container. Also, the stream's
   // time_base appears to be updated within this call.
   status = ::avformat_write_header( _formatContext, nullptr );
   if ( status < 0 )
      throw std::runtime_error( "VideoExporter - Error writing file header" );

   _ptsIncrement = _formatContext->streams[0]->time_base.den / _outParams.fps;
}

void GenericVideoExporter::initializeVideo( const AVCodec* codec )
{
   AVStream* video_st = ::avformat_new_stream( _formatContext, nullptr );
   video_st->time_base.num = 1;
   video_st->time_base.den = _outParams.fps;
   video_st->id = _formatContext->nb_streams - 1;

   _videoCodecContext = ::avcodec_alloc_context3( codec );
   _videoCodecContext->time_base.num = 1;
   _videoCodecContext->time_base.den = _outParams.fps;
   _videoCodecContext->gop_size = 40/*12*/; // aka keyframe interval
   _videoCodecContext->max_b_frames = 0;
   _videoCodecContext->width = _outParams.width;
   _videoCodecContext->height = _outParams.height;
   _videoCodecContext->pix_fmt = static_cast<AVPixelFormat>( _outParams.pfmt );
   if ( _formatContext->oformat->flags & AVFMT_GLOBALHEADER )
      _videoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

   ::av_opt_set( _videoCodecContext->priv_data, "preset", "fast", 0 );
   ::av_opt_set( _videoCodecContext->priv_data, "crf", "18", AV_OPT_SEARCH_CHILDREN );

   int status = ::avcodec_open2( _videoCodecContext, nullptr, nullptr );
   if ( status != 0 )
      throw std::runtime_error( "VideoExporter - Error opening video codec context" );

   status = ::avcodec_parameters_from_context( video_st->codecpar, _videoCodecContext );
   if ( status != 0 )
      throw std::runtime_error( "VideoExporter - Error setting video stream parameters" );
}

void GenericVideoExporter::initializeAudio( const AVCodec* codec )
{
   AVStream* audio_st = ::avformat_new_stream( _formatContext, nullptr );
   audio_st->time_base.num = 1;
   audio_st->time_base.den = _outParams.audioSampleRate;
   audio_st->id = _formatContext->nb_streams - 1;

   _audioCodecContext = ::avcodec_alloc_context3( codec );
   _audioCodecContext->channels = 2;
   _audioCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
   _audioCodecContext->sample_rate = _outParams.audioSampleRate;
   _audioCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
   _audioCodecContext->bit_rate = 128000;

   if ( _formatContext->oformat->flags & AVFMT_GLOBALHEADER )
      _audioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

   int status = ::avcodec_open2( _audioCodecContext, nullptr, nullptr );
   if ( status != 0 )
      throw std::runtime_error( "VideoExporter - Error opening audio codec context" );

   status = ::avcodec_parameters_from_context( audio_st->codecpar, _audioCodecContext );
   if ( status != 0 )
      throw std::runtime_error( "VideoExporter - Error setting audio stream parameters" );
}

void GenericVideoExporter::initializeFrames()
{
   _colorConversionFrame = ::av_frame_alloc();
   _colorConversionFrame->width = _inParams.width;
   _colorConversionFrame->height = _inParams.height;
   _colorConversionFrame->format = _inParams.pfmt;
   int status = ::av_frame_get_buffer( _colorConversionFrame, 0 );
   if ( status != 0 )
      throw std::runtime_error( "VideoExporter - Error initializing color-conversion frame" );

   _videoFrame = ::av_frame_alloc();
   _videoFrame->width = _outParams.width;
   _videoFrame->height = _outParams.height;
   _videoFrame->format = _outParams.pfmt;
   status = ::av_frame_get_buffer( _videoFrame, 0 );
   if ( status != 0 )
      throw std::runtime_error( "VideoExporter - Error initializing video frame" );
   _videoFrame->pts = 0LL;

   int flags = SWS_FAST_BILINEAR; // doesn't matter too much since we're just doing a colorspace conversion
   AVPixelFormat inPfmt = static_cast<AVPixelFormat>( _inParams.pfmt );
   AVPixelFormat outPfmt = static_cast<AVPixelFormat>( _outParams.pfmt );

   _swsContext = ::sws_getContext( _inParams.width, _inParams.height, inPfmt,
                                   _outParams.width, _outParams.height, outPfmt,
                                   flags, nullptr, nullptr, nullptr );
   if ( _swsContext == nullptr )
      throw std::runtime_error( "VideoExporter - Error initializing color-converter" );

   if ( _audioCodecContext != nullptr )
   {
      _audioFrame = ::av_frame_alloc();
      _audioFrame->format = AV_SAMPLE_FMT_FLTP;
      _audioFrame->nb_samples = _audioCodecContext->frame_size;
      _audioFrame->channel_layout = AV_CH_LAYOUT_STEREO;
      _audioFrame->channels = 2;
      _audioFrame->sample_rate = _outParams.audioSampleRate;
      status = ::av_frame_get_buffer( _audioFrame, 0 );
      if ( status != 0 )
         throw std::runtime_error( "VideoExporter - Error initializing audio frame" );
      _audioFrame->pts = 0LL;
   }
}

void GenericVideoExporter::initializePackets()
{
   _videoPacket = ::av_packet_alloc();
   ::av_init_packet( _videoPacket );

   _audioPacket = ::av_packet_alloc();
   ::av_init_packet( _audioPacket );
}

void GenericVideoExporter::exportFrames( int videoFrameCount )
{
   // Accumulate the initial packet of compressed video (actually 35 video frames)
   _videoFrame->nb_samples = 0;
   int endFrameIndex = pushVideoUntilPacketFilled( 0 );

   // Write the first packet of compressed video
   _videoPacket->stream_index = 0;
   int status = ::av_interleaved_write_frame( _formatContext, _videoPacket );
   if ( status < 0 )
      throw std::runtime_error( "VideoExporter - error writing initial compressed video packet" );

   // We'll need to push a bunch of audio packets through in order to "catch up" to the video
   if ( !_videoOnly )
   {
      int64_t numAudioSamplesToPush = endFrameIndex * _outParams.audioSampleRate / _outParams.fps;
      for ( int64_t numAudioSamplesPushed = 0; numAudioSamplesPushed < numAudioSamplesToPush; )
      {
         // For AAC, first packet will be 2048 samples; always 1024 after that
         _audioFrame->nb_samples = 0;
         int64_t ptsBefore = _audioFrame->pts;
         pushAudioUntilPacketFilled();

         int64_t elapsed = _audioFrame->pts - ptsBefore;
         numAudioSamplesPushed += elapsed;
         _audioPacket->stream_index = 1;
         status = ::av_interleaved_write_frame( _formatContext, _audioPacket );
         if ( status < 0 )
            throw std::runtime_error( "VideoExporter - error writing compressed audio packet" );
      }
   }

   // ... and beyond the inital write_frame() calls...
   while ( endFrameIndex <= videoFrameCount )
   {
      // Some housekeeping for cancel and progress reporting
      if ( _queryForCancel != nullptr && _queryForCancel() )
      {
         ::avio_closep( &_formatContext->pb );
         return;
      }
      double exportPercentage = double(endFrameIndex) / videoFrameCount;
      int progressAsInt = int( 100 * exportPercentage );
      if ( _progressReporter != nullptr )
         _progressReporter( progressAsInt );

      _videoFrame->nb_samples = 0;
      endFrameIndex = pushVideoUntilPacketFilled( endFrameIndex );

      // Write the packet of compressed video
      _videoPacket->stream_index = 0;
      int status = ::av_interleaved_write_frame( _formatContext, _videoPacket );
      if ( status < 0 )
         throw std::runtime_error( "VideoExporter - error writing compressed video packet" );

      // Process and write some (typically 1 to 3) packets of audio to keep roughly in sync with video
      if ( !_videoOnly )
      {
         int64_t numAudioSamplesToPush = endFrameIndex * _outParams.audioSampleRate / _outParams.fps - _audioFrame->pts;
         int64_t numAudioFramesToPush = numAudioSamplesToPush / _audioCodecContext->frame_size;
         for ( int64_t i = 0; i < numAudioFramesToPush; ++i )
         {
            _audioFrame->nb_samples = 0;
            pushAudioUntilPacketFilled();

            _audioPacket->stream_index = 1;
            status = ::av_interleaved_write_frame( _formatContext, _audioPacket );
            if ( status < 0 )
               throw std::runtime_error( "VideoExporter - error writing compressed audio packet" );
         }
      }
   }

   // Finally, clear out any buffered data
   status = ::avcodec_send_frame( _videoCodecContext, nullptr );
   if ( status < 0 )
      throw std::runtime_error( "VideoExporter - error clearing compressed-video cache" );
   while ( 1 )
   {
      status = ::avcodec_receive_packet( _videoCodecContext, _videoPacket );
      if ( status == AVERROR_EOF )
         break;
      if ( status == 0 )
      {
         _videoPacket->stream_index = 0;
         status = ::av_interleaved_write_frame( _formatContext, _videoPacket );
         if ( status < 0 )
            throw std::runtime_error( "VideoExporter - error writing cached video frame data" );
      }
   }

   if ( !_videoOnly )
   {
      status = ::avcodec_send_frame( _audioCodecContext, nullptr );
      if ( status < 0 )
         throw std::runtime_error( "VideoExporter - error clearing compressed-audio cache" );
      while ( 1 )
      {
         status = ::avcodec_receive_packet( _audioCodecContext, _audioPacket );
         if ( status == AVERROR_EOF )
            break;
         if ( status == 0 )
         {
            _audioPacket->stream_index = 1;
            status = ::av_interleaved_write_frame( _formatContext, _audioPacket );
            if ( status < 0 )
               throw std::runtime_error( "VideoExporter - error writing cached audio frame data" );
         }
      }
   }
}

void GenericVideoExporter::completeExport()
{
   int status = ::av_write_trailer( _formatContext );
   if ( status != 0 )
      throw std::runtime_error( "VideoExporter - Error writing file trailer" );

   status = ::avio_closep( &_formatContext->pb );
   if ( status != 0 )
      throw std::runtime_error( "VideoExporter - Error closing output file" );
}

void GenericVideoExporter::cleanup()
{
   if ( _videoPacket != nullptr )
      ::av_packet_free( &_videoPacket );
   if ( _audioPacket != nullptr )
      ::av_packet_free( &_audioPacket );

   if ( _colorConversionFrame != nullptr )
      ::av_frame_free( &_colorConversionFrame );
   if ( _videoFrame != nullptr )
      ::av_frame_free( &_videoFrame );
   if ( _audioFrame != nullptr )
      ::av_frame_free( &_audioFrame );

   if ( _formatContext != nullptr )
   {
      if ( _formatContext->pb != nullptr )
         ::avio_closep( &_formatContext->pb );
      ::avformat_free_context( _formatContext );
      _formatContext = nullptr;
   }

   if ( _audioCodecContext != nullptr )
      ::avcodec_free_context( &_audioCodecContext );

   if ( _videoCodecContext != nullptr )
      ::avcodec_free_context( &_videoCodecContext );

   if ( _swsContext != nullptr )
   {
      ::sws_freeContext( _swsContext );
      _swsContext = nullptr;
   }
}

int GenericVideoExporter::pushVideoUntilPacketFilled( int index )
{
   int status = 0;

   uint8_t* data[] = { _colorConversionFrame->data[0], nullptr, nullptr, nullptr };
   int stride[] = { _colorConversionFrame->linesize[0], 0, 0, 0 };
   int frameHeight = _colorConversionFrame->height;
   int frameSize = stride[0] * frameHeight;

   do
   {
      _getVideo( data[0], frameSize, index++ );

      int height = ::sws_scale( _swsContext, data, stride, 0, frameHeight, _videoFrame->data, _videoFrame->linesize );
      if ( height != _videoCodecContext->height )
         throw std::runtime_error( "VideoExporter - color conversion error" );

      status = ::avcodec_send_frame( _videoCodecContext, _videoFrame );
      if ( status < 0 )
         throw std::runtime_error( "VideoExporter - error sending video frame to compresssor" );
      _videoFrame->pts += _ptsIncrement;

      status = ::avcodec_receive_packet( _videoCodecContext, _videoPacket );
      if ( status == AVERROR( EAGAIN ) )
         continue;
      if ( status < 0 )
         throw std::runtime_error( "VideoExporter - error receiving compressed video" );
   } while ( status != 0 );

   return index;
}

void GenericVideoExporter::pushAudioUntilPacketFilled()
{
   int status = 0;

   float *dstLeft = reinterpret_cast<float *>( _audioFrame->buf[0]->data );
   float *dstRight = reinterpret_cast<float *>( _audioFrame->buf[1]->data );

   do
   {
      // todo - handle when we can't get a full frame of audio
      _getAudio( dstLeft, dstRight, _audioCodecContext->frame_size );
      _audioFrame->nb_samples = _audioCodecContext->frame_size;

      status = ::avcodec_send_frame( _audioCodecContext, _audioFrame );
      if ( status < 0 )
         throw std::runtime_error( "VideoExporter - error sending audio frame to compresssor" );
      _audioFrame->pts += _audioCodecContext->frame_size;

      status = ::avcodec_receive_packet( _audioCodecContext, _audioPacket );
      if ( status == AVERROR( EAGAIN ) )
         continue;
      if ( status < 0 )
         throw std::runtime_error( "VideoExporter - error receiving compressed audio" );
   } while ( status != 0 );
}


namespace
{
    GenericVideoExporter::Params makeParams( int width, int height, int fps, int audioSampleRate )
    {
        GenericVideoExporter::Params p =
        {
            AV_PIX_FMT_RGB24,
            width,
            height,
            fps,
            audioSampleRate
        };
        return p;
    }
}

VideoExporter::VideoExporter( wxWindow *parent,
                              int width, int height, float scaleFactor,
                              unsigned int frameDuration, unsigned int frameCount,
                              int audioChannelCount, int audioSampleRate,
                              const std::string& outPath,
                              log4cpp::Category &logger_base )
    : GenericVideoExporter( outPath, makeParams( width * scaleFactor, height * scaleFactor, 1000u / frameDuration, audioSampleRate ) )
    , _frameCount( frameCount )
{
    if ( audioChannelCount != 2 )
        throw std::runtime_error( "VideoExporter - assumes stereo input/output currently" );
}

bool VideoExporter::Export()
{
    // todo - enable cancel and progress updates!!

    initialize();
    exportFrames( _frameCount );
    completeExport();

    return true;
}
