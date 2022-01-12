set from=opck@sinux.com.cn
set user=opck@sinux.com.cn
set pass=Mail_2021
set to=%GITLAB_USER_EMAIL%
set subj="cppcheck report" 
set mail= error.txt
set attach= error.txt
set server=smtp.mxhichina.com

.\CI\windows10\blat %mail% -to %to% -base64 -charset gb2312 -subject %subj% -attach %attach% -server %server% -f %from% -u %user% -pw %pass%
