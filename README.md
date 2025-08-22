main_unpack.sh can be renamed to unpack.sh and placed in the /home/dan/dev/tcp_iot directory of 243
server_unpack.sh can be renamed to unpack.sh and placed in the /home/dan/dev/tcp_iot directory of 239 (if using that as a server for testing)
client_unpack.sh can be renamed to unpack.sh and placed in the /home/dan/dev/tcp_iot directory of 154, 147 & 151 (if using those as a clients for testing)
server_Makefile, Makefile_147, Makefile_154 & Makefile_151 all get renamed to 'Makefile' and put in either server or client subdirectories
by unpack.sh 

1) zip up entire tcp_iot directory and upload to 243:/home/dan 
2) ssh into 243 and run unpack which unzips all the zipped files and copies to directories and then tars up a file 
	called server.tar & client.tar and scp's each one out to 147, 154 & 151
3) go to the server (tcp_iot directory) and run unpack.sh which untar's all the files and compiles the server program 
4) go to each client and run unpack.sh which does the same for the clients 

timers:

timer_thread in thread_server.c sends the SEND_STATUS command to each client 
client sends back a UPDATE_STATUS msg with client name as param 
when server gets the UPDATE_STATUS msg, it will get the current time using
'now = time(NULL);' and place 'now' in the time_stamp field for the current 
client in pthreads_list[] 
then check_time_thread will periodically compare all the entries in time_stamp 
with the current time to compare the current time to the last time the client 
got a msg 
