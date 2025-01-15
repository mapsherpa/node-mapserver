#include "ms_hashtable.hpp"

Nan::Persistent<v8::FunctionTemplate> MSHashTable::constructor;

void MSHashTable::Initialize(v8::Local<v8::Object> target) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(MSHashTable::New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Hashtable").ToLocalChecked());

  Nan::SetNamedPropertyHandler(
        tpl->InstanceTemplate()
      , NamedPropertyGetter
      , NamedPropertySetter
      , NULL
      , NULL
      , NULL);

  target->Set(Nan::GetCurrentContext(), Nan::New("Hashtable").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
  constructor.Reset(tpl);
}

MSHashTable::MSHashTable(hashTableObj *table) : ObjectWrap(), this_(table) {}

MSHashTable::MSHashTable() : ObjectWrap(), this_(0) {}

MSHashTable::~MSHashTable() { }

NAN_METHOD(MSHashTable::New)
{
  if (!info.IsConstructCall()) {
    Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
    return;
  }

  if (info[0]->IsExternal()) {
    v8::Local<v8::External> ext = info[0].As<v8::External>();
    void *ptr = ext->Value();
    MSHashTable *f =  static_cast<MSHashTable *>(ptr);
    f->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
    return;
  }

  info.GetReturnValue().Set(info.This());
}

v8::Local<v8::Value> MSHashTable::NewInstance(hashTableObj *ptr) {
  Nan::EscapableHandleScope scope;
  MSHashTable* obj = new MSHashTable();
  obj->this_ = ptr;
  v8::Local<v8::Value> ext = Nan::New<v8::External>(obj);

  v8::Local<v8::Function> f = Nan::GetFunction(Nan::New(constructor)).ToLocalChecked();
  Nan::MaybeLocal<v8::Object> maybe_local = Nan::NewInstance(f, 1, &ext);

  return scope.Escape(maybe_local.ToLocalChecked());
}

NAN_PROPERTY_GETTER(MSHashTable::NamedPropertyGetter) {
  MSHashTable *table = Nan::ObjectWrap::Unwrap<MSHashTable>(info.Holder());
  const char *value = msLookupHashTable(table->this_, TOSTR(property));
  if (value == NULL){
    info.GetReturnValue().Set(Nan::Undefined());
    return;
  }
  info.GetReturnValue().Set(Nan::New(value).ToLocalChecked());
}

NAN_PROPERTY_SETTER(MSHashTable::NamedPropertySetter) {
  MSHashTable *table = Nan::ObjectWrap::Unwrap<MSHashTable>(info.Holder());
  msInsertHashTable(table->this_, *v8::String::Utf8Value(v8::Isolate::GetCurrent(), property), TOSTR(value));
  info.GetReturnValue().Set(value);
}


