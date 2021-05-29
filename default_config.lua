-- Port to expose web page and web socket on
port = 57778

-- The pipelines to run. The end of each pipeline must be a compatible input to webrtcbin. All the pipelines are passed to webrtcbin as separate input streams. The combination of pipelines must also be compatible with the webrtcbin inputs
pipelines = {
        "videotestsrc pattern=ball is-live=true ! x264enc pass=5 quantizer=25 speed-preset=6 ! video/x-h264,width=1920,height=1080,profile=baseline ! h264parse ! rtph264pay config-interval=-1 name=payloader aggregate-mode=zero-latency ! application/x-rtp,media=video,encoding-name=H264,payload=96"
}
