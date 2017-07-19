#!/usr/bin/env python

import time
import requests
import signal
import snowboydecoder

url = 'http://zynq-robot:1999/zynq_ps::command_processor::arg_7_seq_i'

interrupted = False


def signal_handler(signal, frame):
    global interrupted
    interrupted = True

def interrupt_callback():
    global interrupted
    return interrupted

signal.signal(signal.SIGINT, signal_handler)

# this is called from the background thread
def send_cmd(cmd):
    snowboydecoder.play_audio_file(snowboydecoder.DETECT_DONG)
    robot_res = requests.post(url, data=cmd)
    print("you said \"" + cmd + "\", robot responded \"" + robot_res.text + "\"")


models = ["memorize.pmdl", "record.pmdl", "vsi.pmdl", "replay.pmdl"]
sensitivity = [0.5]*len(models)

callbacks = [lambda: send_cmd("memorize"),
             lambda: snowboydecoder.play_audio_file(snowboydecoder.DETECT_DING),
             lambda: snowboydecoder.play_audio_file(snowboydecoder.DETECT_DONG),
             lambda: send_cmd("replay")]

detector = snowboydecoder.HotwordDetector(models, sensitivity=sensitivity)
print('Yes? Listening... Press Ctrl+C to exit')
detector.start(detected_callback=callbacks,
               interrupt_check=interrupt_callback,
               sleep_time=0.03)

detector.terminate()
