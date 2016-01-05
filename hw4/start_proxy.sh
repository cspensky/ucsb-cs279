java -jar /tools/burp/burpsuite_free_v1.6.31.jar &

chromium-browser -user-data-dir=/tmp/chrome --proxy-server="127.0.0.1:8080"
