#!/usr/bin/env python

import time
import requests
import speech_recognition as sr

url = 'http://zynq-robot:1999/zynq_ps::command_processor::arg_7_seq_i'

# this is called from the background thread
def callback(recognizer, audio):
    # received audio data, now we'll recognize it using Google Speech Recognition
    try:
        rec_res = recognizer.recognize_google(audio)
        robot_res = requests.post(url, data=rec_res)
        print("you said \"" + rec_res + "\", robot said \"" + robot_res.text + "\"")
    except sr.UnknownValueError:
        print("could not understand audio")
    except sr.RequestError as e:
        print("network error; {0}".format(e))


r = sr.Recognizer()
m = sr.Microphone()
with m as source:
    r.adjust_for_ambient_noise(source)  # we only need to calibrate once, before we start listening

# start listening in the background (note that we don't have to do this inside a `with` statement)
stop_listening = r.listen_in_background(m, callback)
# `stop_listening` is now a function that, when called, stops background listening

# do some other computation for 5 seconds, then stop listening and keep doing other computations
# for _ in range(50): time.sleep(0.1)  # we're still listening even though the main thread is doing other things
# stop_listening()  # calling this function requests that the background listener stop listening
while True: time.sleep(0.1)
