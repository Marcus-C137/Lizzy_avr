from os import PRIO_PGRP, error, wait
from os.path import exists
import socket
import time

print("running script")

hex_file = open(".pio/build/curiosity_nano_4809/firmware.hex")
data = hex_file.read()
hex_file.close()
s = socket.socket()
s.connect(("192.168.1.14", 8888))
message = "SEND " + data
print("SENDING");
s.send(message.encode())
    
buffer = ''      
while True:
    data = s.recv(1024).decode('utf-8')
    if data:
        buffer += data
    else:
        break
print(buffer) 
s.close()
time.sleep(2)

# s = socket.socket()
# s.connect(("192.168.1.14", 8888))
# message = "FUSESET " + "02 01"
# try:
#     s.send(message.encode())
#     print("sent message: " + message)
# except err:
#     print(err)

# buffer = '' 
# while True:
#     data = s.recv(1024).decode('utf-8')
#     if data:
#         buffer += data
#     else:
#         break
# print(buffer)
# s.close()
# time.sleep(2)

s = socket.socket()
s.connect(("192.168.1.14", 8888))
message = "WRITE"
try:
    s.send(message.encode())
    print("sent message: " + message)
except err:
    print(err)

buffer = '' 
while True:
    data = s.recv(1024).decode('utf-8')
    if data:
        buffer += data
    else:
        break
print(buffer)
