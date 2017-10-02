import Foundation
import Starscream
import CocoaAsyncSocket

class NetworkSetting {
    
    static var host = "127.0.0.1"
    static var tcpPort = 8000
    static var udpPort = 8001
    
    static func update(host: String, tcpPort: Int, udpPort: Int = tcpPort + 1) {
        self.host = host
        self.tcpPort = tcpPort
        self.udpPort = udpPort
    }
    
    static var address: String {
        return "ws://\(self.host):\(self.tcpPort)/ws"
    }
}

class Network: WebSocketDelegate {
    
    static let shared = Network()

    private var websocket: WebSocket?

    func connect() {
        guard let url = URL(string: NetworkSetting.address) else {
            print("[TCP] could not create url from " + NetworkSetting.address)
            return
        }
        websocket = WebSocket(url: url)
        websocket?.delegate = self
        websocket?.connect()
    }

    func send(_ data: Data) {
        websocket?.write(data: data)
    }
    
    // websocket delegate

    func websocketDidConnect(_ websocket: Starscream.WebSocket) {
        EventBus.post(.connected)
    }

    func websocketDidDisconnect(_ websocket: Starscream.WebSocket, error: NSError?) {
        EventBus.post(.disconnected)
        print("websocket did receive disconnect")
    }

    func websocketDidReceiveData(_ websocket: Starscream.WebSocket, data: Data) {
        WireBackend.shared.didReceiveFromServer(data)
    }

    func websocketDidReceiveMessage(_ socket: WebSocket, text: String) {
        print("websocketDidReceiveMessage")
    }
}

class UDPNetwork: NSObject {
    
    private var udpSocket: GCDAsyncUdpSocket?
    private var isResponsedAUTH = false
    private var isResponsedESTA = false
    
    var postBuffer: Data = Data()
    
    var isUdpSessionEstablished: Bool {
        get {
            return isResponsedAUTH == true && isResponsedESTA == true
        }
    }
    
    func responseAUTH(value: Bool) {
        isResponsedAUTH = value
    }
    
    func responseESTA(value: Bool) {
        isResponsedESTA = value
    }

    func ensureUdpSessionEstablished(completion: (Bool, Bool) -> (Void)) {
        completion(isResponsedAUTH, isResponsedESTA)
    }
    
    func connect() {
        self.udpSocket = GCDAsyncUdpSocket(delegate: self, delegateQueue: DispatchQueue.main)
        
        do {
            try udpSocket?.bind(toPort: 0)
            try udpSocket?.beginReceiving()
        } catch  {
            print(error)
            self.udpSocket = nil
        }
        
    }
    
    func send(_ data: Data) {
        let tag = Int(Date.timeIntervalSinceReferenceDate)
        udpSocket?.send(data, toHost: NetworkSetting.host, port: UInt16(NetworkSetting.udpPort), withTimeout: 30, tag: tag)
    }
}

extension UDPNetwork: GCDAsyncUdpSocketDelegate {
    
    func udpSocket(_ sock: GCDAsyncUdpSocket, didConnectToAddress address: Data) {
        print("UDP didConnectToAddress \(NetworkSetting.host):\(NetworkSetting.udpPort)")
    }
    
    func udpSocket(_ sock: GCDAsyncUdpSocket, didNotConnect error: Error?) {
        print("UDP didNotConnect \(NetworkSetting.host):\(NetworkSetting.udpPort)")
    }
    
    func udpSocket(_ sock: GCDAsyncUdpSocket, didSendDataWithTag tag: Int) {
    }
    
    func udpSocket(_ sock: GCDAsyncUdpSocket, didNotSendDataWithTag tag: Int, dueToError error: Error?) {
        print("UDP didNotSendDataWithTag \(tag). Error: \(String(describing: error?.localizedDescription))")
    }
    
    func udpSocket(
        _ sock: GCDAsyncUdpSocket, didReceive data: Data, fromAddress address: Data, withFilterContext filterContext: Any?) {
        
        let kChunkSize = 1024
        
        // Append data
        postBuffer.append(data)
        
        // Only did receive if not is chunk size
        if data.count != kChunkSize {
            WireBackend.shared.didReceiveFromServer(postBuffer, isUDP: true)
            postBuffer.removeAll()
        }
    }
}
