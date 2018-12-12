package edu.iastate.cpre588;

import java.io.IOException;
import java.net.Inet4Address;

public class Client {

    public static void main(String[] args) {
        byte[] payload = "Hello World".getBytes();
        try {
            PacketModel packet = new PacketModel(
                    (Inet4Address) Inet4Address.getLocalHost(),
                    (Inet4Address) Inet4Address.getLoopbackAddress(),
                    (short) 100,
                    payload
            );

            PacketController.sendPacket(Inet4Address.getLoopbackAddress(), packet);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
