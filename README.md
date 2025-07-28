main_unpack.sh can be renamed to unpack.sh and placed in the /home/dan/dev/tcp_iot directory of 243
server_unpack.sh can be renamed to unpack.sh and placed in the /home/dan/dev/tcp_iot directory of 239 (if using that as a server for testing)
client_unpack.sh can be renamed to unpack.sh and placed in the /home/dan/dev/tcp_iot directory of 154, 147 & 151 (if using those as a clients for testing)

1) zip up entire tcp_iot directory and upload to 243:/home/dan/dev/tcp_iot 
2) run unpack which unzips all the zipped files and copies to directories and then tars up a file called server.tar & client.tar 
	and scp's each one out to 147, 154 & 151
3) go to the server (tcp_iot directory) and run unpack.sh which untar's all the files and compiles the server program 
4) go to each client and run unpack.sh which does the same for the clients 
