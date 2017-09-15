import Cocoa

@NSApplicationMain
class AppDelegate: Application, NSApplicationDelegate {

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        EventBus.addListener(about: .connected, didReceive: { notification in
            if !Auth.shared.login() {
                LoginViewController.popup()
            }
        })

        EventBus.addListener(about: .disconnected, didReceive: { notification in
            let alert = NSAlert()
            alert.messageText = "Disconnected"
            alert.informativeText = "Not connected to server"
            alert.addButton(withTitle: "Ok")
            alert.runModal()
        })

        WireBackend.shared.connect()
        
        // Register notification
        NSApp.registerForRemoteNotifications(matching: .alert)
    }
    
    static func ask(title: String, subtitle: String, cancelable: Bool, done:(String?)->Void) {
        let alert = NSAlert()
        
        alert.addButton(withTitle: "OK")
        if cancelable {
            alert.addButton(withTitle: "Cancel")
        }
        
        alert.messageText = title
        alert.informativeText = subtitle
        
        let textField = NSTextField(frame: NSRect(x: 0, y: 0, width: 200, height: 24))
        alert.accessoryView = textField
        alert.window.initialFirstResponder = textField
        
        let ok = alert.runModal() == NSAlertFirstButtonReturn
        done(ok ? textField.stringValue : nil)
    }
    
    func application(_ application: NSApplication, didRegisterForRemoteNotificationsWithDeviceToken deviceToken: Data) {
        // Convert token to string
        let deviceTokenString = deviceToken.reduce("", {$0 + String(format: "%02X", $1)})
        
        // Print it to console
        print("APNs device token: \(deviceTokenString)")
        UserDefaults.standard.set(deviceTokenString, forKey: Constant.kDeviceTokenKey)
    }
    
    func application(_ application: NSApplication, didFailToRegisterForRemoteNotificationsWithError error: Error) {
        // Print the error to console (you should alert the user that registration failed)
        print("APNs registration failed: \(error)")
        UserDefaults.standard.set(nil, forKey: Constant.kDeviceTokenKey)
    }
    
    func application(_ application: NSApplication, didReceiveRemoteNotification userInfo: [String : Any]) {
//        if let apsDictionary = userInfo["aps"] as? NSDictionary {
//
//        }
    }
}
