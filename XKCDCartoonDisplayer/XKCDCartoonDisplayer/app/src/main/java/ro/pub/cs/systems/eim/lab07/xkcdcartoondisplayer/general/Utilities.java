package ro.pub.cs.systems.eim.lab07.xkcdcartoondisplayer.general;

import org.apache.http.HttpEntity;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Utilities {

    public static BufferedReader getReader(HttpEntity httpEntity) throws IOException {
        return new BufferedReader(new InputStreamReader(httpEntity.getContent()));
    }

}
