package org.dmitriimolchanov;

import org.dmitriimolchanov.model.DeviceStruct;
import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import java.net.URI;
import java.net.URISyntaxException;

import org.json.*;

public class Subscriber implements MqttCallback {

    private final int qos = 1;
    private String topic = "*******/feeds/temperature";
    private MqttClient client;
    public static DeviceStruct deviceStruct;

    public Subscriber(String uri) throws MqttException, URISyntaxException {
        this(new URI(uri));
    }

    public Subscriber(URI uri) throws MqttException {
        String host = String.format("tcp://%s:%d", uri.getHost(), uri.getPort());
        String[] auth = this.getAuth(uri);
        String username = auth[0];
        String password = auth[1];
        String clientId = "any";
        if (!uri.getPath().isEmpty())
            this.topic = uri.getPath().substring(1);

        MqttConnectOptions conOpt = new MqttConnectOptions();
        conOpt.setCleanSession(true);
        conOpt.setUserName(username);
        conOpt.setPassword(password.toCharArray());

        this.client = new MqttClient(host, clientId, new MemoryPersistence());
        this.client.setCallback(this);
        this.client.connect(conOpt);

        this.client.subscribe(this.topic, qos);
    }

    private String[] getAuth(URI uri) {
        String a = uri.getAuthority();
        String[] first = a.split("@");
        return first[0].split(":");
    }

    public void sendMessage(String payload) throws MqttException {
        String[] strings = payload.split("#");
        MqttMessage message = new MqttMessage(strings[1].getBytes());
        message.setQos(qos);
        this.client.publish(strings[0], message); // Blocking publish
    }

    /**
     * @see MqttCallback#connectionLost(Throwable)
     */
    public void connectionLost(Throwable cause) {
        System.out.println("Connection lost because: " + cause);
        System.exit(1);
    }

    /**
     * @see MqttCallback#deliveryComplete(IMqttDeliveryToken)
     */
    public void deliveryComplete(IMqttDeliveryToken token) {
    }

    /**
     * @see MqttCallback#messageArrived(String, MqttMessage)
     */
    public void messageArrived(String topic, MqttMessage message) throws MqttException {
        parser(String.format("%s", new String(message.getPayload())));
    }

    public void parser(String jsonString) {
        deviceStruct = new DeviceStruct();
        JSONObject obj = new JSONObject(jsonString);
        deviceStruct.setId(obj.getInt("id"));
        deviceStruct.setPressure(obj.getDouble("pressure"));
        deviceStruct.setAir_temperature(obj.getDouble("air_temperature"));
        deviceStruct.setAir_humidity(obj.getDouble("air_humidity"));
        deviceStruct.setSoil_humidity(obj.getDouble("soil_humidity"));
        deviceStruct.setDate(obj.getString("date"));

        System.out.println(jsonString);
    }

    public DeviceStruct getDeviceStruct() {
        return deviceStruct;
    }
}