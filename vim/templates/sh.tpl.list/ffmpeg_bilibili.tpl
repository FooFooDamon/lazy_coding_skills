#!/bin/bash

shopt -s expand_aliases

alias ffmpeg='ffmpeg -nostdin'

# Cache path: Android/data/tv.danmaku.bili/download

codec_args="-c:v hevc -c:a aac"

cache_dir=$(ls -d ./c_1558861542/* | grep '.\+/[0-9]\+$')

video_args=$(ffprobe -i ${cache_dir}/video.m4s 2>&1 | grep -i "Stream .\+: Video:" | sed 's/^.\+[, ]\([0-9]\+\) fps.\+[, ]\([0-9km]\+\) tbn.*$/-r \1 -video_track_timescale \2/')

audio_args=$(ffprobe -i ${cache_dir}/audio.m4s 2>&1 | grep -i "Stream .\+: Audio:" | sed 's/^.\+[, ]\([0-9km]\+\) Hz.*$/-ar \1/')

bitstream_filters="-bsf:v hevc_mp4toannexb -bsf:a aac_adtstoasc"

set -xe

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
        cache_dir=$(ls -d ${dir_prefix}${i}/* | grep '.\+/[0-9]\+$')
        #cache_dir=$(ls -d ${i}/* | grep '.\+/[0-9]\+$')
        episode=$(printf "%04d\n" $((${i} - 5053 + 1)))
        #episode=$(printf "%04d\n" $((${episode} + 1)))

        echo "${dir_prefix}${i} -> ${episode}"

        time ffmpeg -i ${cache_dir}/video.m4s -i ${cache_dir}/audio.m4s -c copy ${episode}.mp4
    done
else
    slice_dirs=(c_1558861542 c_1558873445 c_1558873600 c_1558873658 c_1558873842 c_1558878827 c_1558881233 c_1558884963 c_1558890739 c_1558890764)
    weird_num=80
    end_duration="00:11:09"

    for i in $(seq 0 $((${#slice_dirs[@]} - 1)))
    do
        slice_dir=${slice_dirs[${i}]}/${weird_num}
        ii=$(printf "%02d\n" ${i})

        #time ffmpeg -i ${slice_dir}/video.m4s -i ${slice_dir}/audio.m4s ${codec_args} -ss "00:00:00" -to "${end_duration}" ${video_args} ${audio_args} ${ii}.mp4
        time ffmpeg -i ${slice_dir}/video.m4s -i ${slice_dir}/audio.m4s ${codec_args} -ss "00:00:00" -to "${end_duration}" ${video_args} ${audio_args} ${bitstream_filters} ${ii}.ts
    done

    if [ ${op_type} -gt 4 ]; then
        #ls *.mp4 | awk '{ printf("file '%s'\n", $1); }' > slices.txt
        ls *.ts | awk '{ printf("file '%s'\n", $1); }' > slices.txt

        time ffmpeg -f concat -i slices.txt -c copy 白面包青天完整版.mp4 && rm slices.txt || :
    fi
fi
