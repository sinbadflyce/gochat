//
//  Office356Logger.swift
//  Chat
//
//  Created by Pham Hoa on 8/21/17.
//  Copyright © 2017 ys1382. All rights reserved.
//

import Foundation
import JWTDecode
import MSAL

class OfficeAuthentication: NSObject, URLSessionDelegate {
    
    //-- SharedIntance
    static let shared = OfficeAuthentication()
    
    //-- Init
    private override init() {
        do {
            // Initialize a MSALPublicClientApplication with a given clientID and authority
            self.applicationContext = try MSALPublicClientApplication.init(clientId: kClientID, authority: kAuthority)
        } catch {
            NSLog("Unable to create Application Context. Error: \(error)")
        }
    }
    
    // Constants
    
    let kClientID = "044e6315-8adc-4dce-9e8e-d9c4d8fef806"
    let kAuthority = "https://login.microsoftonline.com/common/oauth2/v2.0/authorize"
    
    let kGraphURI = "https://graph.microsoft.com/v1.0/me"
    let kContactGraphURI = "https://graph.microsoft.com/beta/me/contacts"
    let kScopes: [String] = ["https://graph.microsoft.com/user.read", "https://graph.microsoft.com/contacts.read"]
    
    // Properties
    
    var accessToken: String?
    var applicationContext = MSALPublicClientApplication.init()
    var user: MSALUser?
    
    func login(_ completedHandler: @escaping ((String?)->())) {
        
        func interactionRequiredToken() {
            self.applicationContext.acquireToken(forScopes: self.kScopes) { (result, error) in
                if error == nil {
                    self.accessToken = (result?.accessToken)!
                    self.user = result?.user
                    NSLog("Access token is \(self.accessToken ?? "nil")")
                    if let idToken = result?.idToken {
                        self.getUserInfo(from: idToken, completedHanlder: completedHandler)
                    }
                } else  {
                    NSLog("Could not acquire token: \(error ?? "No error information" as! Error)")
                }
            }
        }
        
        do {
            // We check to see if we have a current logged in user. If we don't, then we need to sign someone in.
            // We throw an interactionRequired so that we trigger the interactive signin.
            
            if  try self.applicationContext.users().isEmpty {
                throw NSError.init(domain: "MSALErrorDomain", code: MSALErrorCode.interactionRequired.rawValue, userInfo: nil)
            } else {
                
                // Acquire a token for an existing user silently
                
                try self.applicationContext.acquireTokenSilent(forScopes: self.kScopes, user: applicationContext.users().first) { (result, error) in
                    
                    if error == nil {
                        self.accessToken = (result?.accessToken)!
                        self.user = result?.user
                        
                        NSLog("Refreshing token silently")
                        NSLog("Refreshed Access token is \(self.accessToken ?? "nil")")
                        if let idToken = result?.idToken {
                            self.getUserInfo(from: idToken, completedHanlder: completedHandler)
                        }
                    } else {
                        NSLog("Could not acquire token silently: \(error ?? "No error information" as! Error)")
                        if (error! as NSError).code == MSALErrorCode.interactionRequired.rawValue {
                            interactionRequiredToken()
                        }
                    }
                }
            }
        }  catch let error as NSError {
            
            // interactionRequired means we need to ask the user to sign-in. This usually happens
            // when the user's Refresh Token is expired or if the user has changed their password
            // among other possible reasons.
            
            if error.code == MSALErrorCode.interactionRequired.rawValue {
                interactionRequiredToken()
            } else {
            }
            
        } catch {
            // This is the catch all error.
            NSLog("Unable to acquire token. Got error: \(error)")
        }
    }
    
    func signout() {
        accessToken = nil
        do {
            
            // Removes all tokens from the cache for this application for the provided user
            // first parameter:   The user to remove from the cache
            
            try self.applicationContext.remove(self.applicationContext.users().first)
        } catch let error {
            NSLog("Received error signing user out: \(error)")
        }
        user = nil
    }
    
    func getContacts(_ completedHandler: @escaping(([Contact]?)->())) {
        if let url = URL(string: "\(kContactGraphURI)?$top=50") {
            getContacts(url: url, completedHandler)
        } else {
            completedHandler(nil)
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
        
        //        let parameters = [String: AnyObject]()
        //        let parameterString = parameters.stringFromHttpParameters()
        
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
    
    private func getUserInfo(from idToken: String, completedHanlder: @escaping ((String?)->())) {
        if let jwt = try? decode(jwt: idToken) {
            print(jwt.body)
            if var username = jwt.claim(name: "preferred_username").string {
                let patern = "@[A-Za-z]++\\.[A-Za-z]++$"
                if self.matches(for: patern, in: username).count > 0 {
                    username = username.replacingOccurrences(of: patern, with: "", options: .regularExpression, range: nil)
                }
                completedHanlder(username)
            }
        } else {
            completedHanlder(nil)
        }
    }
    
    private func matches(for regex: String, in text: String) -> [String] {
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
