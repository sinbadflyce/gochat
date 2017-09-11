//
//  Office356Logger.swift
//  Chat
//
//  Created by Pham Hoa on 8/21/17.
//  Copyright © 2017 ys1382. All rights reserved.
//

import Foundation
import JWTDecode
import NXOAuth2Client
import WebKit

class OfficeAuthentication: NSObject, URLSessionDelegate, WKNavigationDelegate {
    
    //-- SharedIntance
    static let shared = OfficeAuthentication()

    // Constants
    
    let kClientID = "d6a01562-2f48-4634-a4e2-0676e57598de"
    let kSecret = "viH8zWm1FCH6cj01bqe9N25"
    let kAuthority = "https://login.microsoftonline.com/common/oauth2/v2.0/authorize"
    let kTokenUrl = "https://login.microsoftonline.com/common/oauth2/v2.0/token"
    let kResource = "https://graph.windows.net"
    let kRedirectUri = "Chat://ys1382.Chat"
    let kGraphURI = "https://graph.microsoft.com/v1.0/me"
    let kContactGraphURI = "https://graph.microsoft.com/beta/me/contacts"
    let kScopes: [String] = ["https://graph.microsoft.com/user.read", "https://graph.microsoft.com/contacts.read"]
    
    // Properties
    
    var account: NXOAuth2Account? {
        get {
            return (NXOAuth2AccountStore.sharedStore() as? NXOAuth2AccountStore)?.accounts.first as? NXOAuth2Account
        }
    }
    
    var accessToken: String? {
        get {
            let acessToken = account?.accessToken.accessToken
            return acessToken
        }
    }
    
    var authenticatedHandler: ((String?)->())?

    //-- Init
    private override init() {
        (NXOAuth2AccountStore.sharedStore() as? NXOAuth2AccountStore)?.setClientID(kClientID, secret: "", scope: Set.init(kScopes), authorizationURL: URL.init(string: kAuthority)!, tokenURL: URL.init(string: kTokenUrl)!, redirectURL: URL.init(string: kRedirectUri)!, keyChainGroup: "com.microsoft.azureactivedirectory.samples.graph.QuickStart", forAccountType: "myGraphService")
    }
    
    private func addObserver() {
        // Remove current Observer
        NotificationCenter.default.removeObserver(self)

        // Add new Observer
        NotificationCenter.default.addObserver(self, selector: #selector(NXOAuth2AccountStoreAccountsDidChange(_:)), name: NSNotification.Name.NXOAuth2AccountStoreAccountsDidChange, object: NXOAuth2AccountStore.sharedStore())
        NotificationCenter.default.addObserver(self, selector: #selector(NXOAuth2AccountStoreDidFailToRequestAccess(_:)), name: NSNotification.Name.NXOAuth2AccountStoreDidFailToRequestAccess, object: NXOAuth2AccountStore.sharedStore())
    }
    
    
    func login(_ completedHandler: ((String?)->())?) {
        addObserver()
        authenticatedHandler = completedHandler
        if let oauthStore = NXOAuth2AccountStore.sharedStore() as? NXOAuth2AccountStore {
            let configuration = NSMutableDictionary.init(dictionary: oauthStore.configuration(forAccountType: "myGraphService"))
            let customHeaderFields = ["Content-Type" : "application/x-www-form-urlencoded"]
            configuration.setValue(customHeaderFields, forKey: kNXOAuth2AccountStoreConfigurationCustomHeaderFields)
            oauthStore.setConfiguration(configuration as! [AnyHashable : Any], forAccountType: "myGraphService")
            
            oauthStore.requestAccessToAccount(withType: "myGraphService") { (url) in
                if let url = url {
                    let urlRequest = URLRequest.init(url: url)
                    AuthenticationWebview.show()
                    AuthenticationWebview.shared?.webView.load(urlRequest)
                    AuthenticationWebview.shared?.webView.navigationDelegate = self
                }
            }
        }
    }
    
    func signout() {
        if account != nil {
            (NXOAuth2AccountStore.sharedStore() as? NXOAuth2AccountStore)?.removeAccount(account)
        }
    }
    
    func getContacts(_ completedHandler: @escaping(([Contact]?)->())) {
        if let url = URL(string: "\(kContactGraphURI)?$top=50") {
            getContacts(url: url, completedHandler)
        } else {
            completedHandler(nil)
        }
    }
    
    
    // Private
    
    @objc private func NXOAuth2AccountStoreAccountsDidChange(_ aNotification: Notification) {
        AuthenticationWebview.closeWindow()
        if let info = aNotification.userInfo {
            if let error = info[NXOAuth2AccountStoreErrorKey] as? NSError {
                print(error.localizedDescription)
            } else {
                print("Success!! We have an access token.")
                DispatchQueue.main.asyncAfter(deadline: .now() + 2, execute: { 
                    self.getUserInfo()
                })
            }
        }
    }
    
    @objc private func NXOAuth2AccountStoreDidFailToRequestAccess(_ aNotification: Notification) {
        AuthenticationWebview.closeWindow()
        if let error = aNotification.userInfo?[NXOAuth2AccountStoreErrorKey] as? NSError {
            print(error.localizedDescription)
        }
    }
    
    private func getContacts(url: URL,_ completedHandler: @escaping(([Contact]?)->())) {
        struct Holder {
            static var contactsList = [Contact]()
        }
        
        if self.accessToken == nil {
            completedHandler(nil)
            return
        }
        
        let sessionConfig = URLSessionConfiguration.default
        
        // Specify the Graph API endpoint
        var request = URLRequest(url: url)
        
        // Set the Authorization header for the request. We use Bearer tokens, so we specify Bearer + the token we got from the result
        request.setValue("Bearer \(self.accessToken!)", forHTTPHeaderField: "Authorization")
        let urlSession = URLSession(configuration: sessionConfig, delegate: self, delegateQueue: OperationQueue.main)
        
        urlSession.dataTask(with: request) { data, response, error in
            let result = try? JSONSerialization.jsonObject(with: data!, options: [])
            if result != nil, let dict = result as? [String: AnyObject] {
                print(dict)
                
                if let contacts = dict["value"] as? [[String: AnyObject]] {
                    
                    for contactDict in contacts {
                        let contactBuilder = Contact.Builder()
                        
                        if var nickName = contactDict["nickName"] as? String {
                            let patern = "^live:"
                            if self.matches(for: patern, in: nickName).count > 0 {
                                nickName = nickName.replacingOccurrences(of: patern, with: "", options: .regularExpression, range: nil)
                            }
                            contactBuilder.setId(nickName).setName(nickName)
                            if let contact = try? contactBuilder.build() {
                                Holder.contactsList.append(contact)
                            }
                        }
                    }
                }
                
                // recursive
                if let nextLink = dict["@odata.nextLink"] as? String, let url = URL(string: nextLink) {
                    self.getContacts(url: url, completedHandler)
                } else {
                    completedHandler(Holder.contactsList)
                }
            }
        }.resume()
    }
    
    private func getUserInfo() {
        if self.accessToken == nil {
            return
        }
        
        let sessionConfig = URLSessionConfiguration.default
        
        // Specify the Graph API endpoint
        guard let url = URL.init(string: kGraphURI) else {
            return
        }
        
        var request = URLRequest(url: url)
        
        // Set the Authorization header for the request. We use Bearer tokens, so we specify Bearer + the token we got from the result
        request.setValue("Bearer \(self.accessToken!)", forHTTPHeaderField: "Authorization")
        let urlSession = URLSession(configuration: sessionConfig, delegate: self, delegateQueue: OperationQueue.main)
        
        urlSession.dataTask(with: request) { data, response, error in
            let result = try? JSONSerialization.jsonObject(with: data!, options: [])
            if result != nil, let dict = result as? [String: AnyObject] {
                print(dict)
                if let email = dict["userPrincipalName"] as? String {
                    let patern = "@[A-Za-z]++\\.[A-Za-z]++$"
                    var username = email
                    if self.matches(for: patern, in: username).count > 0 {
                        username = username.replacingOccurrences(of: patern, with: "", options: .regularExpression, range: nil)
                    }
                    self.authenticatedHandler?(username)
                }
            }
            }.resume()
    }
    
    // MARK: WKNavigationDelegate
    
    func webView(_ webView: WKWebView, decidePolicyFor navigationAction: WKNavigationAction, decisionHandler: @escaping (WKNavigationActionPolicy) -> Swift.Void) {
        decisionHandler(WKNavigationActionPolicy.allow)
    }
    
    func webView(_ webView: WKWebView, didReceiveServerRedirectForProvisionalNavigation navigation: WKNavigation!) {
        if let stringUrl = webView.url?.absoluteString, (stringUrl as NSString).range(of: kRedirectUri, options: .caseInsensitive).location != NSNotFound {
            (NXOAuth2AccountStore.sharedStore() as? NXOAuth2AccountStore)?.handleRedirectURL(webView.url!)
        }
    }
    
    func webView(_ webView: WKWebView, decidePolicyFor navigationResponse: WKNavigationResponse, decisionHandler: @escaping (WKNavigationResponsePolicy) -> Swift.Void) {
        if let response = navigationResponse.response as? HTTPURLResponse {
            if response.statusCode == 200 {
                decisionHandler(WKNavigationResponsePolicy.allow)
            } else {
                decisionHandler(WKNavigationResponsePolicy.allow)
            }
        } else {
            decisionHandler(WKNavigationResponsePolicy.allow)
        }
    }
    
    func matches(for regex: String, in text: String) -> [String] {
        do {
            let regex = try NSRegularExpression(pattern: regex)
            let nsString = text as NSString
            let results = regex.matches(in: text, range: NSRange(location: 0, length: nsString.length))
            return results.map { nsString.substring(with: $0.range)}
        } catch let error {
            print("invalid regex: \(error.localizedDescription)")
            return []
        }
    }
}
