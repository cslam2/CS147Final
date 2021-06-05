import json
import requests
import time
import numpy as np
from scipy import interpolate
from datetime import datetime
from queue import Queue
from flask import Flask, Response, render_template, request

api_key = "API key goes here"
lat = "33.8753"
lon = "-117.5664"
exclude = "currently,daily,alerts"
onecall = f"https://api.openweathermap.org/data/2.5/onecall?lat={lat}&lon={lon}&exclude={exclude}&appid={api_key}&units=imperial"
weather = requests.get(onecall)
weather_dict = json.loads(weather.text)
hourly = weather_dict['hourly']
cTemp = []
flTemp = []
humid = []
for i in range(48):
	cTemp.append(hourly[i]['temp'])
	flTemp.append(hourly[i]['feels_like'])
	humid.append(hourly[i]['humidity'])

def interpData(data):
	x=np.arange(0,48)
	y=np.array(data)
	f = interpolate.interp1d(x,y)
	secs = np.arange(0,47,1/3600)
	result = f(secs)
	return result

cTemp = interpData(cTemp)
flTemp = interpData(flTemp)
humid = interpData(humid)

application = Flask(__name__)
qH = Queue(maxsize = 3)
qT = Queue(maxsize = 3)
qLH = Queue(maxsize = 0)
qLT = Queue(maxsize = 0)
qLflT = Queue(maxsize = 0)
for i in range(len(cTemp)):
	qLT.put(cTemp[i])
	qLflT.put(flTemp[i])
	qLH.put(humid[i])

@application.route("/")
def index():
    return render_template("index.html", type = "Humidity")

@application.route("/get-data")
def get_data():
	qH.put(request.args.get("hum"))
	qT.put(request.args.get("temp"))
def generate_random_data():
    	while True:
        	json_data = json.dumps(
            	{
                	"time": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                	"hum": qH.get(),
			"temp": qT.get(),
			"humL": qLH.get(),
			"tempL": qLT.get(),
			"fltempL": qLflT.get(),
            	}
        	)
        	yield f"data:{json_data}\n\n"
        	time.sleep(1)


@application.route("/chart-data")
def chart_data():
    return Response(generate_random_data(), mimetype="text/event-stream")


if __name__ == "__main__":
    application.run(host="0.0.0.0", debug=True, threaded=True)