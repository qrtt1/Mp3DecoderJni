package qty.player;

import java.io.File;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

public class Playback {

    public static void main(String[] args) {
        Mp3Decoder decoder = new Mp3Decoder();
        String media = "sample.mp3";
        decoder.open(new File(media));
        byte[] buffer = new byte[decoder.getRequiredBufferSize()];
        int read = 0;
        
        SourceDataLine line = null;
        
        AudioFormat fmt = new AudioFormat(decoder.getSampleRate(),
                16 /*default is pcm-16bits*/, decoder.getChannels(), true,
                false);

        try {
            line = (SourceDataLine) AudioSystem.getSourceDataLine(fmt);
            line.open(fmt);
            line.start();
            
            while((read = decoder.decode(buffer))!=-1 )
            {
                    line.write(buffer, 0, read);
            }
            
            line.drain();
            line.stop();
            
        } catch (LineUnavailableException ex) {
        } finally {
            if (line != null)
                line.close();
        }
    }
}
