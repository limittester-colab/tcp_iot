1) zip up entire tcp_iot directory and upload to 243:/home/dan 
2) ssh into 243 and run unpack which unzips all the zipped files and copies to directories and then tars up a file 
	called server.tar & client.tar and scp's each one out to 147, 154 & 146
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

aux_client2a.c - is what was used in the multi-client project except it has 
to be run from a session of each client e.g. to run the scripts or the 
cabin you have to be logged into a session of 154 - the scripts for all the 
cabin ports are in the /dev/tcp_iot directory - this is because it sends a 
ipc msg directly to the client program running so there is no dest param 
like in the multi-client program which sent the ipc msg to the aux_client 
which then sent it to the server 

