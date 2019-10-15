#include <math.h>

#include <napi.h>
#include <nan.h>

static Napi::Function stringify;
static Napi::Function concat;
static int initialized = 0;

Napi::Value UnPack(const Napi::CallbackInfo& info);
Napi::Value Pack(const Napi::CallbackInfo& info);
Napi::String Stringify(Napi::Env env, Napi::Object object);
Napi::Value ThrowIfMissing(const Napi::CallbackInfo& info);

Napi::Value ThrowIfMissing(const Napi::CallbackInfo& info) {
  throw Napi::TypeError::New(info.Env(), "Undefined is not a function");
}

#define check_initialized() do { if (initialized == 0) \
    throw Napi::TypeError::New(info.Env(), "Not initialized"); } while (0);

Napi::Value UnPack(const Napi::CallbackInfo& info) {
  check_initialized();
  Napi::Env env = info.Env();
  size_t length = info.Length();
  if (length != 1)
    throw Napi::TypeError::New(env, "unpack takes one argument");

  return env.Null();
}

Napi::Value Pack(const Napi::CallbackInfo& info) {
  check_initialized();
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


  Napi::String stringified = Stringify(env, turfs);
  const char *turfsString = stringified.Utf8Value().c_str();
  size_t turfsStringLength = strlen(turfsString);


  Napi::Buffer<char> data = Napi::Buffer<char>::Copy(env, turfsString, turfsStringLength);
  Napi::Buffer<float> packed = Napi::Buffer<float>::Copy(env, floats, turfSectionLength);

  // call concat with static initializer list
  // Napi::Buffer<float> output = concat.Call({packed, data}).As<Napi::Buffer<float>>();

  // call concat with napi_value vector
  // std::vector<napi_value> args_vector;
  // napi_value data_value;
  // std::memcpy(&data_value, &data, sizeof (napi_value));
  // args_vector.push_back(data_value);
  // napi_value packed_value;
  // std::memcpy(&packed_value, &packed, sizeof (napi_value));
  // args_vector.push_back(packed_value);

  // return concat.Call(args_vector);

  // just return one vector
  (void) data;
  return packed;
}

Napi::String Stringify(Napi::Env env, Napi::Object object) {
  Nan::JSON NanJSON;
  napi_value value1 = napi_value(object);
  v8::Local<v8::Object> value2;

  assert(sizeof (v8::Local<v8::Object>) == sizeof (napi_value));
  std::memcpy(&value2, &value1, sizeof (v8::Local<v8::Object>));

  Nan::MaybeLocal<v8::String> valueStringified = NanJSON.Stringify(value2);
  if (!valueStringified.IsEmpty())
  {
    v8::Local<v8::Value> local = valueStringified.ToLocalChecked();
    napi_value local_napi_value = reinterpret_cast<napi_value>(*local);
    Napi::Value sv = Napi::Value::From(env, local_napi_value);
    return sv.As<Napi::String>();
  }
  throw Napi::TypeError::New(env, "Error stringifying data");
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
  initialized = 1;
  return env.Undefined();
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
  Napi::Object global = env.Global();
  Napi::Object JSON = global.Get("JSON").As<Napi::Object>();
  stringify = JSON.Get("stringify").As<Napi::Function>();
  concat = Napi::Function::New(env, ThrowIfMissing);

  exports.Set(Napi::String::New(env, "init_"), Napi::Function::New(env, Initialize));
  exports.Set(Napi::String::New(env, "pack"), Napi::Function::New(env, Pack));
  exports.Set(Napi::String::New(env, "unpack"), Napi::Function::New(env, UnPack));
  return exports;
};

NODE_API_MODULE(turf_file, init);
