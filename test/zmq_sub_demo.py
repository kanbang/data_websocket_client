import zmq
import time
 

context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.connect("tcp://localhost:5551")
socket.setsockopt_string(zmq.SUBSCRIBE,'')
while True:
    response = socket.recv_string()
    print(response)