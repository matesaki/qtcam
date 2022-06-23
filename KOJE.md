# Extension for the KOJE project

## MQTT communication

Connection informations are taken from configuration file [-c <config.ini>]

All communication is made through Mosquitto (MQTT). On linux install: `sudo apt install -y mosquitto-clients`.
Master PC send commands on topic "cmd".
Slave PCs respond on topic "info/<name>".


Sending commands (in first terminal):
```
mosquitto_pub -t cmd -m "init"        # Try to connect to all cameras
mosquitto_pub -t cmd -m "swtrigger"   # Switch to SW trigger all cameras
mosquitto_pub -t cmd -m "notrigger"   # Switch to regular (video) mode all cameras
```


Read statuses (in second terminal):
```
mosquitto_sub -t "info/#" -v
```
