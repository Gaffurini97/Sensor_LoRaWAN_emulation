## IMAGE CREATION


docker build -t lorawanlocale_en1 . (directory end-device)

docker build -t lorawanlocale_gw1 . (directory gateway)

docker build -t lorawanlocale_nodered . (directory NodeRed)



## CONFIGURATION
This configuration is set up for 100 end-device, before run the container it needed to change some parameters:

Backend server gateway configuration:
1. /gateway/cfd-data/config.json change the server_address

Register the end devices to the backend


## START

$ ./start.sh <number_end_devices>

$ docker-compose up
