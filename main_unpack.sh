if [ -e /home/dan/tcp_iot.zip ]
then
unzip -o ~/tcp_iot.zip
mv server_Makefile server/Makefile
tar cf server.tar server/*.h server/*.c cmd_tasks.c tasks.c cs_client/* queue/* load_cmds.c mytypes.h cmd_types.h tasks.h
clear
scp server.tar 192.168.88.239:/home/dan/dev

mv Makefile_154 client/Makefile
tar cf client.tar client/*.h client/*.c cmd_tasks.c tasks.c cs_client/* queue/* load_cmds.c mytypes.h cmd_types.h tasks.h
scp client.tar 192.168.88.154:/home/dan/dev

rm client.tar
mv Makefile_151 client/Makefile
tar cf client.tar client/*.h client/*.c cmd_tasks.c tasks.c cs_client/* queue/* load_cmds.c mytypes.h cmd_types.h tasks.h
scp client.tar 192.168.88.151:/home/dan/dev

rm client.tar
mv Makefile_147
tar cf client.tar client/*.h client/*.c cmd_tasks.c tasks.c cs_client/* queue/* load_cmds.c mytypes.h cmd_types.h tasks.h
scp client.tar 192.168.88.147:/home/dan/dev

rm server.tar
rm client.tar
rm ~/tcp_iot.zip
exit
fi
echo "tcp_iot.zip not found"
