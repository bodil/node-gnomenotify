#include <v8.h>
#include <node.h>
#include <node_events.h>
#include <libnotify/notify.h>
#include <string>
#include <iostream>

using namespace v8;
using namespace node;

static Persistent<String> show_symbol;
static Persistent<String> update_symbol;
static Persistent<String> close_symbol;
static Persistent<String> set_hint_symbol;

class Notification : public EventEmitter {
    public:
        static void Initialize(Handle<Object> target) {
            HandleScope scope;
            Local<FunctionTemplate> t = FunctionTemplate::New(New);
            t->Inherit(EventEmitter::constructor_template);
            t->InstanceTemplate()->SetInternalFieldCount(1);

            show_symbol = NODE_PSYMBOL("show");
            update_symbol = NODE_PSYMBOL("update");
            close_symbol = NODE_PSYMBOL("close");
            set_hint_symbol = NODE_PSYMBOL("set_hint");
            NODE_SET_PROTOTYPE_METHOD(t, "show", Show);
            NODE_SET_PROTOTYPE_METHOD(t, "update", Update);
            NODE_SET_PROTOTYPE_METHOD(t, "close", Close);
            NODE_SET_PROTOTYPE_METHOD(t, "set_hint", SetHint);

            target->Set(String::NewSymbol("Notification"), t->GetFunction());
        }

        NotifyNotification* notification;

    protected:
        static Handle<Value> New(const Arguments& args) {
            HandleScope scope;

            if(args.Length() != 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
                return ThrowException(
                    Exception::Error(String::New("Notification constructor takes exactly three string arguments."))
                );
            }
            String::Utf8Value summary(args[0]->ToString()),
                body(args[1]->ToString()),
                icon(args[2]->ToString());
            Notification* n = new Notification(*summary, *body, *icon);
            n->Wrap(args.This());

            return args.This();
        }

        Notification(const char* summary, const char* body, const char* icon) : EventEmitter() {
            notification = notify_notification_new(summary, body, icon, NULL);
        }

        static Handle<Value> Show(const Arguments& args) {
            Notification* n = ObjectWrap::Unwrap<Notification>(args.This());
            HandleScope scope;
            GError* error = NULL;
            bool success = notify_notification_show(n->notification, &error);
            if (!success) {
                return ThrowException(
                    Exception::Error(String::Concat(String::New("Notification::Show: "), String::New(error->message)))
                );
            }
            return Undefined();
        }

        static Handle<Value> Update(const Arguments& args) {
            Notification* n = ObjectWrap::Unwrap<Notification>(args.This());
            HandleScope scope;
            if(args.Length() != 3 || !args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
                return ThrowException(
                    Exception::Error(String::New("Notification constructor takes exactly three string arguments."))
                );
            }
            String::Utf8Value summary(args[0]->ToString()),
                body(args[1]->ToString()),
                icon(args[2]->ToString());
            bool success = notify_notification_update(n->notification, *summary, *body, *icon);
            if (!success) {
                return ThrowException(
                    Exception::Error(String::New("Notification::Update: failed."))
                );
            }
            return Undefined();
        }

        static Handle<Value> Close(const Arguments& args) {
            Notification* n = ObjectWrap::Unwrap<Notification>(args.This());
            HandleScope scope;
            GError* error = NULL;
            bool success = notify_notification_close(n->notification, &error);
            if (!success) {
                return ThrowException(
                    Exception::Error(String::Concat(String::New("Notification::Close: "), String::New(error->message)))
                );
            }
            return Undefined();
        }

        static Handle<Value> SetHint(const Arguments& args) {
            Notification* n = ObjectWrap::Unwrap<Notification>(args.This());
            HandleScope scope;
            if(args.Length() != 2 || !args[0]->IsString()) {
                return ThrowException(
                    Exception::Error(String::New("set_hint() takes exactly two arguments; the first must be a string."))
                );
            }
            String::Utf8Value key(args[0]->ToString());
            Local<Value> value = args[1];
            if (value->IsString()) {
                String::Utf8Value v_str(value->ToString());
                notify_notification_set_hint_string(n->notification, *key, *v_str);
            } else if (value->IsInt32()) {
                notify_notification_set_hint_int32(n->notification, *key, value->Int32Value());
            } else if (value->IsNumber()) {
                notify_notification_set_hint_double(n->notification, *key, value->NumberValue());
            } else {
                return ThrowException(
                    Exception::Error(String::New("set_hint: Unhandled value type."))
                );
            }

            return Undefined();
        }

};

Handle<Value> NotifyInit(const Arguments& args) {
    HandleScope scope;
    if(args.Length() != 1 || !args[0]->IsString()) {
        return ThrowException(
            Exception::Error(String::New("notify_init takes exactly one string argument."))
        );
    }
    String::Utf8Value appname(args[0]->ToString());
    return scope.Close(Boolean::New(notify_init(*appname)));
}

Handle<Value> NotifyGetAppName(const Arguments& args) {
    HandleScope scope;
    return scope.Close(String::New(notify_get_app_name()));
}

extern "C" void init(Handle<Object> target) {
    HandleScope scope;

    Local<FunctionTemplate> n_init = FunctionTemplate::New(NotifyInit);
    target->Set(String::New("notify_init"), n_init->GetFunction());

    Local<FunctionTemplate> n_get_app_name = FunctionTemplate::New(NotifyGetAppName);
    target->Set(String::New("notify_get_app_name"), n_get_app_name->GetFunction());

    Notification::Initialize(target);
}

