Ryan McCrory
ryanamccrory@gmail.com
To build my program, enter "make" in the command line. This will build the
executable "httpserver". Enter "./httpserver", followed by the IP address of the HTTP
server to contact, and then a port number above 1024.
Then, in another terminal, enter a get or put request using curl. For example, for get, enter 
curl -s http://127.0.0.1:8888/ --request-target ABCDEFarqdeXYZxyzf012345-ab, and for put, enter 
curl -s -T my_local_file http://127.0.0.1:8888/ --request-target ABCDEFarqdeXYZxyzf012345-ab. 
The request target must be a 27 ascii characters. This will run the program.
