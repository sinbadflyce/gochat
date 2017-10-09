package red.tel.chat;

import android.util.Log;

import com.neovisionaries.ws.client.ProxySettings;
import com.neovisionaries.ws.client.WebSocket;
import com.neovisionaries.ws.client.WebSocketAdapter;
import com.neovisionaries.ws.client.WebSocketException;
import com.neovisionaries.ws.client.WebSocketFactory;
import com.neovisionaries.ws.client.WebSocketFrame;
import com.neovisionaries.ws.client.WebSocketState;

import java.util.List;
import java.util.Map;

import red.tel.chat.EventBus.Event;

// websocket façade
public class Network {

    private static final String serverUrl = "ws://192.168.2.135:8000/ws";
    private static final String TAG = "Network";
    private static final int CONNECTION_TIMEOUT = 1000;
    private WebSocket webSocket;
    private static volatile Network ourInstance = null;

    public static Network getInstance() {
        if (ourInstance == null) {
            synchronized (Network.class) {
                if (ourInstance == null) {
                    ourInstance = new Network();
                }
            }
        }
        return ourInstance;
    }

    public WebSocket getWebSocket() {
        return webSocket;
    }

    public void onInitConnectServer() {
        WebSocketFactory factory = new WebSocketFactory();
        ProxySettings settings = factory.getProxySettings();
        settings.setServer(serverUrl);

        try {
            WebSocketAdapter webSocketAdapter = new WebSocketAdapter() {
                @Override
                public void onStateChanged(WebSocket websocket, WebSocketState newState) throws Exception {
                    Log.d(TAG, "onStateChanged: " + newState.name());
                }

                @Override
                public void onConnected(WebSocket websocket, Map<String, List<String>> headers) throws Exception {
                    Log.i(TAG, "Connected");
                    EventBus.announce(Event.CONNECTED);
                }

                @Override
                public void onConnectError(WebSocket websocket, WebSocketException cause) throws Exception {
                    Log.e(TAG, "Connect error");
                    EventBus.announce(Event.DISCONNECTED);
                }

                @Override
                public void onDisconnected(WebSocket websocket, WebSocketFrame serverCloseFrame, WebSocketFrame clientCloseFrame, boolean closedByServer) throws Exception {
                    Log.i(TAG, "Disconnected");
                    EventBus.announce(Event.DISCONNECTED);
                }

                @Override
                public void onError(WebSocket websocket, WebSocketException cause) throws Exception {
                    Log.e(TAG, "Error" + cause.getLocalizedMessage());
                }

                @Override
                public void onUnexpectedError(WebSocket websocket, WebSocketException cause) throws Exception {
                    Log.e(TAG, "Unexpected error");
                }

                @Override
                public void onBinaryMessage(WebSocket websocket, byte[] binary) throws Exception {
                    Log.i(TAG, "onBinaryMessage " + binary.length + " bytes");
                    WireBackend.shared().onReceiveFromServer(binary);
                }
            };

            WebSocketFactory webSocketFactory = new WebSocketFactory();
            webSocketFactory.setConnectionTimeout(CONNECTION_TIMEOUT);
            webSocket = webSocketFactory.createSocket(serverUrl);
            webSocket.addListener(webSocketAdapter);
            webSocket.connectAsynchronously();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    synchronized void send(byte[] bytes) {
        webSocket.sendBinary(bytes);
    }
}
