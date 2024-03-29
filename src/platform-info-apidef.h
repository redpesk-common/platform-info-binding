static const struct afb_auth _afb_auths_platform_info[] = {
	{ .type = afb_auth_Permission, .text = "urn:AGL:permission::platform:info:get" },
    { .type = afb_auth_Permission, .text = "urn:AGL:permission::platform:info:set" }
};

void afv_info(afb_req_t req);
void afv_get(afb_req_t req);
void afv_get_all_info(afb_req_t req);
void afv_set(afb_req_t req);
void afv_subscribe(afb_req_t req);
void afv_unsubscribe(afb_req_t req);

static const afb_verb_t _afb_verbs_platform_info[] = {
    {
        .verb = "info",
        .callback = afv_info,
        .auth = &_afb_auths_platform_info[0],
        .info = "Info regarding platform-info binding.",
        .session = AFB_SESSION_NONE
    },
    {
        .verb = "get",
        .callback = afv_get,
        .auth = &_afb_auths_platform_info[0],
        .info = "Get a platform data.",
        .session = AFB_SESSION_NONE
    },{
        .verb = "get_all_info",
        .callback = afv_get_all_info,
        .auth = &_afb_auths_platform_info[0],
        .info = "Get a set of platform data.",
        .session = AFB_SESSION_NONE
    },
    {
        .verb = "set",
        .callback = afv_set,
        .auth = &_afb_auths_platform_info[1],
        .info = "Set a platform data.",
        .session = AFB_SESSION_NONE
    },
    {
        .verb = "unsubscribe",
        .callback = afv_unsubscribe,
        .auth = NULL,
        .info = "Unsubscribe to changes (hotplug event, failures, ...)",
        .session = AFB_SESSION_NONE
    },
    {
        .verb = "subscribe",
        .callback = afv_subscribe,
        .auth = NULL,
        .info = "Subscribe to changes (hotplug event, failures, ...)",
        .session = AFB_SESSION_NONE
    },
    {
        .verb = NULL,
        .callback = NULL,
        .auth = NULL,
        .info = NULL,
        .session = 0
	}
};

int init(afb_api_t api);

const afb_binding_t afbBindingExport = {
    .api = "platform-info",
    .specification = NULL,
    .info = "",
    .verbs = _afb_verbs_platform_info,
    .preinit = NULL,
    .init = init,
    .onevent = NULL,
    .userdata = NULL,
    .provide_class = NULL,
    .require_class = NULL,
    .require_api = NULL,
    .noconcurrency = 0
};
