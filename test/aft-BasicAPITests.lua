_AFT.testVerbStatusSuccess("Ping_verb_test", "helloworld", "ping", {})
_AFT.testVerbStatusSuccess("TestArgs_verb_test", "helloworld", "testargs", { cezam = "open" })
_AFT.testVerbStatusError("TestArgs_value_error_verb_test", "helloworld", "testargs", { cezam = "close" })
_AFT.testVerbStatusError("TestArgs_key_error_verb_test", "helloworld", "testargs", { abracadabdra = "open" })
