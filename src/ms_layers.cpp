#include "ms_layers.hpp"
#include "ms_layer.hpp"

Nan::Persistent<v8::FunctionTemplate> MSLayers::constructor;

void MSLayers::Initialize(v8::Local<v8::Object> target) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(MSLayers::New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Layers").ToLocalChecked());

  Nan::SetIndexedPropertyHandler(tpl->InstanceTemplate(), IndexGetter, NULL, NULL, NULL, NULL);
  Nan::SetNamedPropertyHandler(tpl->InstanceTemplate(), NamedGetter, NULL, NULL, NULL, NULL);

  target->Set(Nan::GetCurrentContext(), Nan::New("Layers").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked()).Check();
  constructor.Reset(tpl);
}

MSLayers::MSLayers(mapObj *map) : ObjectWrap(), this_(map) {}

MSLayers::MSLayers() : ObjectWrap(), this_(0) {}

MSLayers::~MSLayers() { }

NAN_METHOD(MSLayers::New) {
  MSLayers *obj;

  if (!info.IsConstructCall()) {
    Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
    return;
  }

  if (info[0]->IsExternal()) {
    v8::Local<v8::External> ext = info[0].As<v8::External>();
    void* ptr = ext->Value();
    obj = static_cast<MSLayers*>(ptr);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
    return;
  }
  info.GetReturnValue().Set(info.This());
}

v8::Local<v8::Value> MSLayers::NewInstance(mapObj *ptr) {
  Nan::EscapableHandleScope scope;
  MSLayers* obj = new MSLayers();
  obj->this_ = ptr;
  v8::Local<v8::Value> ext = Nan::New<v8::External>(obj);

  v8::Local<v8::Function> f = Nan::GetFunction(Nan::New(constructor)).ToLocalChecked();
  Nan::MaybeLocal<v8::Object> maybe_local = Nan::NewInstance(f, 1, &ext);

  return scope.Escape(maybe_local.ToLocalChecked());
}

NAN_INDEX_GETTER(MSLayers::IndexGetter) {
  MSLayers *layers = Nan::ObjectWrap::Unwrap<MSLayers>(info.Holder());
  if ((int)index < layers->this_->numlayers) {
    layerObj* layer = GET_LAYER(layers->this_, index);
    if (layer != NULL) { 
      info.GetReturnValue().Set(MSLayer::NewInstance(layer));
    }
  }
}

NAN_PROPERTY_GETTER(MSLayers::NamedGetter) {
  MSLayers *layers = Nan::ObjectWrap::Unwrap<MSLayers>(info.Holder());
  if (STRCMP(property, "length")) {
    info.GetReturnValue().Set(layers->this_->numlayers);
  } else {
    int i;
    for (i=0; i<layers->this_->numlayers; i++) {
      layerObj* layer = GET_LAYER(layers->this_, i);
      if (layer != NULL && layer->name != NULL && STRCMP(property, layer->name)) {
        info.GetReturnValue().Set(MSLayer::NewInstance(layer));
        return;
      }
    }
  }
}


