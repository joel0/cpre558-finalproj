package edu.iastate.cpre588;

import java.io.IOException;
import java.net.ServerSocket;

public class Main {
    /** The port for sending and receiving packets. */
    public static final int LISTEN_PORT = 1337;

    public static void main(String[] args) {
        ServerSocket server;
        try {
            server = new ServerSocket(LISTEN_PORT);
//            PacketModel p = PacketController.getPacket(server);
            PacketController.getPacketAsync(server);
            System.out.println("Client 1 connected");
            PacketController.getPacketAsync(server);
            server.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
