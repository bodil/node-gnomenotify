var libnotify = require("./build/default/libnotify");
var assert = require("assert");

libnotify.notify_init("node-libnotify-test");
assert.equal(libnotify.notify_get_app_name(), "node-libnotify-test");

var n = new libnotify.Notification("ohai!", "This is a test notification.",
                                    "dialog-warning");
n.set_hint("x-canonical-append", "");
n.set_urgency("critical");
n.show();
setTimeout(function() {
    n.update("HAIL INGLIP", "Y U NO GROPAGA?", "dialog-error");
    n.show();
    var ct = 0;
    var rep = function() {
        if (ct < 5) {
            ct++;
            var n2 = new libnotify.Notification("HAIL INGLIP", "Y U NO " + ct, ct % 2 ? "dialog-warning" : "dialog-error");
            n2.set_hint("x-canonical-append", "");
            n2.set_timeout(1000);
            n2.show();
            setTimeout(rep, 1000);
        }
    };
    setTimeout(rep, 1000);
}, 1000);

