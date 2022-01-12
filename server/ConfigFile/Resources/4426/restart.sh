#!/bin/bash
#监控软件定时任务
#*/1 * * * * /opt/MonitorSoft/restart.sh
#*/1 * * * * sleep 10; /opt/MonitorSoft/restart.sh
#*/1 * * * * sleep 20; /opt/MonitorSoft/restart.sh
#*/1 * * * * sleep 30; /opt/MonitorSoft/restart.sh
#*/1 * * * * sleep 40; /opt/MonitorSoft/restart.sh
#*/1 * * * * sleep 50; /opt/MonitorSoft/restart.sh


curdate=`date +%Y%m%d%H:%M:%S`
pid=`ps -aux | grep SMCC | grep -v grep | awk '{print $2}'`
flowIP=`/usr/sbin/ip addr | grep 132.29.1.121`
export DISPLAY=:0.0
if [ ! -n  "$pid" ]; then
	echo "$curdate exe go dead" >> /DATA/4426/code/server/bin/run.log
	cd /DATA/4426/code/server/bin/
	nohup /DATA/4426/code/server/bin/SMCC  >> /DATA/4426/code/server/bin/run.log 2>&1 &
	echo "$curdate 监控服务器重启" >> /DATA/4426/code/server/bin/run.log
	if [ -n "$flowIP" ];then 
		#如果程序崩溃，浮动ip还在这台机器上，执行切换 主 -> 备
		/usr/local/heartbeat/share/heartbeat/hb_standby
		echo "$curdate 1.3 master to 1.4 master" >> /DATA/4426/code/server/bin/run.log
	fi	

fi

hostname=`hostname`
role=""
if [ "$hostname" == "master" ];then
	role=`/opt/redis/bin/redis-cli -h 132.29.1.3 -a 123456 -p 6379 info | grep role:master`
else 
	role=`/opt/redis/bin/redis-cli -h 132.29.1.4 -a 123456 -p 6379 info | grep role:master`
fi


role1=`/opt/redis/bin/redis-cli -h 132.29.1.3 -a 123456 -p 6379 info | grep role:master`
role2=`/opt/redis/bin/redis-cli -h 132.29.1.4 -a 123456 -p 6379 info | grep role:master`

if [ "$role1" == "$role2" ]; then
	echo "数据库主备状态异常" >> /DATA/4426/code/server/bin/run.log
	echo "主：$role1" >> /DATA/4426/code/server/bin/run.log
	echo "备：$role2" >> /DATA/4426/code/server/bin/run.log
fi

	echo "主：$role1" >> /DATA/4426/code/server/bin/run.log
	echo "备：$role2" >> /DATA/4426/code/server/bin/run.log
#如果浮动ip为空
if [ ! -n  "$flowIP" ]; then
	#当浮动IP为空，100%需要切换当前机器为redis为备机
	if [ "$hostname" == "master" ];then
		#如果是redis是备机
		if [ -n  "$role" ]; then
			/opt/redis/bin/redis-cli -h 132.29.1.3 -a 123456 -p 6379 slaveof 132.29.1.4 6379
			/opt/redis/bin/redis-cli -h 132.29.1.4 -a 123456 -p 6379 slaveof NO ONE
			echo "$curdate 当浮动IP为空，100%需要切换当前机器为redis为备机，手动进行切换" >> /DATA/4426/code/server/bin/run.log
		fi
	else 
		#如果是redis是备机
		if [ -n  "$role" ]; then
			/opt/redis/bin/redis-cli -h 132.29.1.4 -a 123456 -p 6379 slaveof 132.29.1.3 6379
			/opt/redis/bin/redis-cli -h 132.29.1.3 -a 123456 -p 6379 slaveof NO ONE
			echo "$curdate 当浮动IP为空，100%需要切换当前机器为redis为备机，手动进行切换" >> /DATA/4426/code/server/bin/run.log
		fi
	fi
else
	#如果是redis是备机
	if [ ! -n  "$role" ]; then
		if [ "$hostname" == "master" ];then
			echo master >> /DATA/4426/code/server/bin/run.log
			/opt/redis/bin/redis-cli -h 132.29.1.4 -a 123456 -p 6379 slaveof 132.29.1.3 6379
			/opt/redis/bin/redis-cli -h 132.29.1.3 -a 123456 -p 6379 slaveof NO ONE
		else 
			echo slvae >> /DATA/4426/code/server/bin/run.log
			/opt/redis/bin/redis-cli -h 132.29.1.3 -a 123456 -p 6379 slaveof 132.29.1.4 6379
			/opt/redis/bin/redis-cli -h 132.29.1.4 -a 123456 -p 6379 slaveof NO ONE
		fi
		echo "$curdate 当前机器浮动IP存在，但是redis是备机，手动进行切换" >> /DATA/4426/code/server/bin/run.log
	else
		echo "$curdate 当前机器为主机不用进行切换" >> /DATA/4426/code/server/bin/run.log	
	fi
fi

