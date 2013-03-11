# !/bin/bash
dir_name=("accident_and_disaster"     "automobile"        "boxing"                "environment"         "machinery_manufacturing"  "religion"            "table_tennis" "accounting_and_taxation"   "badminton"         "chemical_industry"     "fencing"             "military"              "retail"               "advertisement_media"       "banking"           "consumer_electronics"  "funds"               "mining_and_metals"        "rugby_union"     "agriculture_and_forestry"  "baseball"          "cricket"               "golf"                "motor_racing"             "sailing"            "airlines_and_airports"     "basketball"        "crime_and_law"         "health"             "science"            "american_football"      "cycling"               "horse_racing"        "olympics"                 "shipping_and_ports"  "travel"        "billiards"         "education"             "ice_hockey"          "politics"                 "shooting"            "volleyball" "apparel"                   "bonds"             "energy"                "insurance"           "rallying"                 "skiing"              "weightlifting" "athletic"                  "bowling"    "entertainment"         "job_human_resource"  "real_estate"              "soccer");


dir_count=${#dir_name[*]}

time_start=`date`

while((dir_count != 0))
do
let "dir_count=$dir_count-1"

dir_name_current=${dir_name[dir_count]}

command_line="/billy/${dir_name_current}/${dir_name_current}/text/ /home/ape/Dev/cvs/ImRoBot5/egg2/egg2-0-1/utest/data/ ${dir_name_current} 0"
echo $commad_line
./egg2bigfile ${command_line}
   if [ $? != 0 ];then
       
	   echo "dump err!!!!"

	   time_end=`date`
	   echo "time_start : $time_start"
	   echo "time_end : $time_end"

	   exit -1
   fi
#    while(($? != 0))
 #   do
  #         ./egg2bigfile ${command_line}
   # done

done
time_end=`date`
echo "time_start : $time_start"
echo "time_end : $time_end"

