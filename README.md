# Movie Intro

Based on [henryleberre/Matrixify](github.com/henryleberre/Matrixify).

Here's a useful command (Generates a 1280x1080 x264 mp4 @ 30 fps. Perfect for Discord (<8MB & everything supports x264)):

```
ffmpeg -y -framerate 60 -i temp/output_frames/frame_%d.png -vcodec libx264 -filter:v "fps=30, scale=1280:-1" output.mp4
```