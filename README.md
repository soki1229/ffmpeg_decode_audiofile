# ffmpeg_decode_audiofile
Decoding+resampling mp3 file within FFmpeg API, via C++

__Sequence:

  1. decode input audio file(.mp3)

  2. write decoded pcm data as .pcm file

  _//3. resample pcm data for proper usage_

  4. convert .pcm file to .wav file by attaching WAV header ahead.
