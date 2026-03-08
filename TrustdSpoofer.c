// TrustdSpoofer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <syslog.h>
#include <mach/mach.h>
#include <Security/Security.h>

#define LOG(fmt, ...) syslog(LOG_NOTICE, "🔓 [TrustdSpoofer] " fmt, ##__VA_ARGS__)

// 原始函数指针
static OSStatus (*orig_SecTrustEvaluate)(SecTrustRef, SecTrustResultType *) = NULL;
static Boolean (*orig_SecTrustEvaluateWithError)(SecTrustRef, CFErrorRef *) = NULL;

// Hook SecTrustEvaluate
OSStatus hooked_SecTrustEvaluate(SecTrustRef trust, SecTrustResultType *result) {
    LOG("SecTrustEvaluate 被调用 - 返回官方证书");
    if (result) *result = kSecTrustResultUnspecified;
    return errSecSuccess;
}

// Hook SecTrustEvaluateWithError
Boolean hooked_SecTrustEvaluateWithError(SecTrustRef trust, CFErrorRef *error) {
    LOG("SecTrustEvaluateWithError 被调用 - 返回官方证书");
    if (error) *error = NULL;
    return true;
}

// Hook MISValidateSignature
int hooked_MISValidateSignature(const char *path, void *info) {
    LOG("MISValidateSignature 被调用: %s", path);
    return 0;
}

// 内存补丁函数
void patch_function(void *target, void *replacement) {
    if (!target || !replacement) return;
    
    vm_address_t addr = (vm_address_t)target;
    vm_protect(mach_task_self(), addr, 16, FALSE, 
               VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
    
    uint32_t *instructions = (uint32_t *)target;
    uint64_t repl_addr = (uint64_t)replacement;
    
    instructions[0] = 0x58000051;  // LDR X17, #8
    instructions[1] = 0xD61F0220;  // BR X17
    instructions[2] = (uint32_t)(repl_addr & 0xFFFFFFFF);
    instructions[3] = (uint32_t)(repl_addr >> 32);
}

// 构造函数
__attribute__((constructor))
static void init() {
    LOG("==================================");
    LOG("TrustdSpoofer 已加载到进程: %s", getprogname());
    LOG("PID: %d", getpid());
    LOG("==================================");
    
    if (strcmp(getprogname(), "trustd") == 0) {
        void *secTrustEval = dlsym(RTLD_DEFAULT, "SecTrustEvaluate");
        void *secTrustEvalErr = dlsym(RTLD_DEFAULT, "SecTrustEvaluateWithError");
        void *misValidate = dlsym(RTLD_DEFAULT, "MISValidateSignature");
        
        if (secTrustEval) {
            patch_function(secTrustEval, hooked_SecTrustEvaluate);
            LOG("✅ SecTrustEvaluate 已钩住");
        }
        
        if (secTrustEvalErr) {
            patch_function(secTrustEvalErr, hooked_SecTrustEvaluateWithError);
            LOG("✅ SecTrustEvaluateWithError 已钩住");
        }
        
        if (misValidate) {
            patch_function(misValidate, hooked_MISValidateSignature);
            LOG("✅ MISValidateSignature 已钩住");
        }
        
        LOG("✅ 所有钩子安装完成");
    }
}
