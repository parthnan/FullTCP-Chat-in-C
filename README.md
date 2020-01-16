# TCP Chat System using Socket Programming

Project to make a fully functional TCP Chat server in C, along with the client programs. Uses C sockets, netdb, netinet, arpa/inet libraries. Multiple clients can connect, each with unique usernames decided via prompt at connection time. Each user's message will be displayed along with the Date,Time,IP Address as shown below: 

In the image,  the bottom terminal is the server, the top two are two clients connected though the server. The two clients send each other greetings. 


![alt text](https://raw.githubusercontent.com/parthnan/FullTCP-Chat-in-C/master/images/4-1-3.png)



Users can be forced on a 60 second auto-timeout. The Server allows viewing all connected users and their usernames via the command "/list", and also provides a 10 second warning before shutdown(if requested). Example of Warning is shown below.

![alt text](https://raw.githubusercontent.com/parthnan/FullTCP-Chat-in-C/master/images/4-2-5.png)


Secret Messages between two users supported too as shown below, between users "one" and "two":

![alt text](https://raw.githubusercontent.com/parthnan/FullTCP-Chat-in-C/master/images/4-2-3.png)


