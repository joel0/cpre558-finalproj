package edu.iastate.cpre588;

import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;

public class PacketController {

    public static void sendPacket(InetAddress nextHop, PacketModel packet) throws IOException {
        Socket s = new Socket(nextHop, Main.LISTEN_PORT);
        byte[] packetBytes = packet.toBytes();
        s.getOutputStream().write(packetBytes);
        s.close();
    }

    public static PacketModel getPacket(ServerSocket server) throws IOException {
        PacketModel packet;

        Socket s = server.accept();
        packet = PacketModel.fromStream(s.getInputStream());
        s.close();

        return packet;
    }

    public static void getPacketAsync(ServerSocket server) throws IOException {
        Socket recvSocket;
        Thread recvThread;

        recvSocket = server.accept();
        recvThread = new PacketReadThread(recvSocket);
        recvThread.start();
    }

    /**
     * A thread that reads a packet from the network.  TODO: does something with it.
     */
    private static class PacketReadThread extends Thread {
        /** The Socket that is connected to the computer sending the packet. */
        private Socket recvSocket;

        PacketReadThread(Socket recvSocket) {
            this.recvSocket = recvSocket;
        }

        @Override
        public void run() {
            try {
                PacketModel packet;

                packet = PacketModel.fromStream(recvSocket.getInputStream());
                recvSocket.close();

                // TODO do something with the packet
                System.out.println(packet.toString());
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
