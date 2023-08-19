# Control electric loads with Enphase technology & SUN ... for hobbyist 

I have a photovoltaic system managed by Enphase S-metered since mai 2023 and want to heat the electric boiler water with the sun
My goal is to read Power Consumed & Power Produced LIVE values (in real-time)

This doc https://enphase.com/download/iq-gateway-access-using-local-apis-or-local-ui-token-based-authentication-tech-brief gives me help 
- Follow "Obtaining a token via web UI"
- Use Code and adapt parameters P1 P2 to your configuration  
- Add libraries ESP8266WiFi.h WiFiUdp.h ESP_SSLClient.h
- With Format=0 you see raw data sended by Enphase S-metered (request = /ivp/meters/readings)
- With Format=1 you can read Solar production (Solar_W) & provider (Grid_W) values in Watt
- minus Grid_W value means you are richer !  

Tested with software version D7.6.175  

My full version manages this electric boiler :
- During day period : boil if Power available
- During night period : boil if necessary
- ~6 kWH / day

I come from Arduino (Due & MKR) world & appreciate the power of ESP8266 with a lot of libraries

Gerald Litzistorf - aout 2023 - gelit.ch - 20 Märklin trains on the move ... managed by Arduino Due !
