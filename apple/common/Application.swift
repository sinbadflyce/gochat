
import Foundation
import CoreMedia

class Application : AppleApplicationDelegate {

    static let kCompressedPlayback = false
    static let kUncompressedPlayback = false
    
    static let kTcpHost = "kTcpHost"
    static let kTcpPort = "kTcpPort"
    static let kUdpPort = "kUdpPort"
    static let kVideoWidth = "kVideoWidth"
    static let kVideoHeight = "kVideoHeight"

    var playback: IOSessionProtocol?
    
    override init() {

        // start time
        
        _ = HostTimeInfo.shared
        
        // crash on unhandled exceptions
        
        UserDefaults.standard.register(defaults: ["NSApplicationCrashOnExceptions": true]);
        
        // server: host, tcp and udp ports
        let kTcpHost = UserDefaults.standard.string(forKey: Application.kTcpHost)
        let kTcpPort = UserDefaults.standard.string(forKey: Application.kTcpPort)
        let kUdpPort = UserDefaults.standard.string(forKey: Application.kUdpPort)
        
        if let host = kTcpHost, let sTcpPort = kTcpPort, let tcpPort = Int(sTcpPort),
            let sUdpPort = kUdpPort, let udpPort = Int(sUdpPort)  {
            NetworkSetting.update(host: host, tcpPort: tcpPort, udpPort: udpPort)
            print("[CK] Launch app with user setting, host = \(host), tcpPort = \(sTcpPort), udpPort = \(sUdpPort)")
        } else {
            print("[CK] Launch app with default host, ports")
        }
        
        // video dimension
        
        let videoWidth = UserDefaults.standard.string(forKey: Application.kVideoWidth)
        let videoHeight = UserDefaults.standard.string(forKey: Application.kVideoHeight)
        
        if videoWidth != nil && videoHeight != nil {
            AV.shared.defaultVideoDimension = CMVideoDimensions(width: Int32(videoWidth!)!,
                                                                height: Int32(videoHeight!)!)
        }
        
        // playback testing
        
        var playback: IOSessionProtocol?

        checkIO {
            
            if Application.kCompressedPlayback {
                playback = try AV.shared.audioCompressedPlayback()
            }
            
            if Application.kUncompressedPlayback {
                playback = try AV.shared.audioUncompressedPlayback()
            }
            
            try playback?.start()
        }
        
        self.playback = playback
    }
}
