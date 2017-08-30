import Foundation
import Starscream

class Model {

    static let shared = Model()
    static let textsKey = "texts"
    var roster = [String:Contact]()
    var texts = [Text]()
    var unreads = [String:Int]()
    var watching: String? {
        didSet {
            if let watching = watching {
                unreads[watching] = 0
            }
        }
    }

    private enum ModelError: Error {
        case keyNotHandled
    }

    private init() {
        EventBus.addListener(about: .authenticated, didReceive: { notification in
            WireBackend.shared.sendLoad(key: Model.textsKey)
        })
    }

    func didReceivePresence(_ wire: Wire) {
        for contact in wire.contacts {
            roster[contact.id] = contact
        }
        EventBus.post(about:.presence)
    }

    func didReceiveText(body: Data, from peerId: String) {
        if peerId != watching {
            unreads[peerId] = (unreads[peerId] ?? 0) + 1
        }
        
        addText(body: body, from: peerId, to: Auth.shared.username!)
    }

    func addText(body: Data, from: String, to: String) {
        do {
            let text = try Text.Builder().setTo(to).setFrom(from).setBody(body).build()
            texts.append(text)
            storeText()
            EventBus.post(.text)
        } catch {
            print(error.localizedDescription)
        }
    }

    private func storeText() {
        do {
            let storage = try Voip.Builder().setTextStorage(texts).build().data()
            WireBackend.shared.sendStore(key: Model.textsKey, value: storage)
        } catch {
            print(error.localizedDescription)
        }
    }

    func didReceiveContacts(_ contacts: [Contact]) {
        _ = self.syncContacts(with: contacts)
        EventBus.post(about:.contacts)
    }
    
    func getOfficeContactList() {
        if Auth.shared.loginType == .office {
            OfficeAuthentication.shared.getContacts({ (contacts) in
                if let contacts = contacts {
                    if self.syncContacts(with: contacts) {
                        // send to server if there is diffrence between current and new contact
                        WireBackend.shared.sendContacts(Array(self.roster.values))
                    }
                }
            })
        }
    }

    // return true if there is diffrence between current and new contact
    func syncContacts(with newContacts: [Contact]) -> Bool {
        var isNotSame = false
        
        for contact in newContacts {
            if !roster.contains(where: { $0.key == contact.id }) {
                roster[contact.id] = contact
                isNotSame = true
            }
        }
        EventBus.post(about:.contacts)
        return isNotSame
    }
    
    func didReceiveStore(key: Data, value: Data) throws {
        guard key == "texts".data(using: .utf8) else {
            throw ModelError.keyNotHandled
        }
        let parsed = try Voip.parseFrom(data: value)
        texts = parsed.textStorage
        EventBus.post(.texts)
    }

    func setContacts(_ ids: [String]) {
        var update = [String:Contact]()
        for id in ids {
            if let existing = roster[id] {
                update[id] = existing
            } else {
                // To-do: create new id base on name
                update[id] = try? Contact.Builder().setId(id).setName(id).build()
            }
        }
        roster = update
        WireBackend.shared.sendContacts(Array(roster.values))
    }
}
