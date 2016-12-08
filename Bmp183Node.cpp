/**
 * \file Bmp183Node.cpp
 *
 *  Created by Scott Erholm on 08/22/16.
 *  Copyright (c) 2016 Agilatech. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "Bmp183Node.h"

namespace bmp183 {
    
    using v8::FunctionCallbackInfo;
    using v8::FunctionTemplate;
    using v8::Function;
    using v8::Persistent;
    using v8::Isolate;
    using v8::Context;
    using v8::Local;
    using v8::Handle;
    using v8::Object;
    using v8::String;
    using v8::Value;
    using v8::Number;
    using v8::Boolean;
    
    Persistent<Function> Bmp183Node::constructor;
    Bmp183Drv* Bmp183Node::driver = 0;
    
    void Bmp183Node::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();
        
        // prep the constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        
        // associates the New function with the class named Bmp183
        tpl->SetClassName(String::NewFromUtf8(isolate, "Bmp183"));
        
        // InstanceTemplate is the ObjectTemplate assocated with the function New
        tpl->InstanceTemplate()->SetInternalFieldCount(1);
        
        NODE_SET_PROTOTYPE_METHOD(tpl, "deviceName", getDeviceName);
        NODE_SET_PROTOTYPE_METHOD(tpl, "deviceType", getDeviceType);
        NODE_SET_PROTOTYPE_METHOD(tpl, "deviceVersion", getDeviceVersion);
        NODE_SET_PROTOTYPE_METHOD(tpl, "deviceNumValues", getDeviceNumValues);
        NODE_SET_PROTOTYPE_METHOD(tpl, "typeAtIndex", getTypeAtIndex);
        NODE_SET_PROTOTYPE_METHOD(tpl, "nameAtIndex", getNameAtIndex);
        NODE_SET_PROTOTYPE_METHOD(tpl, "deviceActive", isDeviceActive);
        NODE_SET_PROTOTYPE_METHOD(tpl, "valueAtIndexSync", getValueAtIndexSync);
        NODE_SET_PROTOTYPE_METHOD(tpl, "valueAtIndex", getValueAtIndex);
        NODE_SET_PROTOTYPE_METHOD(tpl, "operatingMode", setOperatingMode);

        // store a reference to this constructor
        constructor.Reset(isolate, tpl->GetFunction());
        
        exports->Set(String::NewFromUtf8(isolate, "Bmp183"), tpl->GetFunction());
    }
    
    void Bmp183Node::getDeviceName(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        std::string name = Bmp183Drv::getDeviceName();
        Local<String> deviceName = String::NewFromUtf8(isolate, name.c_str());
        
        args.GetReturnValue().Set(deviceName);
    }
    
    void Bmp183Node::getDeviceType(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        std::string type = Bmp183Drv::getDeviceType();
        Local<String> deviceType = String::NewFromUtf8(isolate, type.c_str());
        
        args.GetReturnValue().Set(deviceType);
    }
    
    void Bmp183Node::getDeviceVersion(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        std::string ver = Bmp183Drv::getVersion();
        Local<String> deviceVer = String::NewFromUtf8(isolate, ver.c_str());
        
        args.GetReturnValue().Set(deviceVer);
    }

    void Bmp183Node::getDeviceNumValues (const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        int value = Bmp183Drv::getNumValues();
        Local<Number> deviceNumVals = Number::New(isolate, value);
        
        args.GetReturnValue().Set(deviceNumVals);
    }
    
    void Bmp183Node::getTypeAtIndex (const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        std::string type = driver->getTypeAtIndex(args[0]->NumberValue());
        Local<String> valType = String::NewFromUtf8(isolate, type.c_str());
        
        args.GetReturnValue().Set(valType);
    }
    
    void Bmp183Node::getNameAtIndex (const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        std::string name = driver->getNameAtIndex(args[0]->NumberValue());
        Local<String> valName = String::NewFromUtf8(isolate, name.c_str());
        
        args.GetReturnValue().Set(valName);
    }
    
    void Bmp183Node::isDeviceActive (const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        bool active = driver->isActive();
        Local<Boolean> deviceActive = Boolean::New(isolate, active);
        
        args.GetReturnValue().Set(deviceActive);
    }
    
    void Bmp183Node::getValueAtIndexSync (const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        std::string value = driver->getValueAtIndex(args[0]->NumberValue());
        Local<String> retValue = String::NewFromUtf8(isolate, value.c_str());
        
        args.GetReturnValue().Set(retValue);
    }
    
    void Bmp183Node::getValueAtIndex (const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        Work * work = new Work();
        work->request.data = work;
        
        // get the desired value index from the first param in the JS call
        work->valueIndex = args[0]->NumberValue();
        
        // store the callback from JS in the work package so we can invoke it later
        Local<Function> callback = Local<Function>::Cast(args[1]);
        work->callback.Reset(isolate, callback);
        
        // kick of the worker thread
        uv_queue_work(uv_default_loop(),&work->request,WorkAsync,WorkAsyncComplete);
        
        args.GetReturnValue().Set(Undefined(isolate));
    }
    
    void Bmp183Node::setOperatingMode (const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        
        bool result = driver->setOperatingMode(args[0]->NumberValue());
        Local<Boolean> modeResult = Boolean::New(isolate, result);
        
        args.GetReturnValue().Set(modeResult);
    }
    
    // NOTE: This addon MUST be invoked as a constructor, as in:
    // var bmp183 = new driver.Bmp183('/dev/spidev1.0', 1750, 3);
    // because there are optional args, and the author doesn't yet
    // know how to deal with that if invoked as plain function (duh).
    void Bmp183Node::New(const FunctionCallbackInfo<Value>& args) {
        //Isolate* isolate = args.GetIsolate();
        
        std::string devfile = args[0]->IsUndefined() ? "/dev/spidev1.0" : *v8::String::Utf8Value(args[0]->ToString());
        
        int altitude = args[1]->IsUndefined() ? 0 : args[1]->NumberValue();
        int mode = args[2]->IsUndefined() ? 3 : args[2]->NumberValue();
        
        Bmp183Node* obj = new Bmp183Node(devfile, altitude, mode);
        
        obj->Wrap(args.This());
        
        args.GetReturnValue().Set(args.This());
        
        if (!driver) {
            driver = new Bmp183Drv(devfile, altitude, mode);
        }
        
    }
    
    // called by libuv worker in separate thread
    void Bmp183Node::WorkAsync(uv_work_t *req) {
        Work *work = static_cast<Work *>(req->data);
    
        work->value = driver->getValueAtIndex(work->valueIndex);
    }
    
    // called by libuv in event loop when async function completes
    void Bmp183Node::WorkAsyncComplete(uv_work_t *req, int status) {
        Isolate * isolate = Isolate::GetCurrent();
        
        v8::HandleScope handleScope(isolate);
        
        Work *work = static_cast<Work *>(req->data);
        
        // the work has been done, and now we store the value as a v8 string
        
        Local<String> retValue = String::NewFromUtf8(isolate, work->value.c_str());
        
        // set up return arguments: 0 = error, 1 = returned value
        Handle<Value> argv[] = { Null(isolate) , retValue };
        
        // execute the callback
        Local<Function>::New(isolate, work->callback)->Call(isolate->GetCurrentContext()->Global(), 2, argv);
        
        // Free up the persistent function callback
        work->callback.Reset();
        delete work;
        
    }

    void init(Local<Object> exports) {
        
        Bmp183Node::Init(exports);
        
    }
    
    NODE_MODULE(bmp183, init)
    
}  // namespace bmp183
