package org.dmitriimolchanov.controller;

import org.dmitriimolchanov.Subscriber;
import org.dmitriimolchanov.model.DeviceStruct;
import org.dmitriimolchanov.model.OperatorAction;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.PostMapping;

import javax.validation.Valid;
import java.util.Map;

@Controller
public class MainController {

    private static Subscriber subscriber;

    @Autowired
    public MainController() {
    }

    static {
        try {
            subscriber = new Subscriber("mqtt://login:password@address:port");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @GetMapping("/main_page")
    public String MainPage(Model model, Map<String, DeviceStruct> myVar) {
        model.addAttribute("action", new OperatorAction());
        DeviceStruct deviceStruct = subscriber.getDeviceStruct();
        myVar.put("myVar", deviceStruct);

        return "main_page";
    }

    @PostMapping("/main_page")
    public String MainPagePost(@ModelAttribute("action") @Valid OperatorAction value, Map<String, DeviceStruct> myVar) {
        DeviceStruct deviceStruct = subscriber.getDeviceStruct();
        myVar.put("myVar", deviceStruct);
        if (value.getForced() != null)
            try {
                subscriber.sendMessage("*******/feeds/onoff#forced");
            } catch (Exception e) {
                e.printStackTrace();
            }
            //value.setForced("forced");
        if (value.getDisable() != null) {
            try {
                subscriber.sendMessage("*******/feeds/onoff#disable");
            } catch (MqttException e) {
                e.printStackTrace();
            }
        }
        if (value.getResume() != null) {
            try {
                subscriber.sendMessage("*******/feeds/onoff#resume");
            } catch (MqttException e) {
                e.printStackTrace();
            }
        }

        System.out.println(value.getDisable());
        return "main_page";
    }
}