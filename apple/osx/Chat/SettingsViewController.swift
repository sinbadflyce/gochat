
import Cocoa
import AVFoundation

class SettingsViewController : NSViewController {
    @IBOutlet weak var textFieldServerIP: NSTextField!
    @IBOutlet weak var textFieldPort: NSTextField!
    @IBOutlet weak var textFieldUdpPort: NSTextField!
    @IBOutlet weak var textFieldVideoWidth: NSTextField!
    @IBOutlet weak var textFieldVideoHeight: NSTextField!
    
    func updateVideo(_ format: AVCaptureDeviceFormat) {
        AV.shared.defaultVideoDimension = format.dimensions
        
        textFieldVideoWidth.stringValue = String(AV.shared.defaultVideoDimension!.width)
        textFieldVideoHeight.stringValue = String(AV.shared.defaultVideoDimension!.height)
        
        UserDefaults.standard.set(textFieldVideoWidth.stringValue, forKey: AppDelegate.kVideoWidth)
        UserDefaults.standard.set(textFieldVideoHeight.stringValue, forKey: AppDelegate.kVideoHeight)
        UserDefaults.standard.synchronize()
    }
    
    override func viewDidLoad() {
        textFieldServerIP.stringValue = NetworkSetting.host
        textFieldPort.stringValue = String(NetworkSetting.tcpPort)
        textFieldUdpPort.stringValue = String(NetworkSetting.udpPort)
        
        if let x = AV.shared.defaultVideoDimension {
            textFieldVideoWidth.stringValue = String(x.width)
            textFieldVideoHeight.stringValue = String(x.height)
        }
    }
    
    @IBAction func btnRestartAction(_ sender: Any) {
        textFieldServerIPAction(self)
        textFieldTcpPortAction(self)
        textFieldUdpPortAction(self)
        textFieldVideoWidthAction(self)
        
        let url = URL(fileURLWithPath: Bundle.main.resourcePath!)
        let path = url.deletingLastPathComponent().deletingLastPathComponent().absoluteString
        let task = Process()
        task.launchPath = "/usr/bin/open"
        task.arguments = [path]
        task.launch()
        exit(0)
    }
    
    @IBAction func textFieldServerIPAction(_ sender: Any) {
        UserDefaults.standard.set(textFieldServerIP.stringValue, forKey: AppDelegate.kTcpHost)
    }
    
    @IBAction func textFieldTcpPortAction(_ sender: Any) {
        UserDefaults.standard.set(textFieldPort.stringValue, forKey: AppDelegate.kTcpPort)
    }

    @IBAction func textFieldUdpPortAction(_ sender: Any) {
        UserDefaults.standard.set(textFieldUdpPort.stringValue, forKey: AppDelegate.kUdpPort)
    }

    @IBAction func textFieldVideoWidthAction(_ sender: Any) {
        let text = textFieldVideoWidth.stringValue
        guard let width = Int32(text) else { return }
        guard let format = AV.shared.defaultVideoInputDevice?.inputFormat(width: width) else { return }
        
        updateVideo(format)
    }
    
    @IBAction func textFieldVideoHeightAction(_ sender: Any) {
        let text = textFieldVideoHeight.stringValue
        guard let height = Int32(text) else { return }
        guard let format = AV.shared.defaultVideoInputDevice?.inputFormat(height: height) else { return }
        
        updateVideo(format)
    }

}
