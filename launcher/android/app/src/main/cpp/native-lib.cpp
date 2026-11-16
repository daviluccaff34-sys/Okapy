#include <jni.h>

#include <memory>
#include <mutex>
#include <string>

#include <okapy/core.hpp>

namespace {

std::mutex g_coreMutex;
std::unique_ptr<okapy::Core> g_core;

jstring toJavaString(JNIEnv* env, const std::string& value) {
    return env->NewStringUTF(value.c_str());
}

} // namespace

extern "C" JNIEXPORT jstring JNICALL
Java_com_okapy_launcher_OkapyNative_nativeVersion(JNIEnv* env, jobject) {
    return toJavaString(env, std::string(okapy::kVersion));
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_okapy_launcher_OkapyNative_nativeInitialize(JNIEnv* env, jobject) {
    std::lock_guard<std::mutex> lock(g_coreMutex);

    if (g_core && g_core->isInitialized()) {
        return JNI_TRUE;
    }

    try {
        auto candidate = std::make_unique<okapy::Core>();
        if (!candidate->initialize()) {
            return JNI_FALSE;
        }

        g_core = std::move(candidate);
        return JNI_TRUE;
    } catch (...) {
        return JNI_FALSE;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_okapy_launcher_OkapyNative_nativeShutdown(JNIEnv*, jobject) {
    std::lock_guard<std::mutex> lock(g_coreMutex);
    g_core.reset();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_okapy_launcher_OkapyNative_nativeStatus(JNIEnv* env, jobject) {
    std::lock_guard<std::mutex> lock(g_coreMutex);

    if (!g_core || !g_core->isInitialized()) {
        return toJavaString(env, "Core is not initialized");
    }

    return toJavaString(
        env,
        "Core initialized; registered mods: " + std::to_string(g_core->mods().size())
    );
}
