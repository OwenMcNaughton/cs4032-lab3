import collections
from hashlib import md5
import os
import Queue
import socket
import sys
import threading


rooms = collections.OrderedDict()
client_ids = dict()
client_count = 0
room_count = 0

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('0.0.0.0', int(sys.argv[1])))

sock.listen(1000)

class ThreadHandler(threading.Thread):
    def __init__(self, thread_queue):
        threading.Thread.__init__(self)
        self.queue = thread_queue

    def run(self):
        # Thread loops and waits for connections to be added to the queue
        while True:
            conaddr = self.queue.get()
            handler(conaddr)
            self.queue.task_done()

def announce(room_id, msg):
    for client_id, connection in rooms[room_id].iteritems():
        connection.sendall(msg)

def handler((connection, address)):
    while connection:
        msg = connection.recv(10000).decode('utf-8')
        print("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$")
        if 80806323619409305465361856931992828404 in rooms:
            print(rooms[80806323619409305465361856931992828404].keys())
        if 231755500800015387352945186706554456156 in rooms:
            print(rooms[231755500800015387352945186706554456156].keys())

        if msg.startswith("HELO"):
            response = msg + "\nIP:" + str(address[0]) + "\nPORT:" + str(address[1]) + \
                "\nStudentID:12308914"
            connection.sendall(response)


        elif msg.startswith("KILL_SERVICE"):
            os._exit(1)


        elif msg.startswith("JOIN_CHATROOM"):
            msg  = msg.split("\n")
            room = msg[0].split(":")[1]
            client_name = msg[3].split(":")[1]
            room_id = int(md5(room).hexdigest(), 16)
            client_id = int(md5(client_name).hexdigest(), 16)
            if room_id not in rooms:
                rooms[room_id] = dict()
            if client_id not in rooms[room_id]:
                print(str(client_id) + "  join  " + str(room_id))
                rooms[room_id][client_id] = connection
                response = "JOINED_CHATROOM:" + str(room) + "\nSERVER_IP:10.62.0.156" +\
                    "\nPORT:8000\nROOM_REF:" + str(room_id) + "\nJOIN_ID:" +\
                    str(client_id) + "\n"
                connection.sendall(response)
                joined = "CHAT:" + str(room_id) + "\nCLIENT_NAME:" + str(client_name) +\
                    "\nMESSAGE:" + str(client_name) + " has joined this chatroom.\n\n"
                announce(room_id, joined)


        elif msg.startswith("LEAVE_CHATROOM"):
            msg  = msg.split("\n")
            room_id = int(msg[0].split(":")[1])
            client_id = int(msg[1].split(":")[1])
            client_name = msg[2].split(":")[1]
            print(str(client_id) + "  leave  " + str(room_id))
            left = "LEFT_CHATROOM:" + str(room_id) + "\nJOIN_ID:" + str(client_id) + "\n"
            connection.sendall(left)
            left = "CHAT:" + str(room_id) + "\nCLIENT_NAME:" + str(client_name) +\
                "\nMESSAGE:" + str(client_name) + " has left this chatroom.\n\n"
            announce(room_id, left)
            del rooms[room_id][client_id]


        elif msg.startswith("DISCONNECT"):
            msg  = msg.split("\n")
            client_name = msg[2].split(":")[1]
            client_id = int(md5(client_name).hexdigest(), 16)
            print("DISC  " + str(client_id))
            for room_id in rooms.keys():
                if client_id in rooms[room_id]:
                    left = "CHAT:" + str(room_id) + "\nCLIENT_NAME:" + str(client_name) +\
                        "\nMESSAGE:" + str(client_name) + " has left this chatroom.\n\n"
                    print("LEAVE " + str(room_id))
                    announce(room_id, left)
                    if client_id in rooms[room_id]:
                        del rooms[room_id][client_id]
            connection = False


        elif msg.startswith("CHAT"):
            msg  = msg.split("\n")
            room_id = int(msg[0].split(":")[1])
            client_id = int(msg[1].split(":")[1])
            client_name = msg[2].split(":")[1]
            msg = msg[3].split(":")[1]
            msg = "CHAT:" + str(room_id) + "\nCLIENT_NAME:" +\
                str(client_name) + "\nMESSAGE:" + msg + "\n\n"
            announce(room_id, msg)

queue = Queue.Queue(maxsize=1000)

for i in range(1000):
    thread = ThreadHandler(queue)
    thread.setDaemon(True)
    thread.start()

while True:
    connection, address = sock.accept()
    queue.put((connection, address))
