import time
import urequests
  
def get_weather():
    w_url="http://www.weather.com.cn/data/sk/101010100.html"
    res=urequests.get(w_url,stream=None)
    print(res.json())

get_weather()
     
    

