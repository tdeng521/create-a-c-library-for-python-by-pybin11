#ifndef _PARSE_VIDEO_H_
#define _PARSE_VIDEO_H_

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include <string>
using namespace std;

struct InputProp{
    string input_file;
    int mode;                //1只有视频，2只有音频,3同时有视频和音频
    string video_codec_type;
    int video_bitrate;
    int video_width;
    int video_height;
    double fps;
    string audio_codec_type;
    int audio_sample_rate;
    int audio_bitrate;
    int audio_channels;
    int duration;  //时长    
    int nframes;   //总帧数
    void print_str();
};


int parse_video_info(char* filename, InputProp* ip);

//inux 系统在使用 CMake 编译 c++ 动态链接库的时候, 会加一个前缀lib, libvideoparse.so, 
PYBIND11_MODULE(libvideoparse, m){
    m.doc() = "get the video information,like ffprobe";
    py::class_<InputProp>(m,"InputProp")
        .def(py::init())
        .def_readwrite("input_file",&InputProp::input_file)
        .def_readwrite("mode",&InputProp::mode)
        .def_readwrite("video_codec_type",&InputProp::video_codec_type)
        .def_readwrite("video_bitrate",&InputProp::video_bitrate)
        .def_readwrite("video_width",&InputProp::video_width)
        .def_readwrite("video_height",&InputProp::video_height)
        .def_readwrite("fps",&InputProp::fps)
        .def_readwrite("audio_codec_type",&InputProp::audio_codec_type)
        .def_readwrite("audio_sample_rate",&InputProp::audio_sample_rate)
        .def_readwrite("audio_bitrate",&InputProp::audio_bitrate)
        .def_readwrite("audio_channels",&InputProp::audio_channels)
        .def_readwrite("duration",&InputProp::duration)
        .def_readwrite("nframes",&InputProp::nframes)
        .def("print_str",&InputProp::print_str);
    m.def("parse_video_info",&parse_video_info);
}


#endif