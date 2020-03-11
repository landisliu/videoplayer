#include "sdlrenderwnd.h"
#include <QMessageBox>



PacketQueue::PacketQueue()
    : m_head(nullptr)
    , m_tail(nullptr)
    , m_count(0)
    , m_mutex(nullptr)
    , m_cond(nullptr)
{
   m_mutex = SDL_CreateMutex();
   m_cond =  SDL_CreateCond();
}

PacketQueue::~PacketQueue()
{

}

bool PacketQueue::IsEmpty() const
{
    return m_count == 0;
}

void PacketQueue::InQueue(AVPacket* packet)
{
    SDL_LockMutex(m_mutex);
    PacketItem* item = new PacketItem;
    if(av_packet_ref(&item->m_packet, packet) != 0)
    {
        printf("inqueue error.");
    }

    if(IsEmpty())
    {
        m_head = item;
        m_tail = item;
    }
    else {
        item->m_next = m_head;
        m_head->m_prev = item;
        m_head = item;
    }
    m_count++;
    printf("count:%d\n", m_count);
    fflush(stdout);
    SDL_UnlockMutex(m_mutex);
}

bool PacketQueue::DeQueue(AVPacket& packet)
{
    SDL_LockMutex(m_mutex);
    if(IsEmpty())
    {
        SDL_UnlockMutex(m_mutex);
        return false;
    }

    if(av_packet_ref(&packet, &m_tail->m_packet) != 0)
    {
        printf("deque error.\n");
        SDL_UnlockMutex(m_mutex);
        return false;
    }

    if(m_tail == m_head)
    {
        m_tail = nullptr;
        m_head = nullptr;
    }
    else {
        m_tail = m_tail->m_prev;
    }

    m_count--;
    SDL_UnlockMutex(m_mutex);
}

//视频处理
int thread_exit = 0;
int refresh_video(void *opaque){
    thread_exit=0;
    while (thread_exit==0) {
        SDL_Event event;
        event.type = REFRESH_EVENT;
        SDL_PushEvent(&event);
        //SDL_Delay();
    }
    thread_exit=0;
    //Break
    SDL_Event event;
    event.type = BREAK_EVENT;
    SDL_PushEvent(&event);
    return 0;
}

//音频处理
static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;

void read_audio_data(void *udata, Uint8 *stream, int len) {
    SDL_memset(stream, 0, len);
    if (audio_len == 0)
        return;
    len = (len > audio_len ? audio_len : len);

    SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}


#define MAX_AUDIO_FRAME_SIZE  192000


int SDLRenderWnd::m_nRef = 0;
SDLRenderWnd::SDLRenderWnd(QWidget *parent)
    : QWidget(parent)
    , m_pTexture(nullptr)
    , m_screen_width(320)
    , m_screen_height(240)
    , m_exit(false)
{
    setUpdatesEnabled(false);
    SDL_Related_Init();
    FFMPEG_Init();
//    m_pWindow = SDL_CreateWindowFrom((void*)winId());
//    SDL_SetWindowTitle(m_pWindow, "VideoPlayer");
//    SDL_SetWindowSize(m_pWindow, m_screen_width, m_screen_height);
//    SDL_SetWindowPosition(m_pWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    m_pWindow = SDL_CreateWindow("test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_screen_width, m_screen_height, 0);
    SDL_SetWindowResizable(m_pWindow, SDL_bool::SDL_TRUE);
    m_pRender = SDL_CreateRenderer(m_pWindow, -1, 0);
}

SDLRenderWnd::~SDLRenderWnd()
{
    if (m_pWindow)
        SDL_DestroyWindow(m_pWindow);
    if (m_pRender)
        SDL_DestroyRenderer(m_pRender);
    if (m_pTexture)
        SDL_DestroyTexture(m_pTexture);
    SDL_Related_Uninit();
}

void SDLRenderWnd::FFMPEG_Init()
{
    av_register_all();
    avformat_network_init();
}

void SDLRenderWnd::PresentRedWindow()
{
    SDL_Surface* surface = SDL_GetWindowSurface(m_pWindow);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0x00, 0x00));
    SDL_UpdateWindowSurface(m_pWindow);
    SDL_Delay(2000);
    SDL_FreeSurface(surface);
    surface = NULL;

}
void SDLRenderWnd::ShowError(const QString& message)
{
    QMessageBox::information(NULL, "not select file", message,  QMessageBox::Ok);
}

 void SDLRenderWnd::PlayVideo( const QString& filename)
 {
         int videoindex = -1;
         int audioindex = -1;
         int ret, got_picture;

         QByteArray qb  = filename.toUtf8();
         char* filepath= qb.data();

         m_pAvFormatContext = avformat_alloc_context();

         if(avformat_open_input(&m_pAvFormatContext,filepath,NULL,NULL)!=0){
             ShowError("Couldn't open input stream.\n");
             return;
         }
         if(avformat_find_stream_info(m_pAvFormatContext,NULL)<0){
             ShowError("Couldn't find stream information.\n");
             return;
         }
         for(int i=0; i<m_pAvFormatContext->nb_streams; i++)
         {
             if(m_pAvFormatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
                 videoindex=i;
             }
             if(m_pAvFormatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO)
             {
                 audioindex=i;
             }
         }

         if(videoindex==-1 || audioindex == -1){
             ShowError("Didn't find a video stream.\n");
             return;
         }


         m_pCodecCtx=m_pAvFormatContext->streams[videoindex]->codec;
         m_pCodec=avcodec_find_decoder(m_pCodecCtx->codec_id);

         m_pAudioCodecCtx=m_pAvFormatContext->streams[audioindex]->codec;
         m_pAudioCodec=avcodec_find_decoder(m_pAudioCodecCtx->codec_id);

         if(m_pCodec==NULL || m_pAudioCodec == NULL){
             ShowError("Codec not found.\n");
             return;
         }

         if(avcodec_open2(m_pCodecCtx, m_pCodec,NULL)<0){
             ShowError("Could not open codec.\n");
             return;
         }

         if(avcodec_open2(m_pAudioCodecCtx, m_pAudioCodec,NULL)<0){
             ShowError("Could not open codec.\n");
             return;
         }

         m_pFrame=av_frame_alloc();
         m_pFrameYUV=av_frame_alloc();

         m_pAudioFrame = av_frame_alloc();

          uint8_t *out_buffer =(uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, m_screen_width, m_screen_height));

         avpicture_fill((AVPicture *)m_pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, m_screen_width, m_screen_height);

         m_pSwsContext = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
             m_screen_width, m_screen_height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

        m_pPacket=(AVPacket *)av_malloc(sizeof(AVPacket));


        Uint32 pixformat= SDL_PIXELFORMAT_IYUV;

        m_pTexture = SDL_CreateTexture(m_pRender,pixformat, SDL_TEXTUREACCESS_STREAMING,m_screen_width,m_screen_height);

        SDL_AudioSpec spec;
        spec.freq = 44100;
        spec.format = AUDIO_S16SYS;
        spec.channels = 2;
        spec.silence = 0;
        spec.samples = 1024;
        spec.callback = read_audio_data;
        spec.userdata = NULL;
        if(SDL_OpenAudio(&spec, NULL)<0)
        {
            ShowError("can not open audio");
        }

        SDL_PauseAudio(0);

        int64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
        int out_nb_sambles = 1024;
        enum AVSampleFormat sample_fmt = AV_SAMPLE_FMT_S16;
        int out_sample_rate = 44100;
        int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
        int buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_sambles, sample_fmt, 1);
        uint8_t* buffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE*2);
        int64_t in_channel_layout = av_get_default_channel_layout(m_pAudioCodecCtx->channels);

        struct SwrContext *convert_ctx = swr_alloc();
        convert_ctx = swr_alloc_set_opts(convert_ctx, out_channel_layout, sample_fmt, out_sample_rate, in_channel_layout, m_pAudioCodecCtx->sample_fmt, m_pAudioCodecCtx->sample_rate, 0, NULL);
        swr_init(convert_ctx);

         SDL_Thread *refresh_thread = SDL_CreateThread(refresh_video,NULL,NULL);
         SDL_Event event;

         while(1){
             //Wait
             SDL_WaitEvent(&event);
             if(event.type==REFRESH_EVENT){

                 if(av_read_frame(m_pAvFormatContext, m_pPacket)>=0){
                                 if(m_pPacket->stream_index==videoindex){
                                     ret = avcodec_decode_video2(m_pCodecCtx, m_pFrame, &got_picture, m_pPacket);
                                     if(ret < 0){
                                         ShowError("Decode Error");
                                         return;
                                     }
                                     if(got_picture){
                                         sws_scale(m_pSwsContext, (const uint8_t* const*)m_pFrame->data, m_pFrame->linesize, 0, m_pCodecCtx->height, m_pFrameYUV->data, m_pFrameYUV->linesize);
                                         PresentFrame(m_pFrameYUV->data[0], m_pFrameYUV->linesize[0]);
                                     }
                                 }
                                 else if(m_pPacket->stream_index==audioindex)
                                 {
                                     ret = avcodec_decode_audio4(m_pAudioCodecCtx, m_pAudioFrame, &got_picture, m_pPacket);
                                                     if(ret < 0){
                                                         ShowError("Decode Error.\n");
                                                     }

                                                     if(got_picture){
                                                         pq.InQueue(m_pPacket);
                                                         swr_convert(convert_ctx, &buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **)m_pAudioFrame->data, m_pAudioFrame->nb_samples);
                                                         audio_chunk = buffer;
                                                         audio_len = buffer_size;
                                                         audio_pos = audio_chunk;
                                                         while(audio_len > 0)
                                                         {
                                                             SDL_Delay(2);
                                                         }
                                                     }
                                 }
                                 av_free_packet(m_pPacket);
                             }else{
                                 //Exit Thread
                                 thread_exit=1;
                             }

             }else if(event.type==SDL_WINDOWEVENT){
                 //If Resize
                 int width = 0;
                 int height = 0;
                 SDL_GetWindowSize(m_pWindow,&width,&height);
                 if(width != m_screen_width || height != m_screen_height)
                 {
                     m_screen_width = width;
                     m_screen_height = height;

                     sws_freeContext(m_pSwsContext);
                     m_pSwsContext = NULL;

                     m_pSwsContext = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
                         m_screen_width, m_screen_height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
                     av_free(out_buffer);

                     out_buffer =(uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, m_screen_width, m_screen_height));
                    av_frame_free(&m_pFrameYUV);

                    m_pFrameYUV = av_frame_alloc();

                    avpicture_fill((AVPicture *)m_pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, m_screen_width, m_screen_height);

                   SDL_DestroyTexture(m_pTexture);
                   m_pTexture = NULL;
                   m_pTexture = SDL_CreateTexture(m_pRender,pixformat, SDL_TEXTUREACCESS_STREAMING,m_screen_width,m_screen_height);

                 }
             }else if(event.type==SDL_QUIT){
                 thread_exit=1;
             }else if(event.type == SDL_MOUSEBUTTONDOWN)
             {
                 thread_exit = 1;
             }
             else if(event.type==BREAK_EVENT){
                 break;
             }
         }
 }

 void SDLRenderWnd::PresentFrame(const void *pixels, int pitch)
 {
     SDL_UpdateTexture( m_pTexture, NULL, pixels, pitch);

     SDL_RenderClear( m_pRender );
     SDL_RenderCopy( m_pRender, m_pTexture, NULL, NULL);
     SDL_RenderPresent( m_pRender );
 }
 void SDLRenderWnd::Clear()
 {}

void SDLRenderWnd::SDL_Related_Init()
{
    if (0 == m_nRef++)
    {
        SDL_Init(SDL_INIT_VIDEO);
    }
}

void SDLRenderWnd::SDL_Related_Uninit()
{
    if (0 == --m_nRef)
    {
        SDL_Quit();
    }
}

void SDLRenderWnd::resizeEvent(QResizeEvent *event)
{}
