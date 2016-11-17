from concurrent.futures import ThreadPoolExecutor
import socket
import sys


rooms = dict()
room_ids = dict()
client_ids = dict()
client_count = 0
room_count = 0

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(('0.0.0.0', int(sys.argv[1])))

sock.listen(1000)

def announce(room_id, msg):
    for connection in rooms[room_id]:
        connection.sendall(msg)

def handler(connection, address):
    while connection:
        msg = str(connection.recv(10000), 'utf-8');

        if msg.startswith("HELO"):
            response = msg + "\nIP:" + address[0] + "\nPORT:" + address[1] + \
                "\nStudentID:12308914"
            connection.sendall(response)
        if msg.startswith("KILL_SERVICE"):
            kill_serv(connection)


        if msg.startswith("JOIN_CHATROOM"):
            msg  = msg.split("\n")
            room = msg[0].split(":")[1]
            client_name = msg[3].split(":")[1]
            if client_name not in client_ids.keys():
                client_count += 1
                client_ids[client_name] = client_count
            client_id = client_ids[client_name]
            room_id = -1
            if room in rooms:
                room_id = rooms[room]
            else:
                room_count += 1
                room_id = room_count
                rooms[room_ids] = dict()
            if client_id not in rooms[room_ids[room]]:
                rooms[room_id][client_id] = connection
                response = "JOINED_CHATROOM:" + room + "\nSERVER_IP:10.62.0.156" +\
                    "\nPORT:8000\nROOM_REF:" + room_id + "JOIN_ID:\n" +\
                    client_id
                connection.sendall(response)
                joined = "CHAT:" + room_id + "\nCLIENT_NAME:" + client +\
                    "\nMESSAGE:" + client_name + "has joined the chat room.\n\n"
                announce(room_id, joined)


        if msg.startswith("LEAVE_CHATROOM"):
            msg  = msg.split("\n")
            room_id = int(msg[0].split(":")[1])
            client_id = int(msg[2].split(":")[1])
            client_name = msg[3].split(":")[1]
            left = "LEFT_CHATROOM:" + room_id + "\nJOIN_ID:" + client_id + "\n"
            connection.sendall(left)
            left = "CHAT:" + room_id + "\nCLIENT_NAME:" + client_name +\
                "\nMESSAGE:" + client_name + "has left the chat room.\n\n"
            announce(room_id, left)
            del rooms[room_id][client_id]


        if msg.startswith("DISCONNECT"):
            msg  = msg.split("\n")
            client_name = msg[2].split(":")[1]
            client_id = client_ids[client_name]
            for room_id in rooms.keys():
                del rooms[room_id][client_id]
                if client_id in rooms[room_id]:
                    left = "CHAT:" + room_id + "\nCLIENT_NAME:" + client_name +\
                        "\nMESSAGE:" + client_name + "has left the chat room.\n\n"
            connection = False


        if msg.startswith("CHAT"):
            msg  = msg.split("\n")
            room_id = int(msg[0].split(":")[1])
            client_id = int(msg[1].split(":")[1])
            client_name = msg[2].split(":")[1]
            msg = msg[3].split(":")[1]
            msg = "CHAT:" + room_id + "\nCLIENT_NAME:" +\
                client_name + "\nMESSAGE:" + msg
            announce(room_id, msg)



executor = ThreadPoolExecutor(max_workers=1000)
while True:
    connection, address = sock.accept()
    executor.submit(handler, connection, address)
