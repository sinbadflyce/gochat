import Foundation
import Starscream
import CocoaAsyncSocket

class Network: WebSocketDelegate {

    //static var address = "ws://sinbadflyce.com:8000/ws"
    static var address = "ws://192.168.2.135:8000/ws"

    static let shared = Network()

    private var websocket: WebSocket?

    func connect() {
        guard let url = URL(string: Network.address) else {
            print("could not create url from " + Network.address)
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
    
    //static var host = "sinbadflyce.com"
    static var host = "192.168.2.135"
    static var port: UInt16 = 8001
    
    private var udpSocket: GCDAsyncUdpSocket?
    
    var postBuffer: Data = Data()
    
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
        udpSocket?.send(data, toHost: UDPNetwork.host, port: UDPNetwork.port, withTimeout: 30, tag: tag)
    }
}

extension UDPNetwork: GCDAsyncUdpSocketDelegate {
    
    func udpSocket(_ sock: GCDAsyncUdpSocket, didConnectToAddress address: Data) {
        print("UDP didConnectToAddress \(UDPNetwork.host):\(UDPNetwork.port)")
    }
    
    func udpSocket(_ sock: GCDAsyncUdpSocket, didNotConnect error: Error?) {
        print("UDP didNotConnect \(UDPNetwork.host):\(UDPNetwork.port)")
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
            WireBackend.shared.didReceiveFromServer(postBuffer)
            postBuffer.removeAll()
        }
    }
}
