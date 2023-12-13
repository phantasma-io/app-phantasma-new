import pytest

from application_client.boilerplate_command_sender import BoilerplateCommandSender, Errors
from application_client.boilerplate_response_unpacker import unpack_get_public_key_response, unpack_sign_tx_response
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID
from utils import ROOT_SCREENSHOT_PATH, check_signature_validity
from phantasma_py.Tx import Transaction
from phantasma_py.Types import PhantasmaKeys, Address
from phantasma_py.Types.Extensions import Base16
from phantasma_py.VM import ScriptBuilder

# In this tests we check the behavior of the device when asked to sign a transaction
TAG = "MY_SIGN_TX"

# In this test se send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
def test_sign_tx_short_tx(firmware, backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/44'/60'/0'/0/0"

    # First we need to get the public key of the device in order to build the transaction
    #rapdu = client.get_public_key(path=path)
  
    keys = PhantasmaKeys.from_wif("L5UEVHBjujaR1721aZM5Zm5ayjDyamMZS9W35RE9Y9giRkdf3dVx")
    amount = 10000000
    testSB = ScriptBuilder()
    testSB = testSB.AllowGas(keys.Address.Text, Address.NullText, 10000, 21000)
    testSB = testSB.CallInterop("Runtime.TransferTokens", [keys.Address.Text, keys.Address.Text, "SOUL", str(amount) ])
    testSB = testSB.SpendGas(keys.Address.Text)
    script = testSB.EndScript()

    # Create the transaction that will be sent to the device for signing
    transaction = Transaction(
        "mainnet",  # NEXUS (mainnet or testnet)
        "main",  # CHAIN
        script,  # SCRIPT
        None,
        # EXPIRATION (Leave it empty and the module will create a valid one for
        # you)
        "PHANTASMAROCKS"  # PAYLOAD
    )
    txSerialized = Base16.decode_uint8_array(transaction.toString(False))

    print(TAG, Base16.encode_uint8_array(txSerialized))

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_tx(path=path, transaction=txSerialized):
        # Validate the on-screen request by performing the navigation appropriate for this device
        if firmware.device.startswith("nano"):
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
            [NavInsID.BOTH_CLICK],
            "Approve",
            ROOT_SCREENSHOT_PATH,
            test_name)
        else:
            navigator.navigate_until_text_and_compare(NavInsID.USE_CASE_REVIEW_TAP,
            [NavInsID.USE_CASE_REVIEW_CONFIRM,
            NavInsID.USE_CASE_STATUS_DISMISS],
            "Hold to sign",
            ROOT_SCREENSHOT_PATH,
            test_name)
    # The device as yielded the result, parse it and ensure that the signature is correct
    #response = client.get_async_response().data
    #'''_, der_sig, _ = unpack_sign_tx_response(response)
    #assert check_signature_validity(public_key, der_sig, transaction)'''


# In this test se send to the device a transaction to sign and validate it on screen
# This test is mostly the same as the previous one but with different values.
# In particular the long memo will force the transaction to be sent in multiple chunks
'''
def test_sign_tx_long_tx(firmware, backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44'/60'/0'/0/0"

    keys = PhantasmaKeys.from_wif("L5UEVHBjujaR1721aZM5Zm5ayjDyamMZS9W35RE9Y9giRkdf3dVx")
    amount = 10000000
    testSB = ScriptBuilder()
    testSB = testSB.AllowGas(keys.Address.Text, Address.NullText, 10000, 21000)
    testSB = testSB.CallInterop("Runtime.TransferTokens", [keys.Address.Text, keys.Address.Text, "SOUL", str(amount), str(amount),  str(amount), str(amount) ,  str(amount), str(amount)  ])
    testSB = testSB.SpendGas(keys.Address.Text)
    script = testSB.EndScript()

    # Create the transaction that will be sent to the device for signing
    transaction = Transaction(
        "mainnet",  # NEXUS (mainnet or testnet)
        "main",  # CHAIN
        script,  # SCRIPT
        None,
        # EXPIRATION (Leave it empty and the module will create a valid one for
        # you)
        "PHANTASMAROCKS"  # PAYLOAD
    )
    txSerialized = Base16.decode_uint8_array(transaction.toString(False))

    print(TAG, Base16.encode_uint8_array(txSerialized))


    with client.sign_tx(path=path, transaction=txSerialized):
        if firmware.device.startswith("nano"):
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                      [NavInsID.BOTH_CLICK],
                                                      "Approve",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)
        else:
            navigator.navigate_until_text_and_compare(NavInsID.USE_CASE_REVIEW_TAP,
                                                      [NavInsID.USE_CASE_REVIEW_CONFIRM,
                                                       NavInsID.USE_CASE_STATUS_DISMISS],
                                                      "Hold to sign",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)
    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    assert check_signature_validity(public_key, der_sig, transaction)


# Transaction signature refused test
# The test will ask for a transaction signature that will be refused on screen
def test_sign_tx_refused(firmware, backend, navigator, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44'/60'/0'/0/0"

    keys = PhantasmaKeys.from_wif("L5UEVHBjujaR1721aZM5Zm5ayjDyamMZS9W35RE9Y9giRkdf3dVx")
    amount = 10000000
    testSB = ScriptBuilder()
    testSB = testSB.AllowGas(keys.Address.Text, Address.NullText, 10000, 21000)
    testSB = testSB.CallInterop("Runtime.TransferTokens", [keys.Address.Text, keys.Address.Text, "SOUL", str(amount), str(amount),  str(amount), str(amount) ,  str(amount), str(amount)  ])
    testSB = testSB.SpendGas(keys.Address.Text)
    script = testSB.EndScript()

    # Create the transaction that will be sent to the device for signing
    transaction = Transaction(
        "mainnet",  # NEXUS (mainnet or testnet)
        "main",  # CHAIN
        script,  # SCRIPT
        None,
        # EXPIRATION (Leave it empty and the module will create a valid one for
        # you)
        "PHANTASMAROCKS"  # PAYLOAD
    )
    txSerialized = Base16.decode_uint8_array(transaction.toString(False))

    if firmware.device.startswith("nano"):
        with pytest.raises(ExceptionRAPDU) as e:
            with client.sign_tx(path=path, transaction=txSerialized):
                navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                          [NavInsID.BOTH_CLICK],
                                                          "Reject",
                                                          ROOT_SCREENSHOT_PATH,
                                                          test_name)

        # Assert that we have received a refusal
        assert e.value.status == Errors.SW_DENY
        assert len(e.value.data) == 0
    else:
        for i in range(6):
            instructions = [NavInsID.USE_CASE_REVIEW_TAP] * i
            instructions += [NavInsID.USE_CASE_REVIEW_REJECT,
                             NavInsID.USE_CASE_CHOICE_CONFIRM,
                             NavInsID.USE_CASE_STATUS_DISMISS]
            with pytest.raises(ExceptionRAPDU) as e:
                with client.sign_tx(path=path, transaction=transaction):
                    navigator.navigate_and_compare(ROOT_SCREENSHOT_PATH,
                                                   test_name + f"/part{i}",
                                                   instructions)
            # Assert that we have received a refusal
            assert e.value.status == Errors.SW_DENY
            assert len(e.value.data) == 0
'''