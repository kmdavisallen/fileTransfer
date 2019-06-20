"""
 Author: Kevin Allen
 Program: ftclient
 Description: Implements the client side of the file transfer program, much of the 
    socket setup taken from class notes
 CS372 Project 2
 Last modified 11/23/18
"""
from socket import*
import sys
import os.path


"""
functions sets up a server side socket 
pre-conditions: takes a user supplied port number as a string
post-condintions: returns a server socket 
"""
def dataConSetup(str):
    serverPort = int(float(str)) #convert from string to an integer
    serverSocket = socket(AF_INET, SOCK_STREAM)
    serverSocket.bind(('', serverPort))
    serverSocket.listen(1)
    return serverSocket

"""
fucntion validates arguments and build the request string
pre-conditions:none
post-conditions:  arguments are validated and function returns a string containing the commands for the server
"""
def buildRequest():
    if len(sys.argv) == 5:
        if sys.argv[3] != '-l':
            print("Incorrect command please try again") 
            sys.exit(1)
        elif int(float(sys.argv[4])) > 65535 or int(float(sys.argv[4])) < 49152:
            print("please chose a transfer port betweeen 49152 and 65535")
            sys.exit(1)
        else:
            commandString = sys.argv[3] + ' ' + sys.argv[4]
            return commandString
    elif len(sys.argv) == 6:
        if sys.argv[3] != '-g':
            print("Incorect command, please try again")
            sys.exit(1)
        elif int(float(sys.argv[5])) > 65535 or int(float(sys.argv[5])) < 49152:
            print("please chose a transfer port betweeen 49152 and 65535")
            sys.exit(1)
        else:
            commandString = sys.argv[3] + ' ' + sys.argv[4] + ' ' + sys.argv[5]
            return commandString
    else:
        print("Incorrect number of arguments")
        sys.exit(1)

"""
function makes contact with the server and sends the commands
pre-conditions:  commands have been validated and concatinated into a string
post-conditions: server has recieved the command string
"""
def initContact(str):
    clientSocket.connect((sys.argv[1], int(float(sys.argv[2]))))
    bytesSent = clientSocket.send(str.encode())
    if  bytesSent < len(str):
        print("Error connecting to server, please try again")
        sys.exit(1)

"""
function setups up server side socket and revieves data from the "client" side
pre-condtions: command line parameters are validated
post-conditons: client recieves either directory listing or requested file or error from file errors
"""
def recieveData():
    if len(sys.argv) == 5:
        serveSock = dataConSetup(sys.argv[4])
        dataSock, addr = serveSock.accept()
        print("recieving directory listing")
        message = dataSock.recv(1024).decode()
        while len(message) > 0: #display list listing
            print(message)
            message = dataSock.recv(1024).decode()
    else:
        serveSock = dataConSetup(sys.argv[5])
        dataSock, addr = serveSock.accept()
        msg = clientSocket.recv(1024).decode()
        if msg == "file not found":
            sys.exit(1)
        if os.path.isfile(sys.argv[4]):          # checking for duplicate files, adapted from https://stackoverflow.com/questions/29295654/what-does-python3-open-x-mode-do
            print("duplicate file name detected, exiting")
            sys.exit(1)
        else:
            file = open(sys.argv[4], 'x')
            message = dataSock.recv(1024).decode()
            file.write(message)
            print("transfer complete")
            file.close()

clientSocket = socket(AF_INET, SOCK_STREAM)
initContact(buildRequest())
recieveData()
clientSocket.close()

