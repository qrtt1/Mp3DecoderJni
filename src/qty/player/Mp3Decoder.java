package qty.player;

import java.io.File;
import java.io.FileOutputStream;

public class Mp3Decoder {

    static {
        System.loadLibrary("decoder");
    }

    private int handle = 0;
    private long rate = 0;
    private int channels = 0;
    private int encoding = 0;
    private int requiredBuffersize = 0;

    public native void open(File path);

    public native int decode(byte[] buffer);

    public native void close();

    @Override
    public String toString() {
        return String.format(
                "handle:0x%x, encoding: %d, rate: %d, channels: %d", handle,
                encoding, rate, channels);
    }

    @Override
    protected void finalize() throws Throwable {
        close();
        super.finalize();
    }

    public int getRequiredBufferSize() {
        return requiredBuffersize;
    }

    public float getSampleRate() {
        return rate;
    }

    public int getChannels() {
        return channels;
    }

    public static void main(String[] args) {
        Mp3Decoder d = new Mp3Decoder();
        String media = "sample.mp3";
        d.open(new File(media));
        System.out.println(d);
        byte[] buffer = new byte[d.requiredBuffersize];
        int read = 0;
        try {
            FileOutputStream fout = new FileOutputStream(new File("out.raw"));
            while ((read = d.decode(buffer)) != -1) {
                fout.write(buffer, 0, read);
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            d.close();
        }
    }

}
