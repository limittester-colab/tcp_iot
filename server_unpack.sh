clear
rm server
if [ -e /home/dan/dev/server.tar ]
then
tar xvf /home/dan/dev/server.tar
mv server/* .
rmdir server
make clean
make &> out.txt
if grep -q error out.txt
then
find2 error out.txt
exit
fi
if grep -q undefined out.txt
then
find2 undefined out.txt
exit
fi

rm /home/dan/dev/server.tar
rm *.o
clear
ls -ltr server
exit
fi
echo "server.tar not exist"
