#include <math.h>

#include <napi.h>
#include <node.h>
#include <nan.h>
#include <v8.h>


static Napi::Function stringify;
static Napi::Function concat;

Napi::Value UnPack(const Napi::CallbackInfo& info) {
  if (concat.IsNull())
    throw Napi::TypeError::New(info.Env(), "Not initialized");

  Napi::Env env = info.Env();
  size_t length = info.Length();
  if (length != 1)
    throw Napi::TypeError::New(env, "unpack takes one argument");

  return env.Null();
}

Napi::Buffer<float> Pack(const Napi::CallbackInfo& info) {
  if (concat.IsNull())
    throw Napi::TypeError::New(info.Env(), "Not initialized");

  Napi::Env env = info.Env();
  size_t length = info.Length();
  if (length != 2)
    throw Napi::TypeError::New(env, "pack takes two arguments");

  Napi::Value arg1 = info[0];
  if (!arg1.IsArray())
    throw Napi::TypeError::New(env, "pack takes 1st argument array of shapes");

  Napi::Value arg2 = info[1];
  if (!arg2.IsArray())
    throw Napi::TypeError::New(env, "pack takes 2nd argument array of turfs");

  Napi::Array shapes = arg1.As<Napi::Array>();
  Napi::Array turfs = arg2.As<Napi::Array>();

  size_t shapesLength = shapes.Length();
  if (shapesLength != turfs.Length())
  {
    throw Napi::TypeError::New(env, "Array lengths don't match");
  }

  unsigned int turfSectionLength = 0;
  for (unsigned int i = 0; i < shapesLength; ++i)
  {
    Napi::Value element = shapes[i];
    if (!element.IsArray())
      throw Napi::TypeError::New(env, "pack takes 1st argument array of arrays");
    Napi::Array shape = element.As<Napi::Array>();

    // iterate through shape coordinates
    size_t shapeLength = shape.Length();
    for (unsigned int j = 0; j < shapeLength; ++j)
    {
      Napi::Value shapeElement = shape[j];
      if (!shapeElement.IsNumber())
        throw Napi::TypeError::New(env, "pack takes 1st argument array of arrays of floats");
      turfSectionLength++;
    }
    turfSectionLength++;
  }

  float floats [ turfSectionLength ] = { };
  unsigned int floatsIndex = 0;
  for (unsigned int i = 0; i < shapesLength; ++i) {
    Napi::Array shape = ((Napi::Value) shapes[i]).As<Napi::Array>();
    for (unsigned int j = 0; j < shape.Length(); ++j) {
      Napi::Number coord = ((Napi::Value) shape[j]).As<Napi::Number>();
      floats[floatsIndex++] = float(coord);
    }
    floats[floatsIndex++] = Napi::Number::New(env, NAN);
  }

  Napi::String stringified = stringify.Call({turfs}).As<Napi::String>();
  const char *turfsString = stringified.Utf8Value().c_str();
  size_t turfsStringLength = strlen(turfsString);

  Napi::Buffer<char> data = Napi::Buffer<char>::Copy(env, turfsString, turfsStringLength);
  Napi::Buffer<float> packed = Napi::Buffer<float>::Copy(env, floats, turfSectionLength);

  Napi::Buffer<float> output = concat.Call({packed, data}).As<Napi::Buffer<float>>();

  return output;
}

static inline
napi_value JsValueFromV8LocalValue(v8::Local<v8::Value> local) {
  return reinterpret_cast<napi_value>(*local);
}

Napi::Value Test(const Napi::CallbackInfo& info) {
  v8::Local<v8::String> json_string = Nan::New("{ \"JSON\": \"object\" }").ToLocalChecked();

  Nan::JSON NanJSON;
  Nan::MaybeLocal<v8::Value> result = NanJSON.Parse(json_string);
  if (!result.IsEmpty()) {
    v8::Local<v8::Value> val = result.ToLocalChecked();

    Napi::Object value = Napi::Value::From(info.Env(), JsValueFromV8LocalValue(val)).As<Napi::Object>();
    napi_value value1 = napi_value(value);
    v8::Local<v8::Object> value2;

    printf("%lu %lu\n", sizeof (v8::Local<v8::Object>), sizeof (napi_value));

    std::memcpy(&value2, &value1, sizeof (v8::Local<v8::Object>));

    Nan::MaybeLocal<v8::String> valueStringified = NanJSON.Stringify(value2);
    if (!valueStringified.IsEmpty())
    {
      return Napi::Value::From(info.Env(), JsValueFromV8LocalValue(valueStringified.ToLocalChecked()));
    }
  }

  return info.Env().Null();
}

Napi::Value ThrowIfMissing(const Napi::CallbackInfo& info) {
  throw Napi::TypeError::New(info.Env(), "Undefined is not a function");
}

Napi::Value Initialize(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Value arg1 = info[0];
  if (!arg1.IsFunction())
    throw Napi::TypeError::New(env, "Require is not a function");

  Napi::Function require = arg1.As<Napi::Function>();
  Napi::Value bufferValue = require.Call({Napi::String::New(env, "buffer")});
  Napi::Object buffer = bufferValue.As<Napi::Object>();
  Napi::Object Buffer = buffer.Get("Buffer").As<Napi::Function>();
  concat = Buffer.Get("concat").As<Napi::Function>();
  return env.Undefined();
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
  Napi::Object global = env.Global();
  Napi::Object JSON = global.Get("JSON").As<Napi::Object>();
  stringify = JSON.Get("stringify").As<Napi::Function>();
  concat = Napi::Function::New(env, ThrowIfMissing);

  exports.Set(Napi::String::New(env, "test"), Napi::Function::New(env, Test));
  exports.Set(Napi::String::New(env, "init_"), Napi::Function::New(env, Initialize));
  exports.Set(Napi::String::New(env, "pack"), Napi::Function::New(env, Pack));
  exports.Set(Napi::String::New(env, "unpack"), Napi::Function::New(env, UnPack));
  return exports;
};

NODE_API_MODULE(turf_file, init);
