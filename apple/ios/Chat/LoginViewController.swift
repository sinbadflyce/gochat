import UIKit

class LoginViewController: UIViewController {
    @IBOutlet weak var username: UITextField!
    @IBOutlet weak var password: UITextField!

    override func viewDidLoad() {
        EventBus.addListener(about: .authenticated) {_ in 
            self.dismiss(animated: true, completion: {})
        }
    }

    @IBAction func didClickLogin(_ sender: Any) {
        Auth.shared.loginNormal(username: username.text!, password: password.text!)
    }
    
    @IBAction func didClickRegister(_ sender: Any) {
        Auth.shared.register(username: username.text!, password: password.text!)
    }
}
