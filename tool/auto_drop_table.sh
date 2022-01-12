#!/bin/bash
##指定保存表的天数，超过保存天数情况下，进行删除
source /etc/profile

isql -d statusdb -p 2004 -Usysdba/szoscar55 -t -c "select  distinct table_name  from ALL_TAB_PRIVS where  table_name like 'STATUS_HISTORY%' and replace(right(table_name,8),'_','')::date<dateadd('DAY',current_date(),-30)::date
 ;" > table_name_drop.txt

for i in $(cat table_name_drop.txt); 
do

echo $i

isql -d statusdb -p 2004 -Usysdba/szoscar55 -c "drop table $i"

done


