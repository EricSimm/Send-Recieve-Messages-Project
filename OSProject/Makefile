CC = g++

#typing make == typing make all in the terminal
default: all

#Call sender and reciever
all: sender recv

#Call sender object and produce sender
sender: sender.o
	$(CC) sender.o -o sender

#Call reciever object and produce reciever
recv: recv.o
	$(CC) recv.o -o recv
	

#Receiver object
recv.o: recv.cpp
	$(CC) -c recv.cpp

#Sender object
sender.o: sender.cpp
	$(CC) -c sender.cpp 

clean:
	rm -rf *o all
