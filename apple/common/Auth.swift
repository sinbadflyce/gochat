import Foundation

enum LoginType: Int {
    case normal = 1
    case office = 2
}

// placeholder until we have real authentication
class Auth {

    static let shared = Auth()
    var loginType: LoginType?
    var username: String?
    var password: String?
    var token: String?
    
    private static let usernameKey  = "username"
    private static let passwordKey  = "password"
    private static let tokenKey     = "token"
    private static let loginTypeKey = "loginType"

    init() {
        username = UserDefaults.standard.string(forKey: Auth.usernameKey)
        password = UserDefaults.standard.string(forKey: Auth.passwordKey)
        token = UserDefaults.standard.string(forKey: Auth.tokenKey)
        let type = UserDefaults.standard.integer(forKey: Auth.loginTypeKey)
        loginType = LoginType.init(rawValue: type)
    }

    func save() {
        UserDefaults.standard.set(username, forKey: Auth.usernameKey)
        UserDefaults.standard.set(password, forKey: Auth.passwordKey)
        UserDefaults.standard.set(token, forKey: Auth.tokenKey)
        UserDefaults.standard.set(loginType?.rawValue ?? 0, forKey: Auth.loginTypeKey)
    }

    func login() -> Bool {
        if token != nil {
            loginToServer(type: .office)
            return true
        } else if username != nil {
            loginToServer(type: .normal)
            return true
        } else {
            return false
        }
    }
    
    func loginNormal(username: String, password: String) {
        self.username = username
        self.password = password
        self.loginToServer(type: .normal)
    }
    
    func loginWithOffice() {
        OfficeAuthentication.shared.login { (nickName) in
            self.password = nickName
            self.username = nickName
            self.token = OfficeAuthentication.shared.accessToken
            
            self.loginToServer(type: .office)
        }
    }

    func register(username: String, password: String) {
        loginNormal(username: username, password: password)
    }
    
    private func loginToServer(type: LoginType) {
        loginType = type
        let loginBuilder = Login.Builder()
        loginBuilder.setType(UInt32(type.rawValue))
        
        if username != nil {
            loginBuilder.setUserName(username!)
        }
        
        if token != nil {
            loginBuilder.setAuthenToken(token!)
        }
        
        
        // fake
        loginBuilder.setDeviceToken("abcd")
//        if let deviceToken = UserDefaults.standard.string(forKey: Constant.kDeviceTokenKey) {
//            loginBuilder.setDeviceToken(deviceToken)
//        }
        
        do {
            let login = try loginBuilder.build()
            WireBackend.shared.login(login: login)
        } catch {
            //
        }
    }
    
    func clearUser() {
        username = nil
        password = nil
        token = nil
        save()
    }

    private func authenticated(sessionId sid: String) {
        save()
    }
}
