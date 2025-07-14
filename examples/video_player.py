#!/usr/bin/env python
from pydbus import SessionBus
import io
from PIL import Image

import os
import shutil

from yt_dlp import YoutubeDL
from pathlib import Path
import ffmpeg

import time
# Connect to the session bus
bus = SessionBus()
interface_name = "ca.applism.miradock"
object_path = "/ca/applism/MiraDock"
fps = 60
url = 'https://youtu.be/FtutLA63Cp8'
ctx = {
    "outtmpl": "./video.%(ext)s",
    'force_overwrites': True,
    'nooverwrites': False,
}
buffer = io.BytesIO()
with  YoutubeDL(ctx) as ydl:
    info = ydl.extract_info(url, download=True)
    print(ydl.prepare_filename(info))
    stream = ffmpeg.input(ydl.prepare_filename(info))
    fps = info.get('fps')
    try:
        shutil.rmtree('./apple')
    except:
        print("eh should be ok")
    try:
        os.mkdir('./apple')
    except:
        print("probably fine")
    stream = ffmpeg.filter(stream, 'scale', height='100', width='-2') # 100 is the highest, 80 for 16:9 video doesn't cut off
    stream = ffmpeg.output(stream, 'apple/test%d.jpg')
    ffmpeg.run(stream)



frame_count = 0
for root, dirs, files in os.walk('./apple'):
    frame_count = len(files)
dock = bus.get(interface_name, object_path)

spf = 1 / fps
end = 0
start = time.time()
for i in range(frame_count):
    if (i * spf) - end < 0: # negative, we're behind
        continue
    elif (i * spf) - end >= spf: # if we're ahead by at least a frame... somehow
        time.sleep((i* spf) - end)
        print("huh???", i, (i * spf) - end)
    file_name = "apple/test" + str(i+1) + ".jpg"
    im = Image.open(file_name).convert('RGB').transpose(method=Image.Transpose.FLIP_LEFT_RIGHT).transpose(method=Image.Transpose.FLIP_TOP_BOTTOM).transpose(method=Image.Transpose.ROTATE_90) # mirroring (rotating gets higher resolution)
    # also converting to 'RGB' above makes sure the format is right and that we remove the alpha channel
    b = io.BytesIO()
    im.save(b, "jpeg", progressive=False)
    dock.SetCellBackground(8 + 1, b.getvalue())
    dock.Refresh() # dont forget to tell the Stream Dock to update the screen!
    end = time.time() - start

print("done!")

