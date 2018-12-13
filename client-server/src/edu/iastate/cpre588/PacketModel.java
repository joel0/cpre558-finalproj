package edu.iastate.cpre588;

import java.io.IOException;
import java.io.InputStream;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

// Packet definition:
//      0       1       2       3       <
//  0   |---------Source IPv4----------|
//  4   |-------Destination IPv4-------|
//  8   |--Period(ms)--||Payload length|
//  12  |Payload...>

/**
 * Represents a packet and its header data.  Allows for serialization and deserialization for network transmit.
 */
public class PacketModel {
    /** The source IP address of this packet. */
    private Inet4Address sourceIp;
    /** The destination IP of this packet (not necessarily the next hop). */
    private Inet4Address destinationIp;
    /** The period between instances of this packet. In milliseconds. */
    private short period;
    /** This packet's byte array payload. */
    private byte[] payload;

    public PacketModel(Inet4Address sourceIp, Inet4Address destinationIp, short period, byte[] payload) {
        this.sourceIp = sourceIp;
        this.destinationIp = destinationIp;
        this.period = period;
        this.payload = payload;
    }

    /**
     * Serializes this packet into an array of bytes for network transmit.
     *
     * @return the byte array representing this packet's data
     */
    public byte[] toBytes() {
        ByteBuffer outBuf = ByteBuffer.allocate(payload.length + 12);
        outBuf.order(ByteOrder.LITTLE_ENDIAN);

        outBuf.put(sourceIp.getAddress());
        outBuf.put(destinationIp.getAddress());
        outBuf.putShort(period);
        assert payload.length <= Short.toUnsignedInt((short) -1);
        outBuf.putShort((short) payload.length);
        outBuf.put(payload);
        assert outBuf.position() == 12 + payload.length;

        return outBuf.array();
    }

    /**
     * Reads a complete packet from a stream into a new PacketModel.
     *
     * @param stream the (probably network) stream to read this packet from
     * @return a PacketModel parsed from the stream
     */
    public static PacketModel fromStream(InputStream stream) throws IOException {
        byte[] buffer;
        Inet4Address sourceIp;
        Inet4Address destinationIp;

        // Read source IP
        buffer = stream.readNBytes(4);
        sourceIp = (Inet4Address) InetAddress.getByAddress(buffer);

        // Read destination IP
        buffer = stream.readNBytes(4);
        destinationIp = (Inet4Address) InetAddress.getByAddress(buffer);

        // Read period
        buffer = stream.readNBytes(2);
        short period = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN).getShort();

        // Read payload length
        buffer = stream.readNBytes(2);
        int payloadSize = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN).getShort();

        // Read payload
        buffer = stream.readNBytes(payloadSize);

        return new PacketModel(sourceIp, destinationIp, period, buffer);
    }

    @Override
    public String toString() {
        return String.format("From: %s, To: %s, Data size: %d",
                sourceIp.toString(), destinationIp.toString(), payload.length);
    }
}
