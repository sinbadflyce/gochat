import UIKit
import Starscream

class DetailViewController: UIViewController {

    @IBOutlet weak var detailDescriptionLabel: UILabel!
    @IBOutlet weak var input: UITextField!
    @IBOutlet weak var transcript: UITextView!
    @IBOutlet weak var videoBarButtonItem: UIBarButtonItem!
    @IBOutlet weak var audioBarButtonItem: UIBarButtonItem!
    @IBOutlet weak var viewInputBottomConstraint: NSLayoutConstraint!
    
    var callInfo: NetworkCallInfo? {
        didSet {
            if callInfo == nil {
                audioBarButtonItem.tintColor = nil
                videoBarButtonItem.tintColor = nil
                audioBarButtonItem.isEnabled = true
                videoBarButtonItem.isEnabled = true
            } else if callInfo!.proposal.video {
                videoBarButtonItem.tintColor = UIColor.red
            } else if callInfo!.proposal.audio {
                audioBarButtonItem.tintColor = UIColor.red
            }
        }
    }

    @IBAction func sendClicked(_ sender: Any) {
        guard let body = input.text, let whom = Model.shared.watching else {
            print("could not create Text")
            return
        }
        
        Model.shared.addText(body: body.data(using: .utf8)!, from: Auth.shared.username!, to: whom)
        VoipBackend.sendText(body, peerId: whom)
        input.text = ""
    }

    @IBAction func videoBarButtonItemAction(_ sender: UIBarButtonItem) {
        if stopCallIfNeeded(videoBarButtonItem) == false {
            _ = callVideoAsync(Model.shared.watching!)
        }
    }

    @IBAction func audioBarButtonItemAction(_ sender: UIBarButtonItem) {
        if stopCallIfNeeded(audioBarButtonItem) == false {
            _ = callAudioAsync(Model.shared.watching!)
        }
    }
    
    private func stopCallIfNeeded(_ button: UIBarButtonItem) -> Bool {
        guard button.tintColor == UIColor.red else { return false }

        stopCallAsync(self.callInfo!)
        callInfo = nil
        
        return true
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        if let id = Model.shared.watching {
            self.title = Model.shared.roster[id]?.name
        } else {
            self.title = ""
        }
        self.updateTranscript()
       
        EventBus.addListener(about: .text) { notification in
            self.updateTranscript()
        }
        
        let tap = UITapGestureRecognizer.init(target: self, action: #selector(dismissKeyboard))
        tap.cancelsTouchesInView = false
        self.transcript.addGestureRecognizer(tap)
        
        NotificationCenter.default.addObserver(self, selector: #selector(self.keyboardNotification(notification:)), name: NSNotification.Name.UIKeyboardWillChangeFrame, object: nil)
    }

    override func viewDidDisappear(_ animated: Bool) {
        
        // Bug
        if self.navigationController?.viewControllers.contains(self) == false {
            Model.shared.watching = nil
        }
    }

    private func updateTranscript() {
        if let whom = Model.shared.watching {
            let textsFiltered = Model.shared.texts
                .filter({ text in text.to == whom || text.from == whom })
            let textsReduced = textsFiltered.reduce("", { sum, text in sum + lineOf(text) } )
            transcript.text = textsReduced
        }
    }
    
    private func lineOf(_ text: Text) -> String {
        return text.from + ": " + String(data: text.body, encoding: .utf8)!  + "\n"
    }
    
    @objc private func dismissKeyboard() {
        self.view.endEditing(true)
    }
    
    @objc func keyboardNotification(notification: NSNotification) {
        if let userInfo = notification.userInfo {
            let endFrame = (userInfo[UIKeyboardFrameEndUserInfoKey] as? NSValue)?.cgRectValue
            let duration:TimeInterval = (userInfo[UIKeyboardAnimationDurationUserInfoKey] as? NSNumber)?.doubleValue ?? 0
            let animationCurveRawNSN = userInfo[UIKeyboardAnimationCurveUserInfoKey] as? NSNumber
            let animationCurveRaw = animationCurveRawNSN?.uintValue ?? UIViewAnimationOptions.curveEaseInOut.rawValue
            let animationCurve:UIViewAnimationOptions = UIViewAnimationOptions(rawValue: animationCurveRaw)
            if (endFrame?.origin.y)! >= UIScreen.main.bounds.size.height {
                self.viewInputBottomConstraint.constant = 0
            } else {
                self.viewInputBottomConstraint.constant = endFrame?.size.height ?? 0
            }
            UIView.animate(withDuration: duration,
                           delay: TimeInterval(0),
                           options: animationCurve,
                           animations: { self.view.layoutIfNeeded() },
                           completion: nil)
        }
    }
}
