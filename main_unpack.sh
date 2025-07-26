if [ -e /home/dan/tcp_iot.zip ]
then
unzip -o ~/tcp_iot.zip
tar cf server.tar server/*.h server/*.c server/Makefile *.h
clear
scp server.tar 192.168.88.147:/home/dan/dev
tar cf client.tar client/*.h client/*.c client/Makefile *.h
scp client.tar 192.168.88.154:/home/dan/dev
scp client.tar 192.168.88.151:/home/dan/dev
rm server.tar
rm client.tar
rm ~/tcp_iot.zip
exit
fi
echo "tcp_iot.zip not found"
