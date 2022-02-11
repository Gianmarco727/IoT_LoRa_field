#this code is used to send a HTTP post message to the influxdb server in order to flush data from the bucket.
import requests
import datetime
next_year = datetime.date.today().year +1
#you need to specify here which bucket you want to flush
ip = "http://#your_ip#:8086/api/v2/delete?org=field&bucket=myfield"
#here you need to specify which label to flush in predicate, you can use logic operator
data = {"start": "2020-10-15T00:00:00Z", "stop": str(next_year)+"-10-15T00:00:00Z", "predicate": 'type="forecast"'}
headers = {'Authorization': '#your_token#'}
r = requests.post(url=ip, headers = headers, json=data)
if(r.status_code<400): print('flush compleate')

