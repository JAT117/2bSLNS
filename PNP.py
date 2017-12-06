import os
import subprocess
import time

while True:
  s = subprocess.call(["arp", "-n"])
   time.sleep(1)
   t = subprocess.call(["clear"])
