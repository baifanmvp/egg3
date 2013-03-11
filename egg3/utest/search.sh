i=1
cnt=1
while [ true ]
do
cnt=1
while [ $cnt -le 20 ]
do
printf  "search\nrandom\n$((RANDOM%198))\n\n"|./eggUtest ItfTest  &
let "cnt=cnt+1"
echo "-------$cnt------------"
#sleep 0.1

done
sleep 10
done