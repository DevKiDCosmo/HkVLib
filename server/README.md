# MQTT Server


# API Endpoints

## Device Registry

Method	Route	Description
GET	/id/<mac>/<teamid>	Assign or return device ID (plain-text int for firmware)
GET	/devices	List all registered devices (JSON)
GET	/devices/<mac>	Get a single device (JSON)
DELETE	/devices/<mac>	Remove a device
GET	/devices/save	Force immediate CSV save