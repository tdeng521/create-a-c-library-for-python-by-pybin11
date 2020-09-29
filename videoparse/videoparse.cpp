#include <stdlib.h>
#include <stdio.h>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avstring.h>
#include <libavutil/opt.h>
}
#include "videoparse.h"

static void print_fps(double d, const char *postfix)
{
    uint64_t v = lrintf(d * 100);
    if (!v)
        av_log(NULL, AV_LOG_INFO, "%1.4f %s", d, postfix);
    else if (v % 100)
        av_log(NULL, AV_LOG_INFO, "%3.2f %s", d, postfix);
    else if (v % (100 * 1000))
        av_log(NULL, AV_LOG_INFO, "%1.0f %s", d, postfix);
    else
        av_log(NULL, AV_LOG_INFO, "%1.0fk %s", d / 1000, postfix);
}

void get_video_info(AVFormatContext *ic, int index,
                    const char *url, InputProp* ip){
    int i;
    uint8_t *printed = ic->nb_streams ? (uint8_t*)av_mallocz(ic->nb_streams) : NULL;
    if (ic->nb_streams && !printed)
        return;
    
    ip->input_file = string(url);
    if (ic->duration != AV_NOPTS_VALUE) {
        int hours, mins, secs, us;
        int64_t duration = ic->duration + (ic->duration <= INT64_MAX - 5000 ? 5000 : 0);
        secs  = duration / AV_TIME_BASE;
        us    = duration % AV_TIME_BASE;
        ip->duration = secs; //保留到秒
        mins  = secs / 60;
        secs %= 60;
        hours = mins / 60;
        mins %= 60;
        av_log(NULL, AV_LOG_INFO, "%02d:%02d:%02d.%02d", hours, mins, secs,
                (100 * us) / AV_TIME_BASE);
    } else {
        av_log(NULL, AV_LOG_INFO, "N/A");
    }
    
    for(i = 0; i < ic->nb_streams; i++){
        if(!printed[i]){
            char buf[256];
            int flags = ic->iformat->flags;
            AVStream *st = ic->streams[i];
            AVDictionaryEntry *lang = av_dict_get(st->metadata, "language", NULL, 0);
            char *separator = (char*)ic->dump_separator;
            AVCodecContext *avctx;
            int ret;

            avctx = avcodec_alloc_context3(NULL);
            if (!avctx)
                return;

            ret = avcodec_parameters_to_context(avctx, st->codecpar);
            if (ret < 0) {
                avcodec_free_context(&avctx);
                return;
            }

            // Fields which are missing from AVCodecParameters need to be taken from the AVCodecContext
            avctx->properties = st->codec->properties;
            avctx->codec      = st->codec->codec;
            avctx->qmin       = st->codec->qmin;
            avctx->qmax       = st->codec->qmax;
            avctx->coded_width  = st->codec->coded_width;
            avctx->coded_height = st->codec->coded_height;

            if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                ip->video_width = st->codec->coded_width;
                ip->video_height = st->codec->coded_height;
                ip->video_bitrate = st->codec->bit_rate;
                ip->nframes = st->nb_frames;

                int fps = st->avg_frame_rate.den && st->avg_frame_rate.num;
                int tbr = st->r_frame_rate.den && st->r_frame_rate.num;
                int tbn = st->time_base.den && st->time_base.num;
                int tbc = st->codec->time_base.den && st->codec->time_base.num;
                if(fps){
                    ip->fps = av_q2d(st->avg_frame_rate);
                }
                // if (fps || tbr || tbn || tbc)
                //     av_log(NULL, AV_LOG_INFO, "%s", separator);

                // if (fps)
                //     print_fps(av_q2d(st->avg_frame_rate), tbr || tbn || tbc ? "fps, " : "fps");
                // if (tbr)
                //     print_fps(av_q2d(st->r_frame_rate), tbn || tbc ? "tbr, " : "tbr");
                // if (tbn)
                //     print_fps(1 / av_q2d(st->time_base), tbc ? "tbn, " : "tbn");
                // if (tbc)
                //     print_fps(1 / av_q2d(st->codec->time_base), "tbc");

                avctx = st->codec;
                // 寻找视频解码器
                AVCodec *codec = avcodec_find_decoder(avctx->codec_id);
                if(codec == NULL)
                    break;
    
                ip->video_codec_type = string(codec->long_name); //视频编码器名称

            }
            else if (st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                /* code */
                ip->audio_channels = st->codec->channels;
                ip->audio_bitrate = st->codec->bit_rate;
                ip->audio_sample_rate = st->codec->sample_rate;
                avctx = st->codec;
                // 寻找视频解码器
                AVCodec *codec = avcodec_find_decoder(avctx->codec_id);
                if(codec == NULL)
                    break;
    
                ip->audio_codec_type = string(codec->long_name); //视频编码器名称
            }
            
        }
    }
    av_free(printed);
}

int parse_video_info(char* filename, InputProp* ip){
    av_register_all();
    AVFormatContext * ifmt_ctx = NULL;
    int ret = 0;
    if((ret = avformat_open_input(&ifmt_ctx,filename,NULL,NULL)) < 0){
        fprintf(stderr,"could not open input file '%s' ",filename);
        return -1;
    }
    if((ret = avformat_find_stream_info(ifmt_ctx,0))<0){
        fprintf(stderr,"Failed to retrieve input stream context \n");
        return -2;
    }
    get_video_info(ifmt_ctx, 0, filename,ip);
    avformat_free_context(ifmt_ctx);
    return ret;
}

void InputProp::print_str(){
    printf("filename: %s \n",input_file.c_str());
    printf("video codec: %s \n",video_codec_type.c_str());
    printf("audio codec: %s \n",audio_codec_type.c_str());
    printf("video resolution: %d x %d, fps: %f, video bitrate: %d kbps\n",video_width,video_height,fps,video_bitrate/1000);
    printf("audio sample rate: %d Hz, bitrate:%d kbps \n",audio_sample_rate,audio_bitrate/1000);
    printf("video frame count: %d, duration: %d s \n",nframes,duration);
}
