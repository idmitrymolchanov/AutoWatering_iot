package org.dmitriimolchanov.connection;

import org.dmitriimolchanov.model.DeviceStruct;

import java.sql.*;

public class ConnectionH2 {

    public static void setData() {
        try {
            Class.forName("org.h2.Driver").newInstance();
            Connection conn = DriverManager.getConnection("jdbc:h2:test",
                    "sa", "");
            Statement st = null;
            st = conn.createStatement();
            st.execute("INSERT INTO TEST VALUES(default,'HELLO')");
            st.execute("INSERT INTO TEST(NAME) VALUES('JOHN')");
            String name1 = "Jack";
            String q = "insert into TEST(name) values(?)";
            PreparedStatement st1 = null;

            st1 = conn.prepareStatement(q);
            st1.setString(1, name1);
            st1.execute();

            ResultSet result;
            result = st.executeQuery("SELECT * FROM TEST");
            while (result.next()) {
                String name = result.getString("NAME");
                System.out.println(result.getString("ID")+" "+name);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public DeviceStruct getData() {
        return null;
    }
}
