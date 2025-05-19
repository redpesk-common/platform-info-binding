from afb_test import AFBTestCase, configure_afb_binding_tests, run_afb_binding_tests

import libevdev
import libafb
import unittest
import json
import time

bindings = {"platform-info": f"platform-info-binding.so"}


def setUpModule():
    configure_afb_binding_tests(bindings=bindings)


class TestsEventsVerbs(AFBTestCase):

    def test_subscribe_unsubscribe(self):

        dicto = []
        request_list = [
            # When there is a [] in the we except the binding to return no event because of the filter
            # when there is [''] the filter is supposed to fail so the binding return events without filter
            #
            # Cases where the binding must return an event without filtering anything
            ({"event": "monitor-devices"}, [""]),
            (
                {
                    "event": "monitor-devices",
                    "filter": {"properties": {"notafilter": "input"}},
                },
                [""],
            ),
            ({"event": "monitor-devices", "filter": {"notatag": [":seat:"]}}, [""]),
            (
                {"event": "monitor-devices", "filter": {"tags": ":seat:"}},
                [""],
            ),  # Should raise a warning
            # Cases where the binding must return an event with the values respecting the filter
            (
                {
                    "event": "monitor-devices",
                    "filter": {"properties": {"SUBSYSTEM": "input"}},
                },
                ["properties", "SUBSYSTEM", "input"],
            ),
            (
                {"event": "monitor-devices", "filter": {"TAGS": ":seat:"}},
                ["properties", "TAGS", ":seat:"],
            ),
            # Cases where the filter must prevent the binding from returning an event
            (
                {
                    "event": "monitor-devices",
                    "filter": {"properties": {"SUBSYSTEM": "wontfindit"}},
                },
                [],
            ),
            (
                {
                    "event": "monitor-devices",
                    "filter": {"properties": {"SUBSYSTEM": "disk", "DEVTYPE": "disk"}},
                },
                [],
            ),
            ({"event": "monitor-devices", "filter": {"TAGS": ":wontfindit:"}}, []),
            # Cases to tests the masking of event's information
            (
                {
                    "event": "monitor-devices",
                    "mask": {
                        "properties": ["ID_INPUT", "NAME"],
                        "attributes": ["name", "subsystem"],
                    },
                },
                ["mask", "ID_INPUT", "NAME", "name", "subsystem"],
            ),
            (
                {
                    "event": "monitor-devices",
                    "mask": {"properties": ["ID_INPUT", "NAME"]},
                },
                ["mask", "ID_INPUT", "NAME"],
            ),
            (
                {
                    "event": "monitor-devices",
                    "mask": {"attributes": ["name", "subsystem"]},
                },
                ["mask", "name", "subsystem"],
            ),
        ]

        def evt_detect(binder, evt_name, userdata, data):
            nonlocal dicto
            if "attributes" in data.keys():
                dicto = data

        evt = libafb.evthandler(
            self.binder,
            {
                "uid": "platform-info",
                "pattern": "platform-info/device_changed",
                "callback": evt_detect,
            },
        )

        for request, values in request_list:

            dicto = []

            sub = libafb.callsync(self.binder, "platform-info", "subscribe", request)

            dev = libevdev.Device()
            dev.name = "TEST DEVICE"
            uinput = dev.create_uinput_device()

            timeout = time.time() + 2
            while dicto == [] and time.time() < timeout:
                True

            # Generic test cases
            assert sub.status == 0

            if len(values) != 0:
                assert dicto != []
                name = dicto.get("properties", {}).get("NAME")
                if name is not None:
                    assert name == '"TEST DEVICE"'

            # Cases where the filter is valid so we're testing the return contains the corrects values
            if len(values) > 2:
                if values[1] == "SUBSYSTEM":
                    assert dicto[values[0]][values[1]] == values[2]

                if values[1] == "tags":
                    assert dicto[values[0]][values[1]] == values[2]

                if values[0] == "mask":
                    for value in values[1:]:
                        assert (
                            value in dicto["properties"] or value in dicto["attributes"]
                        )

            del uinput._uinput

            libafb.callsync(self.binder, "platform-info", "unsubscribe", request)

        libafb.evtdelete(self.binder, "platform-info/device_changed")


if __name__ == "__main__":
    run_afb_binding_tests(bindings)
