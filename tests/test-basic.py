from afb_test import AFBTestCase, configure_afb_binding_tests, run_afb_binding_tests

import libafb
import unittest
import json

bindings = {"platform-info": f"platform-info-binding.so"}


def setUpModule():
    configure_afb_binding_tests(bindings=bindings)


class TestBasicsVerbs(AFBTestCase):

    def test_get(self):

        core_dict = {
            "gpu_name": "gpu",
            "soc_family": "cpu range",
            "cpu_arch": "x86",
            "soc_name": "cpu name",
            "cpu_cache_kb": 16384,
            "board_model": "motherboard",
            "cpu_count": 8,
            "cpu_compatibility": "maybe",
            "memory_total_mb": 16384,
            "cpu_freq_mhz": 2000,
            "soc_id": "cpu model",
            "soc_revision": 10,
            "soc_vendor": "intel or amd",
        }

        for key in core_dict:
            r = libafb.callsync(self.binder, "platform-info", "get", key)
            assert r.status == 0
            assert r.args[0] == core_dict[key]

        os_dict = {"os_name": "fedora", "os_version": "40"}

        for key in os_dict.keys():
            r = libafb.callsync(self.binder, "platform-info", "get", key)
            assert r.status == 0
            assert r.args[0] == os_dict[key]

        r = libafb.callsync(self.binder, "platform-info", "get", "not_existing")
        assert r.status == 0
        assert r.args[0] == []

    def test_getall(self):
        dicto = {
            "gpu_name": "gpu",
            "soc_family": "cpu range",
            "cpu_arch": "x86",
            "soc_name": "cpu name",
            "cpu_cache_kb": 16384,
            "board_model": "motherboard",
            "cpu_count": 8,
            "cpu_compatibility": "maybe",
            "memory_total_mb": 16384,
            "cpu_freq_mhz": 2000,
            "soc_id": "cpu model",
            "soc_revision": 10,
            "soc_vendor": "intel or amd",
            "os_version": "40",
            "os_name": "fedora",
            "ethernet_devices": [],
            "bluetooth_devices": [],
            "wifi_devices": [],
            "can_devices": [],
        }

        r = libafb.callsync(self.binder, "platform-info", "get_all_info")
        assert r.status == 0
        assert r.args[0] == dicto

    "Test info verb"

    def test_info(self):
        r = libafb.callsync(self.binder, "platform-info", "info")
        assert r.status == 0


if __name__ == "__main__":
    run_afb_binding_tests(bindings)
