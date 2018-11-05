#Socket info taken from: https://docs.python.org/2/library/socket.html

import socket
import struct
import sys

#Possibly overlycomplicated method for taking IP address in hex and printing it as dotted decimal
def hexToDecimal(nextSlaveIP):
    #Have an issue where the nextSlaveIP is dropping the leading zero, so this solves that
    if len(str(nextSlaveIP)) == 9:
        dotOne = "0x0" + str(nextSlaveIP[2:3])
        dotOne = int(dotOne, 16)

        dotTwo = "0x" + str(nextSlaveIP[3:5])
        dotTwo = int(dotTwo, 16)

        dotThree = "0x" + str(nextSlaveIP[5:7])
        dotThree = int(dotThree, 16)

        dotFour = "0x" + str(nextSlaveIP[7:9])
        dotFour = int(dotFour, 16)

    else:
        dotOne = "0x" + str(nextSlaveIP[2:4])
        dotOne = int(dotOne, 16)

        dotTwo = "0x" + str(nextSlaveIP[4:6])
        dotTwo = int(dotTwo, 16)

        dotThree = "0x" + str(nextSlaveIP[6:8])
        dotThree = int(dotThree, 16)

        dotFour = "0x" + str(nextSlaveIP[8:])
        dotFour = int(dotFour, 16)

    print(str(dotFour) + "." + str(dotThree) + "." + str(dotTwo) + "." + str(dotOne))

def main():
    slave_gid = 30 #Don't have a group number, so have just been using 30 for testing
    slave_magic = 0x4A6F7921
    slave_magic_hex = hex(slave_magic)
    status = 1

    #Error for incorrect command line argument
    if (len(sys.argv) != 3):
        print("Command line argument: slave masterHostname masterPortNumber")
        sys.exit(1)

    #Set hostname and portnumber from arguments
    hostname = sys.argv[1]
    portNumber = int(sys.argv[2])

    while(status != 0):
        #Connect to the master
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((hostname, portNumber))

        #Print the join request
        print("\n----------------------------------------------------");
        print("Slave Join Request:")
        print("Magic Number: " + str(slave_magic_hex))
        print("GID: " + str(slave_gid) + "\n")

        #Form the request and send it to the master
        req = struct.pack('>bl', slave_gid, slave_magic)
        print("Sending the request to the master")
        sock.send(req)

        # Receive the response from the master
        print("\nWaiting for repsonse from the master\n")
        res = sock.recv(4096)

        #Unpacks C structs to python string
        resu = struct.unpack('<blbl', res)

        #Create variables and fill them with the corresponding info from the unpacked string
        master_gid = resu[0]
        master_magic = hex(resu[1])
        rid = resu[2]
        nextSlaveIP = hex(resu[3])

        #Print the message from the master
        print("Master Reponse:")
        print("Master GID: " + str(master_gid))
        print("Master Magic Number: " + str(master_magic))
        print("My RID: " + str(rid))
        sys.stdout.write("Next Slave IP: ")
        hexToDecimal(nextSlaveIP)
        print("----------------------------------------------------\n");
        status = input("Slave successfully added.\nPhase II functionality not added yet. For now, input 0 to exit: ")
    #End of while

    print("\n")
    sock.close()

if __name__ == "__main__":
    main()