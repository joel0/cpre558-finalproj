package edu.iastate.cpre588;

import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;

/**
 * Represents a packet and its header data.  Allows for serialization and deserialization for network transmit.
 */
public class PacketModel {
    /** The source IP address of this packet. */
    private InetAddress sourceIp;
    /** The destination IP of this packet (not necessarily the next hop). */
    private InetAddress destinationIp;
    /** This packet's byte array payload. */
    private byte[] payload;

    public PacketModel(InetAddress sourceIp, InetAddress destinationIp, byte[] payload) {
        assert sourceIp.getClass() == destinationIp.getClass();
        this.sourceIp = sourceIp;
        this.destinationIp = destinationIp;
        this.payload = payload;
    }

    /**
     * Serializes this packet into an array of bytes for network transmit.
     *
     * @return the byte array representing this packet's data
     */
    public byte[] toBytes() {
        // TODO
        return null;
    }

    /**
     * Reads a complete packet from a stream into a new PacketModel.
     *
     * @param stream the (probably network) stream to read this packet from
     * @return a PacketModel parsed from the stream
     */
    public static PacketModel fromStream(InputStream stream) throws IOException {
        byte[] buffer;
        boolean ipv6;
        InetAddress sourceIp;
        InetAddress destinationIp;

        ipv6 = stream.read() == 1;

        buffer = stream.readNBytes(ipv6 ? 16 : 4);
        sourceIp = InetAddress.getByAddress(buffer);

        buffer = stream.readNBytes(ipv6 ? 16 : 4);
        destinationIp = InetAddress.getByAddress(buffer);

        int payloadSize = stream.read();

        buffer = stream.readNBytes(payloadSize);

        return new PacketModel(sourceIp, destinationIp, buffer);
    }

    @Override
    public String toString() {
        return String.format("From: %s, To: %s, Data size: %d",
                sourceIp.toString(), destinationIp.toString(), payload.length);
    }
}
