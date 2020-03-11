#ifndef SDLRENDERWND_H
#define SDLRENDERWND_H

#include <QWidget>
extern "C" {
#include "SDL.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

//Refresh Event
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
//Break
#define BREAK_EVENT  (SDL_USEREVENT + 2)

struct PacketItem
{
    PacketItem()
        : m_next(nullptr)
        , m_prev(nullptr)
    {}

    AVPacket m_packet;
    PacketItem* m_next;
    PacketItem* m_prev;
};

struct PacketQueue
{
  PacketItem* m_head;
  PacketItem* m_tail;
  int m_count;
  SDL_mutex* m_mutex;
  SDL_cond* m_cond;
  PacketQueue();
  ~PacketQueue();

  void InQueue(AVPacket* packet);
  bool DeQueue(AVPacket& packet);
  bool IsEmpty() const;
};

class SDLRenderWnd : public QWidget
{
    Q_OBJECT

public:
    SDLRenderWnd(QWidget *parent = nullptr);
    ~SDLRenderWnd();

    void Clear();

protected:
    virtual void resizeEvent(QResizeEvent *event);
private:
    static void SDL_Related_Init();
    static void SDL_Related_Uninit();
    static void FFMPEG_Init();
    static void FFMPEG_UnInit();
    static void ShowError(const QString& message);

public slots:
    void PresentFrame(const void *pixels, int pitch);
    void PresentRedWindow();
    void PlayVideo(const QString& filename);

private:
    SDL_Renderer*        m_pRender;
    SDL_Texture*         m_pTexture;
    SDL_Window*          m_pWindow;

    AVFormatContext* m_pAvFormatContext;
    AVCodecContext* m_pCodecCtx;
    AVCodec* m_pCodec;

    AVCodecContext* m_pAudioCodecCtx;
    AVCodec* m_pAudioCodec;

    AVPacket* m_pPacket;
    AVFrame* m_pFrame;
    AVFrame* m_pAudioFrame;
    AVFrame* m_pFrameYUV;
    SwsContext* m_pSwsContext;

    int  m_screen_width;
    int  m_screen_height;
    bool m_exit;
    static int            m_nRef;
    PacketQueue           pq;
};

#endif // SDLRENDERWND_H
