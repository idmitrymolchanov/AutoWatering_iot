package org.dmitriimolchanov.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

@AllArgsConstructor
@NoArgsConstructor
@Getter
@Setter
public class DeviceStruct {
    private int id;
    private double pressure;
    private double air_temperature;
    private double air_humidity;
    private double soil_humidity;
    private String date;
}
