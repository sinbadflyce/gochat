//
//  AuthenticationWebview.swift
//  Chat
//
//  Created by Pham Hoa on 9/6/17.
//  Copyright © 2017 ys1382. All rights reserved.
//

import Cocoa
import WebKit

class AuthenticationWebview: NSWindowController {

    @IBOutlet var webViewWindow: NSWindow!
    @IBOutlet weak var webView: WKWebView!
    
    static var shared: AuthenticationWebview?

    override func windowDidLoad() {
        super.windowDidLoad()
    
        // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    }

    static func show() {
        AuthenticationWebview.shared = AuthenticationWebview(windowNibName: "Webview")
        AuthenticationWebview.shared?.showWindow(nil)
        AuthenticationWebview.shared?.webViewWindow.makeKey()
    }
    
    static func closeWindow() {
        AuthenticationWebview.shared?.close()
        AuthenticationWebview.shared = nil
    }
}
