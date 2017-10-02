import Foundation

class Crypto {

    private let cellSeal: TSCellSeal
    private let localPublicKey: Data
    private let localPrivateKey: Data
    private var peers = [String:Peer]()

    init(password: String) {
        let key = password.data(using: .utf8)!
        cellSeal = TSCellSeal(key: key)

        let keyGeneratorEC: TSKeyGen = TSKeyGen(algorithm: .EC)!
        localPrivateKey = keyGeneratorEC.privateKey as Data
        localPublicKey = keyGeneratorEC.publicKey as Data
    }

    func keyDerivationEncrypt(data: Data) -> Data? {
        do {
            return try cellSeal.wrap(data, context: nil)
        } catch let error as NSError {
            print(error.localizedDescription)
            return nil
        }
    }

    func keyDerivationDecrypt(ciphertext: Data) -> Data? {
        do {
            return try cellSeal.unwrapData(ciphertext, context: nil)
        } catch let error as NSError {
            print("Error occurred while decrypting \(error)", #function)
            return nil
        }
    }

    func isSessionEstablished(peerId: String) -> Bool {
        let peer = self.peer(peerId)
        if peer.status == .begun {
            peer.sendPublicKey(isResponse: false)
        }
        return peer.status == .sessionEstablished
    }

     private func peer(_ peerId: String) -> Peer {
        if peers[peerId] == nil {
            let peer = Peer(peerId: peerId)
            peers[peerId] = peer
        }
        return peers[peerId]!
    }

    func setPublicKey(key: Data, peerId: String, isResponse: Bool) {
        let peer = self.peer(peerId)
        peer.setServerPublicKey(key: key, isResponse: isResponse)
    }

    func resendPublicKey(peerId: String, isResponse: Bool) {
        let peer = self.peer(peerId)
        peer.sendPublicKey(isResponse: isResponse)
    }
    
    func didReceivePayload(_ payload: Data, from peerId: String, isUDP: Bool = false) {
        peer(peerId).didReceive(payload, isUDP: isUDP)
    }

    func encrypt(data: Data, peerId: String) -> Data? {
        let peer = self.peer(peerId)
        return peer.encrypt(data)
    }
}

private class Transport: TSSessionTransportInterface {
    private var serverId: String?
    private var serverPublicKeyData: Data?

    func setupKeys(_ serverId: String, serverPublicKey: Data) {
        self.serverId = serverId
        serverPublicKeyData = serverPublicKey
    }

    override func publicKey(for binaryId: Data!) throws -> Data {
        if serverId!.data(using: .utf16)! != binaryId {
            print("mismatch")
        } else {
            print("retrieved public key for \(String(describing: String(data: binaryId!, encoding: String.Encoding.utf16)!))")
        }
        return serverPublicKeyData!
    }
}

private class Peer {

    enum Status {
        case begun
        case publicKeySent
        case sessionEstablished
    }
    var status: Status = .begun

    var transport = Transport()
    var session: TSSession?
    var clientIdData: Data
    var peerId: String
    var clientPrivateKey: Data? = nil
    var clientPublicKey: Data? = nil

    init(peerId: String) {
        clientIdData = Auth.shared.username!.data(using: .utf8)!
        self.peerId = peerId

        guard let keyGeneratorEC: TSKeyGen = TSKeyGen(algorithm: .EC) else {
            print("Error occurred while initialising object keyGeneratorEC", #function)
            return
        }
        clientPrivateKey = keyGeneratorEC.privateKey as Data
        clientPublicKey = keyGeneratorEC.publicKey as Data
    }

    func setServerPublicKey(key: Data, isResponse: Bool) {
        transport.setupKeys(peerId, serverPublicKey: key)
        session = TSSession(userId: clientIdData, privateKey: clientPrivateKey, callbacks: transport)
        print("[CRYPTO] create crypto session on peerId: \(peerId)")
        if isResponse {
            connect()
        } else {
            sendPublicKey(isResponse: true)
        }
    }

    func sendPublicKey(isResponse: Bool) {
        
        // logging
        isResponse ? print("[KEY] response public key to \(peerId)") : print("[KEY] send public key to \(peerId)")
        
        // send or response
        status = .publicKeySent
        WireBackend.shared.sendPublicKey(clientPublicKey!, to: peerId, isResponse: isResponse)
    }

    func connect() {
        do {
            guard let message = try session?.connectRequest() else {
                print("[CRYPTO] error: could not connectRequest")
                return
            }
            
            print("[CRYPTO] success to connect crypto session on peerId: \(peerId)")
            WireBackend.shared.sendHandshake(message: message, to: peerId)
        } catch {
            print("[CRYPTO] error: \(error.localizedDescription)")
        }
    }

    func didReceive(_ data: Data, isUDP: Bool = false) {
        
        guard let s =  session else {
            print("\(isUDP ? "[UDP]" : "[TCP]") Peer [\(self.peerId)] has session being null")
            self.status = .begun
            return
        }
        
        // logging status
        if status != .sessionEstablished {
            print("\(isUDP ? "[UDP]" : "[TCP]") status is \(status)")
        }
        
        do {
            let decryptedMessage = try s.unwrapData(data)
            if !session!.isSessionEstablished() { // themis says: send this back
                print("\(isUDP ? "[UDP]" : "[TCP]") themis says: send this back")
                WireBackend.shared.sendHandshake(message: decryptedMessage, to: peerId)
            } else if status != .sessionEstablished { // themis says: session now established
                print("\(isUDP ? "[UDP]" : "[TCP]") themis says: session now established")
                didEstablishSession(sendThisToo: decryptedMessage)
            } else { // themis says: here is the decrypted message
                //print("themis says: here is the decrypted message")
                VoipBackend.didReceiveFromPeer(decryptedMessage, from: peerId)
            }
        } catch {
            if let session = session, session.isSessionEstablished() {
                print("\(isUDP ? "[UDP]" : "[TCP]") session in error: \(error.localizedDescription)")
                print("\(isUDP ? "[UDP]" : "[TCP]") themis says: session now established 2")
                didEstablishSession() // themis says: session now established (it can happen this way too)
            } else {
                print(error.localizedDescription)
            }
        }
    }

    private func didEstablishSession(sendThisToo: Data? = nil) {
        status = .sessionEstablished
        if let message = sendThisToo {
            WireBackend.shared.sendHandshake(message: message, to: peerId)
        }
        WireBackend.shared.handshook(with: peerId)
    }

    func encrypt(_ data: Data) -> Data? {
        do {
            guard let wrappedMessage: Data = try self.session?.wrap(data) else {
                print("Error occurred during wrapping message ", #function)
                return nil
            }
            return wrappedMessage
        } catch let error as NSError {
            print("Error occurred while wrapping message \(error)", #function)
            return nil
        }
    }
}
