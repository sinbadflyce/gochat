import Foundation

// for use of Wire.proto
class WireBackend {

    static let shared = WireBackend()
    private let network = Network()
    private let udpNetwork = UDPNetwork()
    private var sessionId: String?
    private var crypto: Crypto?
    private var queues = [String:[Hold]]()
    private var loginType: LoginType?
    
    var isUdpSessionEstablished: Bool {
        let res = udpNetwork.isUdpSessionEstablished
        return res
    }
    
    func connect() {
        network.connect()
        udpNetwork.connect()
    }

    private func send(_ wireBuilder: Wire.Builder) {
        do {
            var annotated = wireBuilder
            if let sessionId = sessionId {
                annotated = annotated.setSessionId(sessionId)
            }
            network.send(try annotated.build().data())
        } catch {
            print(error.localizedDescription)
        }
    }

    private func udpSend(_ wireBuilder: Wire.Builder) {
        do {
            var annotated = wireBuilder
            if let sessionId = sessionId {
                annotated = annotated.setSessionId(sessionId)
            }
            udpNetwork.send(try annotated.build().data())
        } catch {
            print(error.localizedDescription)
        }
    }
    
    // encryption handshake

    func sendPublicKey(_ localPublicKey: Data, to: String, isResponse: Bool) {
        let which: Wire.Which = isResponse ? .publicKeyResponse : .publicKey
        let wireBuilder = Wire.Builder().setPayload(localPublicKey).setWhich(which).setTo(to)
        send(wireBuilder)
    }

    private func didReceivePublicKey(_ haber: Wire) {
        let isResponse = haber.which == .publicKeyResponse
        crypto!.setPublicKey(
            key: haber.payload,
            peerId:haber.from,
            isResponse: isResponse)

        // logging
        print("[KEY] Received public key from \(haber.from)")
    }
    
    func sendHandshake(message: Data, to peerId: String) {
        let wireBuilder = Wire.Builder().setPayload(message).setWhich(.handshake).setTo(peerId)
        send(wireBuilder)
    }

    func handshook(with peerId: String) {
        if var q = queues[peerId] {
            for hold in q {
                encryptAndSend(data: hold.data, peerId: peerId)
            }
            q.removeAll()
            queues[peerId] = q
        }
    }

    func sendUDPEstablished(sessionId: String, from: String) {
        let wire = Wire.Builder().setWhich(.udpEstablished).setSessionId(sessionId).setFrom(from)
        udpSend(wire)
    }
        
    // queue messages while waiting for handshake to complete

    struct Hold {
        var data: Data
        var peerId: String
    }

    private func enqueue(data: Data, peerId: String) {
        let hold = Hold(data: data, peerId: peerId)
        var q = queues[peerId]
        if q == nil {
            queues[peerId] = [hold]
        } else {
            q!.append(hold)
        }
    }
    
    // communication with server

    func didReceiveFromServer(_ data: Data, isUDP: Bool = false) {
        guard let wire = try? Wire.parseFrom(data:data) else {
            print("\(isUDP ? "[UDP]" : "[TCP]") Could not deserialize wire")
            return
        }
        if let sid = wire.sessionId, sessionId == nil {
            authenticated(sessionId: sid)
            Model.shared.getOfficeContactList()
        }

        //-- UDP response
        if wire.which == .loginResponse {
            print("\(isUDP ? "[UDP]" : "[TCP]") read \(data.count) bytes for \(wire.which) from server")
            self.udpNetwork.responseAUTH(value: true)
            return
        }

        //-- UDP response
        if wire.which == .udpEstablished {
            print("\(isUDP ? "[UDP]" : "[TCP]") read \(data.count) bytes for \(wire.which) from server")
            self.udpNetwork.responseESTA(value: true)
            return
        }
        
        if wire.which != .payload {
            print("\(isUDP ? "[UDP]" : "[TCP]") read \(data.count) bytes for \(wire.which) from server")
        }
        do {
            switch wire.which {
            case .contacts:             Model.shared.didReceiveContacts(wire.contacts)
            case .presence:             Model.shared.didReceivePresence(wire)
            case .store:                try didReceiveStore(wire)
            case .handshake:            fallthrough
            case .payload:              crypto!.didReceivePayload(wire.payload, from: wire.from, isUDP: isUDP)
            case .publicKey:            fallthrough
            case .publicKeyResponse:    didReceivePublicKey(wire)
            default:                    print("\(isUDP ? "[UDP]" : "[TCP]") did not handle \(wire.which)")
            }
        } catch {
            print("\(isUDP ? "[UDP]" : "[TCP]") error: \(error.localizedDescription)")
        }
    }
    
    // tell the server to store data
    func sendStore(key: String, value: Data) {
        do {
            guard let encrypted = crypto?.keyDerivationEncrypt(data: value) else {
                print("could not encrypt store")
                return
            }
            let store = try Store.Builder().setKey(key.data(using: .utf8)!).build()
            let wireBuilder = Wire.Builder().setStore(store).setWhich(.store).setPayload(encrypted)
            send(wireBuilder)
        } catch {
            print(error.localizedDescription)
        }
    }

    // request the server to send back stored data
    func sendLoad(key: String) {
        let wireBuilder = Wire.Builder().setWhich(.load).setPayload(key.data(using: .utf8)!)
        send(wireBuilder)
    }

    // the server sent back stored data, due to a .load request
    private func didReceiveStore(_ wire: Wire) throws {
        guard let value = crypto?.keyDerivationDecrypt(ciphertext: wire.payload) else {
            print("could not decrypt store")
            return
        }
        try Model.shared.didReceiveStore(key: wire.store.key, value: value)
    }

    func login(login: Login) {
        loginType = LoginType(rawValue: Int(login.type))
        let wireBuilder = Wire.Builder().setLogin(login).setWhich(.login)
        send(wireBuilder)
        udpSend(wireBuilder)
    }

    private func authenticated(sessionId sid: String) {
        sessionId = sid
        Auth.shared.save()
        crypto = Crypto(password: Auth.shared.password!)
        EventBus.post(.authenticated)
        sendUDPEstablished(sessionId: sessionId!, from: Auth.shared.username!)
    }

    func ensureUdpSessionEstablished() {

        // main thread
        DispatchQueue.main.async {
            
            // firt one more check
            if self.isUdpSessionEstablished {
                return
            }
            
            // ensure Udp Session Established
            self.udpNetwork.ensureUdpSessionEstablished { (isResponsedAUTH: Bool , isResponsedESTA: Bool) -> (Void) in
                
                // resend UDP Authentication
                if !isResponsedAUTH {
                    if let login = Auth.shared.loginInfo {
                        let wireBuilder = Wire.Builder().setLogin(login).setWhich(.login)
                        print("UDP resend login")
                        self.udpSend(wireBuilder)
                    }
                }
                
                // resend UDP Established
                if !isResponsedESTA {
                    if let sid = self.sessionId, let usn = Auth.shared.username {
                        print("UDP established")
                        self.sendUDPEstablished(sessionId: sid, from: usn)
                    }
                }
            }
        }
    }
    
    func sendContacts(_ contacts: [Contact]) {
        let wireBuilder = Wire.Builder().setContacts(contacts).setWhich(.contacts)
        send(wireBuilder)
    }

    func send(data: Data, peerId: String) {
        if crypto!.isSessionEstablished(peerId: peerId) {
            encryptAndSend(data: data, peerId: peerId)
        } else {
            enqueue(data: data, peerId: peerId)
        }
    }

    func udpSend(data: Data, peerId: String) {
        if crypto!.isSessionEstablished(peerId: peerId) {
            encryptAndUdpSend(data: data, peerId: peerId)
        }
    }
    
    private func encryptAndSend(data: Data, peerId: String) {
        guard let encrypted = crypto?.encrypt(data: data, peerId: peerId) else {
            print("encryption failed")
            return
        }
        let payloadBuilder = Wire.Builder().setPayload(encrypted).setWhich(.payload).setTo(peerId)
        send(payloadBuilder)
    }
    
    private func encryptAndUdpSend(data: Data, peerId: String) {
        guard let encrypted = crypto?.encrypt(data: data, peerId: peerId) else {
            print("encryption failed")
            return
        }
        let payloadBuilder = Wire.Builder().setPayload(encrypted).setWhich(.payload).setTo(peerId)
        udpSend(payloadBuilder)
    }
}
