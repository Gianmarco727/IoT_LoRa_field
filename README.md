# IoT_LoRa_field
A private LoRa deployment aimed at monitoring ground parameters of an agricultural field and display the data in a privately hosted database.  
The files: Sender_node_1, and 2 should be loaded into an ESP LoRa device, more precisely a TTGO OLED LoRa.  
The file: Receiver_node_V0 should be loaded into and Arduino UNO with a Dragino LoRa HAT.  
The NodeRed_packethandler.json can be imported into the Node-RED environment.  
As for the database, InfluxDB was adopted and Grafana for the visualization of the data.  
https://facebook.github.io/prophet/ was used to predict data.  
