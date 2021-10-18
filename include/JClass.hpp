#ifndef __GUSC_JCLASS_HPP
#define __GUSC_JCLASS_HPP 1

#include <jni.h>
#include "JEnv.hpp"
#include "JObject.hpp"
#include "JString.hpp"
#include "private/signature.hpp"

namespace gusc::Jni
{

class JObject;

class JClass
{
public:
    JClass(JEnv& initEnv, jclass initClass) :
        env(initEnv),
        cls(initClass){}
    
    inline std::string getClassName()
    {
        jmethodID getNameId = getMethodId("getName", "()Ljava/lang/String;");
        return JString(env, static_cast<jstring>(env->CallObjectMethod(cls, getNameId)));
    }

    template<typename... TArgs>
    JObject createObjectSign(const char* signature, const TArgs&... args)
    {
        const auto methodId = getMethodId("<init>", signature);
        return JObject(env->NewObject(cls, methodId, std::forward<const TArgs&>(args)...));
    }

    template<typename... TArgs>
    JObject createObject(const TArgs&... args)
    {
        constexpr auto sign = Private::getArgumentSignature(std::forward<const TArgs&>(args)...);
        return createObjectSign(sign.str, std::forward<const TArgs&>(args)...);
    }
    
    inline jmethodID getStaticMethodId(const char* name, const char* signature)
    {
        auto methodId = env->GetStaticMethodID(cls, name, signature);
        if (!methodId)
        {
            throw std::runtime_error(std::string("Can't find static method ") + name + " with signature " + signature);
        }
        return methodId;
    }
    
    inline jmethodID getMethodId(const char* name, const char* signature)
    {
        auto methodId = env->GetMethodID(cls, name, signature);
        if (!methodId)
        {
            throw std::runtime_error(std::string("Can't find instance method ") + name + " with signature " + signature);
        }
        return methodId;
    }

    inline jfieldID getStaticFieldId(const char* name, const char* signature)
    {
        auto fieldId = env->GetStaticFieldID(cls, name, signature);
        if (!fieldId)
        {
            throw std::runtime_error(std::string("Can't find static field ") + name + " with signature " + signature);
        }
        return fieldId;
    }
    
    inline jfieldID getFieldId(const char* name, const char* signature)
    {
        auto fieldId = env->GetFieldID(cls, name, signature);
        if (!fieldId)
        {
            throw std::runtime_error(std::string("Can't find instance field ") + name + " with signature " + signature);
        }
        return fieldId;
    }
    
private:
    JEnv& env;
    jclass cls { nullptr };
};

inline JClass JEnv::getClass(const char* classPath)
{
    auto cls = env->FindClass(classPath);
    if (!cls)
    {
        throw std::runtime_error(std::string("Can't find ") + classPath + " Java class");
    }
    return JClass(*this, cls);
}

inline JClass JEnv::getObjectClass(jobject jniObject)
{
    auto cls = env->GetObjectClass(jniObject);
    if (!cls)
    {
        throw std::runtime_error("Class not found");
    }
    return JClass(*this, cls);
}

inline jmethodID JObject::getMethodId(const char* name, const char* signature)
{
    auto env = JVM::getEnv();
    auto cls = env.getObjectClass(obj);
    return cls.getMethodId(name, signature);
}

inline jfieldID JObject::getFieldId(const char* name, const char* signature)
{
    auto env = JVM::getEnv();
    auto cls = env.getObjectClass(obj);
    return cls.getFieldId(name, signature);
}

}

#endif // __GUSC_JCLASS_HPP