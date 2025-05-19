from afb_test import AFBTestCase, configure_afb_binding_tests, run_afb_binding_tests

import libafb
import unittest
import json

bindings = {"platform-info": f"platform-info-binding.so"}


def setUpModule():
    configure_afb_binding_tests(bindings=bindings)


class TestBasicsVerbs(AFBTestCase):
    
    "Test get verb"

    def test_get(self):

        with open('/etc/platform-info/core.json', 'r') as f:
            core_dict = json.load(f)

        for key in core_dict:
            r = libafb.callsync(self.binder, "platform-info", "get", key)
            assert r.status == 0
            assert r.args[0] == core_dict[key]
        
        with open('/etc/platform-info/os.json', 'r') as f:
            os_dict = json.load(f)

        for key in os_dict.keys():
            r = libafb.callsync(self.binder, "platform-info", "get", key)
            assert r.status == 0
            assert r.args[0] == os_dict[key]
            
    "Test get verb fail"

    def test_get_fail(self):
        try:
            r = libafb.callsync(self.binder, "platform-info", "get", "no_existing")
            assert False
        except RuntimeError as e:
            assert str(e) == "invalid-request"

        
    "Test get_all_info verb"    

    def test_getall(self):
        dicto = {}
        
        for path in ["/etc/platform-info/core.json", "/etc/platform-info/os.json", "/etc/platform-info/devices.json"]:
            with open(path, "r") as f:
                data = json.load(f)
                dicto.update(data)

        r = libafb.callsync(self.binder, "platform-info", "get_all_info")
        assert r.status == 0
        assert r.args[0] == dicto

    "Test info verb"

    def test_info(self):
        r = libafb.callsync(self.binder, "platform-info", "info")
        assert r.status == 0


if __name__ == "__main__":
    run_afb_binding_tests(bindings)
