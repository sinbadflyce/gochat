import Foundation
import RNCryptor
import SignalProtocolC
//import SignalProtocolObjC

class Crypto {

    private let password: String
    private static var lock = NSRecursiveLock()

    init(password: String) {
        self.password = password
        setupSignal()
    }

    func keyDerivationEncrypt(data: Data) -> Data {
        return RNCryptor.encrypt(data: data, withPassword: password)
    }

    func keyDerivationDecrypt(ciphertext: Data) -> Data? {
        do {
            return try RNCryptor.decrypt(data: ciphertext, withPassword: password)
        } catch {
            print(error)
            return nil
        }
    }

    func signalSetup() {

//        public init(sessionStore: SignalSessionStore, preKeyStore: SignalPreKeyStore, signedPreKeyStore: SignalSignedPreKeyStore, identityKeyStore: SignalIdentityKeyStore, senderKeyStore: SignalSenderKeyStore)

//        var sessionStore = SignalSessionStore()
//        var signalStore = SignalStore(sessionStore, preKeyStore, signedPreKeyStore, identityKeyStore, senderKeyStore)
//        var signalContext = SignalContext(storage: signalStore)
    }

    func setupSignal() {

        let globalContext = signalLibraryInitialization()
        signalClientInstallTime(globalContext: globalContext)
        signalBuildSession(globalContext: globalContext)
    }

    func signalBuildSession(globalContext: OpaquePointer?) {
        var storeContext: OpaquePointer?
        signal_protocol_store_context_create(&storeContext, globalContext)
        var sessionStore = signal_protocol_session_store()

        sessionStore.store_session_func = { address, record, recordLen, userData in
            return LocalStorage.store(record: record, size: recordLen, forAddress: address)
        }
        sessionStore.load_session_func = { record, address, userData in
            return LocalStorage.load(record: record, forAddress: address)
        }
        sessionStore.get_sub_device_sessions_func = { sessions, name, nameLen, userData in
            return 0
        }
        sessionStore.contains_session_func = { address, userData in
            return LocalStorage.contains(address: address)
        }
        sessionStore.delete_session_func = { address, userData in
            return LocalStorage.delete(address: address)
        }
        sessionStore.delete_all_sessions_func = { name, nameLen, userData in
            return LocalStorage.deleteAll(name: name, nameLen: nameLen)
        }

        sessionStore.destroy_func = { userData in }
        sessionStore.user_data = nil
        signal_protocol_store_context_set_session_store(storeContext, &sessionStore)

        var preKeyStore = signal_protocol_pre_key_store()
        preKeyStore.load_pre_key = { record, preKeyId, userData in
            return 0
        }
        preKeyStore.store_pre_key = { preKeyId, record, recordLen, userData in
            return 0
        }
        preKeyStore.contains_pre_key = { preKeyId, userData in
            return 0
        }
        preKeyStore.remove_pre_key = { preKeyId, userData in
            return 0
        }
        preKeyStore.destroy_func = { userData in }
        preKeyStore.user_data = nil
        signal_protocol_store_context_set_pre_key_store(storeContext, &preKeyStore);

        var signedPreKeyStore = signal_protocol_signed_pre_key_store()
        signedPreKeyStore.load_signed_pre_key = { record, preKeyId, userData in
            return 0
        }
        signedPreKeyStore.store_signed_pre_key = { preKeyId, record, recordLen, userData in
            return 0
        }
        signedPreKeyStore.contains_signed_pre_key = { preKeyId, userData in
            return 0
        }
        signedPreKeyStore.remove_signed_pre_key = { preKeyId, userData in
            return 0
        }
        signedPreKeyStore.destroy_func = { userData in }
        signedPreKeyStore.user_data = nil
        signal_protocol_store_context_set_signed_pre_key_store(storeContext, &signedPreKeyStore);

        var identityKeyStore = signal_protocol_identity_key_store()
        identityKeyStore.get_identity_key_pair = { publicData, privateData, userData in
            return 0
        }
        identityKeyStore.get_local_registration_id = { userData, registrationId in
            return 0
        }
        identityKeyStore.save_identity = { address, keyData, keyLen, userData in
            return 0
        }
        identityKeyStore.is_trusted_identity = { address, keyData, keyLen, userData in
            return 0
        }
        identityKeyStore.destroy_func = { userData in }
        identityKeyStore.user_data = nil
        signal_protocol_store_context_set_identity_key_store(storeContext, &identityKeyStore);

        var address = signal_protocol_address(name: "+14159998888", name_len: 12, device_id: 1)
        var builder: OpaquePointer?
        session_builder_create(&builder, storeContext, &address, globalContext);

//        session_builder_process_pre_key_bundle(builder, retrievedPreKey);

        var cipher: OpaquePointer?
        session_cipher_create(&cipher, storeContext, &address, globalContext);

//        var messageLen = 99
//        session_cipher_encrypt(cipher, message, messageLen, &encryptedMessage);
//        let serialized = ciphertext_message_get_serialized(encryptedMessage);
    }

    func signalClientInstallTime(globalContext: OpaquePointer?) {

        if LocalStorage.loadBoolean(forKey: .signalClientInstallTime) {
            return
        }
        LocalStorage.store(true, forKey: .signalClientInstallTime)

        var result: Int32
        var identityKeyPair: OpaquePointer?
        var registrationId: UInt32 = 0
        let extendedRange: Int32 = 0
        let startId: UInt32 = 0
        let count: UInt32 = 100
        var preKeysHead: OpaquePointer?
        var signedPreKey: OpaquePointer?
        let signedPreKeyId: UInt32 = 5
        let timestamp: UInt64 = 0

        result = signal_protocol_key_helper_generate_identity_key_pair(&identityKeyPair, globalContext)
        checkForError(result: result, name: "signal_protocol_key_helper_generate_identity_key_pair")
        result = signal_protocol_key_helper_generate_registration_id(&registrationId, extendedRange, globalContext)
        checkForError(result: result, name: "signal_protocol_key_helper_generate_registration_id")
        result = signal_protocol_key_helper_generate_pre_keys(&preKeysHead, startId, count, globalContext)
        checkForError(result: result, name: "signal_cosignal_protocol_key_helper_generate_pre_keysntext_create")
        result = signal_protocol_key_helper_generate_signed_pre_key(&signedPreKey,
                                                                    identityKeyPair,
                                                                    signedPreKeyId,
                                                                    timestamp,
                                                                    globalContext)
        checkForError(result: result, name: "signal_protocol_key_helper_generate_signed_pre_key")
    }

    func signalLibraryInitialization() -> OpaquePointer? {

        var globalContext: OpaquePointer?
        var result: Int32

        result = signal_context_create(&globalContext, &Crypto.lock)
        checkForError(result: result, name: "signal_context_create")
        var provider = signal_crypto_provider()
        result = signal_context_set_crypto_provider(globalContext, &provider)
        checkForError(result: result, name: "signal_context_set_crypto_provider")
        result = signal_context_set_locking_functions(globalContext,
                                                { _ in Crypto.lock.lock() },
                                                { _ in Crypto.lock.unlock() })
        checkForError(result: result, name: "signal_context_set_locking_functions")

        return globalContext
    }

    func checkForError(result: Int32, name: String) {
        if result != 0 {
            print("error for \(name): \(result)")
        }
    }
}