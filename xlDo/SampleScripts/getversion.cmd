rem getVersion 3 ways

curl http://127.0.0.1:49913/xlDoAutomation -d "{\"cmd\":\"getVersion\"}" 

curl http://127.0.0.1:49913/getVersion

xldo -c "{\"cmd\":\"getVersion\"}" 

pause