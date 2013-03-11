if [ -z "$1" ];then
fcgipath=/var/www/cgi-bin/ 
else
fcgipath=$1
fi
echo  $fcgipath
make
sudo /etc/init.d/apache2 stop

cp ./eggPM.fcgi ${fcgipath}eggPM.fcgi
cp ./egg.fcgi ${fcgipath}eggChunk.fcgi
cp ./egg.fcgi ${fcgipath}egg.fcgi
chmod a+s ${fcgipath}egg.fcgi
chmod a+s ${fcgipath}eggChunk.fcgi
chmod a+s ${fcgipath}eggPM.fcgi
sudo /etc/init.d/apache2 start
