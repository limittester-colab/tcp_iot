rm client
if [ -e /home/dan/dev/client.tar ]
then
tar xvf /home/dan/dev/client.tar
mv client/* .
rmdir client
make clean
make &> out.txt
if grep -q error out.txt
then 
find2 error out.txt
exit 1
fi
if grep -q undefined out.txt
then
find2 undefined out.txt
exit 1
fi
rm /home/dan/dev/client.tar
rm *.o
ls -ltr client
exit
fi
echo "client.tar not exists"
