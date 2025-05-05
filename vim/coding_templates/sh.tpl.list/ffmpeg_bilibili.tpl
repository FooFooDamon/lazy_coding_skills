#!/bin/bash

# Cache path: Android/data/tv.danmaku.bili/download

set -xe

codec_args="-c:v hevc -c:a aac"

#cache_dir=64
cache_dir=c_1558861542/80

video_args=$(ffprobe -i ${cache_dir}/video.m4s 2>&1 | grep -i "Stream .\+: Video:" | sed 's/^.\+[, ]\([0-9]\+\) fps.\+[, ]\([0-9km]\+\) tbn.*$/-r \1 -video_track_timescale \2/')

audio_args=$(ffprobe -i ${cache_dir}/audio.m4s 2>&1 | grep -i "Stream .\+: Audio:" | sed 's/^.\+[, ]\([0-9km]\+\) Hz.*$/-ar \1/')

bitstream_filters="-bsf:v hevc_mp4toannexb -bsf:a aac_adtstoasc"

op_type=5

if [ ${op_type} -eq 1 ]; then # All-in-one video
    time ffmpeg -i ${cache_dir}/video.m4s -i ${cache_dir}/audio.m4s -c copy Superhero_Movie（蜻蜓侠）.mp4
elif [ ${op_type} -eq 2 ]; then # All-in-one video with extra contents at the end
    time ffmpeg -i ${cache_dir}/video.m4s -i ${cache_dir}/audio.m4s ${codec_args} -ss "00:00:00" -to "01:15:00" ${video_args} ${audio_args} ${bitstream_filters} Superhero_Movie（蜻蜓侠）.mp4
elif [ ${op_type} -eq 3 ]; then # Episodes
    start_num=5423
    end_num=5551
    dir_prefix=14
    #episode=0

    time for i in $(seq ${start_num} ${end_num}) # 9527 9531 9600 ...
    do
        cache_dir=${dir_prefix}${i}/64
        #cache_dir=${i}/64
        episode=$(printf "%04d\n" $((${i} - 5053 + 1)))
        #episode=$(printf "%04d\n" $((${episode} + 1)))

        echo "${dir_prefix}${i} -> ${episode}"

        time ffmpeg -i ${cache_dir}/video.m4s -i ${cache_dir}/audio.m4s -c copy ${episode}.mp4
    done
else
    # NOTE: Executing a complex ffmpeg command in a for-loop or while-loop might cause some weird bug, like:
    #       breaking out unexpectedly in the middle of loop, or reporting the invalid-argument error.
    #time ffmpeg -i c_1558861542/80/video.m4s -i c_1558861542/80/audio.m4s ${codec_args} -ss "00:00:00" -to "00:11:09" ${video_args} ${audio_args} 01.mp4
    time ffmpeg -i c_1558861542/80/video.m4s -i c_1558861542/80/audio.m4s ${codec_args} -ss "00:00:00" -to "00:11:09" ${video_args} ${audio_args} ${bitstream_filters} 01.ts
    time ffmpeg -i c_1558873445/80/video.m4s -i c_1558873445/80/audio.m4s ${codec_args} -ss "00:00:00" -to "00:11:09" ${video_args} ${audio_args} ${bitstream_filters} 02.ts
    time ffmpeg -i c_1558873600/80/video.m4s -i c_1558873600/80/audio.m4s ${codec_args} -ss "00:00:00" -to "00:11:09" ${video_args} ${audio_args} ${bitstream_filters} 03.ts
    time ffmpeg -i c_1558873658/80/video.m4s -i c_1558873658/80/audio.m4s ${codec_args} -ss "00:00:00" -to "00:11:09" ${video_args} ${audio_args} ${bitstream_filters} 04.ts
    time ffmpeg -i c_1558873842/80/video.m4s -i c_1558873842/80/audio.m4s ${codec_args} -ss "00:00:00" -to "00:11:09" ${video_args} ${audio_args} ${bitstream_filters} 05.ts
    time ffmpeg -i c_1558878827/80/video.m4s -i c_1558878827/80/audio.m4s ${codec_args} -ss "00:00:00" -to "00:11:09" ${video_args} ${audio_args} ${bitstream_filters} 06.ts
    time ffmpeg -i c_1558881233/80/video.m4s -i c_1558881233/80/audio.m4s ${codec_args} -ss "00:00:00" -to "00:11:09" ${video_args} ${audio_args} ${bitstream_filters} 07.ts
    time ffmpeg -i c_1558884963/80/video.m4s -i c_1558884963/80/audio.m4s ${codec_args} -ss "00:00:00" -to "00:11:09" ${video_args} ${audio_args} ${bitstream_filters} 08.ts
    time ffmpeg -i c_1558890739/80/video.m4s -i c_1558890739/80/audio.m4s ${codec_args} -ss "00:00:00" -to "00:11:09" ${video_args} ${audio_args} ${bitstream_filters} 09.ts
    time ffmpeg -i c_1558890764/80/video.m4s -i c_1558890764/80/audio.m4s ${codec_args} -ss "00:00:00" -to "00:07:26" ${video_args} ${audio_args} ${bitstream_filters} 10.ts

    if [ ${op_type} -gt 4 ]; then
        #ls *.mp4 | awk '{ printf("file '%s'\n", $1); }' > slices.txt
        ls *.ts | awk '{ printf("file '%s'\n", $1); }' > slices.txt

        time ffmpeg -f concat -i slices.txt -c copy 白面包青天完整版.mp4 && rm slices.txt || :
    fi
fi
