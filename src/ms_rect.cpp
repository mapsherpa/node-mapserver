#include "ms_rect.hpp"

Nan::Persistent<v8::FunctionTemplate> MSRect::constructor;

void MSRect::Initialize(v8::Local<v8::Object> target) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(MSRect::New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("Rect").ToLocalChecked());

  RW_ATTR(tpl, "minx", PropertyGetter, PropertySetter);
  RW_ATTR(tpl, "miny", PropertyGetter, PropertySetter);
  RW_ATTR(tpl, "maxx", PropertyGetter, PropertySetter);
  RW_ATTR(tpl, "maxy", PropertyGetter, PropertySetter);

  Nan::SetPrototypeMethod(tpl, "project", Project);

  target->Set(Nan::GetCurrentContext(), Nan::New("Rect").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
  constructor.Reset(tpl);
}

MSRect::MSRect(rectObj *rect) : ObjectWrap(), this_(rect) {
  owner = false;
}

MSRect::MSRect() : ObjectWrap(), this_(0) { owner = false; }

MSRect::~MSRect() {
  if (this_ && owner) {
    free(this_);
    owner = false;
  }
}

NAN_METHOD(MSRect::New) {
  MSRect *obj;
  double t;

  if (!info.IsConstructCall()) {
    Nan::ThrowError("Cannot call constructor as function, you need to use 'new' keyword");
    return;
  }

  if (info[0]->IsExternal()) {
    v8::Local<v8::External> ext = info[0].As<v8::External>();
    void* ptr = ext->Value();
    obj = static_cast<MSRect*>(ptr);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
    return;
  }

  rectObj *rect = (rectObj *)calloc(1, sizeof(rectObj));
  if(!rect) {
    info.GetReturnValue().Set(info.This());
    return;
  }

  if (info.Length() == 0) {
    rect->minx = -1;
    rect->miny = -1;
    rect->maxx = -1;
    rect->maxy = -1;
  } else if (info.Length() == 1) {

    if (!info[0]->IsObject()) {
      Nan::ThrowError("single argument constructor requires a Rect object");
      return;
    }

    v8::Local<v8::Object> obj = info[0].As<v8::Object>();

    if (obj->IsNull() || obj->IsUndefined() || !Nan::New(MSRect::constructor)->HasInstance(obj)) {
      Nan::ThrowError("single argument to Rect constructor must be a Rect object");
      return;
    }

    MSRect *inRect = Nan::ObjectWrap::Unwrap<MSRect>(obj);

    memcpy(rect, inRect->this_, sizeof(rectObj));

  } else if (info.Length() == 4) {

    if (!ISNUMERIC(info,0) || !ISNUMERIC(info,1) || !ISNUMERIC(info,2) || !ISNUMERIC(info,3)) {
      Nan::ThrowTypeError("Rect constructor requires four numeric values");
      return;
    }
    double minx = info[0]->NumberValue(Nan::GetCurrentContext()).ToChecked();
    double miny = info[1]->NumberValue(Nan::GetCurrentContext()).ToChecked();
    double maxx = info[2]->NumberValue(Nan::GetCurrentContext()).ToChecked();
    double maxy = info[3]->NumberValue(Nan::GetCurrentContext()).ToChecked();
    /* coerce correct extent */
    if (minx > maxx) {
      t = maxx;
      maxx = minx;
      minx = t;
    }
    if (miny > maxy) {
      t = maxy;
      maxy = miny;
      miny = t;
    }
    rect->minx = minx;
    rect->miny = miny;
    rect->maxx = maxx;
    rect->maxy = maxy;
  } else {
    Nan::ThrowError("Rect objects take 0, 1 or 4 arguments.");
    return;
  }

  obj = new MSRect(rect);
  obj->owner = true;
  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

v8::Local<v8::Value> MSRect::NewInstance(rectObj *ptr) {
  Nan::EscapableHandleScope scope;
  MSRect* obj = new MSRect();
  obj->this_ = ptr;
  v8::Local<v8::Value> ext = Nan::New<v8::External>(obj);

  v8::Local<v8::Function> f = Nan::GetFunction(Nan::New(constructor)).ToLocalChecked();
  Nan::MaybeLocal<v8::Object> maybe_local = Nan::NewInstance(f, 1, &ext);

  return scope.Escape(maybe_local.ToLocalChecked());
}

NAN_GETTER(MSRect::PropertyGetter) {
  MSRect *rect = Nan::ObjectWrap::Unwrap<MSRect>(info.Holder());

  if (STRCMP(property, "minx")) {
    info.GetReturnValue().Set(rect->this_->minx);
  } else if (STRCMP(property, "miny")) {
    info.GetReturnValue().Set(rect->this_->miny);
  } else if (STRCMP(property, "maxx")) {
    info.GetReturnValue().Set(rect->this_->maxx);
  } else if (STRCMP(property, "maxy")) {
    info.GetReturnValue().Set(rect->this_->maxy);
  }
}

NAN_SETTER(MSRect::PropertySetter) {
  MSRect *rect = Nan::ObjectWrap::Unwrap<MSRect>(info.Holder());

  if (STRCMP(property, "minx")) {
    rect->this_->minx = value->NumberValue(Nan::GetCurrentContext()).ToChecked();
  } else if (STRCMP(property, "miny")) {
    rect->this_->miny = value->NumberValue(Nan::GetCurrentContext()).ToChecked();
  } else if (STRCMP(property, "maxx")) {
    rect->this_->maxx = value->NumberValue(Nan::GetCurrentContext()).ToChecked();
  } else if (STRCMP(property, "maxy")) {
    rect->this_->maxy = value->NumberValue(Nan::GetCurrentContext()).ToChecked();
  }
}

NAN_METHOD(MSRect::Project) {
  MSRect *rect = Nan::ObjectWrap::Unwrap<MSRect>(info.Holder());

  v8::Local<v8::Object> obj;
  MSProjection *projIn;
  MSProjection *projOut;

  if (info.Length() != 2) {
    Nan::ThrowError("projecting a rect requires two projection arguments");
    return;
  }

  if (!info[0]->IsObject()) {
    Nan::ThrowTypeError("first argument to project must be Projection object");
    return;
  }

  obj = info[0].As<v8::Object>();

  if (obj->IsNull() || obj->IsUndefined() || !Nan::New(MSProjection::constructor)->HasInstance(obj)) {
    Nan::ThrowTypeError("first argument to project must be Projection object");
    return;
  }

  projIn = Nan::ObjectWrap::Unwrap<MSProjection>(obj);

  if (!info[1]->IsObject()) {
    Nan::ThrowTypeError("second argument to project must be Projection object");
    return;
  }

  obj = info[1].As<v8::Object>();

  if (obj->IsNull() || obj->IsUndefined() || !Nan::New(MSProjection::constructor)->HasInstance(obj)) {
    Nan::ThrowTypeError("second argument to project must be Projection object");
    return;
  }

  projOut = Nan::ObjectWrap::Unwrap<MSProjection>(obj);

  msProjectRect(projIn->this_, projOut->this_, rect->this_);
}